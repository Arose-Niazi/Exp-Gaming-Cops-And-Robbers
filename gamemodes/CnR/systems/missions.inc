/*

	Reusable mission framework + 4 core missions for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 4 (stage 3) — design §6 (missions intro), §6.4 Drug Delivery,
	§6.18 Truck Delivery, §6.17 Trash Pickup, §6.6 Food Delivery, §6.20 (mission
	milestone placement) + docs/research/crazybobs-site-full.md (§Missions —
	exact payouts / cooldowns / structure).

	This is the SHARED mission infrastructure the whole rest of the mission roster
	(M5/M6 — ~15 more missions) is built on. A mission is DATA + a tiny handler:

	  - MissionReg[]   — one row per mission: id, name, civ/cop type, cooldown (in
	                     GAME hours), start marker + reach, whether it needs a
	                     vehicle (and which models qualify), payout-per-checkpoint,
	                     all-complete bonus, and per-checkpoint wanted-if-cop-near.
	  - MissionRoute[] — a pool of candidate checkpoint coords per mission; the
	                     launcher samples a route of MissionReg[][mCheckpoints] from
	                     it (random, no repeats) so every run is a fresh delivery
	                     route (§6.4 "5 randomly-selected locations").

	Adding an M5/M6 mission is ~30 lines: append a MISSION_* id (defines.inc),
	append a MissionReg[] row + its MissionRoute[] pool here, and — only if it
	needs bespoke logic — a small switch arm in Mission_Complete/Mission_OnStartCmd.

	Checkpoint routing uses PER-PLAYER RACE checkpoints (SetPlayerRaceCheckpoint):
	each leg points an arrow at the next drop, and the final leg shows the finish
	flag. On each checkpoint the player is paid the per-cp amount (Bank_GiveCash);
	the final checkpoint pays the all-complete bonus on top and ends the mission.
	Progress rides a GameText HUD line (checkpoint N/M + earnings so far).

	Cooldowns are per-mission-type and PERSIST across sessions in ONE compact
	column: LSplayers.MissionCooldowns, a '|'-delimited list of last-completion
	game-hour stamps (one per mission), mirroring the SkinsSelected/GenderChanges
	delimited-string precedent (no per-mission column bloat). The "clock" is a
	monotonic game-hour = g_GameDayCounter*24 + GameHour (never rewinds on the
	game-week rollover). Loaded in loading_data.inc, saved on completion + on
	disconnect via Mission_SavePlayer. DECISION: compact persistent column (not
	session-only) — cooldowns survive relogs, which matters for the 5/12 game-hour
	gates, and it is a single column so the schema stays lean.

	Abort conditions (all clean up the race-checkpoint + reset state):
	  - death (OnPlayerDeath), disconnect (OnPlayerDisconnect)
	  - leaving the mission vehicle for a vehicle mission (OnPlayerStateChange)
	  - /quitmission (/cancelmission /stopmission /cancel) at any time
	  - straying MISSION_OFFROUTE_DIST from the active checkpoint (stuck/teleport)
	  - being jailed mid-run (the tick catches it)

	>>> M5/M6 EXTENSION POINT — this framework generalises to the whole §6 roster:
	    checkpoint routes (Vehicle Theft, House Delivery), vehicle-gated jobs
	    (Courier, Lawn Mowing, Tractor), cop-only missions (Routine Patrol,
	    Domestic Disturbance — MISSION_TYPE_COP is already wired), timed windows
	    (the cooldown-hours field doubles as the run window), and payout+bonus.
	    grep "TODO(M5)" / "TODO(M6)" for the documented hooks. <<<

	Credits:
		crazybobs-site-full.md §Missions for the mechanic grounding.

	Public API (FUNCTION publics — cross-module; the CP callback + hooks call these):
		Mission_IsOnMission(playerid)                 : bool — is a mission active
		Mission_Start(playerid, missionId)            : begin a mission (validated)
		Mission_NextCheckpoint(playerid)              : advance to / place the next CP
		Mission_Complete(playerid)                    : final CP reached — pay bonus + end
		Mission_Abort(playerid, const reason[])       : cancel + clean up (any reason)
		Mission_OnCheckpoint(playerid)                : dispatched from the race-CP callback
		Mission_OnCooldown(playerid, missionId)       : bool — still cooling down
		Mission_SetCooldown(playerid, missionId)      : stamp completion (now, game-hours)
		Mission_OnPlayerDeath(playerid)               : abort on death (hook)
		Mission_OnDisconnect(playerid)                : abort + persist (hook)
		Mission_OnPlayerStateChange(playerid, new, old): vehicle-exit abort (hook)
		Mission_OnTick()                              : off-route / jailed guard (GameModeClock)
		Mission_SavePlayer(playerid)                  : persist the cooldown column
		Mission_LoadCooldowns(playerid, const packed[]): parse the loaded column

	Commands:
		/missions (/mission /work /jobs /mis)  open the job launcher (list + cooldowns)
		/quitmission (/cancelmission /stopmission /cancel)  abandon the active mission

	Functions Being Called:
		misc:    IsCop, IsCivilian
		wanted:  GivePlayerWanted, IsPlayerSpawnProtected
		jail:    Jail_IsJailed
		bank:    Bank_GiveCash
*/

