/*

	Housing system for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 5 (stage 3) — design §9.1 (Housing — THE spec), §5.5 (house robbery),
	  §11.3 (the `houses` table), §7 (house lock — distinct from the vehicle lock).
	Grounding: docs/research/crazybobs-site-full.md (§Housing — own 2 / co-own 8 /
	  rent 10, storage caps, spawn-at-house, 2-week inactivity decay, Super Lock,
	  Pets, alarm) + docs/research/economy.md §3 (housing valuation factors). Concrete
	  prices are OUR DESIGN (CB never published a table): $250k-$5M sane range.

	This owns the player-owned houses the CB economy revolves around. Houses live in
	the `houses` DB table (§11.3), loaded async on gamemode init into gHouseInfo[]
	with a streamer pickup + 3D label + map icon at each entrance. Entry teleports
	the player into a per-house virtual world (VW = HOUSE_VW_BASE + index) at the
	house interior; exit returns them to the entrance.

	Mechanics (§9.1):
	  - BUY  (/buyhouse) at an unowned for-sale entrance → Bank charge (bank-first
	    then cash), OwnerAID set, own-cap 2. Police get a 10% discount (§3.1). Each
	    purchase bumps the house value (§3.1 "each sale raises the value").
	  - SELL (/sellhouse) to the server for HOUSE_SELL_PCT% of the price, OR list it
	    ForSale-by-owner at a chosen price.
	  - RENT (/renthouse) — MINIMAL single-renter model: the owner sets a per-game-day
	    rent (/setrent); a tenant renting auto-pays that each game-day they pass in
	    server (House_OnGameDay). (co-owners + a full multi-renter contract are M6.)
	  - STORAGE — money (StorageMoney, /housestore //housewithdraw — dodges wealth
	    tax) + a simple stored-item count (StorageItems). This is what /houserob
	    steals. (The full per-item stash — drugs/condoms/flowers/… — is deferred to
	    M6; the money + item-count is the M5 minimal storage.)
	  - SPAWN (/sethousespawn) → OnPlayerSpawn spawns the owner inside their house.
	  - HOUSE LOCK (/lockhouse //unlockhouse) — owner-set entry lock, distinct from
	    the vehicle lock (§7). SUPER LOCK (/superlock) — a paid upgrade that lowers
	    robbery success (§5.5).
	  - DECAY — a house unvisited for 2 real weeks reverts to for-sale (LastVisited,
	    §3.2). Checked on the low-frequency game-day tick.

	Admin/scripter: /addhouse (scripter) creates a house at the caller's position;
	  /edithouse tweaks price/interior/pet of the nearest house.

	Public API (FUNCTION publics so robbery.inc + the callbacks reach them):
		House_Init()                       : load all houses (async) — OnGameModeInit
		House_OnPlayerLogin(playerid)      : resolve pHouseSpawn + tally owned houses
		House_OnPickup(playerid, pickupid) : entrance pickup → enter/exit hint
		House_OnKeyEnter(playerid)         : KEY_YES at an entrance → /enter (teleport in)
		House_OnPlayerSpawn(playerid)      : spawn the owner inside their /sethousespawn house
		House_OnGameDay()                  : rent charge + 2-week inactivity decay tick
		House_OnDisconnect(playerid)       : drop the inside-house session marker
		House_NearestOwned(playerid, Float:range) : nearest OWNED house entrance in range (-1 = none) — /houserob
		House_OwnerAID(house)              : the OwnerAID of a house index
		House_IsOwnerOnline(house, &ownerid) : is the owner connected? (fills ownerid) — /houserob
		House_RobSteal(playerid, house)    : run the break-in: move storage money to the robber (no mint) — /houserob
		House_OnDialogResponse(...)        : house menu / buy / storage dialogs

	Functions Being Called:
		misc:    IsCop, IsCivilian, isNull
		bank:    Bank_GetWealth, Bank_Give, Bank_Take, Bank_GiveCash, Bank_TakeCash, Bank_GetBalance
		jail:    Jail_SavePlayer, Jail_IsJailed
		wanted:  GivePlayerWanted
*/

//==============================================================================
//	House record + runtime table
//==============================================================================

enum e_House
{
	hID,				// houses.ID (DB primary key) — persisted, stable across reloads
	hOwnerAID,			// players.aID of the owner (0 = unowned / bank-owned, for sale)
	hPrice,				// current market price
	Float:hX,			// entrance world position
	Float:hY,
	Float:hZ,
	hInterior,			// SA-MP interior id of the inside
	hVW,				// per-house virtual world (HOUSE_VW_BASE + index)
	Float:hIntX,		// interior spawn position
	Float:hIntY,
	Float:hIntZ,
	bool:hForSale,		// purchasable (unowned, or owner-listed)
	hRent,				// per-game-day rent (0 = not for rent)
	hRenterAID,			// current tenant aID (0 = none)
	bool:hSuperLock,	// super-lock upgrade fitted (§5.5)
	bool:hLocked,		// owner-set entry lock (§7)
	bool:hHasPet,		// house pet defends against break-ins (§5.5)
	hStorageMoney,		// money stored inside (lootable, dodges wealth tax)
	hStorageItems,		// simple stored-item count [M5 minimal]
	hLastVisited,		// gettime() the owner last visited (2-week decay)
	// runtime streamer handles (not persisted)
	STREAMER_TAG_PICKUP:hPickup,
	STREAMER_TAG_MAP_ICON:hIcon,
	STREAMER_TAG_3D_TEXT_LABEL:hLabel
};

new gHouseInfo[MAX_HOUSES][e_House];
new gHouseCount = 0;	// number of houses loaded from the DB (<= MAX_HOUSES)

// Per-player buy-dialog target (the house index /buyhouse offered). -1 = none.
new g_HouseBuyTarget[MAX_PLAYERS] = { -1, ... };

//==============================================================================
//	Small helpers
//==============================================================================

// A valid, loaded house index?
static bool:House_Valid(house)
	return (house >= 0 && house < gHouseCount);

// House index for a houses.ID (the persisted pHouseSpawn / DB key), or -1.
static House_FindByID(id)
{
	if(id <= 0) return -1;
	for(new i = 0; i < gHouseCount; i++)
		if(gHouseInfo[i][hID] == id) return i;
	return -1;
}

// Is this player the owner of `house`? (aID match — never name-based.)
static bool:House_IsOwner(playerid, house)
{
	if(!House_Valid(house) || !PlayerInfo[playerid][pLoggedIn]) return false;
	return (gHouseInfo[house][hOwnerAID] != 0 && gHouseInfo[house][hOwnerAID] == PlayerInfo[playerid][pID]);
}

// The OwnerAID of a house (0 = unowned). Public so robbery.inc can read it.
FUNCTION House_OwnerAID(house)
	return House_Valid(house) ? gHouseInfo[house][hOwnerAID] : 0;

// Persist one house's mutable state to the DB (async R41). Called after every
// buy/sell/rent/storage/lock change so a restart keeps house state.
static House_Save(house)
{
	if(!House_Valid(house)) return 0;
	new query[512];
	mysql_format(g_SQL, query, sizeof(query),
		"UPDATE `houses` SET OwnerAID=%d, Price=%d, ForSale=%d, Rent=%d, RenterAID=%d, SuperLock=%d, Locked=%d, HasPet=%d, StorageMoney=%d, StorageItems=%d, LastVisited=IF(%d=0,NULL,FROM_UNIXTIME(%d)) WHERE ID=%d LIMIT 1",
		gHouseInfo[house][hOwnerAID], gHouseInfo[house][hPrice], gHouseInfo[house][hForSale] ? 1 : 0,
		gHouseInfo[house][hRent], gHouseInfo[house][hRenterAID], gHouseInfo[house][hSuperLock] ? 1 : 0,
		gHouseInfo[house][hLocked] ? 1 : 0, gHouseInfo[house][hHasPet] ? 1 : 0,
		gHouseInfo[house][hStorageMoney], gHouseInfo[house][hStorageItems],
		gHouseInfo[house][hLastVisited], gHouseInfo[house][hLastVisited], gHouseInfo[house][hID]);
	mysql_pquery(g_SQL, query);
	return 1;
}

