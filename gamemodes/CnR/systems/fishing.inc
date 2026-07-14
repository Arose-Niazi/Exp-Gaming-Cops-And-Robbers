/*

	Fishing system for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 6 (stage 2) - design §9.2 (fishing) + §10.2 (fish × FISH_MARKET) +
	§11.2 (FishPermits + cooler cols) + docs/research/economy.md §8 + docs/research/
	crazybobs-site-full §Fishing.

	CB: "Fishing is one of the best ways to earn your money!" A player buys a fishing
	rod, permits and a cooler at a BAIT SHOP, then /fish at a pier/water spot to reel
	in a catch (species class × weight × rarity). Catches go into a compact COOLER
	inventory (count + total pounds + accumulated base value - no per-fish table), then
	sold at a fish market via /fishsellall for Bank_GiveCash(baseValue × FISH_MARKET
	mult). Selling feeds the Fish Market pool (supply pushes the price down, §10.2).

	A rod reduces the reel-in time + nudges the rarity roll up; a Fisherman skill nudges
	it further (§9.2). A fishing permit is consumed per catch (1 fish/permit) and avoids
	a +1 wanted for fishing near a cop (§9.2). Bonus fish is announced at game-hour 05:00
	(GameModeClock hook) - the first to catch it after that gets a big cash reward (§9.2).
	Rare "baby seal" tier catches are the huge payouts (§8.2).

	Persistence: FishPermits + rod/cooler/sales-permit flags + the cooler aggregate live
	in LSplayers (loaded in loading_data, saved via the shared Jail_SavePlayer UPDATE).

	Public API (called by CnR.pwn):
		Fishing_Init()                      : create Bait Shop / fish-spot / market markers (OnGameModeInit)
		Fishing_Cleanup()                   : destroy them (OnGameModeExit)
		Fishing_OnBonusHour()               : announce the daily bonus fish (GameModeClock 05:00 hook)
		Fishing_OnDisconnect(playerid)      : kill the reel-in timer
		Fishing_OnPlayerDeath(playerid)     : abort an in-progress catch on death
		Fishing_OnDialogResponse(playerid, dialogid, response, listitem) : Bait Shop menu

	Commands:
		/fish                       reel in a catch at a fishing spot
		/baitshop (/bait)           open the Bait Shop buy menu (rod/cooler/permits/seeds)
		/cooler (/fishinventory)    show the cooler contents
		/fishsellall (/fsell)       sell the whole cooler at a fish market
		/rod                        toggle the fishing rod in/out of hand (flavour)

	Functions Being Called:
		bank:     Bank_GiveCash, Bank_TakeCash
		wanted:   GivePlayerWanted
		jail:     Jail_IsJailed, Jail_SavePlayer (persists the fishing cols)
		misc:     IsCop
		skills:   pSkill (SKILL_* - a Fisherman-family skill nudges the rarity roll)
		economy:  StockMarket_UpdateEarnings, StockMarket_PriceMult (M7 stubs)
*/

//==============================================================================
//	Markers - Bait Shops, fishing spots (piers), fish markets (§9.2)
//==============================================================================

// Bait Shops (economy.md §2.3 / crazybobs-site-full §Bait Shops). Only LS-area
// coords are reachable while CITY_ == LS; the others are kept for later cities.
static const Float:g_BaitShops[][3] =
{
	{ 372.900,   -2045.000, 7.800 },	// LS Santa Maria Pier bait shop
	{ 626.700,   1699.400,  6.900 },	// LS Red County / Montgomery bait shop
	{ -2196.000, 2383.000,  4.900 }		// LV Tierra Robada bait shop (kept for later)
};

// Fishing spots - piers / jetties where /fish works (on foot at the water's edge).
// Kept a curated list so /fish is location-gated (design §9.2 "pier/water proximity").
static const Float:g_FishSpots[][3] =
{
	{ 334.700,   -2073.700, 7.800 },	// Santa Maria Pier tip (LS)
	{ 385.500,   -2085.900, 7.800 },	// Santa Maria Pier east (LS)
	{ 1503.900,  2773.900,  10.800 },	// LV pier (kept for later)
	{ 645.200,   1709.800,  6.900 }		// Red County lake edge (LS)
};

