/*

	DM / sniper / duel arenas for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 6 (stage 3) - design §9.6 (DM systems: DMS/sniper/duel + anti-parachute)
	+ §2.5 (safe/no-crime zones, §9.10) + docs/research/jobs-progression.md §6
	(events & arenas) + docs/research/crazybobs-site-full.md §DM Stadium.

	All three arenas share the DM-zone contract: while inside, the player is flagged
	pInDMZone (wanted gain suppressed, cops cannot arrest - reuses the M2 teleport
	DM-zone infra), the parachute (weapon 46) is stripped, PvP is open, and dying
	respawns them INSIDE the arena (CnR.pwn OnPlayerDeath routes here) until /leave.
	Entering saves the player's prior position / interior / VW / health / armour /
	weapons; /leave (or a duel end) restores that snapshot cleanly so nobody loses
	their loadout. Entry is blocked while jailed / on a mission / kidnapped.

	- DM Stadium (DMS) `/dms` (alias /enter): $5,000 entry, a fixed free-for-all
	  weapon kit + 2500 ammo per weapon per life (crazybobs dm-stadium page); each
	  death costs $1,000; no life / life-insurance loss. /leave (alias /exit) restores.
	- Sniper arena `/sniper`: sniper-rifle-only kit, same entry economics + DM-zone.
	- Duel `/duel <id> [stake]`: challenge a player; they /accept or /deny. Both ante
	  the stake via Bank_TakeCash (escrow - NO mint), teleport to a private duel VW,
	  3-2-1 countdown, first-blood decides it; the WINNER is credited the pot (sum of
	  both antes) via Bank_GiveCash. Quitting / leaving / dying mid-duel = forfeit,
	  the pot goes to the opponent. /duel cancel withdraws a pending challenge.

	Persistence: arenas are session-only (no DB). The pre-arena snapshot lives in
	pInfo and is dropped/restored on /leave, duel-end, death-out, or disconnect.

	Public API (called by CnR.pwn):
		Arena_OnPlayerDeath(playerid, killerid)  : arena respawn + death fee + duel result
		Arena_OnDisconnect(playerid)             : forfeit a live duel, drop a pending offer
		Arena_OnDialogResponse(...)              : (none - arenas are command-driven)
		Arena_IsInArena(playerid)                : bool gate other systems can read
		Arena_UpdateSafeZone(playerid)           : refresh pInSafeZone from the safe-zone table
		Arena_IsSafeZone(Float:x,Float:y,Float:z): bool - is a point inside a no-crime zone

	Functions Being Called:
		bank:     Bank_GiveCash, Bank_TakeCash, Bank_GetCash
		wanted:   ClearPlayerWanted, GivePlayerSpawnProtection
		jail:     Jail_IsJailed
		missions: Mission_IsOnMission
		teleport: StripParachute (anti-parachute weapon-46 strip)
		misc:     IsCop
*/

//==============================================================================
//	Fixed weapon kits (design §9.6 / jobs §6.1) - { weaponid, ammo } pairs
//==============================================================================

// DM Stadium free-for-all kit (Score Guide "shredding people with your sawnoffs"
// -> sawn-off + a spread). Raw weapon ids (matches cop /refill style). Ammo is the
// per-life ARENA_LIFE_AMMO (2500) count.
static const g_DMSKit[][2] =
{
	{ 26, ARENA_LIFE_AMMO },	// sawn-off shotgun (the signature)
	{ 29, ARENA_LIFE_AMMO },	// MP5 (SMG)
	{ 31, ARENA_LIFE_AMMO },	// M4 (assault rifle)
	{ 24, ARENA_LIFE_AMMO },	// desert eagle
	{ 16, ARENA_LIFE_AMMO }		// grenades
};

// Sniper-only kit (§6.5) - sniper rifle + a sidearm so a missed shot is not a
// total dead-end.
static const g_SniperKit[][2] =
{
	{ 34, ARENA_LIFE_AMMO },	// sniper rifle (the whole point)
	{ 24, ARENA_LIFE_AMMO }		// desert eagle sidearm
};

// Standard duel kit (§6.4) - a fair balanced spread both sides get.
static const g_DuelKit[][2] =
{
	{ 24, ARENA_LIFE_AMMO },	// desert eagle
	{ 26, ARENA_LIFE_AMMO },	// sawn-off shotgun
	{ 31, ARENA_LIFE_AMMO }		// M4
};

//==============================================================================
//	Safe / no-crime zones (design §2.5 / §9.10) - hospital & RPC bar locations
//==============================================================================
// Extends the police-property no-crime rule to a general safe-zone set. A point
// within SAFE_ZONE_RADIUS of one of these is a no-crime zone (wanted suppressed).
// LS coords (the active city); the others are kept for later cities.
static const Float:g_SafeZones[][3] =
{
	{ 1172.2,  -1323.6, 15.4 },	// LS All Saints General Hospital (§9.10 hospital)
	{ 2034.0,  -1403.0, 17.2 },	// LS East LS hospital
	{ 246.0,   -40.0,   1.6 }	// RPC exterior (Rodeo, LS - no-crime, §RPC)
};

