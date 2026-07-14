/*

	Wanted-level system for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 3 (stage 1) - design §2 (the spine of the whole gamemode).

	This is the core dependency the cop/arrest/jail stage (§3) builds on. It owns
	the wanted state, the name-colour + radar-blip contract, decay (paused during
	pursuit), spawn protection, and the crime events wired now (kills, car-jacks,
	drive-bys). Everything else calls the public API below.

	Credits:
		crime-cop-loop.md §2-3 for the mechanic grounding.

	Public API (FUNCTION publics - cross-module, stage 2 calls these):
		GivePlayerWanted(playerid, amount, const reason[])   : stack, clamp 0..10 (§2.2)
		SetPlayerWanted(playerid, level, const reason[])      : absolute set (§2.2)
		GetPlayerWanted(playerid)                             : current level (§2.2)
		GiveWanted(playerid, amount, const crimeName[])       : task alias of Give (announces crime)
		SetWanted(playerid, level, const reason[])            : task alias of Set
		ClearWanted(playerid, const reason[])                 : drop to 0 (arrest/jail/leave-city)
		GetWantedTier(playerid)                               : WANTED_TIER_* (§2.1)
		SetVisualContact(copid, suspectid)                    : pause decay (stage 2 /vc stub)
		GivePlayerSpawnProtection(playerid)                   : start the spawn-protection window
		IsPlayerSpawnProtected(playerid)                      : bool
		Wanted_ApplyColour(playerid)                          : re-apply name/blip colour
		Wanted_OnGameModeClockTick()                          : hook from GameModeClock() (1s)

	Functions Being Called:
		misc:     IsCop
		messages: IRC_SendMessage
*/

//==============================================================================
//	Internal helpers
//==============================================================================

// Tier for a raw wanted value (§2.1 thresholds).
static WantedTierOf(level)
{
	if(level >= WANTED_MOSTWANTED_MIN) return WANTED_TIER_MOSTWANTED;
	if(level >= WANTED_WARRANT_MIN)    return WANTED_TIER_WARRANT;
	if(level >= WANTED_SUSPECT_MIN)    return WANTED_TIER_SUSPECT;
	return WANTED_TIER_INNOCENT;
}

// Colour for a tier (name + radar blip). Cops always read as blue regardless of
// their own (zero) wanted level.
static Wanted_ColourFor(playerid)
{
	if(IsCop(playerid)) return COLOR_COP_BLIP;
	switch(WantedTierOf(PlayerInfo[playerid][pWanted]))
	{
		case WANTED_TIER_SUSPECT:    return COLOR_WANTED_YELLOW;
		case WANTED_TIER_WARRANT:    return COLOR_WANTED_ORANGE;
		case WANTED_TIER_MOSTWANTED: return COLOR_WANTED_DARKORANGE;
	}
	return COLOR_INNOCENT;
}

//==============================================================================
//	Colour / marker contract (design §2.2)
//==============================================================================

// Apply the name colour + own radar blip for one player, then mirror the blip
// to every other connected player so the whole server reads the same colour.
// Cops additionally see the suspect's wanted stars via SetPlayerWantedLevel
// (this is the scoreboard/HUD star mirror the spec asks for).
FUNCTION Wanted_ApplyColour(playerid)
{
	if(!IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) return 0;
	new colour = Wanted_ColourFor(playerid);
	SetPlayerColor(playerid, colour);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		SetPlayerMarkerForPlayer(i, playerid, colour);
	}
	// Mirror the wanted stars onto the client HUD/scoreboard (cops read these).
	SetPlayerWantedLevel(playerid, PlayerInfo[playerid][pWanted]);
	return 1;
}

//==============================================================================
//	Crime announcement (design §2.2 - player + nearby cops)
//==============================================================================