// Fish markets - where the cooler is sold (24/7 / Bait Shop, economy.md §8.2).
static const Float:g_FishMarkets[][3] =
{
	{ 372.900,   -2045.000, 7.800 },	// Santa Maria Pier bait shop (also a market)
	{ 626.700,   1699.400,  6.900 }		// Red County / Montgomery bait shop
};

static STREAMER_TAG_MAP_ICON:g_BaitIcon[sizeof(g_BaitShops)];
static STREAMER_TAG_MAP_ICON:g_FishSpotIcon[sizeof(g_FishSpots)];

// Daily bonus fish (§9.2) - announced at 05:00; the first catch after that wins it.
static bool:g_BonusFishActive = false;
static g_BonusFishSpecies[24];

//==============================================================================
//	Setup / teardown
//==============================================================================

FUNCTION Fishing_Init()
{
	for(new i = 0; i < sizeof(g_BaitShops); i++)
		g_BaitIcon[i] = CreateDynamicMapIcon(g_BaitShops[i][0], g_BaitShops[i][1], g_BaitShops[i][2], 52, 0, -1, -1, -1, 250.0, MAPICON_LOCAL);
	for(new i = 0; i < sizeof(g_FishSpots); i++)
		g_FishSpotIcon[i] = CreateDynamicMapIcon(g_FishSpots[i][0], g_FishSpots[i][1], g_FishSpots[i][2], 42, 0, -1, -1, -1, 250.0, MAPICON_LOCAL);
	return 1;
}

FUNCTION Fishing_Cleanup()
{
	for(new i = 0; i < sizeof(g_BaitShops); i++)
		if(DestroyDynamicMapIcon(g_BaitIcon[i])) {}
	for(new i = 0; i < sizeof(g_FishSpots); i++)
		if(DestroyDynamicMapIcon(g_FishSpotIcon[i])) {}
	return 1;
}

//==============================================================================
//	Proximity helpers
//==============================================================================

static Fishing_AtBaitShop(playerid)
{
	for(new i = 0; i < sizeof(g_BaitShops); i++)
		if(IsPlayerInRangeOfPoint(playerid, FISH_SPOT_RANGE, g_BaitShops[i][0], g_BaitShops[i][1], g_BaitShops[i][2]))
			return 1;
	return 0;
}

static Fishing_AtSpot(playerid)
{
	for(new i = 0; i < sizeof(g_FishSpots); i++)
		if(IsPlayerInRangeOfPoint(playerid, FISH_SPOT_RANGE, g_FishSpots[i][0], g_FishSpots[i][1], g_FishSpots[i][2]))
			return 1;
	return 0;
}

static Fishing_AtMarket(playerid)
{
	for(new i = 0; i < sizeof(g_FishMarkets); i++)
		if(IsPlayerInRangeOfPoint(playerid, FISH_SPOT_RANGE, g_FishMarkets[i][0], g_FishMarkets[i][1], g_FishMarkets[i][2]))
			return 1;
	return 0;
}

// Is a cop within sight of this player (drives the no-permit wanted, §9.2)?
static Fishing_IsCopInSight(playerid)
{
	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || i == playerid) continue;
		if(!PlayerInfo[i][pLoggedIn] || !PlayerInfo[i][pSpawned]) continue;
		if(!IsCop(i)) continue;
		if(IsPlayerInRangeOfPoint(i, FISH_COPSIGHT_RANGE, x, y, z)) return 1;
	}
	return 0;
}

// The player's cooler capacity (cooler upgrade vs bare-hands).
static Fishing_CoolerCap(playerid)
	return PlayerInfo[playerid][pHasFishCooler] ? FISH_COOLER_CAP : FISH_NOCOOLER_CAP;

//==============================================================================
//	Fish species / rarity roll (§9.2 / §8.2)
//==============================================================================