//==============================================================================
//	Mission registry — the data table (append rows for M5/M6; never renumber ids)
//==============================================================================

enum e_MissionReg
{
	mName[40],					// display name (launcher list + HUD)
	mType,						// MISSION_TYPE_CIVILIAN / MISSION_TYPE_COP (entry gate)
	mCooldownHrs,				// cooldown in GAME hours (also the natural run-window throttle)
	mCheckpoints,				// number of delivery checkpoints in a run (all 4 use 5)
	mCpPay,						// cash per checkpoint (Bank_GiveCash)
	mBonus,						// all-complete bonus paid on the final checkpoint
	mWantedPerCp,				// wanted added per checkpoint if a cop is in sight (0 = none)
	bool:mNeedsVehicle,			// must be driving to start (vehicle-gated job)
	bool:mLandOnly,				// land vehicles only (blocks air/sea for driving jobs)
	Float:mStartX,				// launcher marker (where the job is picked up / info shown)
	Float:mStartY,
	Float:mStartZ,
	mRouteCount					// number of candidate points in this mission's MissionRoute pool
}

// The registry. Order MUST match the MISSION_* enum in defines.inc.
// Start markers are the VERIFIED §6 city start locations (Los Santos):
//   Drug Delivery  — Las Colinas         (§6.4)
//   Truck Delivery — LS docks depot area  (§6.18 — trucks spawn at depots)
//   Trash Pickup   — LS city (Trashmaster job, roams the city) (§6.17)
//   Food Delivery  — LS city (food-vehicle job) (§6.6)
static const MissionReg[MISSION_COUNT][e_MissionReg] =
{
	// MISSION_DRUG_DELIVERY (§6.4) — civ, WL0 to start, 5 CP, $5k/cp + $50k, 5hr CD, +2 wanted/cp if cop near.
	{ "Drug Delivery",	MISSION_TYPE_CIVILIAN,	MDRUG_COOLDOWN_HRS,	5,	MDRUG_CP_PAY,	MDRUG_BONUS,	MDRUG_WANTED_PER_CP,	true,	true,	2278.7715, -85.6265, 26.4453,	10 },
	// MISSION_TRUCK_DELIVERY (§6.18) — civ, needs a delivery truck, 5 CP, 12hr window/CD.
	{ "Truck Delivery",	MISSION_TYPE_CIVILIAN,	MTRUCK_COOLDOWN_HRS,	5,	MTRUCK_CP_PAY,	MTRUCK_BONUS,	0,						true,	true,	2749.7590, -2454.5906, 13.6350,	10 },
	// MISSION_TRASH_PICKUP (§6.17) — civ, needs a Trashmaster, 5 CP, $2.5k/cp + $25k.
	{ "Trash Pickup",	MISSION_TYPE_CIVILIAN,	MTRASH_COOLDOWN_HRS,	5,	MTRASH_CP_PAY,	MTRASH_BONUS,	0,						true,	true,	2434.5364, -1521.5165, 23.9946,	10 },
	// MISSION_FOOD_DELIVERY (§6.6) — civ, needs a food vehicle, 5 CP, $2.5k/cp + $25k.
	{ "Food Delivery",	MISSION_TYPE_CIVILIAN,	MFOOD_COOLDOWN_HRS,	5,	MFOOD_CP_PAY,	MFOOD_BONUS,	0,						true,	true,	2105.5417, -1806.5814, 13.5547,	10 }
};