// Announce a committed crime to the offender and to every cop within radar
// range, and log it for the future Discord bridge via IRC_SendMessage (no-op).
static Wanted_AnnounceCrime(playerid, const reason[], amount)
{
	if(isNull(reason)) return 0;

	new string[160];
	format(string, sizeof(string), ""COL_RED"* Crime: %s "COL_WHITE"(+%d wanted - level %d/%d).", reason, amount, PlayerInfo[playerid][pWanted], MAX_WANTED);
	SendClientMessage(playerid, COLOR_ERROR, string);

	// Nearby cops get a heads-up with the suspect's name/id and location.
	new Float:x, Float:y, Float:z, zoneName[MAX_ZONE_NAME];
	GetPlayerPos(playerid, x, y, z);
	format(zoneName, sizeof(zoneName), "%s", PlayerInfo[playerid][pZoneName]);
	format(string, sizeof(string), ""COL_ORANGE"[CRIME] "COL_WHITE"%s (%d) - %s "COL_YELLOW"(WL %d) "COL_WHITE"near %s.", PlayerInfo[playerid][pUserName], playerid, reason, PlayerInfo[playerid][pWanted], isNull(zoneName) ? ("unknown") : (zoneName));
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || i == playerid) continue;
		if(!IsCop(i)) continue;
		if(IsPlayerInRangeOfPoint(i, 250.0, x, y, z)) SendClientMessage(i, COLOR_WANTED_ORANGE, string);
	}

	// Future Discord bridge (retired IRC no-op today).
	format(string, sizeof(string), "[CRIME] %s (%d): %s (+%d, WL %d).", PlayerInfo[playerid][pUserName], playerid, reason, amount, PlayerInfo[playerid][pWanted]);
	IRC_SendMessage(string, IRC_NORMAL_COLOR);
	return 1;
}

// Report a tier promotion to the player so escalation (suspect->warrant->most
// wanted) is legible.
static Wanted_ReportTier(playerid, oldTier)
{
	new newTier = WantedTierOf(PlayerInfo[playerid][pWanted]);
	if(newTier == oldTier) return 0;
	switch(newTier)
	{
		case WANTED_TIER_INNOCENT:   SendClientMessage(playerid, COLOR_SERVER_INFO, "You are no longer wanted. Your name is "COL_WHITE"white"COL_SERVER_INFO" again.");
		case WANTED_TIER_SUSPECT:    SendClientMessage(playerid, COLOR_WANTED_YELLOW, "You are now a "COL_YELLOW"Suspect"COL_WHITE" (cops can ticket you).");
		case WANTED_TIER_WARRANT:    SendClientMessage(playerid, COLOR_WANTED_ORANGE, "A "COL_ORANGE"Warrant"COL_WHITE" has been issued - cops can now arrest you on foot.");
		case WANTED_TIER_MOSTWANTED: SendClientMessage(playerid, COLOR_WANTED_DARKORANGE, ""COL_RED"MOST WANTED"COL_WHITE" - cops may arrest OR take you down with lethal force.");
	}
	return 1;
}

//==============================================================================
//	Public API (design §2.2)
//==============================================================================

FUNCTION GetPlayerWanted(playerid)
	return PlayerInfo[playerid][pWanted];

FUNCTION GetWantedTier(playerid)
	return WantedTierOf(PlayerInfo[playerid][pWanted]);

// Absolute set (escape=10, holdup=+7 land via Give). Clamps 0..MAX_WANTED.
FUNCTION SetPlayerWanted(playerid, level, const reason[])
{
	if(!IsPlayerConnected(playerid)) return 0;
	if(level < 0) level = 0;
	if(level > MAX_WANTED) level = MAX_WANTED;
	new oldTier = WantedTierOf(PlayerInfo[playerid][pWanted]);
	PlayerInfo[playerid][pWanted] = level;
	PlayerInfo[playerid][pWantedDecayCounter] = 0;
	Wanted_ApplyColour(playerid);
	Wanted_ReportTier(playerid, oldTier);
	if(!isNull(reason) && level > 0) Wanted_AnnounceCrime(playerid, reason, 0);
	return 1;
}