// Rarity multiplier for a tier (weight × rarity × base = value, §8.2).
static Fishing_RarityMult(tier)
{
	switch(tier)
	{
		case FISH_RARITY_LEGENDARY: return 12;	// baby seal / killer fish - the big payout
		case FISH_RARITY_RARE:      return 5;	// turtle / marlin
		case FISH_RARITY_UNCOMMON:  return 2;	// bass / catfish
	}
	return 1;								// common - trout / boot
}

// A random species name for a rarity tier (flavour only - the value is weight×rarity).
static Fishing_SpeciesName(tier, out[], len)
{
	switch(tier)
	{
		case FISH_RARITY_LEGENDARY:
		{
			static const names[][24] = { "Baby Seal", "Killer Fish", "Great White" };
			format(out, len, "%s", names[random(sizeof(names))]);
		}
		case FISH_RARITY_RARE:
		{
			static const names[][24] = { "Sea Turtle", "Blue Marlin", "Sturgeon" };
			format(out, len, "%s", names[random(sizeof(names))]);
		}
		case FISH_RARITY_UNCOMMON:
		{
			static const names[][24] = { "Sea Bass", "Catfish", "Salmon" };
			format(out, len, "%s", names[random(sizeof(names))]);
		}
		default:
		{
			static const names[][24] = { "Trout", "Mackerel", "Old Boot" };
			format(out, len, "%s", names[random(sizeof(names))]);
		}
	}
	return 1;
}

// Does the player have a Fisherman-family skill (nudges the rarity roll, §9.2)? The
// choosable civilian skills are set at City Hall (M6 stage 1). CB never named a
// dedicated "Fisherman" skill in our enum; the Street Vendor is the closest trade
// perk, so it reads as the fishing bonus here [OUR DESIGN mapping].
static bool:Fishing_HasSkillBonus(playerid)
	return (PlayerInfo[playerid][pSkill] == SKILL_STREETVENDOR);

// Roll a rarity tier. A rod + skill push the roll toward rarer tiers (§9.2).
static Fishing_RollRarity(playerid)
{
	new roll = random(100);
	// Subtract bonuses from the roll so a lower effective roll -> rarer tier bands.
	if(PlayerInfo[playerid][pHasFishingRod]) roll -= FISH_ROD_RARITY_BONUS * 3;
	if(Fishing_HasSkillBonus(playerid))      roll -= FISH_SKILL_RARITY_BONUS * 3;
	if(roll < 0) roll = 0;
	if(roll < FISH_ROLL_LEGENDARY) return FISH_RARITY_LEGENDARY;
	if(roll < FISH_ROLL_RARE)      return FISH_RARITY_RARE;
	if(roll < FISH_ROLL_UNCOMMON)  return FISH_RARITY_UNCOMMON;
	return FISH_RARITY_COMMON;
}

//==============================================================================
//	/fish - reel in a catch (§9.2)
//==============================================================================