//==============================================================================
//	Forward declarations (statics used before their definition below)
//==============================================================================

static Duel_PlaceSide(self, opp, stake, owner, vw, side);
static Duel_ClearState(playerid);

//==============================================================================
//	Internal helpers
//==============================================================================

FUNCTION Arena_IsInArena(playerid)
	return (PlayerInfo[playerid][pArena] != ARENA_NONE);

// Is a world point inside a no-crime safe zone (hospital / RPC)?
FUNCTION Arena_IsSafeZone(Float:x, Float:y, Float:z)
{
	for(new i = 0; i < sizeof(g_SafeZones); i++)
	{
		if(VectorSize(x - g_SafeZones[i][0], y - g_SafeZones[i][1], z - g_SafeZones[i][2]) <= SAFE_ZONE_RADIUS)
			return 1;
	}
	return 0;
}

// Refresh pInSafeZone from the player's current position - called on zone
// enter/leave (server/zones.inc). Cheap: a handful of distance checks.
FUNCTION Arena_UpdateSafeZone(playerid)
{
	if(!IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) return 0;
	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	PlayerInfo[playerid][pInSafeZone] = (Arena_IsSafeZone(x, y, z) != 0);
	return 1;
}

// Gate shared by every arena entry: must be spawned, free, and not busy with a
// jail / mission / kidnap / other arena. Returns 1 if OK, else messages + 0.
static Arena_CanEnter(playerid)
{
	if(!PlayerInfo[playerid][pSpawned])
		return SendClientMessage(playerid, COLOR_ERROR, "You must be spawned to enter an arena."), 0;
	if(Arena_IsInArena(playerid))
		return SendClientMessage(playerid, COLOR_ERROR, "You are already inside an arena. Use "COL_WHITE"/leave"COL_RED" first."), 0;
	if(PlayerInfo[playerid][pAdminJailed] || Jail_IsJailed(playerid))
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot enter an arena while jailed."), 0;
	if(Mission_IsOnMission(playerid))
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot enter an arena while on a mission."), 0;
	if(PlayerInfo[playerid][pKidnappedBy] != -1 || PlayerInfo[playerid][pKidnapVictim] != -1)
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot enter an arena while a kidnapping is in progress."), 0;
	if(PlayerInfo[playerid][pOnJury])
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot enter an arena while sitting on a jury."), 0;
	return 1;
}

// Snapshot the player's world state + full loadout before an arena teleport, so
// /leave / duel-end can restore it cleanly (design §6.4 "restore ... on finish").
static Arena_SaveState(playerid)
{
	if(PlayerInfo[playerid][pArenaSaved]) return 0;	// never overwrite a live snapshot
	GetPlayerPos(playerid, PlayerInfo[playerid][pArenaBackX], PlayerInfo[playerid][pArenaBackY], PlayerInfo[playerid][pArenaBackZ]);
	GetPlayerFacingAngle(playerid, PlayerInfo[playerid][pArenaBackA]);
	PlayerInfo[playerid][pArenaBackInt] = GetPlayerInterior(playerid);
	PlayerInfo[playerid][pArenaBackVW] = GetPlayerVirtualWorld(playerid);
	GetPlayerHealth(playerid, PlayerInfo[playerid][pArenaBackHP]);
	GetPlayerArmour(playerid, PlayerInfo[playerid][pArenaBackArmour]);
	for(new slot = 0; slot < 13; slot++)
		GetPlayerWeaponData(playerid, slot, PlayerInfo[playerid][pArenaBackWeap][slot], PlayerInfo[playerid][pArenaBackAmmo][slot]);
	PlayerInfo[playerid][pArenaSaved] = true;
	return 1;
}