// Stacking add - the crime path every handler uses. Suppressed inside DM zones,
// safe (police) property and during spawn protection (§2.5).
FUNCTION GivePlayerWanted(playerid, amount, const reason[])
{
	if(!IsPlayerConnected(playerid)) return 0;
	if(amount <= 0) return 0;

	// No wanted gain inside DM zones / safe zones / spawn protection (§2.5). The
	// safe-zone set is police property (pInPoliceProperty) plus the general no-crime
	// zones (hospital / RPC) flagged by pInSafeZone (M6 stage 3, §9.10).
	if(PlayerInfo[playerid][pInDMZone]) return 0;
	if(PlayerInfo[playerid][pInPoliceProperty]) return 0;
	if(PlayerInfo[playerid][pInSafeZone]) return 0;
	if(IsPlayerSpawnProtected(playerid)) return 0;

	new oldTier = WantedTierOf(PlayerInfo[playerid][pWanted]);
	PlayerInfo[playerid][pWanted] += amount;
	if(PlayerInfo[playerid][pWanted] > MAX_WANTED) PlayerInfo[playerid][pWanted] = MAX_WANTED;
	PlayerInfo[playerid][pWantedDecayCounter] = 0;	// reset decay on fresh crime

	Wanted_ApplyColour(playerid);
	Wanted_AnnounceCrime(playerid, reason, amount);
	Wanted_ReportTier(playerid, oldTier);
	return 1;
}

// Drop to innocent (arrest/jail complete, left the city, church confession).
FUNCTION ClearPlayerWanted(playerid, const reason[])
{
	if(!IsPlayerConnected(playerid)) return 0;
	PlayerInfo[playerid][pWanted] = 0;
	PlayerInfo[playerid][pWantedDecayCounter] = 0;
	PlayerInfo[playerid][pVCByCop] = -1;
	PlayerInfo[playerid][pVCFreezeUntil] = 0;
	Wanted_ApplyColour(playerid);
	if(!isNull(reason))
	{
		new string[96];
		format(string, sizeof(string), "Your wanted level was cleared (%s).", reason);
		SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	}
	return 1;
}

// ---- Task-named aliases (prompt API) --------------------------------------
// The prompt asks for GiveWanted/SetWanted/ClearWanted/GetWantedTier; these
// forward to the canonical spec functions so both naming sets are callable.
FUNCTION GiveWanted(playerid, amount, const crimeName[])
	return GivePlayerWanted(playerid, amount, crimeName);

FUNCTION SetWanted(playerid, level, const reason[])
	return SetPlayerWanted(playerid, level, reason);

FUNCTION ClearWanted(playerid, const reason[])
	return ClearPlayerWanted(playerid, reason);

//==============================================================================
//	Visual contact (decay pause) - stage 2's /vc drives this
//==============================================================================

// A cop reports visual contact on a suspect: freeze the suspect's decay for
// VC_FREEZE_SECS. Stage 2's /vc command is a thin wrapper over this public.
FUNCTION SetVisualContact(copid, suspectid)
{
	if(!IsPlayerConnected(copid) || !IsPlayerConnected(suspectid)) return 0;
	if(!IsCop(copid)) return 0;
	if(PlayerInfo[suspectid][pWanted] < WANTED_SUSPECT_MIN) return 0;

	PlayerInfo[suspectid][pVCByCop] = copid;
	PlayerInfo[suspectid][pVCFreezeUntil] = gettime() + VC_FREEZE_SECS;
	PlayerInfo[suspectid][pWantedDecayCounter] = 0;
	return 1;
}

//==============================================================================
//	Spawn protection (design §2.5)
//==============================================================================

FUNCTION GivePlayerSpawnProtection(playerid)
{
	PlayerInfo[playerid][pSpawnProtectUntil] = gettime() + SPAWN_PROTECT_SECS;
	return 1;
}

FUNCTION IsPlayerSpawnProtected(playerid)
	return (PlayerInfo[playerid][pSpawnProtectUntil] != 0 && gettime() < PlayerInfo[playerid][pSpawnProtectUntil]);

//==============================================================================
//	Decay tick - hooked from GameModeClock() (1-second timer, design §2.4)
//==============================================================================