// (Re)build the entrance streamer objects (pickup + label + map icon) for a house
// so a buy/sell that changes owner/for-sale immediately reflects on the map/label.
static House_Refresh(house)
{
	if(!House_Valid(house)) return 0;

	if(DestroyDynamicPickup(gHouseInfo[house][hPickup])) {}
	if(DestroyDynamicMapIcon(gHouseInfo[house][hIcon])) {}
	if(DestroyDynamic3DTextLabel(gHouseInfo[house][hLabel])) {}

	// Entrance pickup in the main world (VW 0, interior 0).
	gHouseInfo[house][hPickup] = CreateDynamicPickup(HOUSE_PICKUP_MODEL, 1,
		gHouseInfo[house][hX], gHouseInfo[house][hY], gHouseInfo[house][hZ], 0, 0, -1, 100.0);

	// Map icon: green for a for-sale house, blue for an owned one.
	new iconModel = (gHouseInfo[house][hOwnerAID] == 0) ? HOUSE_ICON_FORSALE : HOUSE_ICON_OWNED;
	gHouseInfo[house][hIcon] = CreateDynamicMapIcon(gHouseInfo[house][hX], gHouseInfo[house][hY], gHouseInfo[house][hZ],
		iconModel, 0, 0, 0, -1, HOUSE_ICON_RANGE, MAPICON_LOCAL);

	// Entrance label.
	new label[144];
	if(gHouseInfo[house][hOwnerAID] == 0)
		format(label, sizeof(label), ""COL_GREEN"[ House #%d — For Sale ]\n"COL_WHITE"Price: "COL_GREEN"$%d\n"COL_YELLOW"/buyhouse"COL_WHITE" to purchase", gHouseInfo[house][hID], gHouseInfo[house][hPrice]);
	else if(gHouseInfo[house][hForSale])
		format(label, sizeof(label), ""COL_ORANGE"[ House #%d — Owner Selling ]\n"COL_WHITE"Price: "COL_GREEN"$%d\n"COL_YELLOW"/buyhouse"COL_WHITE" to purchase", gHouseInfo[house][hID], gHouseInfo[house][hPrice]);
	else
		format(label, sizeof(label), ""COL_WHITE"[ House #%d ]\n%sPress "COL_YELLOW"~k~~CONVERSATION_YES~"COL_WHITE" or "COL_YELLOW"/enter"COL_WHITE" at the door", gHouseInfo[house][hID], gHouseInfo[house][hLocked] ? (""COL_RED"Locked. ") : (""));
	gHouseInfo[house][hLabel] = CreateDynamic3DTextLabel(label, 0xFFFFFFFF,
		gHouseInfo[house][hX], gHouseInfo[house][hY], gHouseInfo[house][hZ] + 0.4, HOUSE_LABEL_RANGE,
		INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, -1, -1, -1, HOUSE_LABEL_RANGE);
	return 1;
}

// The nearest house entrance the player is standing at (any owner), within range.
// world-0 exteriors only (entrances live outside). Returns index or -1.
static House_NearestEntrance(playerid, Float:range)
{
	if(GetPlayerInterior(playerid) != 0 || GetPlayerVirtualWorld(playerid) != 0) return -1;
	new best = -1;
	new Float:bestDist = range + 1.0;
	for(new i = 0; i < gHouseCount; i++)
	{
		new Float:d = GetPlayerDistanceFromPoint(playerid, gHouseInfo[i][hX], gHouseInfo[i][hY], gHouseInfo[i][hZ]);
		if(d <= range && d < bestDist) { best = i; bestDist = d; }
	}
	return best;
}

// The nearest OWNED house entrance in range (for /houserob). Public so robbery.inc
// reaches it. Skips unowned/for-sale houses (nothing to rob there).
FUNCTION House_NearestOwned(playerid, Float:range)
{
	if(GetPlayerInterior(playerid) != 0 || GetPlayerVirtualWorld(playerid) != 0) return -1;
	new best = -1;
	new Float:bestDist = range + 1.0;
	for(new i = 0; i < gHouseCount; i++)
	{
		if(gHouseInfo[i][hOwnerAID] == 0) continue;	// unowned — nothing to rob
		new Float:d = GetPlayerDistanceFromPoint(playerid, gHouseInfo[i][hX], gHouseInfo[i][hY], gHouseInfo[i][hZ]);
		if(d <= range && d < bestDist) { best = i; bestDist = d; }
	}
	return best;
}

// Is the owner of `house` online? Fills `ownerid` with their playerid. Public — the
// house robbery requires the owner to be online (§5.5).
FUNCTION House_IsOwnerOnline(house, &ownerid)
{
	ownerid = INVALID_PLAYER_ID;
	if(!House_Valid(house) || gHouseInfo[house][hOwnerAID] == 0) return 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || !PlayerInfo[i][pLoggedIn]) continue;
		if(PlayerInfo[i][pID] == gHouseInfo[house][hOwnerAID]) { ownerid = i; return 1; }
	}
	return 0;
}

// Recompute an online owner's owned-house count (own-cap gate). Called on login and
// after any ownership change so pOwnedHouseCount stays accurate.
static House_RecountOwned(playerid)
{
	if(!PlayerInfo[playerid][pLoggedIn]) return 0;
	new n = 0;
	for(new i = 0; i < gHouseCount; i++)
		if(gHouseInfo[i][hOwnerAID] == PlayerInfo[playerid][pID]) n++;
	PlayerInfo[playerid][pOwnedHouseCount] = n;
	return n;
}

//==============================================================================
//	Load / cleanup — OnGameModeInit / OnGameModeExit
//==============================================================================

FUNCTION House_Init()
{
	gHouseCount = 0;
	mysql_pquery(g_SQL, "SELECT * FROM `houses` ORDER BY ID ASC", "House_OnLoad", "");
	return 1;
}

// Destroy every house's streamer objects (pickup + map icon + 3D label) on exit.
// Mirrors Moneybag_Cleanup / Moneyrush_Cleanup — called from OnGameModeExit.
FUNCTION House_Cleanup()
{
	for(new i = 0; i < gHouseCount; i++)
	{
		if(DestroyDynamicPickup(gHouseInfo[i][hPickup])) {}
		if(DestroyDynamicMapIcon(gHouseInfo[i][hIcon])) {}
		if(DestroyDynamic3DTextLabel(gHouseInfo[i][hLabel])) {}
	}
	return 1;
}

// Async result handler for House_Init — read the rows into gHouseInfo[] and build
// the entrance streamer objects. A per-house VW is assigned by index so each house
// interior is isolated.
FUNCTION House_OnLoad()
{
	new rows;
	cache_get_row_count(rows);
	if(rows > MAX_HOUSES) rows = MAX_HOUSES;
	gHouseCount = rows;
	for(new i = 0; i < rows; i++)
	{
		cache_get_value_int(i, "ID", gHouseInfo[i][hID]);
		cache_get_value_int(i, "OwnerAID", gHouseInfo[i][hOwnerAID]);
		cache_get_value_int(i, "Price", gHouseInfo[i][hPrice]);
		cache_get_value_float(i, "X", gHouseInfo[i][hX]);
		cache_get_value_float(i, "Y", gHouseInfo[i][hY]);
		cache_get_value_float(i, "Z", gHouseInfo[i][hZ]);
		cache_get_value_int(i, "Interior", gHouseInfo[i][hInterior]);
		cache_get_value_float(i, "IntX", gHouseInfo[i][hIntX]);
		cache_get_value_float(i, "IntY", gHouseInfo[i][hIntY]);
		cache_get_value_float(i, "IntZ", gHouseInfo[i][hIntZ]);
		cache_get_value_bool(i, "ForSale", gHouseInfo[i][hForSale]);
		cache_get_value_int(i, "Rent", gHouseInfo[i][hRent]);
		cache_get_value_int(i, "RenterAID", gHouseInfo[i][hRenterAID]);
		cache_get_value_bool(i, "SuperLock", gHouseInfo[i][hSuperLock]);
		cache_get_value_bool(i, "Locked", gHouseInfo[i][hLocked]);
		cache_get_value_bool(i, "HasPet", gHouseInfo[i][hHasPet]);
		cache_get_value_int(i, "StorageMoney", gHouseInfo[i][hStorageMoney]);
		cache_get_value_int(i, "StorageItems", gHouseInfo[i][hStorageItems]);
		// LastVisited is a DATETIME; the load query has no UNIX_TIMESTAMP alias, so
		// treat an unvisited/NULL as 0 (a fresh purchase stamps it). Decay uses the
		// stamp we write on visit/purchase; a never-visited owned house is stamped
		// on load so it does not decay before its first visit is recorded.
		gHouseInfo[i][hLastVisited] = 0;
		// Assign a unique per-house virtual world.
		gHouseInfo[i][hVW] = HOUSE_VW_BASE + i;
		// A for-sale unowned house cannot be locked/rented against.
		if(gHouseInfo[i][hOwnerAID] == 0)
		{
			gHouseInfo[i][hForSale] = true;
			gHouseInfo[i][hLocked] = false;
		}
		House_Refresh(i);
	}
	printf("[HOUSING] Loaded %d house(s).", gHouseCount);
	// Tally owned-house counts for anyone already logged in (hot script reload).
	for(new pid = 0; pid < MAX_PLAYERS; pid++)
		if(IsPlayerConnected(pid) && !IsPlayerNPC(pid) && PlayerInfo[pid][pLoggedIn]) House_RecountOwned(pid);
	return 1;
}