// Apply the held snapshot NOW (position, interior/VW, health/armour, weapons) and
// consume it. Used both by the alive /leave path and the spawn hook (dead path).
static Arena_ApplySnapshot(playerid)
{
	if(!PlayerInfo[playerid][pArenaSaved]) return 0;
	SetPlayerInterior(playerid, PlayerInfo[playerid][pArenaBackInt]);
	SetPlayerVirtualWorld(playerid, PlayerInfo[playerid][pArenaBackVW]);
	SetPlayerPos(playerid, PlayerInfo[playerid][pArenaBackX], PlayerInfo[playerid][pArenaBackY], PlayerInfo[playerid][pArenaBackZ]);
	SetPlayerFacingAngle(playerid, PlayerInfo[playerid][pArenaBackA]);
	SetCameraBehindPlayer(playerid);

	ResetPlayerWeapons(playerid);
	for(new slot = 0; slot < 13; slot++)
	{
		if(PlayerInfo[playerid][pArenaBackWeap][slot] == 0 || PlayerInfo[playerid][pArenaBackAmmo][slot] == 0) continue;
		GivePlayerWeapon(playerid, PlayerInfo[playerid][pArenaBackWeap][slot], PlayerInfo[playerid][pArenaBackAmmo][slot]);
	}
	SetPlayerHealth(playerid, PlayerInfo[playerid][pArenaBackHP] > 0.0 ? PlayerInfo[playerid][pArenaBackHP] : 100.0);
	SetPlayerArmour(playerid, PlayerInfo[playerid][pArenaBackArmour]);

	PlayerInfo[playerid][pArenaSaved] = false;
	return 1;
}

// Exit an arena and restore the pre-arena state. Clears the arena/DM-zone flags.
// If the player is ALIVE (a clean /leave or duel-win) we restore in place. If they
// are DEAD (died out / duel loss), we set the spawn point to the saved position and
// let Arena_OnPlayerSpawn re-apply the snapshot on the (deferred) respawn - a dead
// player cannot be SetPlayerPos'd reliably.
static Arena_ExitRestore(playerid)
{
	PlayerInfo[playerid][pArena] = ARENA_NONE;
	PlayerInfo[playerid][pInDMZone] = false;
	if(!PlayerInfo[playerid][pArenaSaved]) return 0;

	if(GetPlayerState(playerid) == PLAYER_STATE_WASTED)
	{
		// Deferred: point the respawn at the saved spot; the spawn hook restores.
		SetSpawnInfo(playerid, NO_TEAM, GetPlayerSkin(playerid), PlayerInfo[playerid][pArenaBackX], PlayerInfo[playerid][pArenaBackY], PlayerInfo[playerid][pArenaBackZ], PlayerInfo[playerid][pArenaBackA], 0, 0, 0, 0, 0, 0);
		return 1;
	}
	Arena_ApplySnapshot(playerid);
	return 1;
}

// Resolve the DMS/sniper arena spawn point (with a small random spread so
// free-for-all players don't stack on one point). Also returns the isolated VW.
static Arena_SpawnPoint(kind, &Float:sx, &Float:sy, &Float:sz, &Float:sa, &vw)
{
	switch(kind)
	{
		case ARENA_SNIPER: { sx = SNIPER_X; sy = SNIPER_Y; sz = SNIPER_Z; sa = SNIPER_A; vw = ARENA_VW_SNIPER; }
		default:           { sx = DMS_X;    sy = DMS_Y;    sz = DMS_Z;    sa = DMS_A;    vw = ARENA_VW_DMS; }	// ARENA_DMS
	}
	sx += float(random(20)) - 10.0;
	sy += float(random(20)) - 10.0;
	return 1;
}

// Give the fixed kit for `kind` at full per-life ammo, full HP/armour, strip the
// parachute (anti-parachute) - assumes the player is already positioned in the VW.
static Arena_GiveKit(playerid, kind)
{
	ResetPlayerWeapons(playerid);
	if(kind == ARENA_SNIPER)
	{
		for(new i = 0; i < sizeof(g_SniperKit); i++)
			GivePlayerWeapon(playerid, g_SniperKit[i][0], g_SniperKit[i][1]);
	}
	else
	{
		for(new i = 0; i < sizeof(g_DMSKit); i++)
			GivePlayerWeapon(playerid, g_DMSKit[i][0], g_DMSKit[i][1]);
	}
	SetPlayerHealth(playerid, 100.0);
	SetPlayerArmour(playerid, 100.0);
	StripParachute(playerid);	// anti-parachute (weapon 46 removed)
	return 1;
}

// Teleport + kit for the FIRST entry (player is alive). Death-respawn goes through
// SetSpawnInfo + Arena_OnPlayerSpawn instead (a dead player cannot be SetPlayerPos'd).
static Arena_PlaceNow(playerid, kind)
{
	new Float:sx, Float:sy, Float:sz, Float:sa, vw;
	Arena_SpawnPoint(kind, sx, sy, sz, sa, vw);
	SetPlayerInterior(playerid, 0);
	SetPlayerVirtualWorld(playerid, vw);
	SetPlayerPos(playerid, sx, sy, sz);
	SetPlayerFacingAngle(playerid, sa);
	SetCameraBehindPlayer(playerid);
	// SetSpawnInfo so a subsequent death respawns them back at the arena (not the
	// city); Arena_OnPlayerSpawn re-kits them on that respawn.
	SetSpawnInfo(playerid, NO_TEAM, GetPlayerSkin(playerid), sx, sy, sz, sa, 0, 0, 0, 0, 0, 0);
	Arena_GiveKit(playerid, kind);
	return 1;
}