// Candidate checkpoint pools — one row per mission (MISSION_MAX_ROUTE_POOL cols).
// The launcher samples mCheckpoints of these (random, no repeats) per run so the
// route is fresh each time (§6.4 "5 randomly-selected locations, any order").
// Points are spread Los-Santos-wide (all street-level, reachable by land vehicle).
static const Float:MissionRoute[MISSION_COUNT][MISSION_MAX_ROUTE_POOL][3] =
{
	// MISSION_DRUG_DELIVERY — LS-wide drops.
	{
		{ 1479.5000, -1755.9000, 12.5500 }, { 2178.5000, -1160.5000, 25.0000 }, { 1930.0000, -1780.0000, 13.3800 },
		{ 2494.0000, -1690.0000, 13.3200 }, { 1170.0000, -1300.0000, 13.5000 }, { 655.0000,  -1275.0000, 13.5800 },
		{ 2100.0000, -1170.0000, 24.9000 }, { 1560.0000, -1000.0000, 24.0000 }, { 1830.0000, -1950.0000, 13.4000 },
		{ 2600.0000, -1310.0000, 25.7000 }
	},
	// MISSION_TRUCK_DELIVERY — depot / warehouse deliveries (docks & industrial).
	{
		{ 2749.7590, -2454.5906, 13.6350 }, { 2411.0000, -2530.0000, 13.6500 }, { 2170.0000, -2280.0000, 13.5000 },
		{ 1080.0000, -2020.0000, 13.0000 }, { 2635.0000, -2020.0000, 13.5500 }, { 1720.0000, -1890.0000, 13.5600 },
		{ 2450.0000, -1520.0000, 24.0000 }, { 970.0000,  -1230.0000, 16.5000 }, { 1930.0000, -2640.0000, 13.5000 },
		{ 2200.0000, -2020.0000, 13.5500 }
	},
	// MISSION_TRASH_PICKUP — bins scattered around the LS streets.
	{
		{ 2434.5364, -1521.5165, 23.9946 }, { 1830.0000, -1680.0000, 13.5300 }, { 2100.0000, -1470.0000, 23.9700 },
		{ 1560.0000, -1360.0000, 13.5000 }, { 2270.0000, -1660.0000, 14.5000 }, { 1290.0000, -1600.0000, 13.5400 },
		{ 2560.0000, -1300.0000, 25.7000 }, { 1990.0000, -1440.0000, 13.5400 }, { 1470.0000, -1770.0000, 12.5500 },
		{ 2660.0000, -1670.0000, 10.0000 }
	},
	// MISSION_FOOD_DELIVERY — food drops around residential LS.
	{
		{ 2105.5417, -1806.5814, 13.5547 }, { 2490.0000, -1960.0000, 13.5500 }, { 1830.0000, -1140.0000, 24.0000 },
		{ 2200.0000, -1150.0000, 25.0000 }, { 1360.0000, -1520.0000, 13.5000 }, { 660.0000,  -1860.0000, 5.5000 },
		{ 2760.0000, -1620.0000, 12.2000 }, { 1470.0000, -1180.0000, 23.9000 }, { 1930.0000, -1300.0000, 13.5600 },
		{ 2410.0000, -1490.0000, 24.0000 }
	}
};

// Accepted vehicle models per vehicle-gated mission. Model = GetVehicleModel().
// Drug Delivery accepts any LAND vehicle (checked generically), so it has no set.
static const MissionTruckModels[] =
{
	// Truck Delivery — the 14 supported delivery-truck models (§6.18 VERIFIED).
	499,	// Benson
	498,	// Boxville
	524,	// Cement Truck
	578,	// DFT-30
	406,	// Dumper
	573,	// Dune
	455,	// Flatbed
	403,	// Linerunner
	414,	// Mule
	443,	// Packer
	413,	// Pony
	515,	// Roadtrain
	514,	// Tanker
	456		// Yankee
};

static const MissionFoodModels[] =
{
	// Food Delivery — Mr.Whoopee, Hotdog, Pizzaboy (§6.6 VERIFIED).
	423,	// Mr Whoopee
	588,	// Hotdog
	448		// Pizzaboy
};

#define MISSION_TRASHMASTER_MODEL	408		// Trashmaster (Trash Pickup gate, §6.17)

//==============================================================================
//	Internal helpers
//==============================================================================

// The monotonic game-hour "clock" cooldowns are stamped against. g_GameDayCounter
// is monotonic (never resets on the game-week rollover), so this never rewinds.
static Mission_NowGameHour()
	return (g_GameDayCounter * MISSION_HOURS_PER_DAY) + GameHour;

// Is a cop within sight (range) of the player? Used for the Drug Delivery wanted
// tick (§6.4). No name-based logic — pure team + proximity + same interior/VW.
// (A generic helper the M5/M6 smuggling missions reuse.)
static Mission_IsCopInSight(playerid)
{
	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(i == playerid || !IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(!IsCop(i)) continue;
		if(GetPlayerInterior(i) != GetPlayerInterior(playerid)) continue;
		if(GetPlayerVirtualWorld(i) != GetPlayerVirtualWorld(playerid)) continue;
		if(IsPlayerInRangeOfPoint(i, MISSION_COPSIGHT_RANGE, x, y, z)) return 1;
	}
	return 0;
}

// Does the player's current vehicle qualify for `missionId`? Vehicle missions only.
// Drug Delivery accepts any land vehicle; the others need a specific model set.
static Mission_VehicleOK(playerid, missionId)
{
	if(!IsPlayerInAnyVehicle(playerid)) return 0;
	new veh = GetPlayerVehicleID(playerid);
	if(veh == 0) return 0;
	if(GetPlayerState(playerid) != PLAYER_STATE_DRIVER) return 0;	// must be the driver
	new model = GetVehicleModel(veh);

	switch(missionId)
	{
		case MISSION_DRUG_DELIVERY:
		{
			// Any land vehicle (no air/sea). Reuse the streamer-free land check.
			return Mission_IsLandVehicle(model);
		}
		case MISSION_TRUCK_DELIVERY:
		{
			for(new i = 0; i < sizeof(MissionTruckModels); i++)
				if(MissionTruckModels[i] == model) return 1;
			return 0;
		}
		case MISSION_TRASH_PICKUP:
		{
			return (model == MISSION_TRASHMASTER_MODEL);
		}
		case MISSION_FOOD_DELIVERY:
		{
			for(new i = 0; i < sizeof(MissionFoodModels); i++)
				if(MissionFoodModels[i] == model) return 1;
			return 0;
		}
	}
	return 0;
}