// Called from loading_data.inc (via CallLocalFunction) once LSplayers is loaded:
// resolve the persisted pHouseSpawn (a houses.ID) to validity + tally owned houses.
FUNCTION House_OnPlayerLogin(playerid)
{
	if(!PlayerInfo[playerid][pLoggedIn]) return 0;
	House_RecountOwned(playerid);
	// pHouseSpawn holds a houses.ID; keep it only if it still resolves to a house
	// this player owns, else fall back to the default city spawn (-1).
	new house = House_FindByID(PlayerInfo[playerid][pHouseSpawn]);
	if(house == -1 || !House_IsOwner(playerid, house)) PlayerInfo[playerid][pHouseSpawn] = -1;
	return 1;
}

//==============================================================================
//	Enter / exit
//==============================================================================

// Teleport a player into a house interior (per-house VW). Marks pInsideHouse and,
// for the owner, refreshes the LastVisited stamp (drives the 2-week decay).
static House_Enter(playerid, house)
{
	if(!House_Valid(house)) return 0;
	SetPlayerInterior(playerid, gHouseInfo[house][hInterior]);
	SetPlayerVirtualWorld(playerid, gHouseInfo[house][hVW]);
	SetPlayerPos(playerid, gHouseInfo[house][hIntX], gHouseInfo[house][hIntY], gHouseInfo[house][hIntZ]);
	SetCameraBehindPlayer(playerid);
	PlayerInfo[playerid][pInsideHouse] = house;
	PlayerInfo[playerid][pInterior] = gHouseInfo[house][hInterior];

	if(House_IsOwner(playerid, house))
	{
		gHouseInfo[house][hLastVisited] = gettime();
		House_Save(house);
	}
	new msg[144];
	format(msg, sizeof(msg), ""COL_WHITE"You entered house "COL_YELLOW"#%d"COL_WHITE". %s", gHouseInfo[house][hID],
		House_IsOwner(playerid, house) ? (""COL_GREEN"/house"COL_WHITE" for the owner menu, "COL_GREEN"/exit"COL_WHITE" to leave.") : (""COL_GREEN"/exit"COL_WHITE" to leave."));
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}

// Return a player to the entrance of the house they are inside.
static House_Leave(playerid)
{
	new house = PlayerInfo[playerid][pInsideHouse];
	if(!House_Valid(house)) { PlayerInfo[playerid][pInsideHouse] = -1; return 0; }
	SetPlayerInterior(playerid, 0);
	SetPlayerVirtualWorld(playerid, 0);
	SetPlayerPos(playerid, gHouseInfo[house][hX], gHouseInfo[house][hY], gHouseInfo[house][hZ] + 0.5);
	SetCameraBehindPlayer(playerid);
	PlayerInfo[playerid][pInsideHouse] = -1;
	PlayerInfo[playerid][pInterior] = 0;
	SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_WHITE"You left the house.");
	return 1;
}

// Attempt to enter the house whose entrance the player is standing at. Honours the
// owner-set house lock: only the owner may enter a locked house (§7). Returns 1 if
// a house entrance was handled (entered or refused), 0 if none was in range.
static House_TryEnter(playerid)
{
	new house = House_NearestEntrance(playerid, HOUSE_ENTER_RANGE);
	if(house == -1) return 0;
	if(gHouseInfo[house][hLocked] && !House_IsOwner(playerid, house))
	{
		SendClientMessage(playerid, COLOR_ERROR, ""COL_RED"* This house is locked."COL_WHITE" Only the owner can enter.");
		return 1;
	}
	House_Enter(playerid, house);
	return 1;
}

// Entrance-pickup hook (from OnPlayerPickUpDynamicPickup). Just a hint — the actual
// enter is KEY_YES //enter (re-checked there), matching the vehicle-lock style.
FUNCTION House_OnPickup(playerid, pickupid)
{
	for(new i = 0; i < gHouseCount; i++)
	{
		if(gHouseInfo[i][hPickup] != STREAMER_TAG_PICKUP:pickupid) continue;
		if(gHouseInfo[i][hOwnerAID] == 0 || gHouseInfo[i][hForSale])
		{
			new msg[144];
			format(msg, sizeof(msg), ""COL_GREEN"* House #%d for sale "COL_WHITE"($%d). Type "COL_YELLOW"/buyhouse"COL_WHITE" to purchase.", gHouseInfo[i][hID], gHouseInfo[i][hPrice]);
			SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
		}
		else SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_WHITE"* This is a house. Press "COL_YELLOW"~k~~CONVERSATION_YES~"COL_WHITE" or type "COL_YELLOW"/enter"COL_WHITE" at the door.");
		return 1;
	}
	return 0;
}

// KEY_YES at an entrance → enter (from OnPlayerKeyStateChange). Returns 1 if a house
// enter/exit was handled so the caller can skip other KEY_YES consumers.
FUNCTION House_OnKeyEnter(playerid)
{
	if(IsPlayerInAnyVehicle(playerid)) return 0;
	// Inside a house? KEY_YES leaves.
	if(House_Valid(PlayerInfo[playerid][pInsideHouse])) return House_Leave(playerid);
	return House_TryEnter(playerid);
}

//==============================================================================
//	Buy / sell
//==============================================================================

// The price this player pays for `house` (police get a discount, §3.1).
static House_PriceFor(playerid, house)
{
	new price = gHouseInfo[house][hPrice];
	if(IsCop(playerid)) price -= (price / 100) * HOUSE_POLICE_DISCOUNT;
	return price;
}

// Charge a price bank-first then cash (mirrors bank.inc's insurance charge). Returns
// 1 if fully paid (money deducted), 0 if unaffordable (nothing deducted).
static House_ChargeBankFirst(playerid, price)
{
	if(price <= 0) return 1;
	if(Bank_GetWealth(playerid) < price) return 0;
	new remaining = price;
	if(PlayerInfo[playerid][pBankMoney] >= remaining)
	{
		Bank_Take(playerid, remaining);
		return 1;
	}
	remaining -= PlayerInfo[playerid][pBankMoney];
	Bank_Take(playerid, PlayerInfo[playerid][pBankMoney]);
	Bank_TakeCash(playerid, remaining);
	return 1;
}