//==============================================================================
//	DM Stadium (DMS) - /dms (alias /enter) + /leave (alias /exit) (§6.1)
//==============================================================================

CMD:dms(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(!Arena_CanEnter(playerid)) return 1;
	if(Bank_GetCash(playerid) < ARENA_ENTRY_FEE)
	{
		new err[96];
		format(err, sizeof(err), "The DM Stadium entry fee is "COL_GREEN"$%d"COL_WHITE" - you cannot afford it.", ARENA_ENTRY_FEE);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	Bank_TakeCash(playerid, ARENA_ENTRY_FEE);

	Arena_SaveState(playerid);
	PlayerInfo[playerid][pArena] = ARENA_DMS;
	PlayerInfo[playerid][pInDMZone] = true;			// suppresses wanted/arrest (§2.5)
	ClearPlayerWanted(playerid, "");				// clean slate inside legal DM
	Arena_PlaceNow(playerid, ARENA_DMS);

	new string[160];
	format(string, sizeof(string), ""COL_GREEN"* DM STADIUM: "COL_WHITE"free-for-all - %d ammo/weapon per life. "COL_RED"Each death costs $%d."COL_WHITE" Type "COL_GREEN"/leave"COL_WHITE" to exit (your lives are safe).", ARENA_LIFE_AMMO, ARENA_DEATH_COST);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	return 1;
}

CMD:sniper(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(!Arena_CanEnter(playerid)) return 1;
	if(Bank_GetCash(playerid) < ARENA_ENTRY_FEE)
	{
		new err[96];
		format(err, sizeof(err), "The sniper arena entry fee is "COL_GREEN"$%d"COL_WHITE" - you cannot afford it.", ARENA_ENTRY_FEE);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	Bank_TakeCash(playerid, ARENA_ENTRY_FEE);

	Arena_SaveState(playerid);
	PlayerInfo[playerid][pArena] = ARENA_SNIPER;
	PlayerInfo[playerid][pInDMZone] = true;
	ClearPlayerWanted(playerid, "");
	Arena_PlaceNow(playerid, ARENA_SNIPER);

	SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_GREEN"* SNIPER ARENA: "COL_WHITE"sniper-only. Take your shots - type "COL_GREEN"/leave"COL_WHITE" to exit.");
	return 1;
}

// /leave (alias /exit): leave any arena. For a live duel this is a forfeit.
CMD:leave(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(!Arena_IsInArena(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You are not inside an arena.");

	// Leaving a live duel is a forfeit - the pot goes to the opponent.
	if(PlayerInfo[playerid][pArena] == ARENA_DUEL && PlayerInfo[playerid][pInDuel])
	{
		Duel_Finish(PlayerInfo[playerid][pDuelOpponent], playerid, true);	// opponent wins by forfeit
		return 1;
	}

	Arena_ExitRestore(playerid);
	GivePlayerSpawnProtection(playerid);	// a few seconds' safety back in the city
	SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_GREEN"* You left the arena."COL_WHITE" Your position, health and weapons were restored.");
	return 1;
}

//==============================================================================
//	Duel - /duel <id> [stake] | /duel cancel + /accept + /deny (§6.4)
//==============================================================================

// Escrow both antes (already validated affordable) into a private duel VW and
// place both duellists at facing spawns with the standard kit. `owner` owns the VW.
static Duel_Begin(a, b, stake, owner)
{
	new vw = ARENA_VW_DUEL_BASE + owner;

	// Both ante via Bank_TakeCash - NO mint; the pot is exactly the sum of antes.
	Bank_TakeCash(a, stake);
	Bank_TakeCash(b, stake);

	Duel_PlaceSide(a, b, stake, owner, vw, 0);
	Duel_PlaceSide(b, a, stake, owner, vw, 1);

	new string[144];
	format(string, sizeof(string), ""COL_GREEN"* DUEL: "COL_WHITE"%s"COL_GREEN" vs "COL_WHITE"%s"COL_GREEN" for "COL_WHITE"$%d"COL_GREEN" - first blood wins. Get ready...", PlayerInfo[a][pUserName], PlayerInfo[b][pUserName], stake * 2);
	SendClientMessage(a, COLOR_SERVER_INFO, string);
	SendClientMessage(b, COLOR_SERVER_INFO, string);

	// 3-2-1 freeze; the timer unfreezes both when it fires.
	TogglePlayerControllable(a, false);
	TogglePlayerControllable(b, false);
	SetTimerEx("Duel_GoLive", DUEL_COUNTDOWN_SECS * 1000, false, "dd", a, b);
	return 1;
}

// Place one side of the duel: save state, flag the duel, teleport + kit.
static Duel_PlaceSide(self, opp, stake, owner, vw, side)
{
	Arena_SaveState(self);
	PlayerInfo[self][pArena] = ARENA_DUEL;
	PlayerInfo[self][pInDMZone] = true;			// suppresses wanted/arrest
	PlayerInfo[self][pInDuel] = true;
	PlayerInfo[self][pDuelOpponent] = opp;
	PlayerInfo[self][pDuelChallenger] = owner;
	PlayerInfo[self][pDuelStake] = stake;
	// clear any lingering offer state
	PlayerInfo[self][pDuelOfferTo] = PlayerInfo[self][pDuelOfferFrom] = -1;
	PlayerInfo[self][pDuelOfferStake] = PlayerInfo[self][pDuelOfferExpiry] = 0;
	ClearPlayerWanted(self, "");

	SetPlayerInterior(self, 0);
	SetPlayerVirtualWorld(self, vw);
	if(side == 0) { SetPlayerPos(self, DUEL_X1, DUEL_Y1, DUEL_Z1); SetPlayerFacingAngle(self, DUEL_A1); }
	else          { SetPlayerPos(self, DUEL_X2, DUEL_Y2, DUEL_Z2); SetPlayerFacingAngle(self, DUEL_A2); }
	SetCameraBehindPlayer(self);
	ResetPlayerWeapons(self);
	for(new i = 0; i < sizeof(g_DuelKit); i++)
		GivePlayerWeapon(self, g_DuelKit[i][0], g_DuelKit[i][1]);
	SetPlayerHealth(self, 100.0);
	SetPlayerArmour(self, 100.0);
	StripParachute(self);
	return 1;
}

// Countdown fired - go live (unfreeze both if still in the duel).
FUNCTION Duel_GoLive(a, b)
{
	if(IsPlayerConnected(a) && PlayerInfo[a][pInDuel]) { TogglePlayerControllable(a, true); SendClientMessage(a, COLOR_SERVER_INFO, ""COL_RED"* FIGHT!"); }
	if(IsPlayerConnected(b) && PlayerInfo[b][pInDuel]) { TogglePlayerControllable(b, true); SendClientMessage(b, COLOR_SERVER_INFO, ""COL_RED"* FIGHT!"); }
	return 1;
}

// End a duel: `winner` takes the pot (sum of both antes) via Bank_GiveCash (NO
// mint). Restores both duellists' pre-duel state. `forfeit` differentiates the
// announce text (leaver/disconnect vs a clean kill).
FUNCTION Duel_Finish(winner, loser, forfeit)
{
	// Pot = both antes (no house cut, no mint - money simply moves loser->winner).
	new stake = 0;
	if(IsPlayerConnected(winner) && PlayerInfo[winner][pInDuel]) stake = PlayerInfo[winner][pDuelStake];
	else if(IsPlayerConnected(loser) && PlayerInfo[loser][pInDuel]) stake = PlayerInfo[loser][pDuelStake];
	new pot = stake * 2;

	new wname[MAX_PLAYER_NAME], lname[MAX_PLAYER_NAME];
	if(IsPlayerConnected(winner)) format(wname, sizeof(wname), "%s", PlayerInfo[winner][pUserName]); else format(wname, sizeof(wname), "a player");
	if(IsPlayerConnected(loser))  format(lname, sizeof(lname), "%s", PlayerInfo[loser][pUserName]);  else format(lname, sizeof(lname), "a player");

	// Tear down the loser first (clears their duel flags before restore).
	if(IsPlayerConnected(loser))
	{
		PlayerInfo[loser][pInDuel] = false;
		TogglePlayerControllable(loser, true);   // recover from a pre-live freeze
		Duel_ClearState(loser);
		Arena_ExitRestore(loser);
		GivePlayerSpawnProtection(loser);
	}
	// Credit the winner the pot, then restore + tear down.
	if(IsPlayerConnected(winner))
	{
		PlayerInfo[winner][pInDuel] = false;
		TogglePlayerControllable(winner, true);  // recover from a pre-live freeze
		if(pot > 0) Bank_GiveCash(winner, pot);
		Duel_ClearState(winner);
		Arena_ExitRestore(winner);
		GivePlayerSpawnProtection(winner);
	}

	new string[176];
	if(forfeit) format(string, sizeof(string), ""COL_GREEN"* DUEL: "COL_WHITE"%s"COL_GREEN" won the $%d pot - "COL_WHITE"%s"COL_GREEN" forfeited.", wname, pot, lname);
	else        format(string, sizeof(string), ""COL_GREEN"* DUEL: "COL_WHITE"%s"COL_GREEN" defeated "COL_WHITE"%s"COL_GREEN" and won the "COL_WHITE"$%d"COL_GREEN" pot!", wname, lname, pot);
	if(IsPlayerConnected(winner)) SendClientMessage(winner, COLOR_SERVER_INFO, string);
	if(IsPlayerConnected(loser))  SendClientMessage(loser, COLOR_SERVER_INFO, string);
	return 1;
}

// Clear a player's duel bookkeeping (does not touch arena snapshot / restore).
static Duel_ClearState(playerid)
{
	PlayerInfo[playerid][pDuelOpponent] = PlayerInfo[playerid][pDuelChallenger] = -1;
	PlayerInfo[playerid][pDuelStake] = 0;
	PlayerInfo[playerid][pDuelOfferTo] = PlayerInfo[playerid][pDuelOfferFrom] = -1;
	PlayerInfo[playerid][pDuelOfferStake] = PlayerInfo[playerid][pDuelOfferExpiry] = 0;
	return 1;
}

CMD:duel(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");

	// /duel cancel - withdraw a pending challenge.
	if(!strcmp(params, "cancel", true))
	{
		new t = PlayerInfo[playerid][pDuelOfferTo];
		if(t == -1) return SendClientMessage(playerid, COLOR_ERROR, "You have no pending duel challenge.");
		if(IsPlayerConnected(t) && PlayerInfo[t][pDuelOfferFrom] == playerid)
		{
			PlayerInfo[t][pDuelOfferFrom] = -1;
			PlayerInfo[t][pDuelOfferStake] = PlayerInfo[t][pDuelOfferExpiry] = 0;
			SendClientMessage(t, COLOR_SERVER_INFO, ""COL_YELLOW"* The duel challenge was withdrawn.");
		}
		PlayerInfo[playerid][pDuelOfferTo] = -1;
		PlayerInfo[playerid][pDuelOfferStake] = PlayerInfo[playerid][pDuelOfferExpiry] = 0;
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "You withdrew your duel challenge.");
	}

	new target, stake;
	if(sscanf(params, "dD(0)", target, stake))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /duel <id> [stake]  (or "COL_WHITE"/duel cancel"COL_WHITE"). The winner takes both antes.");
	if(!IsPlayerConnected(target) || IsPlayerNPC(target)) return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected.");
	if(target == playerid) return SendClientMessage(playerid, COLOR_ERROR, "You cannot duel yourself.");
	if(stake < 0 || stake > DUEL_STAKE_MAX) return SendClientMessage(playerid, COLOR_ERROR, "That stake is invalid.");

	// Both sides must be free to duel.
	if(!Arena_CanEnter(playerid)) return 1;
	if(!PlayerInfo[target][pSpawned] || Arena_IsInArena(target) || PlayerInfo[target][pAdminJailed] || Jail_IsJailed(target) || Mission_IsOnMission(target))
		return SendClientMessage(playerid, COLOR_ERROR, "That player is busy and cannot duel right now.");
	if(PlayerInfo[target][pDuelOfferFrom] != -1)
		return SendClientMessage(playerid, COLOR_ERROR, "That player already has a pending duel challenge.");
	if(PlayerInfo[playerid][pDuelOfferTo] != -1 && gettime() < PlayerInfo[playerid][pDuelOfferExpiry])
		return SendClientMessage(playerid, COLOR_ERROR, "You already have a pending challenge out. Use "COL_WHITE"/duel cancel"COL_WHITE" first.");

	// Both must be able to afford the ante up front (escrow is taken on /accept).
	if(Bank_GetCash(playerid) < stake) return SendClientMessage(playerid, COLOR_ERROR, "You cannot afford that stake.");
	if(Bank_GetCash(target) < stake)   return SendClientMessage(playerid, COLOR_ERROR, "That player cannot afford that stake.");

	PlayerInfo[playerid][pDuelOfferTo] = target;
	PlayerInfo[playerid][pDuelOfferStake] = stake;
	PlayerInfo[playerid][pDuelOfferExpiry] = gettime() + DUEL_OFFER_SECS;
	PlayerInfo[target][pDuelOfferFrom] = playerid;
	PlayerInfo[target][pDuelOfferStake] = stake;
	PlayerInfo[target][pDuelOfferExpiry] = gettime() + DUEL_OFFER_SECS;

	new string[160];
	format(string, sizeof(string), ""COL_GREEN"* You challenged "COL_WHITE"%s"COL_GREEN" to a duel for "COL_WHITE"$%d"COL_GREEN". They have %d seconds to /accept.", PlayerInfo[target][pUserName], stake, DUEL_OFFER_SECS);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	format(string, sizeof(string), ""COL_GREEN"* "COL_WHITE"%s (%d)"COL_GREEN" challenged you to a duel for "COL_WHITE"$%d"COL_GREEN". Type "COL_WHITE"/accept"COL_GREEN" or "COL_WHITE"/deny"COL_GREEN".", PlayerInfo[playerid][pUserName], playerid, stake);
	SendClientMessage(target, COLOR_SERVER_INFO, string);
	return 1;
}

// Try to resolve a pending duel challenge for `playerid` who typed /accept. The
// existing /accept command (cop.inc bribe) calls this FIRST; returns 1 if it
// handled a duel accept, 0 if there was no pending duel (fall through to bribe).
FUNCTION Duel_TryAccept(playerid)
{
	new from = PlayerInfo[playerid][pDuelOfferFrom];
	if(from == -1) return 0;	// no pending duel - let the caller handle /accept
	if(gettime() >= PlayerInfo[playerid][pDuelOfferExpiry])
	{
		PlayerInfo[playerid][pDuelOfferFrom] = -1;
		SendClientMessage(playerid, COLOR_ERROR, "That duel challenge has expired.");
		return 1;
	}
	if(!IsPlayerConnected(from) || PlayerInfo[from][pDuelOfferTo] != playerid)
	{
		PlayerInfo[playerid][pDuelOfferFrom] = -1;
		SendClientMessage(playerid, COLOR_ERROR, "That duel challenge is no longer valid.");
		return 1;
	}
	new stake = PlayerInfo[playerid][pDuelOfferStake];
	// Re-validate both sides are still free + still afford the ante at accept time.
	if(!Arena_CanEnter(playerid)) return 1;
	if(!PlayerInfo[from][pSpawned] || Arena_IsInArena(from) || PlayerInfo[from][pAdminJailed] || Jail_IsJailed(from) || Mission_IsOnMission(from))
		{ SendClientMessage(playerid, COLOR_ERROR, "The challenger is no longer available."); return 1; }
	if(Bank_GetCash(playerid) < stake) { SendClientMessage(playerid, COLOR_ERROR, "You can no longer afford the stake."); return 1; }
	if(Bank_GetCash(from) < stake)     { SendClientMessage(playerid, COLOR_ERROR, "The challenger can no longer afford the stake."); return 1; }

	// The challenger owns the private duel VW.
	Duel_Begin(from, playerid, stake, from);
	return 1;
}

CMD:deny(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new from = PlayerInfo[playerid][pDuelOfferFrom];
	if(from == -1) return SendClientMessage(playerid, COLOR_ERROR, "You have no pending duel challenge.");
	if(IsPlayerConnected(from) && PlayerInfo[from][pDuelOfferTo] == playerid)
	{
		PlayerInfo[from][pDuelOfferTo] = -1;
		PlayerInfo[from][pDuelOfferStake] = PlayerInfo[from][pDuelOfferExpiry] = 0;
		new string[96];
		format(string, sizeof(string), ""COL_YELLOW"* "COL_WHITE"%s"COL_YELLOW" declined your duel.", PlayerInfo[playerid][pUserName]);
		SendClientMessage(from, COLOR_SERVER_INFO, string);
	}
	PlayerInfo[playerid][pDuelOfferFrom] = -1;
	PlayerInfo[playerid][pDuelOfferStake] = PlayerInfo[playerid][pDuelOfferExpiry] = 0;
	return SendClientMessage(playerid, COLOR_SERVER_INFO, "You declined the duel.");
}

//==============================================================================
//	Hooks - death (arena respawn + fee + duel result), disconnect, tick
//==============================================================================

// OnPlayerDeath while inside an arena (CnR.pwn routes here BEFORE the normal city
// respawn, and returns early if this handled the death - returns 1 if it did).
FUNCTION Arena_OnPlayerDeath(playerid, killerid)
{
	if(!Arena_IsInArena(playerid)) return 0;

	// Duel: first blood ends it - the OTHER player wins (killer if valid, else the
	// stored opponent so a self-inflicted death still resolves).
	if(PlayerInfo[playerid][pArena] == ARENA_DUEL && PlayerInfo[playerid][pInDuel])
	{
		new opp = PlayerInfo[playerid][pDuelOpponent];
		new winner = (killerid != INVALID_PLAYER_ID && IsPlayerConnected(killerid) && killerid != playerid && PlayerInfo[killerid][pArena] == ARENA_DUEL) ? killerid : opp;
		Duel_Finish(winner, playerid, false);
		return 1;
	}

	// DMS / sniper: $1,000 death fee (best-effort - skip if broke). No life /
	// life-insurance loss (legal DM, §6.1). The respawn stays INSIDE the arena:
	// point the spawn at the arena spot; Arena_OnPlayerSpawn re-kits on the respawn.
	if(Bank_GetCash(playerid) >= ARENA_DEATH_COST) Bank_TakeCash(playerid, ARENA_DEATH_COST);
	new Float:sx, Float:sy, Float:sz, Float:sa, vw;
	Arena_SpawnPoint(PlayerInfo[playerid][pArena], sx, sy, sz, sa, vw);
	SetSpawnInfo(playerid, NO_TEAM, GetPlayerSkin(playerid), sx, sy, sz, sa, 0, 0, 0, 0, 0, 0);
	new string[96];
	format(string, sizeof(string), ""COL_RED"* You died in the arena "COL_WHITE"(-$%d)"COL_RED". Respawning inside - /leave to exit.", ARENA_DEATH_COST);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	return 1;
}

// OnPlayerSpawn hook (CnR.pwn routes here). If the player is respawning INSIDE an
// arena (DMS/sniper death-respawn), place + re-kit them in the isolated VW and keep
// the DM-zone flag; if a restore is pending (they died out of a duel / arena), apply
// the saved snapshot. Returns 1 if it handled the spawn (city spawn logic skipped).
FUNCTION Arena_OnPlayerSpawn(playerid)
{
	if(IsPlayerNPC(playerid)) return 0;

	// Still inside a DMS/sniper arena - respawn inside with a fresh life.
	if(PlayerInfo[playerid][pArena] == ARENA_DMS || PlayerInfo[playerid][pArena] == ARENA_SNIPER)
	{
		new Float:sx, Float:sy, Float:sz, Float:sa, vw;
		Arena_SpawnPoint(PlayerInfo[playerid][pArena], sx, sy, sz, sa, vw);
		SetPlayerInterior(playerid, 0);
		SetPlayerVirtualWorld(playerid, vw);
		SetPlayerPos(playerid, sx, sy, sz);
		SetPlayerFacingAngle(playerid, sa);
		SetCameraBehindPlayer(playerid);
		PlayerInfo[playerid][pInDMZone] = true;
		Arena_GiveKit(playerid, PlayerInfo[playerid][pArena]);
		return 1;
	}

	// Left the arena while dead (duel loss / died-out) - apply the deferred restore.
	if(PlayerInfo[playerid][pArena] == ARENA_NONE && PlayerInfo[playerid][pArenaSaved])
	{
		Arena_ApplySnapshot(playerid);
		GivePlayerSpawnProtection(playerid);
		return 1;
	}
	return 0;
}

// Disconnect: a live duel is a forfeit (opponent wins the pot); a pending offer is
// dropped both ways; DMS/sniper state just falls away (session-only).
FUNCTION Arena_OnDisconnect(playerid)
{
	// Live duel - forfeit to the opponent.
	if(PlayerInfo[playerid][pArena] == ARENA_DUEL && PlayerInfo[playerid][pInDuel])
	{
		new opp = PlayerInfo[playerid][pDuelOpponent];
		PlayerInfo[playerid][pInDuel] = false;	// stop a re-entrant forfeit
		if(IsPlayerConnected(opp)) Duel_Finish(opp, playerid, true);
	}
	// Drop a pending outgoing offer.
	new t = PlayerInfo[playerid][pDuelOfferTo];
	if(t != -1 && IsPlayerConnected(t) && PlayerInfo[t][pDuelOfferFrom] == playerid)
	{
		PlayerInfo[t][pDuelOfferFrom] = -1;
		PlayerInfo[t][pDuelOfferStake] = PlayerInfo[t][pDuelOfferExpiry] = 0;
	}
	// Drop a pending incoming offer.
	new f = PlayerInfo[playerid][pDuelOfferFrom];
	if(f != -1 && IsPlayerConnected(f) && PlayerInfo[f][pDuelOfferTo] == playerid)
	{
		PlayerInfo[f][pDuelOfferTo] = -1;
		PlayerInfo[f][pDuelOfferStake] = PlayerInfo[f][pDuelOfferExpiry] = 0;
	}
	return 1;
}

// GameModeClock 1s hook - expire stale pending duel offers (no new timer).
FUNCTION Arena_OnGameModeClockTick()
{
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(PlayerInfo[i][pDuelOfferTo] == -1) continue;
		if(gettime() < PlayerInfo[i][pDuelOfferExpiry]) continue;
		// Offer lapsed - clear both sides.
		new t = PlayerInfo[i][pDuelOfferTo];
		if(IsPlayerConnected(t) && PlayerInfo[t][pDuelOfferFrom] == i)
		{
			PlayerInfo[t][pDuelOfferFrom] = -1;
			PlayerInfo[t][pDuelOfferStake] = PlayerInfo[t][pDuelOfferExpiry] = 0;
			SendClientMessage(t, COLOR_SERVER_INFO, ""COL_YELLOW"* The duel challenge expired.");
		}
		PlayerInfo[i][pDuelOfferTo] = -1;
		PlayerInfo[i][pDuelOfferStake] = PlayerInfo[i][pDuelOfferExpiry] = 0;
		SendClientMessage(i, COLOR_SERVER_INFO, ""COL_YELLOW"* Your duel challenge expired (no answer).");
	}
	return 1;
}