// Land-vehicle test (blocks planes/helis/boats for the driving jobs). Covers the
// SA-MP air/sea model ranges; everything else is treated as land.
static Mission_IsLandVehicle(model)
{
	switch(model)
	{
		// Aircraft.
		case 460, 464, 476, 511, 512, 513, 519, 520, 553, 577, 592, 593: return 0;	// planes
		case 417, 425, 447, 465, 469, 487, 488, 497, 501, 548, 563: return 0;		// helicopters
		// Boats.
		case 430, 446, 452, 453, 454, 472, 473, 484, 493, 595: return 0;
	}
	return 1;
}

// Place / refresh the race checkpoint for the current leg. type is the arrow for a
// mid-route leg or the finish flag on the final leg.
static Mission_PlaceRaceCP(playerid)
{
	new missionId = PlayerInfo[playerid][pOnMission];
	if(missionId == -1) return 0;
	new idx = PlayerInfo[playerid][pMissionCheckpoint];		// next leg to reach (0-based)
	new len = PlayerInfo[playerid][pMissionRouteLen];
	if(idx >= len) return 0;

	new Float:cx = PlayerInfo[playerid][pMissionRouteX][idx];
	new Float:cy = PlayerInfo[playerid][pMissionRouteY][idx];
	new Float:cz = PlayerInfo[playerid][pMissionRouteZ][idx];

	// Next-leg coords feed the on-checkpoint arrow (points at the following drop).
	new Float:nx = cx, Float:ny = cy, Float:nz = cz;
	new bool:isFinal = (idx == len - 1);
	if(!isFinal)
	{
		nx = PlayerInfo[playerid][pMissionRouteX][idx + 1];
		ny = PlayerInfo[playerid][pMissionRouteY][idx + 1];
		nz = PlayerInfo[playerid][pMissionRouteZ][idx + 1];
	}

	// Ground race checkpoint: a directional arrow on a mid-route leg, the finish
	// flag on the final leg (open.mp race-CP types: GROUND_NORMAL=0, GROUND_FINISH=1).
	new CP_TYPE:cpType = isFinal ? CP_TYPE_GROUND_FINISH : CP_TYPE_GROUND_NORMAL;

	SetPlayerRaceCheckpoint(playerid, cpType, cx, cy, cz, nx, ny, nz, MISSION_CP_SIZE);
	return 1;
}

// Live HUD line — checkpoint N/M + earnings so far (design §6 "monitor status").
static Mission_ShowHUD(playerid)
{
	new missionId = PlayerInfo[playerid][pOnMission];
	if(missionId == -1) return 0;
	new done = PlayerInfo[playerid][pMissionCheckpoint];
	new len = PlayerInfo[playerid][pMissionRouteLen];
	new gt[144];
	format(gt, sizeof(gt), "~g~%s~n~~w~Checkpoint ~y~%d~w~/~y~%d~n~~w~Earned ~g~$%d", MissionReg[missionId][mName], done, len, PlayerInfo[playerid][pMissionData]);
	GameTextForPlayer(playerid, gt, 4000, 4);
	return 1;
}

// Clear all active-mission state (checkpoint + fields). Does NOT touch cooldowns.
static Mission_ClearState(playerid)
{
	if(IsPlayerConnected(playerid)) DisablePlayerRaceCheckpoint(playerid);
	PlayerInfo[playerid][pOnMission] = -1;
	PlayerInfo[playerid][pMissionCheckpoint] = 0;
	PlayerInfo[playerid][pMissionRouteLen] = 0;
	PlayerInfo[playerid][pMissionVehicle] = -1;
	PlayerInfo[playerid][pMissionData] = 0;
	for(new i = 0; i < MISSION_MAX_CHECKPOINTS; i++)
	{
		PlayerInfo[playerid][pMissionRouteX][i] = 0.0;
		PlayerInfo[playerid][pMissionRouteY][i] = 0.0;
		PlayerInfo[playerid][pMissionRouteZ][i] = 0.0;
	}
	return 1;
}

// Build a random route of MissionReg[][mCheckpoints] points from the mission's
// candidate pool (no repeats) into the player's active-route slots. Fisher-Yates
// on a local index list keeps it unbiased and repeat-free.
static Mission_BuildRoute(playerid, missionId)
{
	new want = MissionReg[missionId][mCheckpoints];
	new pool = MissionReg[missionId][mRouteCount];
	if(want > MISSION_MAX_CHECKPOINTS) want = MISSION_MAX_CHECKPOINTS;
	if(want > pool) want = pool;

	new order[MISSION_MAX_ROUTE_POOL];
	for(new i = 0; i < pool; i++) order[i] = i;
	// Partial shuffle: pick `want` distinct pool indices.
	for(new i = 0; i < want; i++)
	{
		new j = i + random(pool - i);
		new tmp = order[i]; order[i] = order[j]; order[j] = tmp;
	}
	for(new i = 0; i < want; i++)
	{
		new pt = order[i];
		PlayerInfo[playerid][pMissionRouteX][i] = MissionRoute[missionId][pt][0];
		PlayerInfo[playerid][pMissionRouteY][i] = MissionRoute[missionId][pt][1];
		PlayerInfo[playerid][pMissionRouteZ][i] = MissionRoute[missionId][pt][2];
	}
	PlayerInfo[playerid][pMissionRouteLen] = want;
	return want;
}