// Runs once per real second (no new global timer). Decays each wanted player by
// one level every WANTED_DECAY_SECS, unless a cop holds visual contact.
FUNCTION Wanted_OnGameModeClockTick()
{
	for(new playerid = 0; playerid < MAX_PLAYERS; playerid++)
	{
		if(!IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) continue;
		if(!PlayerInfo[playerid][pLoggedIn]) continue;
		if(PlayerInfo[playerid][pWanted] < WANTED_SUSPECT_MIN) continue;

		// Decay paused while a cop holds visual contact (§2.4).
		if(PlayerInfo[playerid][pVCFreezeUntil] != 0)
		{
			if(gettime() < PlayerInfo[playerid][pVCFreezeUntil]) continue;
			// The hold expired - clear it and resume decay.
			PlayerInfo[playerid][pVCFreezeUntil] = 0;
			PlayerInfo[playerid][pVCByCop] = -1;
		}

		if(++PlayerInfo[playerid][pWantedDecayCounter] < WANTED_DECAY_SECS) continue;

		PlayerInfo[playerid][pWantedDecayCounter] = 0;
		new oldTier = WantedTierOf(PlayerInfo[playerid][pWanted]);
		PlayerInfo[playerid][pWanted]--;
		Wanted_ApplyColour(playerid);
		Wanted_ReportTier(playerid, oldTier);
	}
	return 1;
}

//==============================================================================
//	Persistence hooks (design §2.1 - LSplayers.Wanted)
//==============================================================================

// Apply the loaded wanted level on spawn (colour + blip + HUD stars).
FUNCTION Wanted_OnPlayerSpawn(playerid)
{
	GivePlayerSpawnProtection(playerid);
	Wanted_ApplyColour(playerid);
	return 1;
}

// Persist the current wanted level (called alongside the disconnect save).
FUNCTION Wanted_SavePlayer(playerid)
{
	if(!PlayerInfo[playerid][pLoggedIn]) return 0;
	new string[128];
	mysql_format(g_SQL, string, sizeof(string), "UPDATE "TABLE_PLAYERS_C" SET Wanted=%d WHERE aID=%d LIMIT 1", PlayerInfo[playerid][pWanted], PlayerInfo[playerid][pID]);
	mysql_pquery(g_SQL, string);
	return 1;
}

//==============================================================================
//	Crime events wired now (design §2.3)
//==============================================================================

// Minimal death feed - no death message system existed, so this adds one.
// Fire reasons (37 Flamethrower / 42 Molotov / 44 WEAPON_FIRE / 43 heat-seek
// edge) and other invalid reason ids map to a sane "grilled"/generic verb from
// the roster instead of leaking a raw id (design §7.11 fire "grilled" fix).
static Wanted_DeathFeed(playerid, killerid, reason)
{
	new string[144], weaponName[32];

	// Resolve a readable weapon/cause name.
	switch(reason)
	{
		case 37, 42, 44: format(weaponName, sizeof(weaponName), "fire");	// grilled
		case 51: format(weaponName, sizeof(weaponName), "an explosion");
		case 53: format(weaponName, sizeof(weaponName), "drowning");
		case 54: format(weaponName, sizeof(weaponName), "a collision");
		default:
		{
			if(reason < 0 || reason > 54) format(weaponName, sizeof(weaponName), "unknown causes");
			else
			{
				GetWeaponName(WEAPON:reason, weaponName, sizeof(weaponName));
				if(isNull(weaponName)) format(weaponName, sizeof(weaponName), "unknown causes");
			}
		}
	}

	if(killerid != INVALID_PLAYER_ID && IsPlayerConnected(killerid) && killerid != playerid)
	{
		// "grilled" verb for fire deaths (roster), otherwise "killed ... with".
		if(reason == 37 || reason == 42 || reason == 44)
			format(string, sizeof(string), ""COL_RED"* %s (%d) grilled %s (%d).", PlayerInfo[killerid][pUserName], killerid, PlayerInfo[playerid][pUserName], playerid);
		else
			format(string, sizeof(string), ""COL_RED"* %s (%d) killed %s (%d) with %s.", PlayerInfo[killerid][pUserName], killerid, PlayerInfo[playerid][pUserName], playerid, weaponName);
	}
	else
	{
		if(reason == 37 || reason == 42 || reason == 44)
			format(string, sizeof(string), ""COL_RED"* %s (%d) was grilled.", PlayerInfo[playerid][pUserName], playerid);
		else
			format(string, sizeof(string), ""COL_RED"* %s (%d) died from %s.", PlayerInfo[playerid][pUserName], playerid, weaponName);
	}
	SendClientMessageToAll(COLOR_ERROR, string);
	return 1;
}