CMD:fish(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(Jail_IsJailed(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot fish while jailed.");
	if(IsPlayerInAnyVehicle(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Get out of your vehicle and stand at the water's edge to fish.");
	if(PlayerInfo[playerid][pFishing]) return SendClientMessage(playerid, COLOR_ERROR, "You are already reeling in a catch - wait.");
	if(!Fishing_AtSpot(playerid))
		return SendClientMessage(playerid, COLOR_ERROR, "You must stand at a fishing spot (a pier or the water's edge - look for the fish map icons).");

	// Cooler capacity check (§9.2 - a cooler holds more; bare-handed holds a couple).
	if(PlayerInfo[playerid][pCoolerCount] >= Fishing_CoolerCap(playerid))
	{
		new err[144];
		format(err, sizeof(err), "Your %s is full (%d fish). Sell at a fish market with "COL_GREEN"/fishsellall"COL_WHITE" first.",
			PlayerInfo[playerid][pHasFishCooler] ? "cooler" : "hands (buy a cooler at a Bait Shop for more)", PlayerInfo[playerid][pCoolerCount]);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}

	// Lock the spot position (moved-away check) and start the reel-in wait.
	GetPlayerPos(playerid, PlayerInfo[playerid][pFishX], PlayerInfo[playerid][pFishY], PlayerInfo[playerid][pFishZ]);
	PlayerInfo[playerid][pFishing] = true;
	ApplyAnimation(playerid, "SUNBATHE", "batherdown", 4.0, false, false, false, false, 0, true);

	new secs = PlayerInfo[playerid][pHasFishingRod] ? FISH_CATCH_SECS_ROD : FISH_CATCH_SECS;
	if(PlayerInfo[playerid][pFishTimer] != -1) KillTimer(PlayerInfo[playerid][pFishTimer]);
	PlayerInfo[playerid][pFishTimer] = SetTimerEx("Fishing_ReelIn", secs * 1000, false, "d", playerid);

	new msg[128];
	format(msg, sizeof(msg), ""COL_WHITE"You cast your line%s... reeling in (%d sec).", PlayerInfo[playerid][pHasFishingRod] ? " with your rod" : "", secs);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}

// Reel-in resolution (one-shot timer). Rolls species/weight/rarity, consumes a permit
// (or gives +1 wanted near a cop with none, §9.2), banks the catch into the cooler,
// and resolves the daily bonus fish.
FUNCTION Fishing_ReelIn(playerid)
{
	PlayerInfo[playerid][pFishTimer] = -1;
	if(!IsPlayerConnected(playerid) || !PlayerInfo[playerid][pLoggedIn]) { PlayerInfo[playerid][pFishing] = false; return 0; }
	PlayerInfo[playerid][pFishing] = false;
	ClearAnimations(playerid);

	// Must still be at the spot (anti-walk-away).
	if(!IsPlayerInRangeOfPoint(playerid, FISH_SPOT_RANGE, PlayerInfo[playerid][pFishX], PlayerInfo[playerid][pFishY], PlayerInfo[playerid][pFishZ]))
		return SendClientMessage(playerid, COLOR_ERROR, "You moved away from the water - the fish got away.");

	// No-permit wanted: fishing near a cop with no valid permit is a small crime (§9.2).
	if(PlayerInfo[playerid][pFishPermits] <= 0 && Fishing_IsCopInSight(playerid))
		GivePlayerWanted(playerid, FISH_NOPERMIT_WANTED, "Fishing without a permit");

	// Consume one fishing permit per catch (1 fish/permit, §9.2) if any are held.
	if(PlayerInfo[playerid][pFishPermits] > 0) PlayerInfo[playerid][pFishPermits]--;

	new tier = Fishing_RollRarity(playerid);
	new weight = FISH_WEIGHT_MIN + random(FISH_WEIGHT_MAX - FISH_WEIGHT_MIN + 1);
	// Legendary tier trends heavy (baby seal >80lb, §8.2) so the payout lands big.
	if(tier == FISH_RARITY_LEGENDARY && weight < 60) weight += 20;
	new rarity = Fishing_RarityMult(tier);
	new value = weight * rarity * FISH_BASE_PRICE;

	new species[24];
	Fishing_SpeciesName(tier, species, sizeof(species));

	// Bank into the compact cooler aggregate.
	PlayerInfo[playerid][pCoolerCount]++;
	PlayerInfo[playerid][pCoolerWeight] += weight;
	PlayerInfo[playerid][pCoolerValue]  += value;

	new msg[176];
	format(msg, sizeof(msg), ""COL_GREEN"* You caught a %s"COL_WHITE" - %d lb (worth ~"COL_GREEN"$%d"COL_WHITE"). Cooler: %d/%d fish.",
		species, weight, value, PlayerInfo[playerid][pCoolerCount], Fishing_CoolerCap(playerid));
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	if(tier == FISH_RARITY_LEGENDARY)
		SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_YELLOW"A rare catch! That will fetch a fortune at the fish market.");

	// Daily bonus fish (§9.2) - first catch after the 05:00 announce wins the reward.
	if(g_BonusFishActive)
	{
		g_BonusFishActive = false;
		Bank_GiveCash(playerid, FISH_BONUS_REWARD);
		new bmsg[176];
		format(bmsg, sizeof(bmsg), ""COL_YELLOW"BONUS FISH! %s"COL_WHITE" landed the daily bonus fish and won "COL_GREEN"$%d"COL_WHITE"!", PlayerInfo[playerid][pUserName], FISH_BONUS_REWARD);
		SendClientMessageToAll(COLOR_SERVER_INFO, bmsg);
	}

	Jail_SavePlayer(playerid);	// persist the permit decrement + cooler aggregate
	return 1;
}

//==============================================================================
//	/fishsellall - sell the whole cooler at a fish market (§9.2/§10.2)
//==============================================================================

CMD:fishsellall(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(!Fishing_AtMarket(playerid))
		return SendClientMessage(playerid, COLOR_ERROR, "You must be at a fish market (a Bait Shop / 24-7) to sell your fish.");
	if(PlayerInfo[playerid][pCoolerCount] <= 0)
		return SendClientMessage(playerid, COLOR_ERROR, "Your cooler is empty - catch some fish with /fish first.");

	new count = PlayerInfo[playerid][pCoolerCount];
	new weight = PlayerInfo[playerid][pCoolerWeight];
	new base = PlayerInfo[playerid][pCoolerValue];
	// Apply the Fish Market price multiplier (M7 stub returns 1.0 -> FISH_MARKET base
	// mult 1.0 today, design §10.2). floatround keeps it an integer payout.
	new payout = floatround(float(base) * StockMarket_PriceMult(E_STOCK_FISH_MARKET));
	if(payout < 1) payout = 1;

	// Empty the cooler and bank the payout.
	PlayerInfo[playerid][pCoolerCount] = 0;
	PlayerInfo[playerid][pCoolerWeight] = 0;
	PlayerInfo[playerid][pCoolerValue] = 0;
	Bank_GiveCash(playerid, payout);
	PlayerInfo[playerid][pScore]++;

	// Selling feeds the Fish Market pool (supply pushes the price down, §10.2/§8.3).
	StockMarket_UpdateEarnings(E_STOCK_FISH_MARKET, payout, STOCK_FACTOR_FISH);

	Jail_SavePlayer(playerid);	// persist the emptied cooler + score + money

	new msg[176];
	format(msg, sizeof(msg), ""COL_GREEN"* Sold %d fish (%d lb) for "COL_WHITE"$%d"COL_GREEN". +1 score."COL_WHITE, count, weight, payout);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}
alias:fishsellall("fsell")

//==============================================================================
//	/cooler - show the cooler contents (§9.2)
//==============================================================================

CMD:cooler(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new msg[192];
	new payout = floatround(float(PlayerInfo[playerid][pCoolerValue]) * StockMarket_PriceMult(E_STOCK_FISH_MARKET));
	format(msg, sizeof(msg), ""COL_WHITE"Cooler: "COL_GREEN"%d/%d"COL_WHITE" fish, "COL_GREEN"%d"COL_WHITE" lb, market value ~"COL_GREEN"$%d"COL_WHITE". Permits: "COL_GREEN"%d"COL_WHITE".",
		PlayerInfo[playerid][pCoolerCount], Fishing_CoolerCap(playerid), PlayerInfo[playerid][pCoolerWeight], payout, PlayerInfo[playerid][pFishPermits]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	if(!PlayerInfo[playerid][pHasFishingRod])
		SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_WHITE"Buy a rod, cooler and permits at a Bait Shop ("COL_GREEN"/baitshop"COL_WHITE").");
	return 1;
}
alias:cooler("fishinventory")

// /rod - flavour toggle: put the rod in/out of hand (a rod must be owned).
CMD:rod(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(!PlayerInfo[playerid][pHasFishingRod])
		return SendClientMessage(playerid, COLOR_ERROR, "You do not own a fishing rod - buy one at a Bait Shop ("COL_GREEN"/baitshop"COL_WHITE").");
	SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_WHITE"You ready your fishing rod. Stand at the water's edge and use "COL_GREEN"/fish"COL_WHITE".");
	return 1;
}

//==============================================================================
//	/baitshop - buy rod / cooler / permits / seeds (§9.2/§9.3 Bait Shop)
//==============================================================================

CMD:baitshop(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(!Fishing_AtBaitShop(playerid))
		return SendClientMessage(playerid, COLOR_ERROR, "You must be at a Bait Shop to buy fishing gear (look for the map icons).");

	new list[720], row[96];
	// Row order MUST match the Fishing_OnDialogResponse switch below.
	format(row, sizeof(row), ""COL_WHITE"Fishing Rod\t"COL_GREEN"$%d\t%s\n", FISHING_ROD_PRICE, PlayerInfo[playerid][pHasFishingRod] ? (""COL_GREEN"owned") : (""COL_WHITE"faster + higher catch"));
	strcat(list, row, sizeof(list));
	format(row, sizeof(row), ""COL_WHITE"Fish Cooler\t"COL_GREEN"$%d\t%s\n", FISH_COOLER_PRICE, PlayerInfo[playerid][pHasFishCooler] ? (""COL_GREEN"owned") : (""COL_WHITE"holds 10 fish"));
	strcat(list, row, sizeof(list));
	format(row, sizeof(row), ""COL_WHITE"Fish Sales Permit\t"COL_GREEN"$%d\t%s\n", FISH_SALES_PERMIT_PRICE, PlayerInfo[playerid][pHasFishSalesPermit] ? (""COL_GREEN"owned") : (""COL_WHITE"sell to players"));
	strcat(list, row, sizeof(list));
	format(row, sizeof(row), ""COL_WHITE"Fishing Permits x5\t"COL_GREEN"$%d\t"COL_WHITE"have %d/%d\n", FISH_PERMIT_BUNDLE_5, PlayerInfo[playerid][pFishPermits], FISH_PERMIT_MAX);
	strcat(list, row, sizeof(list));
	format(row, sizeof(row), ""COL_WHITE"Fishing Permits x10\t"COL_GREEN"$%d\t"COL_WHITE"have %d/%d\n", FISH_PERMIT_BUNDLE_10, PlayerInfo[playerid][pFishPermits], FISH_PERMIT_MAX);
	strcat(list, row, sizeof(list));
	format(row, sizeof(row), ""COL_WHITE"Fishing Permits x25\t"COL_GREEN"$%d\t"COL_WHITE"have %d/%d\n", FISH_PERMIT_BUNDLE_25, PlayerInfo[playerid][pFishPermits], FISH_PERMIT_MAX);
	strcat(list, row, sizeof(list));
	format(row, sizeof(row), ""COL_WHITE"Hunting Permits x5\t"COL_GREEN"$%d\t"COL_WHITE"have %d/%d\n", HUNT_PERMIT_BUNDLE_5, PlayerInfo[playerid][pHuntPermits], HUNT_PERMIT_MAX);
	strcat(list, row, sizeof(list));
	format(row, sizeof(row), ""COL_WHITE"Hunting Permits x10\t"COL_GREEN"$%d\t"COL_WHITE"have %d/%d\n", HUNT_PERMIT_BUNDLE_10, PlayerInfo[playerid][pHuntPermits], HUNT_PERMIT_MAX);
	strcat(list, row, sizeof(list));
	format(row, sizeof(row), ""COL_WHITE"Drug Seeds x5\t"COL_GREEN"$%d\t"COL_WHITE"have %d/%d\n", DRUG_SEED_BUNDLE_5, PlayerInfo[playerid][pDrugSeeds], DRUG_SEED_MAX);
	strcat(list, row, sizeof(list));

	ShowPlayerDialog(playerid, BAIT_SHOP_DIALOG, DIALOG_STYLE_TABLIST, ""COL_WHITE"Bait Shop\tPrice\tInfo", list, "Buy", "Cancel");
	return 1;
}
alias:baitshop("bait")

// Bait Shop menu resolution (routed from OnDialogResponse in CnR.pwn).
FUNCTION Fishing_OnDialogResponse(playerid, dialogid, response, listitem)
{
	if(dialogid != BAIT_SHOP_DIALOG) return 0;
	if(!response) return 1;	// Cancel
	if(!Fishing_AtBaitShop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You have left the Bait Shop.");

	new price, msg[144];
	switch(listitem)
	{
		case 0:	// Fishing Rod
		{
			if(PlayerInfo[playerid][pHasFishingRod]) return SendClientMessage(playerid, COLOR_ERROR, "You already own a fishing rod.");
			price = StockMarket_BuyPrice(FISHING_ROD_PRICE, E_STOCK_SUPA_SAVE);	// M7 - 24/7 items × SUPA_SAVE (§10.2)
			if(!Bank_TakeCash(playerid, price)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot afford a fishing rod.");
			PlayerInfo[playerid][pHasFishingRod] = true;
			format(msg, sizeof(msg), ""COL_GREEN"* Bought a fishing rod "COL_WHITE"($%d) - faster reel-in + higher catch chance.", price);
		}
		case 1:	// Fish Cooler
		{
			if(PlayerInfo[playerid][pHasFishCooler]) return SendClientMessage(playerid, COLOR_ERROR, "You already own a fish cooler.");
			price = StockMarket_BuyPrice(FISH_COOLER_PRICE, E_STOCK_SUPA_SAVE);	// M7 - 24/7 items × SUPA_SAVE (§10.2)
			if(!Bank_TakeCash(playerid, price)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot afford a fish cooler.");
			PlayerInfo[playerid][pHasFishCooler] = true;
			format(msg, sizeof(msg), ""COL_GREEN"* Bought a fish cooler "COL_WHITE"($%d) - now holds %d fish.", price, FISH_COOLER_CAP);
		}
		case 2:	// Fish Sales Permit
		{
			if(PlayerInfo[playerid][pHasFishSalesPermit]) return SendClientMessage(playerid, COLOR_ERROR, "You already own a fish sales permit.");
			price = StockMarket_BuyPrice(FISH_SALES_PERMIT_PRICE, E_STOCK_SUPA_SAVE);	// M7 - 24/7 items × SUPA_SAVE (§10.2)
			if(!Bank_TakeCash(playerid, price)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot afford a fish sales permit.");
			PlayerInfo[playerid][pHasFishSalesPermit] = true;
			format(msg, sizeof(msg), ""COL_GREEN"* Bought a fish sales permit "COL_WHITE"($%d) - you can sell fish to players.", price);
		}
		case 3, 4, 5:	// Fishing permit bundles (5 / 10 / 25)
		{
			new add = (listitem == 3) ? 5 : (listitem == 4) ? 10 : 25;
			price = (listitem == 3) ? FISH_PERMIT_BUNDLE_5 : (listitem == 4) ? FISH_PERMIT_BUNDLE_10 : FISH_PERMIT_BUNDLE_25;
			price = StockMarket_BuyPrice(price, E_STOCK_SUPA_SAVE);	// M7 - 24/7 items × SUPA_SAVE (§10.2)
			if(PlayerInfo[playerid][pFishPermits] >= FISH_PERMIT_MAX) return SendClientMessage(playerid, COLOR_ERROR, "You already hold the maximum fishing permits (50).");
			if(!Bank_TakeCash(playerid, price)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot afford that permit bundle.");
			PlayerInfo[playerid][pFishPermits] += add;
			if(PlayerInfo[playerid][pFishPermits] > FISH_PERMIT_MAX) PlayerInfo[playerid][pFishPermits] = FISH_PERMIT_MAX;
			format(msg, sizeof(msg), ""COL_GREEN"* Bought %d fishing permits "COL_WHITE"($%d). You now hold %d/%d.", add, price, PlayerInfo[playerid][pFishPermits], FISH_PERMIT_MAX);
		}
		case 6, 7:	// Hunting permit bundles (5 / 10)
		{
			new add = (listitem == 6) ? 5 : 10;
			price = (listitem == 6) ? HUNT_PERMIT_BUNDLE_5 : HUNT_PERMIT_BUNDLE_10;
			price = StockMarket_BuyPrice(price, E_STOCK_SUPA_SAVE);	// M7 - 24/7 items × SUPA_SAVE (§10.2)
			if(PlayerInfo[playerid][pHuntPermits] >= HUNT_PERMIT_MAX) return SendClientMessage(playerid, COLOR_ERROR, "You already hold the maximum hunting permits (20).");
			if(!Bank_TakeCash(playerid, price)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot afford that permit bundle.");
			PlayerInfo[playerid][pHuntPermits] += add;
			if(PlayerInfo[playerid][pHuntPermits] > HUNT_PERMIT_MAX) PlayerInfo[playerid][pHuntPermits] = HUNT_PERMIT_MAX;
			format(msg, sizeof(msg), ""COL_GREEN"* Bought %d hunting permits "COL_WHITE"($%d). You now hold %d/%d.", add, price, PlayerInfo[playerid][pHuntPermits], HUNT_PERMIT_MAX);
		}
		case 8:	// Drug Seeds x5
		{
			price = StockMarket_BuyPrice(DRUG_SEED_BUNDLE_5, E_STOCK_SUPA_SAVE);	// M7 - 24/7 items × SUPA_SAVE (§10.2)
			if(PlayerInfo[playerid][pDrugSeeds] >= DRUG_SEED_MAX) return SendClientMessage(playerid, COLOR_ERROR, "You already hold the maximum drug seeds (10).");
			if(!Bank_TakeCash(playerid, price)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot afford that.");
			PlayerInfo[playerid][pDrugSeeds] += 5;
			if(PlayerInfo[playerid][pDrugSeeds] > DRUG_SEED_MAX) PlayerInfo[playerid][pDrugSeeds] = DRUG_SEED_MAX;
			format(msg, sizeof(msg), ""COL_GREEN"* Bought 5 drug seeds "COL_WHITE"($%d). You now hold %d/%d. Use "COL_GREEN"/plant"COL_WHITE".", price, PlayerInfo[playerid][pDrugSeeds], DRUG_SEED_MAX);
		}
		default: return 1;
	}
	// M7 - buying at the shop feeds the SUPA_SAVE (24/7) pool (demand pushes the
	// price up, §10.2). `price` here is the actual (market-scaled) amount charged.
	StockMarket_UpdateEarnings(E_STOCK_SUPA_SAVE, price, STOCK_FACTOR_SHOP);
	Jail_SavePlayer(playerid);	// persist the purchase
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}

//==============================================================================
//	Clock / lifecycle hooks
//==============================================================================

// Announce the daily bonus fish (GameModeClock 05:00 hook, §9.2). The first catch
// after this wins FISH_BONUS_REWARD (resolved in Fishing_ReelIn).
FUNCTION Fishing_OnBonusHour()
{
	static const g_BonusSpecies[][24] = { "Golden Trout", "Giant Sturgeon", "Rainbow Marlin", "Silver Salmon" };
	format(g_BonusFishSpecies, sizeof(g_BonusFishSpecies), "%s", g_BonusSpecies[random(sizeof(g_BonusSpecies))]);
	g_BonusFishActive = true;
	new msg[176];
	format(msg, sizeof(msg), ""COL_YELLOW"BONUS FISH: "COL_WHITE"a %s has been spotted! The first to "COL_GREEN"/fish"COL_WHITE" it up wins "COL_GREEN"$%d"COL_WHITE"!", g_BonusFishSpecies, FISH_BONUS_REWARD);
	SendClientMessageToAll(COLOR_SERVER_INFO, msg);
	return 1;
}

// Abort an in-progress catch on death (§9.2 - a fresh spawn has no line cast).
FUNCTION Fishing_OnPlayerDeath(playerid)
{
	if(PlayerInfo[playerid][pFishTimer] != -1) { KillTimer(PlayerInfo[playerid][pFishTimer]); PlayerInfo[playerid][pFishTimer] = -1; }
	PlayerInfo[playerid][pFishing] = false;
	return 1;
}

// Kill the reel-in timer on disconnect (the cooler aggregate is saved via Jail_SavePlayer).
FUNCTION Fishing_OnDisconnect(playerid)
{
	if(PlayerInfo[playerid][pFishTimer] != -1) { KillTimer(PlayerInfo[playerid][pFishTimer]); PlayerInfo[playerid][pFishTimer] = -1; }
	PlayerInfo[playerid][pFishing] = false;
	return 1;
}