//==============================================================================
//	Cooldown API (game-hour stamps, persisted in one packed column)
//==============================================================================

// Stamp `missionId` as completed now (monotonic game-hour). Persisted immediately.
FUNCTION Mission_SetCooldown(playerid, missionId)
{
	if(missionId < 0 || missionId >= MISSION_COUNT) return 0;
	PlayerInfo[playerid][pMissionCooldown][missionId] = Mission_NowGameHour();
	Mission_SavePlayer(playerid);
	return 1;
}

// Is `missionId` still cooling down for this player? A stamp of 0 = never done.
FUNCTION Mission_OnCooldown(playerid, missionId)
{
	if(missionId < 0 || missionId >= MISSION_COUNT) return 0;
	new stamp = PlayerInfo[playerid][pMissionCooldown][missionId];
	if(stamp <= 0) return 0;
	new elapsed = Mission_NowGameHour() - stamp;
	if(elapsed < 0) return 0;	// clock somehow behind the stamp — treat as ready
	return (elapsed < MissionReg[missionId][mCooldownHrs]);
}

// Game hours remaining on `missionId`'s cooldown (0 = ready).
static Mission_CooldownLeft(playerid, missionId)
{
	if(!Mission_OnCooldown(playerid, missionId)) return 0;
	new elapsed = Mission_NowGameHour() - PlayerInfo[playerid][pMissionCooldown][missionId];
	new left = MissionReg[missionId][mCooldownHrs] - elapsed;
	return (left < 0) ? 0 : left;
}

//==============================================================================
//	Core lifecycle API (design §6)
//==============================================================================

FUNCTION Mission_IsOnMission(playerid)
	return (PlayerInfo[playerid][pOnMission] != -1);