// Death handler - killer gains wanted scaled by the victim's team (killing a
// cop is worth more), and we build a sane death-feed line (folds in the fire
// "grilled" fix). Called from OnPlayerDeath in CnR.pwn.
FUNCTION Wanted_OnPlayerDeath(playerid, killerid, reason)
{
	// Kill-driven wanted: a valid, different, human killer who is not a cop.
	if(killerid != INVALID_PLAYER_ID && killerid != playerid && IsPlayerConnected(killerid) && !IsPlayerNPC(killerid))
	{
		// Cops taking down a Most Wanted is legitimate (no wanted for the cop);
		// otherwise the killer commits a crime scaled by the victim's team.
		new bool:legalTakedown = (IsCop(killerid) && PlayerInfo[playerid][pWanted] >= WANTED_MOSTWANTED_MIN);
		if(!legalTakedown && !PlayerInfo[killerid][pInDMZone] && !IsPlayerSpawnProtected(killerid))
		{
			new add, reasonStr[48];
			if(IsCop(playerid)) { add = WANTED_KILL_COP; format(reasonStr, sizeof(reasonStr), "Killed a law-enforcement officer"); }
			else if(PlayerInfo[playerid][pWanted] >= WANTED_SUSPECT_MIN) { add = WANTED_KILL_SUSPECT; format(reasonStr, sizeof(reasonStr), "Killed a wanted suspect"); }
			else { add = WANTED_KILL_CIVILIAN; format(reasonStr, sizeof(reasonStr), "Murder of an innocent"); }
			GivePlayerWanted(killerid, add, reasonStr);
		}
	}

	// Death feed (killer, victim, weapon) with the fire "grilled" fix.
	Wanted_DeathFeed(playerid, killerid, reason);
	return 1;
}

// Vehicle-jack detection - a civilian taking the driver seat of a vehicle that
// another (different) player was driving/occupying is a car-jack (§2.3). Called
// from OnPlayerStateChange in CnR.pwn (driver-seat theft of an occupied
// vehicle). Drive-by is handled in OnPlayerWeaponShot -> Wanted_OnDriveBy.
FUNCTION Wanted_OnPlayerStateChange(playerid, newstate, oldstate)
{
	if(newstate != PLAYER_STATE_DRIVER) return 0;
	if(IsCop(playerid)) return 0;					// cops commandeering is not a crime

	new vehicleid = GetPlayerVehicleID(playerid);
	if(vehicleid == 0) return 0;

	// Was another human player occupying this vehicle a moment ago?
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(i == playerid || !IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(GetPlayerVehicleID(i) != vehicleid) continue;
		// Someone else is still in the car we just took the wheel of -> jack.
		GivePlayerWanted(playerid, WANTED_CARJACK, "Vehicle jacking");
		break;
	}
	#pragma unused oldstate
	return 1;
}

// Drive-by shooting - folds the drive-by into attack+wanted (design §2.3, open
// Q9 resolved: distinct crime, small wanted). Called from OnPlayerWeaponShot
// when the shooter is a passenger/driver firing at a valid target.
FUNCTION Wanted_OnPlayerWeaponShot(playerid, hittype, hitid)
{
	if(!IsPlayerInAnyVehicle(playerid)) return 0;
	if(IsCop(playerid)) return 0;
	if(hittype != _:BULLET_HIT_TYPE_PLAYER) return 0;
	if(hitid == INVALID_PLAYER_ID || hitid == playerid) return 0;
	if(!IsPlayerConnected(hitid)) return 0;
	GivePlayerWanted(playerid, WANTED_DRIVEBY, "Drive-by shooting");
	return 1;
}