// Complete a purchase (shared by /buyhouse + the buy dialog). All gates re-checked.
static House_DoBuy(playerid, house)
{
	if(!House_Valid(house)) return SendClientMessage(playerid, COLOR_ERROR, "That house no longer exists.");
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(Jail_IsJailed(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot buy a house while jailed.");
	if(!gHouseInfo[house][hForSale] && gHouseInfo[house][hOwnerAID] != 0)
		return SendClientMessage(playerid, COLOR_ERROR, "That house is not for sale.");
	if(House_IsOwner(playerid, house)) return SendClientMessage(playerid, COLOR_ERROR, "You already own this house.");
	if(PlayerInfo[playerid][pOwnedHouseCount] >= MAX_OWNED_HOUSES)
	{
		new err[128];
		format(err, sizeof(err), "You already own the maximum of %d houses. Sell one first (/sellhouse).", MAX_OWNED_HOUSES);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}

	new price = House_PriceFor(playerid, house);
	if(Bank_GetWealth(playerid) < price)
	{
		new err[128];
		format(err, sizeof(err), "You cannot afford this house — it costs "COL_GREEN"$%d"COL_WHITE" (bank + cash).", price);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	if(!House_ChargeBankFirst(playerid, price)) return SendClientMessage(playerid, COLOR_ERROR, "Payment failed.");

	// The seller: an owner-listed sale pays the previous owner the sale price PLUS
	// their stored house money (it is theirs, not the buyer's); a bank (unowned) sale
	// mints nothing beyond the buyer's spend (the bank keeps it).
	new prevOwner = gHouseInfo[house][hOwnerAID];
	if(prevOwner != 0)
	{
		new sellerProceeds = price + gHouseInfo[house][hStorageMoney];	// sale price + returned safe money (MOVES, no mint)
		for(new i = 0; i < MAX_PLAYERS; i++)
		{
			if(!IsPlayerConnected(i) || IsPlayerNPC(i) || !PlayerInfo[i][pLoggedIn]) continue;
			if(PlayerInfo[i][pID] == prevOwner)
			{
				Bank_Give(i, sellerProceeds);	// the buyer's money + the seller's safe money → seller's bank
				SendClientMessage(i, COLOR_SERVER_INFO, ""COL_GREEN"* One of your houses sold "COL_WHITE"— the sale price and your stored money were banked.");
				Jail_SavePlayer(i);
				House_RecountOwned(i);
				break;
			}
		}
		// (Offline previous owner: the proceeds are not credited in M5 — an offline
		// payout needs an aID-keyed bank UPDATE; deferred like offline bank interest.)
	}

	gHouseInfo[house][hOwnerAID] = PlayerInfo[playerid][pID];
	gHouseInfo[house][hForSale] = false;
	gHouseInfo[house][hLocked] = false;
	gHouseInfo[house][hSuperLock] = false;	// the new owner buys a fresh house — no inherited super lock
	gHouseInfo[house][hHasPet] = false;
	gHouseInfo[house][hRenterAID] = 0;
	gHouseInfo[house][hStorageMoney] = 0;	// the previous owner's safe money was returned to them (or bank-kept if unowned)
	gHouseInfo[house][hStorageItems] = 0;
	gHouseInfo[house][hLastVisited] = gettime();
	// Each sale raises the house value (§3.1 "each sale raises the value").
	new bump = (gHouseInfo[house][hPrice] / 100) * HOUSE_VALUE_BUMP_PCT;
	if(gHouseInfo[house][hPrice] < HOUSE_PRICE_MAX - bump) gHouseInfo[house][hPrice] += bump;
	House_Save(house);
	House_Refresh(house);
	House_RecountOwned(playerid);
	Jail_SavePlayer(playerid);

	new msg[176];
	format(msg, sizeof(msg), ""COL_GREEN"* House #%d purchased "COL_WHITE"for "COL_GREEN"$%d"COL_WHITE"! You now own "COL_GREEN"%d/%d"COL_WHITE" houses. Enter with "COL_YELLOW"/enter"COL_WHITE".", gHouseInfo[house][hID], price, PlayerInfo[playerid][pOwnedHouseCount], MAX_OWNED_HOUSES);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}

CMD:buyhouse(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(IsPlayerInAnyVehicle(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You must be on foot at the house door to buy it.");
	new house = House_NearestEntrance(playerid, HOUSE_ENTER_RANGE);
	if(house == -1) return SendClientMessage(playerid, COLOR_ERROR, "You are not standing at a house entrance. Look for a green house pickup.");
	if(!gHouseInfo[house][hForSale] && gHouseInfo[house][hOwnerAID] != 0)
		return SendClientMessage(playerid, COLOR_ERROR, "That house is not for sale.");

	new price = House_PriceFor(playerid, house);
	new caption[256];
	format(caption, sizeof(caption), ""COL_WHITE"House "COL_YELLOW"#%d"COL_WHITE"\nPrice: "COL_GREEN"$%d"COL_WHITE"%s\nInterior VW: %d\n\nBuy this house? (bank-first, then cash)", gHouseInfo[house][hID], price, IsCop(playerid) ? (" (police -10%)") : (""), gHouseInfo[house][hVW]);
	// Stash the target house index in pInsideHouse's sibling — reuse a scratch via
	// a static keyed by playerid so the dialog resolves the right house.
	g_HouseBuyTarget[playerid] = house;
	ShowPlayerDialog(playerid, HOUSE_BUY_DIALOG, DIALOG_STYLE_MSGBOX, "Buy House", caption, "Buy", "Cancel");
	return 1;
}

CMD:sellhouse(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(IsPlayerInAnyVehicle(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You must be on foot at your house to sell it.");

	// Sell the house at the caller's entrance (or the one they are inside).
	new house = House_Valid(PlayerInfo[playerid][pInsideHouse]) ? PlayerInfo[playerid][pInsideHouse] : House_NearestEntrance(playerid, HOUSE_ENTER_RANGE);
	if(house == -1 || !House_IsOwner(playerid, house))
		return SendClientMessage(playerid, COLOR_ERROR, "You must be at (or inside) a house you own to sell it.");

	// "/sellhouse list <price>" lists it for-sale-by-owner; "/sellhouse" sells to the
	// bank for HOUSE_SELL_PCT% of its value.
	new sub[8], listPrice;
	if(!sscanf(params, "s[8]D(0)", sub, listPrice) && !strcmp(sub, "list", true))
	{
		if(listPrice < HOUSE_PRICE_MIN || listPrice > HOUSE_PRICE_MAX)
		{
			new err[128];
			format(err, sizeof(err), "List price must be between $%d and $%d.", HOUSE_PRICE_MIN, HOUSE_PRICE_MAX);
			return SendClientMessage(playerid, COLOR_ERROR, err);
		}
		gHouseInfo[house][hPrice] = listPrice;
		gHouseInfo[house][hForSale] = true;
		House_Save(house);
		House_Refresh(house);
		new msg[144];
		format(msg, sizeof(msg), ""COL_GREEN"* House #%d listed for sale "COL_WHITE"at "COL_GREEN"$%d"COL_WHITE". You still own it until someone buys it. "COL_YELLOW"/sellhouse"COL_WHITE" to sell to the bank now.", gHouseInfo[house][hID], listPrice);
		return SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	}

	// Sell to the bank now for a percentage of the value.
	new payout = (gHouseInfo[house][hPrice] / 100) * HOUSE_SELL_PCT;
	// Storage money in the house is returned to the owner on sale (it is theirs).
	new refund = gHouseInfo[house][hStorageMoney];
	Bank_Give(playerid, payout + refund);

	gHouseInfo[house][hOwnerAID] = 0;
	gHouseInfo[house][hForSale] = true;
	gHouseInfo[house][hLocked] = false;
	gHouseInfo[house][hSuperLock] = false;
	gHouseInfo[house][hHasPet] = false;
	gHouseInfo[house][hRent] = 0;
	gHouseInfo[house][hRenterAID] = 0;
	gHouseInfo[house][hStorageMoney] = 0;
	gHouseInfo[house][hStorageItems] = 0;
	gHouseInfo[house][hLastVisited] = 0;
	House_Save(house);
	House_Refresh(house);

	// If they were spawning here or standing inside, reset that.
	if(PlayerInfo[playerid][pHouseSpawn] == gHouseInfo[house][hID]) PlayerInfo[playerid][pHouseSpawn] = -1;
	if(PlayerInfo[playerid][pInsideHouse] == house) House_Leave(playerid);
	House_RecountOwned(playerid);
	Jail_SavePlayer(playerid);

	new msg[176];
	format(msg, sizeof(msg), ""COL_GREEN"* House #%d sold to the bank "COL_WHITE"for "COL_GREEN"$%d"COL_WHITE" (+$%d storage returned). Banked. You now own "COL_GREEN"%d/%d"COL_WHITE".", gHouseInfo[house][hID], payout, refund, PlayerInfo[playerid][pOwnedHouseCount], MAX_OWNED_HOUSES);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}

//==============================================================================
//	Storage — money (/housestore //housewithdraw) + item count
//==============================================================================

// Deposit cash into the house storage money (dodges wealth tax, §3.3). Must be
// inside a house you own.
static House_DoStore(playerid, amount)
{
	new house = PlayerInfo[playerid][pInsideHouse];
	if(!House_Valid(house) || !House_IsOwner(playerid, house))
		return SendClientMessage(playerid, COLOR_ERROR, "You must be inside a house you own to store money.");
	if(amount <= 0) return SendClientMessage(playerid, COLOR_ERROR, "Enter a positive amount to store.");
	if(PlayerInfo[playerid][pMoney] < amount) return SendClientMessage(playerid, COLOR_ERROR, "You do not have that much cash on you.");
	if(gHouseInfo[house][hStorageMoney] > MAX_MONEY - amount) return SendClientMessage(playerid, COLOR_ERROR, "The house safe cannot hold that much.");

	Bank_TakeCash(playerid, amount);
	gHouseInfo[house][hStorageMoney] += amount;
	House_Save(house);
	Jail_SavePlayer(playerid);

	new msg[144];
	format(msg, sizeof(msg), ""COL_GREEN"* Stored "COL_WHITE"$%d"COL_GREEN" in the house safe."COL_WHITE" Safe: "COL_GREEN"$%d"COL_WHITE". (Safe from wealth tax — but robbable.)", amount, gHouseInfo[house][hStorageMoney]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}

// Withdraw cash from the house storage money back to hand.
static House_DoWithdraw(playerid, amount)
{
	new house = PlayerInfo[playerid][pInsideHouse];
	if(!House_Valid(house) || !House_IsOwner(playerid, house))
		return SendClientMessage(playerid, COLOR_ERROR, "You must be inside a house you own to take money out.");
	if(amount <= 0) return SendClientMessage(playerid, COLOR_ERROR, "Enter a positive amount to withdraw.");
	if(gHouseInfo[house][hStorageMoney] < amount) return SendClientMessage(playerid, COLOR_ERROR, "The house safe does not hold that much.");
	if(PlayerInfo[playerid][pMoney] > MAX_MONEY - amount) return SendClientMessage(playerid, COLOR_ERROR, "You are carrying too much cash to withdraw that.");

	gHouseInfo[house][hStorageMoney] -= amount;
	Bank_GiveCash(playerid, amount);
	House_Save(house);
	Jail_SavePlayer(playerid);

	new msg[144];
	format(msg, sizeof(msg), ""COL_GREEN"* Took "COL_WHITE"$%d"COL_GREEN" from the house safe."COL_WHITE" Safe: "COL_GREEN"$%d"COL_WHITE".", amount, gHouseInfo[house][hStorageMoney]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}

CMD:housestore(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new amount;
	if(sscanf(params, "d", amount)) return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /housestore [amount]");
	return House_DoStore(playerid, amount);
}
alias:housestore("hstore", "storemoney")

CMD:housewithdraw(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new amount;
	if(sscanf(params, "d", amount)) return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /housewithdraw [amount]");
	return House_DoWithdraw(playerid, amount);
}
alias:housewithdraw("hwithdraw", "hwd")

// Simple stored-item deposit (the M5 minimal item stash — a single count; the full
// per-type stash lands M6). Buys a "storage crate" for a token fee to seed loot the
// house robbery can steal, capped at HOUSE_STORE_ITEMS_MAX.
CMD:housestoreitem(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new house = PlayerInfo[playerid][pInsideHouse];
	if(!House_Valid(house) || !House_IsOwner(playerid, house))
		return SendClientMessage(playerid, COLOR_ERROR, "You must be inside a house you own to stash items.");
	if(gHouseInfo[house][hStorageItems] >= HOUSE_STORE_ITEMS_MAX)
	{
		new err[112];
		format(err, sizeof(err), "The house storage is full (%d/%d items).", gHouseInfo[house][hStorageItems], HOUSE_STORE_ITEMS_MAX);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	gHouseInfo[house][hStorageItems]++;
	House_Save(house);
	new msg[128];
	format(msg, sizeof(msg), ""COL_GREEN"* Stashed an item "COL_WHITE"in the house. Storage: "COL_GREEN"%d/%d"COL_WHITE" items. (Robbable — full item types land in a future update.)", gHouseInfo[house][hStorageItems], HOUSE_STORE_ITEMS_MAX);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}

//==============================================================================
//	Spawn at house / house lock / super lock / rent
//==============================================================================

CMD:sethousespawn(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new house = House_Valid(PlayerInfo[playerid][pInsideHouse]) ? PlayerInfo[playerid][pInsideHouse] : House_NearestEntrance(playerid, HOUSE_ENTER_RANGE);
	if(house == -1 || !House_IsOwner(playerid, house))
		return SendClientMessage(playerid, COLOR_ERROR, "You must be at (or inside) a house you own to set it as your spawn.");
	PlayerInfo[playerid][pHouseSpawn] = gHouseInfo[house][hID];
	Jail_SavePlayer(playerid);
	new msg[128];
	format(msg, sizeof(msg), ""COL_GREEN"* Spawn point set to house #%d."COL_WHITE" You will spawn here. Use "COL_YELLOW"/clearhousespawn"COL_WHITE" to reset.", gHouseInfo[house][hID]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}

CMD:clearhousespawn(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	PlayerInfo[playerid][pHouseSpawn] = -1;
	Jail_SavePlayer(playerid);
	SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_GREEN"* House spawn cleared."COL_WHITE" You will spawn at the default city spawns.");
	return 1;
}

// House lock (§7) — owner-set entry lock, distinct from the vehicle lock. Shared
// body for /lockhouse //unlockhouse.
static House_SetLock(playerid, bool:locked)
{
	new house = House_Valid(PlayerInfo[playerid][pInsideHouse]) ? PlayerInfo[playerid][pInsideHouse] : House_NearestEntrance(playerid, HOUSE_ENTER_RANGE);
	if(house == -1 || !House_IsOwner(playerid, house))
		return SendClientMessage(playerid, COLOR_ERROR, "You must be at (or inside) a house you own to lock it.");
	if(gHouseInfo[house][hLocked] == locked)
		return SendClientMessage(playerid, COLOR_SERVER_INFO, locked ? ("Your house is already locked.") : ("Your house is already unlocked."));
	gHouseInfo[house][hLocked] = locked;
	House_Save(house);
	House_Refresh(house);
	SendClientMessage(playerid, COLOR_SERVER_INFO, locked ?
		(""COL_GREEN"* House locked."COL_WHITE" Only you can enter.") :
		(""COL_GREEN"* House unlocked."COL_WHITE" Others can knock and enter."));
	return 1;
}

CMD:lockhouse(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	return House_SetLock(playerid, true);
}
alias:lockhouse("hlock")

CMD:unlockhouse(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	return House_SetLock(playerid, false);
}
alias:unlockhouse("hunlock")

// Super Lock upgrade (§5.5) — a paid anti-robbery fitting that lowers break-in
// success. Charged bank-first then cash.
CMD:superlock(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new house = House_Valid(PlayerInfo[playerid][pInsideHouse]) ? PlayerInfo[playerid][pInsideHouse] : House_NearestEntrance(playerid, HOUSE_ENTER_RANGE);
	if(house == -1 || !House_IsOwner(playerid, house))
		return SendClientMessage(playerid, COLOR_ERROR, "You must be at (or inside) a house you own to fit a super lock.");
	if(gHouseInfo[house][hSuperLock]) return SendClientMessage(playerid, COLOR_ERROR, "This house already has a super lock fitted.");
	if(Bank_GetWealth(playerid) < HOUSE_SUPERLOCK_PRICE)
	{
		new err[128];
		format(err, sizeof(err), "A super lock costs "COL_GREEN"$%d"COL_WHITE" (bank + cash) — you cannot afford it.", HOUSE_SUPERLOCK_PRICE);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	if(!House_ChargeBankFirst(playerid, HOUSE_SUPERLOCK_PRICE)) return SendClientMessage(playerid, COLOR_ERROR, "Payment failed.");
	gHouseInfo[house][hSuperLock] = true;
	House_Save(house);
	Jail_SavePlayer(playerid);
	new msg[160];
	format(msg, sizeof(msg), ""COL_GREEN"* Super lock fitted "COL_WHITE"($%d)"COL_GREEN"."COL_WHITE" Break-ins are now much harder to pull off (§ house robbery).", HOUSE_SUPERLOCK_PRICE);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}

// Set the per-game-day rent an owner charges a tenant (0 = not for rent). Minimal
// single-renter model (§3.4).
CMD:setrent(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new house = House_Valid(PlayerInfo[playerid][pInsideHouse]) ? PlayerInfo[playerid][pInsideHouse] : House_NearestEntrance(playerid, HOUSE_ENTER_RANGE);
	if(house == -1 || !House_IsOwner(playerid, house))
		return SendClientMessage(playerid, COLOR_ERROR, "You must be at (or inside) a house you own to set its rent.");
	new rent;
	if(sscanf(params, "d", rent))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /setrent [0 to stop renting, or amount per game-day]");
	if(rent != 0 && (rent < HOUSE_RENT_MIN || rent > HOUSE_RENT_MAX))
	{
		new err[128];
		format(err, sizeof(err), "Rent must be 0 (off) or between $%d and $%d per game-day.", HOUSE_RENT_MIN, HOUSE_RENT_MAX);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	gHouseInfo[house][hRent] = rent;
	if(rent == 0) gHouseInfo[house][hRenterAID] = 0;	// stop renting evicts the tenant
	House_Save(house);
	new msg[144];
	if(rent == 0) format(msg, sizeof(msg), ""COL_GREEN"* House #%d is no longer for rent."COL_WHITE, gHouseInfo[house][hID]);
	else format(msg, sizeof(msg), ""COL_GREEN"* House #%d rent set to "COL_WHITE"$%d"COL_GREEN"/game-day."COL_WHITE" Tenants sign with "COL_YELLOW"/renthouse"COL_WHITE".", gHouseInfo[house][hID], rent);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}

// Rent a house you are standing at (minimal single-renter). The tenant is charged
// the rent each game-day they pass in-server (House_OnGameDay). Cannot rent your own.
CMD:renthouse(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new house = House_NearestEntrance(playerid, HOUSE_ENTER_RANGE);
	if(house == -1) return SendClientMessage(playerid, COLOR_ERROR, "You are not standing at a house entrance.");
	if(gHouseInfo[house][hOwnerAID] == 0) return SendClientMessage(playerid, COLOR_ERROR, "That house is unowned — buy it with /buyhouse.");
	if(House_IsOwner(playerid, house)) return SendClientMessage(playerid, COLOR_ERROR, "You already own this house.");
	if(gHouseInfo[house][hRent] <= 0) return SendClientMessage(playerid, COLOR_ERROR, "This house is not available to rent.");
	if(gHouseInfo[house][hRenterAID] != 0) return SendClientMessage(playerid, COLOR_ERROR, "This house is already rented to someone.");
	if(Bank_GetWealth(playerid) < gHouseInfo[house][hRent])
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot afford the first game-day's rent.");
	if(!House_ChargeBankFirst(playerid, gHouseInfo[house][hRent])) return SendClientMessage(playerid, COLOR_ERROR, "Payment failed.");
	gHouseInfo[house][hRenterAID] = PlayerInfo[playerid][pID];
	House_Save(house);
	Jail_SavePlayer(playerid);
	new msg[160];
	format(msg, sizeof(msg), ""COL_GREEN"* You rented house #%d "COL_WHITE"for "COL_GREEN"$%d"COL_WHITE"/game-day (first day paid). It is now your entry + spawn access. "COL_YELLOW"/stoprenting"COL_WHITE" to end.", gHouseInfo[house][hID], gHouseInfo[house][hRent]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}

CMD:stoprenting(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new found = -1;
	for(new i = 0; i < gHouseCount; i++)
		if(gHouseInfo[i][hRenterAID] == PlayerInfo[playerid][pID]) { found = i; break; }
	if(found == -1) return SendClientMessage(playerid, COLOR_ERROR, "You are not renting any house.");
	gHouseInfo[found][hRenterAID] = 0;
	House_Save(found);
	SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_GREEN"* You ended your rental."COL_WHITE" No more rent will be charged.");
	return 1;
}

//==============================================================================
//	House menu + list
//==============================================================================

// Render the owner's house menu for the house they are inside.
static House_ShowMenu(playerid, house)
{
	new caption[512];
	format(caption, sizeof(caption),
		""COL_WHITE"House "COL_YELLOW"#%d"COL_WHITE"  |  Value: "COL_GREEN"$%d"COL_WHITE"\nSafe money: "COL_GREEN"$%d"COL_WHITE"  |  Items: %d/%d\nLock: %s  |  Super lock: %s  |  Rent: %s\nStore money in the safe\nWithdraw money from the safe\n%s\n%s\nSet this house as my spawn\nSell this house to the bank",
		gHouseInfo[house][hID], gHouseInfo[house][hPrice], gHouseInfo[house][hStorageMoney], gHouseInfo[house][hStorageItems], HOUSE_STORE_ITEMS_MAX,
		gHouseInfo[house][hLocked] ? (""COL_RED"ON"COL_WHITE) : (""COL_GREEN"OFF"COL_WHITE),
		gHouseInfo[house][hSuperLock] ? (""COL_GREEN"YES"COL_WHITE) : (""COL_RED"NO"COL_WHITE),
		gHouseInfo[house][hRent] > 0 ? (""COL_GREEN"listed"COL_WHITE) : (""COL_RED"off"COL_WHITE),
		gHouseInfo[house][hLocked] ? ("Unlock the house") : ("Lock the house"),
		gHouseInfo[house][hSuperLock] ? ("Super lock: fitted") : ("Fit a super lock"));
	ShowPlayerDialog(playerid, HOUSE_MENU_DIALOG, DIALOG_STYLE_LIST, "My House", caption, "Select", "Close");
	return 1;
}

CMD:house(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new house = House_Valid(PlayerInfo[playerid][pInsideHouse]) ? PlayerInfo[playerid][pInsideHouse] : House_NearestEntrance(playerid, HOUSE_ENTER_RANGE);
	if(house == -1 || !House_IsOwner(playerid, house))
		return SendClientMessage(playerid, COLOR_ERROR, "You must be at (or inside) a house you own to open the house menu.");
	return House_ShowMenu(playerid, house);
}

CMD:houses(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new count = 0, msg[160];
	SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_WHITE"===== "COL_YELLOW"Your houses"COL_WHITE" =====");
	for(new i = 0; i < gHouseCount; i++)
	{
		if(gHouseInfo[i][hOwnerAID] != PlayerInfo[playerid][pID]) continue;
		count++;
		format(msg, sizeof(msg), ""COL_WHITE"House "COL_YELLOW"#%d"COL_WHITE" — value "COL_GREEN"$%d"COL_WHITE", safe "COL_GREEN"$%d"COL_WHITE", %s%s", gHouseInfo[i][hID], gHouseInfo[i][hPrice], gHouseInfo[i][hStorageMoney], gHouseInfo[i][hLocked] ? (""COL_RED"locked"COL_WHITE) : (""COL_GREEN"unlocked"COL_WHITE), (PlayerInfo[playerid][pHouseSpawn] == gHouseInfo[i][hID]) ? (" "COL_YELLOW"[spawn]"COL_WHITE) : (""));
		SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	}
	if(count == 0) SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_WHITE"You do not own any houses. Buy one at a "COL_GREEN"green house pickup"COL_WHITE" with "COL_YELLOW"/buyhouse"COL_WHITE".");
	else
	{
		format(msg, sizeof(msg), ""COL_WHITE"Owned: "COL_GREEN"%d/%d"COL_WHITE".", count, MAX_OWNED_HOUSES);
		SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	}
	return 1;
}
alias:houses("hlist", "houselist")

CMD:enter(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(House_Valid(PlayerInfo[playerid][pInsideHouse])) return SendClientMessage(playerid, COLOR_ERROR, "You are already inside. Use /exit to leave.");
	if(!House_TryEnter(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You are not standing at a house entrance.");
	return 1;
}

CMD:exit(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(!House_Valid(PlayerInfo[playerid][pInsideHouse])) return SendClientMessage(playerid, COLOR_ERROR, "You are not inside a house.");
	return House_Leave(playerid);
}

//==============================================================================
//	House robbery support (§5.5) — called from robbery.inc's /houserob
//==============================================================================

// Fire the house alarm: notify the owner (if online) + all cops (§5.5).
static House_Alarm(playerid, house)
{
	new zoneName[MAX_ZONE_NAME];
	if(isNull(PlayerInfo[playerid][pZoneName])) format(zoneName, sizeof(zoneName), "the city");
	else format(zoneName, sizeof(zoneName), "%s", PlayerInfo[playerid][pZoneName]);

	// Owner (if online).
	new ownerid;
	if(House_IsOwnerOnline(house, ownerid))
	{
		new omsg[176];
		format(omsg, sizeof(omsg), ""COL_RED"* HOUSE ALARM! "COL_WHITE"Someone is breaking into your house #%d near %s!", gHouseInfo[house][hID], zoneName);
		SendClientMessage(ownerid, COLOR_ERROR, omsg);
	}
	// All cops.
	new cmsg[192];
	format(cmsg, sizeof(cmsg), ""COL_ORANGE"[HOUSE ALARM] "COL_WHITE"%s (%d) is breaking into house #%d near %s! Respond!", PlayerInfo[playerid][pUserName], playerid, gHouseInfo[house][hID], zoneName);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || i == playerid) continue;
		if(IsCop(i)) SendClientMessage(i, COLOR_WANTED_ORANGE, cmsg);
	}
	return 1;
}

// Run the break-in on `house`: roll success against the super-lock/pet defenses,
// then MOVE house-storage money to the robber (no mint) + steal a slice of items,
// trigger the alarm (owner + cops), and stamp cooldown. Returns the money taken
// (>= 0), or -1 if the break-in was FOILED (no loot; the robber may take pet damage).
// The +6 wanted + ROB_HOUSE history flag are applied by the caller in robbery.inc.
FUNCTION House_RobSteal(playerid, house)
{
	if(!House_Valid(house)) return 0;

	// Break-in success roll: base %, minus super-lock / pet penalties, plus the
	// crowbar bonus (a crowbar improves success — §5.7). Read the crowbar flag
	// directly (housing sees PlayerInfo).
	new chance = HOUSEROB_BASE_SUCCESS;
	if(gHouseInfo[house][hSuperLock]) chance -= HOUSEROB_SUPERLOCK_PEN;
	if(gHouseInfo[house][hHasPet]) chance -= HOUSEROB_PET_PEN;
	if(PlayerInfo[playerid][pCrowbarEquipped]) chance += CROWBAR_SUCCESS_BONUS;
	if(chance < 5) chance = 5;
	if(chance > 95) chance = 95;

	// The house alarm always fires on an attempt (owner + cops), even a foiled one
	// (§5.5 "alerts all owners, co-owners and the police anytime someone breaks in").
	House_Alarm(playerid, house);

	if(random(100) >= chance)
	{
		// Foiled — super lock held or a pet drove the intruder off (a pet bites).
		if(gHouseInfo[house][hHasPet])
		{
			new Float:hp;
			GetPlayerHealth(playerid, hp);
			hp -= HOUSEROB_PET_DAMAGE;
			if(hp < 1.0) hp = 1.0;	// a pet mauls but does not kill on the break-in
			SetPlayerHealth(playerid, hp);
			SendClientMessage(playerid, COLOR_ERROR, ""COL_RED"* The house pet attacked you! "COL_WHITE"The break-in failed.");
		}
		else SendClientMessage(playerid, COLOR_ERROR, ""COL_RED"* The lock held — the break-in failed."COL_WHITE);
		return -1;
	}

	// Success — MOVE a percentage of the house safe money to the robber (no mint).
	new take = (gHouseInfo[house][hStorageMoney] / 100) * HOUSEROB_MONEY_PCT;
	if(take > 0)
	{
		gHouseInfo[house][hStorageMoney] -= take;	// the house LOSES exactly what the robber GAINS
		Bank_GiveCash(playerid, take);
	}
	// Steal a slice of stored items too (half of them, §5.5 storage loot).
	new itemsTaken = gHouseInfo[house][hStorageItems] / 2;
	if(itemsTaken > 0) gHouseInfo[house][hStorageItems] -= itemsTaken;

	House_Save(house);

	new msg[176];
	if(take > 0 || itemsTaken > 0) format(msg, sizeof(msg), ""COL_GREEN"* Break-in! "COL_WHITE"You looted "COL_GREEN"$%d"COL_WHITE" and %d item(s) from house #%d. Run!", take, itemsTaken, gHouseInfo[house][hID]);
	else format(msg, sizeof(msg), ""COL_GREEN"* Break-in! "COL_WHITE"but the house safe was empty. You still triggered the alarm — run!");
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return take;
}

//==============================================================================
//	Game-day tick — rent charge + 2-week inactivity decay (§3.2/§3.4)
//==============================================================================

// Called from GameModeClock()'s day rollover in CnR.pwn (NO new global timer). For
// each house: charge an online tenant the per-game-day rent (paid to the owner if
// online, else banked to the owner via aID is deferred — minimal single-renter),
// and revert an owner's unvisited-for-2-weeks house to for-sale (§3.2).
FUNCTION House_OnGameDay()
{
	new now = gettime();
	for(new i = 0; i < gHouseCount; i++)
	{
		if(gHouseInfo[i][hOwnerAID] == 0) continue;

		// --- Rent: charge an ONLINE tenant this game-day (minimal single-renter). ---
		if(gHouseInfo[i][hRenterAID] != 0 && gHouseInfo[i][hRent] > 0)
		{
			new tenantid = INVALID_PLAYER_ID;
			for(new pid = 0; pid < MAX_PLAYERS; pid++)
			{
				if(!IsPlayerConnected(pid) || IsPlayerNPC(pid) || !PlayerInfo[pid][pLoggedIn]) continue;
				if(PlayerInfo[pid][pID] == gHouseInfo[i][hRenterAID]) { tenantid = pid; break; }
			}
			if(tenantid != INVALID_PLAYER_ID)
			{
				if(House_ChargeBankFirst(tenantid, gHouseInfo[i][hRent]))
				{
					new tmsg[144];
					format(tmsg, sizeof(tmsg), ""COL_YELLOW"* Rent: "COL_WHITE"$%d"COL_YELLOW" for house #%d was charged for this game-day."COL_WHITE, gHouseInfo[i][hRent], gHouseInfo[i][hID]);
					SendClientMessage(tenantid, COLOR_SERVER_INFO, tmsg);
					Jail_SavePlayer(tenantid);
					// Pay the online owner (money MOVES tenant->owner, no mint).
					new ownerid;
					if(House_IsOwnerOnline(i, ownerid))
					{
						Bank_Give(ownerid, gHouseInfo[i][hRent]);
						Jail_SavePlayer(ownerid);
					}
				}
				else
				{
					// Cannot pay — evict.
					gHouseInfo[i][hRenterAID] = 0;
					House_Save(i);
					SendClientMessage(tenantid, COLOR_ERROR, ""COL_RED"* You could not pay this game-day's rent "COL_WHITE"and were evicted.");
				}
			}
		}

		// --- Inactivity decay: unvisited for 2 real weeks → revert to for-sale. ---
		// A LastVisited of 0 means "not stamped this session"; a house only decays
		// once the owner has visited (stamping the clock) and then stayed away.
		if(gHouseInfo[i][hLastVisited] != 0 && (now - gHouseInfo[i][hLastVisited]) >= HOUSE_DECAY_SECS)
		{
			new prevOwner = gHouseInfo[i][hOwnerAID];
			gHouseInfo[i][hOwnerAID] = 0;
			gHouseInfo[i][hForSale] = true;
			gHouseInfo[i][hLocked] = false;
			gHouseInfo[i][hSuperLock] = false;
			gHouseInfo[i][hRenterAID] = 0;
			gHouseInfo[i][hLastVisited] = 0;
			House_Save(i);
			House_Refresh(i);
			// Notify + recount an online former owner.
			for(new pid = 0; pid < MAX_PLAYERS; pid++)
			{
				if(!IsPlayerConnected(pid) || IsPlayerNPC(pid) || !PlayerInfo[pid][pLoggedIn]) continue;
				if(PlayerInfo[pid][pID] == prevOwner)
				{
					new emsg[160];
					format(emsg, sizeof(emsg), ""COL_RED"* House #%d was repossessed "COL_WHITE"— you had not visited it in 2 weeks.", gHouseInfo[i][hID]);
					SendClientMessage(pid, COLOR_ERROR, emsg);
					House_RecountOwned(pid);
					break;
				}
			}
		}
	}
	return 1;
}

//==============================================================================
//	Spawn at house / disconnect hooks
//==============================================================================

// Spawn the owner inside their /sethousespawn house (from OnPlayerSpawn). Returns 1
// if the player was placed inside a house (so the caller skips the default spawn
// position), 0 otherwise.
FUNCTION House_OnPlayerSpawn(playerid)
{
	if(!PlayerInfo[playerid][pLoggedIn]) return 0;
	// Respawning at a city spawn (interior 0) — the caller reset interior/VW, so drop
	// any stale inside-house marker before deciding on a house spawn.
	PlayerInfo[playerid][pInsideHouse] = -1;
	if(PlayerInfo[playerid][pHouseSpawn] <= 0) return 0;
	// A jailed / admin-jailed player is placed at the jail by the jail system — never
	// override that with a house spawn (marker already cleared above).
	if(PlayerInfo[playerid][pJailed] || PlayerInfo[playerid][pAdminJailUntil] > gettime()) return 0;
	new house = House_FindByID(PlayerInfo[playerid][pHouseSpawn]);
	// Only spawn there if still owned by this player (else clear the stale spawn).
	if(house == -1 || !House_IsOwner(playerid, house))
	{
		PlayerInfo[playerid][pHouseSpawn] = -1;
		return 0;
	}
	House_Enter(playerid, house);
	return 1;
}

// Drop the inside-house session marker on disconnect (a fresh connect re-reads -1).
FUNCTION House_OnDisconnect(playerid)
{
	PlayerInfo[playerid][pInsideHouse] = -1;
	g_HouseBuyTarget[playerid] = -1;
	return 1;
}

//==============================================================================
//	Scripter — /addhouse (create a house here) + /edithouse
//==============================================================================

// Create a new house at the caller's position. The interior + inside-spawn default
// to a stock SA-MP safehouse interior; /edithouse tweaks price/interior/pet after.
CMD:addhouse(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_SCRIPTER) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	if(gHouseCount >= MAX_HOUSES) return SendClientMessage(playerid, COLOR_ERROR, "The house table is full (MAX_HOUSES).");
	new price;
	if(sscanf(params, "D(500000)", price)) price = 500000;
	if(price < HOUSE_PRICE_MIN) price = HOUSE_PRICE_MIN;
	if(price > HOUSE_PRICE_MAX) price = HOUSE_PRICE_MAX;

	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	// A stock interior 3 (Ryder's / small house) with a sane inside spawn.
	new query[512];
	mysql_format(g_SQL, query, sizeof(query),
		"INSERT INTO `houses` (OwnerAID, Price, X, Y, Z, Interior, IntX, IntY, IntZ, ForSale) VALUES (0, %d, %f, %f, %f, 3, 2496.0, -1695.0, 1014.7, 1)",
		price, x, y, z);
	// Pass the connect race-check token so a slot reused before the INSERT completes
	// is not messaged as the wrong player (g_MysqlRaceCheck guard, per the brief).
	mysql_pquery(g_SQL, query, "House_OnAdded", "dd", playerid, g_MysqlRaceCheck[playerid]);
	return 1;
}

// Async handler for /addhouse — reload the house table so the new row is live with
// its streamer objects, and confirm to the scripter (race-check guarded).
FUNCTION House_OnAdded(playerid, race_check)
{
	new id = cache_insert_id();
	House_Init();	// full reload rebuilds gHouseInfo + all streamer objects
	if(IsPlayerConnected(playerid) && race_check == g_MysqlRaceCheck[playerid])
	{
		new msg[128];
		format(msg, sizeof(msg), ""COL_GREEN"* House #%d created here "COL_WHITE"(for sale). Reloaded the house table.", id);
		SendClientMessage(playerid, COLOR_ADMIN_INFO, msg);
	}
	return 1;
}

// Edit the nearest house: /edithouse [price|interior|pet] [value]. Scripter-only.
CMD:edithouse(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_SCRIPTER) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new field[12], value;
	if(sscanf(params, "s[12]D(0)", field, value))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /edithouse [price | interior | intspawn | pet] [value]");
	new house = House_NearestEntrance(playerid, 6.0);
	if(house == -1) house = PlayerInfo[playerid][pInsideHouse];
	if(!House_Valid(house)) return SendClientMessage(playerid, COLOR_ERROR, "No house nearby (stand at an entrance or inside).");

	new query[256], msg[144];
	if(!strcmp(field, "price", true))
	{
		if(value < HOUSE_PRICE_MIN) value = HOUSE_PRICE_MIN;
		if(value > HOUSE_PRICE_MAX) value = HOUSE_PRICE_MAX;
		gHouseInfo[house][hPrice] = value;
		mysql_format(g_SQL, query, sizeof(query), "UPDATE `houses` SET Price=%d WHERE ID=%d LIMIT 1", value, gHouseInfo[house][hID]);
		mysql_pquery(g_SQL, query);
		House_Refresh(house);
		format(msg, sizeof(msg), ""COL_GREEN"* House #%d price set to $%d.", gHouseInfo[house][hID], value);
	}
	else if(!strcmp(field, "interior", true))
	{
		gHouseInfo[house][hInterior] = value;
		mysql_format(g_SQL, query, sizeof(query), "UPDATE `houses` SET Interior=%d WHERE ID=%d LIMIT 1", value, gHouseInfo[house][hID]);
		mysql_pquery(g_SQL, query);
		format(msg, sizeof(msg), ""COL_GREEN"* House #%d interior set to %d.", gHouseInfo[house][hID], value);
	}
	else if(!strcmp(field, "intspawn", true))
	{
		// Take the caller's current position as the inside spawn.
		GetPlayerPos(playerid, gHouseInfo[house][hIntX], gHouseInfo[house][hIntY], gHouseInfo[house][hIntZ]);
		mysql_format(g_SQL, query, sizeof(query), "UPDATE `houses` SET IntX=%f, IntY=%f, IntZ=%f, Interior=%d WHERE ID=%d LIMIT 1", gHouseInfo[house][hIntX], gHouseInfo[house][hIntY], gHouseInfo[house][hIntZ], GetPlayerInterior(playerid), gHouseInfo[house][hID]);
		gHouseInfo[house][hInterior] = GetPlayerInterior(playerid);
		mysql_pquery(g_SQL, query);
		format(msg, sizeof(msg), ""COL_GREEN"* House #%d inside spawn set to your position (interior %d).", gHouseInfo[house][hID], gHouseInfo[house][hInterior]);
	}
	else if(!strcmp(field, "pet", true))
	{
		gHouseInfo[house][hHasPet] = (value != 0);
		mysql_format(g_SQL, query, sizeof(query), "UPDATE `houses` SET HasPet=%d WHERE ID=%d LIMIT 1", value != 0 ? 1 : 0, gHouseInfo[house][hID]);
		mysql_pquery(g_SQL, query);
		format(msg, sizeof(msg), ""COL_GREEN"* House #%d pet %s.", gHouseInfo[house][hID], value != 0 ? ("enabled") : ("disabled"));
	}
	else return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /edithouse [price | interior | intspawn | pet] [value]");
	SendClientMessage(playerid, COLOR_ADMIN_INFO, msg);
	return 1;
}

//==============================================================================
//	Dialog resolution (routed from OnDialogResponse in CnR.pwn)
//==============================================================================

FUNCTION House_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
	switch(dialogid)
	{
		case HOUSE_BUY_DIALOG:
		{
			new house = g_HouseBuyTarget[playerid];
			g_HouseBuyTarget[playerid] = -1;
			if(!response) return 1;	// Cancel
			return House_DoBuy(playerid, house);
		}
		case HOUSE_MENU_DIALOG:
		{
			if(!response) return 1;
			new house = PlayerInfo[playerid][pInsideHouse];
			if(!House_Valid(house) || !House_IsOwner(playerid, house)) return SendClientMessage(playerid, COLOR_ERROR, "You are no longer in your house.");
			new noparams[1]; noparams[0] = EOS;	// mutable empty params for the callcmd:: redirects (non-const)
			switch(listitem)
			{
				// The caption's non-selectable header lines are indices 0-2; the
				// action rows begin at index 3 (see House_ShowMenu's layout).
				case 3: ShowPlayerDialog(playerid, HOUSE_STORE_DIALOG, DIALOG_STYLE_INPUT, "Store money", ""COL_WHITE"Enter the amount of cash to store in the house safe:", "Store", "Back");
				case 4: ShowPlayerDialog(playerid, HOUSE_WITHDRAW_DIALOG, DIALOG_STYLE_INPUT, "Withdraw money", ""COL_WHITE"Enter the amount to take from the house safe:", "Withdraw", "Back");
				case 5: House_SetLock(playerid, !gHouseInfo[house][hLocked]);
				case 6: { if(!gHouseInfo[house][hSuperLock]) callcmd::superlock(playerid, noparams); else SendClientMessage(playerid, COLOR_SERVER_INFO, "This house already has a super lock."); }
				case 7: callcmd::sethousespawn(playerid, noparams);
				case 8: callcmd::sellhouse(playerid, noparams);
			}
			return 1;
		}
		case HOUSE_STORE_DIALOG:
		{
			if(!response) return House_ShowMenu(playerid, PlayerInfo[playerid][pInsideHouse]);	// Back
			if(!isNumeric(inputtext)) return SendClientMessage(playerid, COLOR_ERROR, "Enter a positive number.");
			return House_DoStore(playerid, strval(inputtext));
		}
		case HOUSE_WITHDRAW_DIALOG:
		{
			if(!response) return House_ShowMenu(playerid, PlayerInfo[playerid][pInsideHouse]);	// Back
			if(!isNumeric(inputtext)) return SendClientMessage(playerid, COLOR_ERROR, "Enter a positive number.");
			return House_DoWithdraw(playerid, strval(inputtext));
		}
	}
	return 0;
}