// Begin `missionId`. Validates all gates (logged in, not already on a mission,
// civ/cop type, jail/spawn-protect, wanted for Drug Delivery, cooldown, vehicle),
// builds a fresh random route, and places the first race checkpoint.
FUNCTION Mission_Start(playerid, missionId)
{
	if(!IsPlayerConnected(playerid) || !PlayerInfo[playerid][pLoggedIn]) return 0;
	if(missionId < 0 || missionId >= MISSION_COUNT) return 0;

	if(Mission_IsOnMission(playerid))
		return SendClientMessage(playerid, COLOR_ERROR, "You are already on a mission. Finish it or /quitmission first.");

	// Shared gates: jailed / spawn-protected players cannot start jobs.
	if(Jail_IsJailed(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot start a mission while you are in jail.");
	if(IsPlayerSpawnProtected(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot start a mission while spawn-protected. Wait a moment.");

	// Civ/cop entry gate.
	if(MissionReg[missionId][mType] == MISSION_TYPE_COP && !IsCop(playerid))
		return SendClientMessage(playerid, COLOR_ERROR, "That mission is for law enforcement only.");
	if(MissionReg[missionId][mType] == MISSION_TYPE_CIVILIAN && IsCop(playerid))
		return SendClientMessage(playerid, COLOR_ERROR, "Law enforcement cannot take civilian jobs.");

	// Drug Delivery requires an innocent (WL0) start (§6.4 VERIFIED).
	if(missionId == MISSION_DRUG_DELIVERY && GetPlayerWanted(playerid) > 0)
		return SendClientMessage(playerid, COLOR_ERROR, "You must be innocent (no wanted level) to start a Drug Delivery.");

	// Per-mission cooldown (game hours).
	if(Mission_OnCooldown(playerid, missionId))
	{
		new err[128];
		format(err, sizeof(err), "%s is on cooldown — try again in %d game hour(s).", MissionReg[missionId][mName], Mission_CooldownLeft(playerid, missionId));
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}

	// Vehicle-gated jobs: must be driving a qualifying vehicle.
	if(MissionReg[missionId][mNeedsVehicle] && !Mission_VehicleOK(playerid, missionId))
	{
		new err[144];
		switch(missionId)
		{
			case MISSION_DRUG_DELIVERY: format(err, sizeof(err), "You must be driving a land vehicle to start a Drug Delivery.");
			case MISSION_TRUCK_DELIVERY: format(err, sizeof(err), "You must be driving a delivery truck (Benson, Mule, Linerunner, Tanker, ...) to start a Truck Delivery.");
			case MISSION_TRASH_PICKUP: format(err, sizeof(err), "You must be driving a Trashmaster to start a Trash Pickup.");
			case MISSION_FOOD_DELIVERY: format(err, sizeof(err), "You must be driving a food vehicle (Mr.Whoopee, Hotdog, or Pizzaboy) to start a Food Delivery.");
			default: format(err, sizeof(err), "You need the right vehicle for that mission.");
		}
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}

	// All gates passed — start it. Bind the mission vehicle (for the exit abort).
	PlayerInfo[playerid][pOnMission] = missionId;
	PlayerInfo[playerid][pMissionCheckpoint] = 0;
	PlayerInfo[playerid][pMissionData] = 0;
	PlayerInfo[playerid][pMissionVehicle] = MissionReg[missionId][mNeedsVehicle] ? GetPlayerVehicleID(playerid) : -1;
	Mission_BuildRoute(playerid, missionId);
	Mission_PlaceRaceCP(playerid);
	Mission_ShowHUD(playerid);

	new string[176];
	format(string, sizeof(string), ""COL_GREEN"* Mission started: "COL_WHITE"%s"COL_GREEN". Reach all "COL_WHITE"%d"COL_GREEN" checkpoints. Payout "COL_WHITE"$%d"COL_GREEN"/checkpoint + "COL_WHITE"$%d"COL_GREEN" completion bonus.",
		MissionReg[missionId][mName], PlayerInfo[playerid][pMissionRouteLen], MissionReg[missionId][mCpPay], MissionReg[missionId][mBonus]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	SendClientMessage(playerid, COLOR_SERVER_INFO, "Follow the "COL_YELLOW"race checkpoint"COL_WHITE" markers. "COL_YELLOW"/quitmission"COL_WHITE" to abandon.");

	// Drug Delivery is smuggling — warn about the cop-sight wanted mechanic.
	if(missionId == MISSION_DRUG_DELIVERY)
		SendClientMessage(playerid, COLOR_ERROR, ""COL_RED"Careful — every drop within sight of a cop adds "COL_WHITE"+2 wanted"COL_RED". Stay clear of the police.");
	return 1;
}

// Advance to the next checkpoint (called after a mid-route drop is scored). Places
// the next race CP and refreshes the HUD. The final CP is handled by Mission_Complete.
FUNCTION Mission_NextCheckpoint(playerid)
{
	if(!Mission_IsOnMission(playerid)) return 0;
	Mission_PlaceRaceCP(playerid);
	Mission_ShowHUD(playerid);
	return 1;
}

// Final checkpoint reached — pay the completion bonus, stamp the cooldown, end.
FUNCTION Mission_Complete(playerid)
{
	new missionId = PlayerInfo[playerid][pOnMission];
	if(missionId == -1) return 0;

	// Completion bonus on top of the per-checkpoint pay already banked.
	new bonus = MissionReg[missionId][mBonus];
	if(bonus > 0) Bank_GiveCash(playerid, bonus);
	PlayerInfo[playerid][pMissionData] += bonus;
	PlayerInfo[playerid][pScore] += 1;	// +1 completion score (on top of per-cp scores)

	new total = PlayerInfo[playerid][pMissionData];
	new name[40];
	format(name, sizeof(name), "%s", MissionReg[missionId][mName]);

	// Stamp the cooldown (persists) BEFORE clearing state so the id is still valid.
	Mission_SetCooldown(playerid, missionId);
	Mission_ClearState(playerid);

	new string[176];
	format(string, sizeof(string), ""COL_GREEN"* %s COMPLETE! "COL_WHITE"Completion bonus "COL_GREEN"$%d"COL_WHITE". Total earned this run: "COL_GREEN"$%d"COL_WHITE". +1 score.", name, bonus, total);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	GameTextForPlayer(playerid, "~g~Mission complete!", 4000, 4);
	return 1;
}

// Cancel + clean up for ANY reason (death, disconnect, vehicle-exit, /quitmission,
// off-route, jail). No payout is clawed back — per-checkpoint cash already banked
// is cash-in-hand and stays. No cooldown is stamped (only completion cools down).
FUNCTION Mission_Abort(playerid, const reason[])
{
	if(!Mission_IsOnMission(playerid)) return 0;
	new name[40];
	format(name, sizeof(name), "%s", MissionReg[PlayerInfo[playerid][pOnMission]][mName]);
	Mission_ClearState(playerid);

	if(IsPlayerConnected(playerid))
	{
		new string[160];
		if(isNull(reason)) format(string, sizeof(string), ""COL_RED"* Mission cancelled: "COL_WHITE"%s"COL_RED".", name);
		else format(string, sizeof(string), ""COL_RED"* Mission cancelled: "COL_WHITE"%s "COL_RED"(%s).", name, reason);
		SendClientMessage(playerid, COLOR_ERROR, string);
	}
	return 1;
}

// Dispatched from OnPlayerEnterRaceCheckpoint (CnR.pwn). Scores the reached drop,
// pays the per-checkpoint amount, applies the drug-delivery cop-sight wanted, then
// either advances or completes. Mission-specific logic is a tiny switch here.
FUNCTION Mission_OnCheckpoint(playerid)
{
	new missionId = PlayerInfo[playerid][pOnMission];
	if(missionId == -1) return 0;

	// Vehicle missions: must still be in the bound mission vehicle to score a drop.
	if(MissionReg[missionId][mNeedsVehicle])
	{
		if(!IsPlayerInAnyVehicle(playerid) || GetPlayerVehicleID(playerid) != PlayerInfo[playerid][pMissionVehicle])
		{
			SendClientMessage(playerid, COLOR_ERROR, "Get back in your mission vehicle to make the drop.");
			return 0;
		}
	}

	// Score this checkpoint.
	PlayerInfo[playerid][pMissionCheckpoint]++;
	new pay = MissionReg[missionId][mCpPay];
	if(pay > 0) Bank_GiveCash(playerid, pay);
	PlayerInfo[playerid][pMissionData] += pay;
	PlayerInfo[playerid][pScore] += 1;	// +1 score per checkpoint (§6 Score Guide)

	new string[144];
	format(string, sizeof(string), ""COL_GREEN"* Checkpoint "COL_WHITE"%d"COL_GREEN"/"COL_WHITE"%d"COL_GREEN" — "COL_WHITE"+$%d"COL_GREEN" (run total $%d).",
		PlayerInfo[playerid][pMissionCheckpoint], PlayerInfo[playerid][pMissionRouteLen], pay, PlayerInfo[playerid][pMissionData]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);

	// Mission-specific per-checkpoint effects (small switch — the extension point).
	switch(missionId)
	{
		case MISSION_DRUG_DELIVERY:
		{
			// +2 wanted for this drop if a cop is in sight (§6.4). GivePlayerWanted
			// itself is suppressed in DM/safe zones and during spawn protection.
			if(MissionReg[missionId][mWantedPerCp] > 0 && Mission_IsCopInSight(playerid))
				GivePlayerWanted(playerid, MissionReg[missionId][mWantedPerCp], "Drug delivery seen by police");
		}
		// TODO(M5): Vehicle Theft — last-car entry sets WL10 (§6.19); Holdup Mission
		// wraps the /holdup system (§6.7). TODO(M6): House Delivery illegal-cargo
		// intercept wanted (§6.8), Illegal Immigrant completion wanted (§6.9).
	}

	// Final checkpoint? Complete; else advance the route.
	if(PlayerInfo[playerid][pMissionCheckpoint] >= PlayerInfo[playerid][pMissionRouteLen])
		Mission_Complete(playerid);
	else
		Mission_NextCheckpoint(playerid);
	return 1;
}

//==============================================================================
//	Abort hooks (death / disconnect / vehicle-exit / off-route)
//==============================================================================

FUNCTION Mission_OnPlayerDeath(playerid)
{
	if(Mission_IsOnMission(playerid)) Mission_Abort(playerid, "you died");
	return 1;
}

FUNCTION Mission_OnDisconnect(playerid)
{
	// Persist the cooldown stamps before we lose the slot, then drop active state.
	if(PlayerInfo[playerid][pLoggedIn]) Mission_SavePlayer(playerid);
	// Clear active-mission state WITHOUT the connected-player message paths.
	if(PlayerInfo[playerid][pOnMission] != -1)
	{
		PlayerInfo[playerid][pOnMission] = -1;
		PlayerInfo[playerid][pMissionCheckpoint] = 0;
		PlayerInfo[playerid][pMissionRouteLen] = 0;
		PlayerInfo[playerid][pMissionVehicle] = -1;
		PlayerInfo[playerid][pMissionData] = 0;
	}
	return 1;
}

// Vehicle missions abort when the driver leaves the bound mission vehicle (§6.17
// "auto-cancels if the Trashmaster is lost", §6.18 "auto-cancels if truck is lost").
FUNCTION Mission_OnPlayerStateChange(playerid, newstate, oldstate)
{
	if(!Mission_IsOnMission(playerid)) return 0;
	new missionId = PlayerInfo[playerid][pOnMission];
	if(!MissionReg[missionId][mNeedsVehicle]) return 0;

	// Left the driver seat (walked out / got knocked out of the vehicle).
	if(oldstate == PLAYER_STATE_DRIVER && newstate != PLAYER_STATE_DRIVER)
	{
		Mission_Abort(playerid, "you left your mission vehicle");
	}
	return 1;
}

// Ride the 1-second GameModeClock tick (no new global timer): abort a run if the
// player strays MISSION_OFFROUTE_DIST from the active checkpoint (teleport/stuck)
// or ends up jailed mid-mission.
FUNCTION Mission_OnTick()
{
	for(new playerid = 0; playerid < MAX_PLAYERS; playerid++)
	{
		if(!IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) continue;
		if(PlayerInfo[playerid][pOnMission] == -1) continue;

		if(Jail_IsJailed(playerid)) { Mission_Abort(playerid, "you were jailed"); continue; }

		new idx = PlayerInfo[playerid][pMissionCheckpoint];
		new len = PlayerInfo[playerid][pMissionRouteLen];
		if(idx >= len) continue;
		if(!IsPlayerInRangeOfPoint(playerid, MISSION_OFFROUTE_DIST,
			PlayerInfo[playerid][pMissionRouteX][idx], PlayerInfo[playerid][pMissionRouteY][idx], PlayerInfo[playerid][pMissionRouteZ][idx]))
			Mission_Abort(playerid, "you strayed too far off-route");
	}
	return 1;
}

//==============================================================================
//	Cooldown persistence — one packed column (LSplayers.MissionCooldowns)
//==============================================================================

// Persist the cooldown stamps as a '|'-delimited list (mirrors SkinsSelected).
// Folded into the shared per-player row so there is no competing UPDATE.
FUNCTION Mission_SavePlayer(playerid)
{
	if(!PlayerInfo[playerid][pLoggedIn]) return 0;
	new packed[96], part[16];
	packed[0] = EOS;
	for(new i = 0; i < MISSION_COUNT; i++)
	{
		format(part, sizeof(part), "%d", PlayerInfo[playerid][pMissionCooldown][i]);
		strcat(packed, part);
		if(i != MISSION_COUNT - 1) strcat(packed, "|");
	}
	new string[192];
	mysql_format(g_SQL, string, sizeof(string), "UPDATE "TABLE_PLAYERS_C" SET MissionCooldowns='%e' WHERE aID=%d LIMIT 1", packed, PlayerInfo[playerid][pID]);
	mysql_pquery(g_SQL, string);
	return 1;
}

// Parse the loaded '|'-delimited MissionCooldowns column into the per-mission
// stamp array. Missing/short lists leave the trailing missions at 0 (ready).
FUNCTION Mission_LoadCooldowns(playerid, const packed[])
{
	for(new i = 0; i < MISSION_COUNT; i++) PlayerInfo[playerid][pMissionCooldown][i] = 0;
	if(isNull(packed)) return 0;

	new idx = 0, part[16], cur = 0, len = strlen(packed);
	while(idx < MISSION_COUNT && cur < len)
	{
		new k = 0;
		while(cur < len && packed[cur] != '|' && k < 15) part[k++] = packed[cur++];
		part[k] = EOS;
		while(cur < len && packed[cur] != '|') cur++;	// skip any overflow chars in this token
		if(packed[cur] == '|') cur++;					// step over the delimiter
		PlayerInfo[playerid][pMissionCooldown][idx] = strval(part);
		idx++;
	}
	return idx;
}

//==============================================================================
//	The launcher — /missions (/mission /work /jobs) : list + cooldowns dialog
//==============================================================================

// Render the job list with per-mission availability (ready / cooldown / wrong
// vehicle). CB-style dialog entry point (design §6 — "/mission" launcher).
static Mission_ShowLauncher(playerid)
{
	new caption[900], row[160];
	caption[0] = EOS;
	for(new m = 0; m < MISSION_COUNT; m++)
	{
		new status[48];
		if(MissionReg[m][mType] == MISSION_TYPE_COP && !IsCop(playerid)) format(status, sizeof(status), "{888888}cops only");
		else if(MissionReg[m][mType] == MISSION_TYPE_CIVILIAN && IsCop(playerid)) format(status, sizeof(status), "{888888}civilians only");
		else if(Mission_OnCooldown(playerid, m)) format(status, sizeof(status), "{CC5555}%d game-hr cooldown", Mission_CooldownLeft(playerid, m));
		else if(MissionReg[m][mNeedsVehicle] && !Mission_VehicleOK(playerid, m)) format(status, sizeof(status), "{DDAA33}need vehicle");
		else format(status, sizeof(status), "{55CC55}READY");

		format(row, sizeof(row), "%s\t$%d/cp + $%d\t%s\n", MissionReg[m][mName], MissionReg[m][mCpPay], MissionReg[m][mBonus], status);
		strcat(caption, row);
	}
	ShowPlayerDialog(playerid, MISSION_START_DIALOG, DIALOG_STYLE_TABLIST, "Jobs & Missions", caption, "Start", "Close");
	return 1;
}

// Dialog resolution (routed from OnDialogResponse in CnR.pwn).
FUNCTION Mission_OnDialogResponse(playerid, response, listitem)
{
	if(!response) return 1;
	if(listitem < 0 || listitem >= MISSION_COUNT) return 1;
	Mission_Start(playerid, listitem);
	return 1;
}

CMD:missions(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	// If already on a mission, /missions shows current status instead of the list.
	if(Mission_IsOnMission(playerid))
	{
		Mission_ShowHUD(playerid);
		new missionId = PlayerInfo[playerid][pOnMission], string[176];
		format(string, sizeof(string), ""COL_WHITE"Active mission: "COL_GREEN"%s"COL_WHITE" — checkpoint "COL_YELLOW"%d"COL_WHITE"/"COL_YELLOW"%d"COL_WHITE", earned "COL_GREEN"$%d"COL_WHITE". "COL_YELLOW"/quitmission"COL_WHITE" to abandon.",
			MissionReg[missionId][mName], PlayerInfo[playerid][pMissionCheckpoint], PlayerInfo[playerid][pMissionRouteLen], PlayerInfo[playerid][pMissionData]);
		return SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	}
	return Mission_ShowLauncher(playerid);
}
alias:missions("mission", "work", "jobs", "mis")

CMD:quitmission(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(!Mission_IsOnMission(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You are not on a mission.");
	Mission_Abort(playerid, "");
	return 1;
}
alias:quitmission("cancelmission", "stopmission", "cancel")
