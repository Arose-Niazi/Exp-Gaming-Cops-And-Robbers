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
	{ "Food Delivery",	MISSION_TYPE_CIVILIAN,	MFOOD_COOLDOWN_HRS,	5,	MFOOD_CP_PAY,	MFOOD_BONUS,	0,						true,	true,	2105.5417, -1806.5814, 13.5547,	10 },
	// MISSION_VEHICLE_THEFT (§6.19) — civ, deliver 5 stolen cars to chop-shop drops
	// (needs a land vehicle to score each drop; the LAST drop forces WL10). Start at
	// LS Market. $15k/car + $75k bonus (=$150k), 15 game-hr cooldown.
	{ "Vehicle Theft",	MISSION_TYPE_CIVILIAN,	MVEHTHEFT_COOLDOWN_HRS,	5,	MVEHTHEFT_CP_PAY,	MVEHTHEFT_BONUS,	MVEHTHEFT_WANTED_PER_CP,	true,	true,	1032.5000, -1032.0000, 32.1000,	10 },
	// MISSION_HOLDUP_MISSION (§6.7) — civ, chain 3 store-holdup checkpoints ON FOOT
	// ($10k min each + a large completion bonus). Start at LS Market. 5 game-hr CD.
	{ "Holdup Mission",	MISSION_TYPE_CIVILIAN,	MHOLDUP_COOLDOWN_HRS,	MHOLDUP_STORES,	MHOLDUP_CP_PAY,	MHOLDUP_BONUS,	MHOLDUP_WANTED_PER_CP,	false,	true,	1315.5000, -1279.0000, 13.5500,	10 },
	// MISSION_AIRPORT_ROBBERY (§6.1) — civ, box-collection mechanic (bespoke handler,
	// NOT the route CPs). Needs a truck/van/garbage truck; the row carries the launcher
	// marker + cooldown only (mCheckpoints/route are unused). Start at LS International.
	{ "Airport Robbery",MISSION_TYPE_CIVILIAN,	MAIRPORT_COOLDOWN_HRS,	1,	0,				MAIRPORT_BONUS,	MAIRPORT_WANTED_START,	true,	true,	1685.0000, -2333.0000, 13.5000,	1 },
	// ---- M6 (stage 4) — the final ~12 missions (append, never renumber). --------
	// MISSION_COURIER_DELIVERY (§6.2) — civ, any land vehicle, 5 city drops, warrant on
	// pickup, distance pay, 12 game-hr window. Start at LS docks (far-from-city pickup).
	{ "Courier Delivery",MISSION_TYPE_CIVILIAN,	MCOURIER_COOLDOWN_HRS,	5,	MCOURIER_CP_PAY,	MCOURIER_BONUS,	0,						true,	true,	2749.0000, -2450.0000, 13.6300,	10 },
	// MISSION_HOUSE_DELIVERY (§6.8) — civ, any land vehicle, 5 house drops (some illegal
	// → +wanted if cop near), 12 game-hr window. Start at LS Ocean Docks (§6.8 VERIFIED).
	{ "House Delivery",	MISSION_TYPE_CIVILIAN,	MHOUSEDEL_COOLDOWN_HRS,	5,	MHOUSEDEL_CP_PAY,	MHOUSEDEL_BONUS,	MHOUSEDEL_WANTED_PER_CP,true,	true,	2165.5000, -2279.5000, 13.5000,	10 },
	// MISSION_ILLEGAL_IMMIGRANT (§6.9) — innocent civ, 5-leg pickup+return route, WANTED
	// ON COMPLETION (not pickup), night reduces cop sight. Start at East Los Santos.
	{ "Illegal Immigrant",MISSION_TYPE_CIVILIAN,MIMMIG_COOLDOWN_HRS,	5,	MIMMIG_CP_PAY,	MIMMIG_BONUS,	0,						true,	true,	2748.0000, -1330.0000, 21.5000,	10 },
	// MISSION_LAWN_MOWING (§6.10) — civ, needs a Mower, timed mow route + completion
	// bonus. LV golf course start (Prickle Pine). Built-in restart wait on cancel.
	{ "Lawn Mowing",	MISSION_TYPE_CIVILIAN,	MLAWN_COOLDOWN_HRS,		5,	MLAWN_CP_PAY,	MLAWN_BONUS,	0,						true,	true,	1237.0000, 1834.0000, 8.5000,	10 },
	// MISSION_PAPERBOY (§6.11) — innocent civ, ON FOOT, 4-MIN REAL-TIME limit, $500/drop.
	// Start at LS Vinewood (§6.11 VERIFIED). mNeedsVehicle=false (on foot).
	{ "Paperboy",		MISSION_TYPE_CIVILIAN,	MPAPERBOY_COOLDOWN_HRS,	5,	MPAPERBOY_CP_PAY,	MPAPERBOY_BONUS,	0,					false,	true,	845.2500, -1042.0000, 22.5000,	10 },
	// MISSION_PICKPOCKET (§6.12) — civ (Pickpocket skill scales), rob $10k from players
	// via /pickpocket (bespoke target-based). Start at LS Willowfield. mCheckpoints unused.
	{ "Pickpocket",		MISSION_TYPE_CIVILIAN,	MPICKPOCKET_COOLDOWN_HRS,1,	0,				MPICKPOCKET_COMPLETE_BONUS,	MPICKPOCKET_WANTED,	false,	true,	2450.0000, -1970.0000, 13.5500,	1 },
	// MISSION_SEXUAL_ENCOUNTER (§6.15) — innocent civ (Rapist/Prostitute scale), 5
	// encounters via /encounter (bespoke). Start at a LS sex shop. mCheckpoints unused.
	{ "Sexual Encounter",MISSION_TYPE_CIVILIAN,	MSEX_COOLDOWN_HRS,		1,	0,				MSEX_COMPLETE_BONUS,	0,				false,	true,	2093.0000, -1785.0000, 13.5500,	1 },
	// MISSION_TRACTOR (§6.16) — civ, needs a Tractor, wraps the /farmjob crop system at
	// CrazyBob's Farm (bespoke hand-off). The row carries marker + cooldown only.
	{ "Tractor Mission",MISSION_TYPE_CIVILIAN,	MTRACTOR_COOLDOWN_HRS,	1,	0,				0,				0,						true,	true,	FARM_X, FARM_Y, FARM_Z,	1 },
	// MISSION_FLOWER_DELIVERY (§6.5) — COP OR CIV, deliver flowers to 5 players via
	// /flowers (bespoke). Start at LS Jefferson. mType is civ but the entry gate is
	// relaxed for this row (Mission_Start special-cases it to allow cops too).
	{ "Flower Delivery",MISSION_TYPE_CIVILIAN,	MFLOWER_COOLDOWN_HRS,	1,	0,				MFLOWER_COMPLETE_BONUS,	0,				false,	true,	2179.0000, -1160.0000, 25.0000,	1 },
	// MISSION_RACE_CHALLENGE (§6.13) — all players, /challenge race framework (bespoke
	// race-CP route from g_RaceReg). The row carries marker + cooldown only.
	{ "Race Challenge",	MISSION_TYPE_CIVILIAN,	MRACE_COOLDOWN_HRS,		1,	0,				0,				0,						false,	false,	1960.0000, -1400.0000, 13.5000,	1 },
	// MISSION_ROUTINE_PATROL (§6.14, COP) — cop-only, 5 sequential PD checkpoints,
	// $2.5k/cp + $25k bonus. Start at LSPD HQ (§6.14 VERIFIED 1582.375,-1635.000).
	{ "Routine Patrol",	MISSION_TYPE_COP,		MPATROL_COOLDOWN_HRS,	5,	MPATROL_CP_PAY,	MPATROL_BONUS,	0,						true,	true,	1582.3750, -1635.0000, 13.5600,	10 },
	// MISSION_DOMESTIC_DISTURBANCE (§6.3, COP) — cop-only, drive PD → a player-owned
	// house (bespoke single-destination). Start at LSPD HQ. mCheckpoints unused.
	{ "Domestic Disturbance",MISSION_TYPE_COP,	MDOMESTIC_COOLDOWN_HRS,	1,	0,				MDOMESTIC_BASE_PAY,	0,					true,	true,	1582.3750, -1635.0000, 13.5600,	1 }
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
	},
	// MISSION_VEHICLE_THEFT — chop-shop delivery drops (industrial / garage yards).
	// Deliver each stolen car here; the final drop pays the bonus + forces WL10.
	{
		{ 1032.5000, -1032.0000, 32.1000 }, { 2645.0000, -2010.0000, 13.5500 }, { 1780.0000, -1900.0000, 13.5600 },
		{ 2200.0000, -1990.0000, 13.5500 }, { 1360.0000, -1780.0000, 13.3800 }, { 2490.0000, -1690.0000, 13.3200 },
		{ 1930.0000, -1080.0000, 25.0000 }, { 620.0000,  -1275.0000, 13.5800 }, { 2760.0000, -2450.0000, 13.6300 },
		{ 1080.0000, -1220.0000, 16.5000 }
	},
	// MISSION_HOLDUP_MISSION — store-holdup checkpoints (near 24-7 / food / gas store
	// clerks across LS). The route framework samples MHOLDUP_STORES of these on foot.
	{
		{ 1315.5000, -1279.0000, 13.5500 }, { 2421.0000, -1509.0000, 24.0000 }, { 1833.0000, -1842.0000, 13.5800 },
		{ 2159.0000, -1204.0000, 23.9000 }, { 1352.0000, -1758.0000, 13.5100 }, { 653.0000,  -1274.0000, 13.5800 },
		{ 2426.0000, -1969.0000, 13.5500 }, { 1930.0000, -1773.0000, 13.5500 }, { 2447.0000, -1959.0000, 13.5500 },
		{ 2109.0000, -1806.0000, 13.5500 }
	},
	// MISSION_AIRPORT_ROBBERY — bespoke box mechanic; the route CPs are unused. One
	// placeholder point (the mission never walks a race route). Kept valid for the array.
	{
		{ 1685.0000, -2333.0000, 13.5000 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 },
		{ 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 },
		{ 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 },
		{ 0.0, 0.0, 0.0 }
	},
	// ---- M6 (stage 4) — the final ~12 missions' route pools (append). -----------
	// MISSION_COURIER_DELIVERY — in-city delivery destinations (§6.2 "always in the city").
	{
		{ 1479.5000, -1755.9000, 12.5500 }, { 2178.5000, -1160.5000, 25.0000 }, { 1930.0000, -1780.0000, 13.3800 },
		{ 2494.0000, -1690.0000, 13.3200 }, { 1170.0000, -1300.0000, 13.5000 }, { 655.0000,  -1275.0000, 13.5800 },
		{ 2100.0000, -1170.0000, 24.9000 }, { 1560.0000, -1000.0000, 24.0000 }, { 1830.0000, -1950.0000, 13.4000 },
		{ 2600.0000, -1310.0000, 25.7000 }
	},
	// MISSION_HOUSE_DELIVERY — residential drop points across LS (stand-ins for houses).
	{
		{ 1298.0000, -730.0000, 92.0000 }, { 2496.0000, -1690.0000, 13.3200 }, { 1560.0000, -1360.0000, 13.5000 },
		{ 2145.0000, -1015.0000, 62.7000 }, { 1360.0000, -1520.0000, 13.5000 }, { 660.0000,  -1860.0000, 5.5000 },
		{ 2760.0000, -1620.0000, 12.2000 }, { 1470.0000, -1180.0000, 23.9000 }, { 1230.0000, -1360.0000, 13.5000 },
		{ 2410.0000, -1490.0000, 24.0000 }
	},
	// MISSION_ILLEGAL_IMMIGRANT — outskirts pickup then city return legs (last = drop).
	{
		{ 2748.0000, -1330.0000, 21.5000 }, { 2540.0000, -1310.0000, 25.7000 }, { 2280.0000, -1200.0000, 25.0000 },
		{ 2050.0000, -1170.0000, 24.9000 }, { 1830.0000, -1140.0000, 24.0000 }, { 1560.0000, -1360.0000, 13.5000 },
		{ 1360.0000, -1520.0000, 13.5000 }, { 1170.0000, -1300.0000, 13.5000 }, { 970.0000,  -1230.0000, 16.5000 },
		{ 655.0000,  -1275.0000, 13.5800 }
	},
	// MISSION_LAWN_MOWING — mow checkpoints around the LV golf course (Prickle Pine).
	{
		{ 1237.0000, 1834.0000, 8.5000 }, { 1200.0000, 1870.0000, 9.0000 }, { 1160.0000, 1900.0000, 9.5000 },
		{ 1120.0000, 1860.0000, 9.5000 }, { 1090.0000, 1820.0000, 9.5000 }, { 1130.0000, 1790.0000, 9.5000 },
		{ 1180.0000, 1770.0000, 9.0000 }, { 1230.0000, 1790.0000, 8.5000 }, { 1270.0000, 1830.0000, 8.5000 },
		{ 1250.0000, 1880.0000, 8.5000 }
	},
	// MISSION_PAPERBOY — house-front on-foot delivery checkpoints around Vinewood/LS.
	{
		{ 845.2500, -1042.0000, 22.5000 }, { 890.0000,  -1010.0000, 26.0000 }, { 930.0000,  -1050.0000, 26.0000 },
		{ 810.0000, -1080.0000, 25.0000 }, { 770.0000,  -1030.0000, 24.5000 }, { 880.0000,  -1120.0000, 25.5000 },
		{ 950.0000, -1120.0000, 26.0000 }, { 1000.0000, -1050.0000, 26.0000 }, { 820.0000,  -960.0000,  40.0000 },
		{ 760.0000, -1120.0000, 24.5000 }
	},
	// MISSION_PICKPOCKET — bespoke target-based; route CPs unused (placeholder point).
	{
		{ 2450.0000, -1970.0000, 13.5500 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 },
		{ 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 },
		{ 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 },
		{ 0.0, 0.0, 0.0 }
	},
	// MISSION_SEXUAL_ENCOUNTER — bespoke target-based; route CPs unused (placeholder).
	{
		{ 2093.0000, -1785.0000, 13.5500 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 },
		{ 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 },
		{ 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 },
		{ 0.0, 0.0, 0.0 }
	},
	// MISSION_TRACTOR — bespoke /farmjob hand-off; route CPs unused (placeholder point).
	{
		{ FARM_X, FARM_Y, FARM_Z }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 },
		{ 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 },
		{ 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 },
		{ 0.0, 0.0, 0.0 }
	},
	// MISSION_FLOWER_DELIVERY — bespoke target-based; route CPs unused (placeholder).
	{
		{ 2179.0000, -1160.0000, 25.0000 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 },
		{ 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 },
		{ 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 },
		{ 0.0, 0.0, 0.0 }
	},
	// MISSION_RACE_CHALLENGE — bespoke race route (from g_RaceReg); CPs unused here.
	{
		{ 1960.0000, -1400.0000, 13.5000 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 },
		{ 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 },
		{ 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 },
		{ 0.0, 0.0, 0.0 }
	},
	// MISSION_ROUTINE_PATROL — 5 sequential patrol checkpoints across LS (§6.14).
	{
		{ 1582.3750, -1635.0000, 13.5600 }, { 1360.0000, -1520.0000, 13.5000 }, { 2100.0000, -1170.0000, 24.9000 },
		{ 2494.0000, -1690.0000, 13.3200 }, { 1930.0000, -1780.0000, 13.3800 }, { 655.0000,  -1275.0000, 13.5800 },
		{ 2600.0000, -1310.0000, 25.7000 }, { 1170.0000, -1300.0000, 13.5000 }, { 1830.0000, -1950.0000, 13.4000 },
		{ 2178.5000, -1160.5000, 25.0000 }
	},
	// MISSION_DOMESTIC_DISTURBANCE — bespoke single-house destination; CPs unused.
	{
		{ 1582.3750, -1635.0000, 13.5600 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 },
		{ 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 },
		{ 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 },
		{ 0.0, 0.0, 0.0 }
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

// Airport Robbery (§6.1) — accepted haulers: trucks, vans and the garbage truck.
// "Truck, van, or garbage truck" (VERIFIED) — we accept the delivery-truck set,
// common vans (Boxville/Mule/Yankee/Benson/Burrito/Pony) and the Trashmaster.
static const MissionAirportModels[] =
{
	408,	// Trashmaster (garbage truck)
	498,	// Boxville (van)
	413,	// Pony (van)
	414,	// Mule (van/truck)
	456,	// Yankee (truck)
	499,	// Benson (truck)
	609		// Boxburg (van)
};

//==============================================================================
//	Vehicle Theft anti-farm state (module-static, per-player)
//==============================================================================

// Per-run set of vehicleids already delivered this Vehicle Theft run (one slot per
// checkpoint; -1 = empty). Blocks the 2-car-alternation farm across all 5 drops.
static g_VehTheftDelivered[MAX_PLAYERS][MISSION_MAX_CHECKPOINTS];

// Snapshot of vehicles the player owned (was last driver of) at Vehicle Theft start.
// Rejects delivering a car that was already theirs before the mission began (§6.19
// "cannot chop your own car"). Must snapshot at START, not at delivery time, because
// driving a stolen car to the checkpoint makes the player its last driver first.
static bool:g_VehTheftPreOwned[MAX_PLAYERS][MAX_VEHICLES + 1];

//==============================================================================
//	M6 (stage 4) target-mission dedup + Race Challenge registry
//==============================================================================

// Distinct-target set for the target-based missions (Flower Delivery §6.5, Pickpocket
// §6.12, Sexual Encounter §6.15). g_MissionTargetHit[actor][target] = already counted
// this target this run. Blocks farming the same player repeatedly for progress.
static bool:g_MissionTargetHit[MAX_PLAYERS][MAX_PLAYERS];

// ---- Race Challenge registry (§6.13) ----------------------------------------
// A race is DATA: name, category (land/bike/boat), a checkpoint route, a par time
// (real-seconds — beat it for a podium bonus), and a completion fee-exempt "first
// attempt free" flag handled per-player. The site lists 28 races; we ship a
// representative set spanning the categories. ADD MORE by appending rows here +
// growing g_RaceCount — no framework change needed.
// Pawn enum array members are single-dimension only, so the route is FLATTENED to
// MRACE_MAX_CHECKPOINTS*3 floats: checkpoint c's coords are indices c*3+0 (X), +1
// (Y), +2 (Z). Access via the Mission_RaceCP* helpers below.
enum e_RaceReg
{
	rName[40],					// display name (dialog + HUD)
	rCategory,					// MRACE_CATEGORY_LAND / _BIKE / _BOAT (vehicle gate)
	rCheckpoints,				// number of checkpoints in this race route
	rParSecs,					// real-seconds par — finish under it for the podium bonus
	Float:rRoute[MRACE_MAX_CHECKPOINTS * 3]	// flattened checkpoint coords (c*3 + {0,1,2})
}

// The shipped race set (representative of the 28 — see report). Land / Bike / Boat.
static const g_RaceReg[][e_RaceReg] =
{
	// Los Santos Circuit — a short city land circuit (§6.13 "Los Santos Circuit 3km").
	{ "Los Santos Circuit", MRACE_CATEGORY_LAND, 5, 150,
		{ 1960.0, -1400.0, 13.5,   2200.0, -1150.0, 25.0,   2490.0, -1690.0, 13.3,   1930.0, -1780.0, 13.4,   1560.0, -1360.0, 13.5,   0.0,0.0,0.0,   0.0,0.0,0.0,   0.0,0.0,0.0 } },
	// San Andreas Tour — a long cross-country land run (§6.13 "27.5km land").
	{ "San Andreas Tour", MRACE_CATEGORY_LAND, 6, 300,
		{ 1960.0, -1400.0, 13.5,   1170.0, -1300.0, 13.5,   655.0, -1275.0, 13.6,   1080.0, -2020.0, 13.0,   2170.0, -2280.0, 13.5,   2749.0, -2454.0, 13.6,   0.0,0.0,0.0,   0.0,0.0,0.0 } },
	// Los Santos Biker Run — bikes only (§6.13 "LV Biker Endurance 9km bikes").
	{ "Los Santos Biker Run", MRACE_CATEGORY_BIKE, 5, 170,
		{ 1960.0, -1400.0, 13.5,   1830.0, -1140.0, 24.0,   2100.0, -1170.0, 24.9,   2600.0, -1310.0, 25.7,   2494.0, -1690.0, 13.3,   0.0,0.0,0.0,   0.0,0.0,0.0,   0.0,0.0,0.0 } },
	// Coastal Boat Sprint — boats only (§6.13 "Boating With Jaws" boat category).
	{ "Coastal Boat Sprint", MRACE_CATEGORY_BOAT, 4, 200,
		{ 660.0, -1860.0, 0.0,   1080.0, -2450.0, 0.0,   1930.0, -2640.0, 0.0,   2760.0, -2450.0, 0.0,   0.0,0.0,0.0,   0.0,0.0,0.0,   0.0,0.0,0.0,   0.0,0.0,0.0 } }
};

//==============================================================================
//	Internal helpers
//==============================================================================

// The monotonic game-hour "clock" cooldowns are stamped against. g_GameDayCounter
// is monotonic (never resets on the game-week rollover), so this never rewinds.
static Mission_NowGameHour()
	return (g_GameDayCounter * MISSION_HOURS_PER_DAY) + GameHour;

// Is a cop within `range` of the player (same interior/VW)? The generic form the
// M5/M6 smuggling missions reuse — no name-based logic, pure team + proximity.
static Mission_IsCopInSightRange(playerid, Float:range)
{
	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(i == playerid || !IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(!IsCop(i)) continue;
		if(GetPlayerInterior(i) != GetPlayerInterior(playerid)) continue;
		if(GetPlayerVirtualWorld(i) != GetPlayerVirtualWorld(playerid)) continue;
		if(IsPlayerInRangeOfPoint(i, range, x, y, z)) return 1;
	}
	return 0;
}

// Is a cop within the default sight range of the player? Used for the Drug Delivery
// wanted tick (§6.4) and the House Delivery illegal-drop check (§6.8).
static Mission_IsCopInSight(playerid)
	return Mission_IsCopInSightRange(playerid, MISSION_COPSIGHT_RANGE);

// Is it currently "night" on the game clock? Illegal Immigrant Transport (§6.9) uses
// this to reduce cop visibility (cops effectively see less at night).
static Mission_IsNight()
	return (GameHour >= MIMMIG_NIGHT_START || GameHour < MIMMIG_NIGHT_END);

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
		case MISSION_VEHICLE_THEFT:
		{
			// Deliver whatever car you stole — any land vehicle qualifies (§6.19).
			return Mission_IsLandVehicle(model);
		}
		case MISSION_AIRPORT_ROBBERY:
		{
			// Truck / van / garbage truck only (§6.1 VERIFIED).
			for(new i = 0; i < sizeof(MissionAirportModels); i++)
				if(MissionAirportModels[i] == model) return 1;
			return 0;
		}
		// ---- M6 (stage 4) vehicle-gated missions --------------------------------
		case MISSION_COURIER_DELIVERY, MISSION_HOUSE_DELIVERY, MISSION_ILLEGAL_IMMIGRANT, MISSION_ROUTINE_PATROL:
		{
			// Any land vehicle (§6.2/§6.8/§6.9 "any vehicle"; §6.14 land only).
			return Mission_IsLandVehicle(model);
		}
		case MISSION_LAWN_MOWING:
		{
			return (model == MLAWN_MOWER_MODEL);		// Mower only (§6.10 VERIFIED)
		}
		case MISSION_TRACTOR:
		{
			return (model == MTRACTOR_MODEL);			// Tractor only (§6.16 VERIFIED)
		}
		case MISSION_DOMESTIC_DISTURBANCE:
		{
			return Mission_IsLandVehicle(model);		// cop responds by land vehicle (§6.3)
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
	// Airport Robbery owns bespoke session state (box pickups, phase, deadline,
	// its own checkpoint) — tear it down BEFORE pOnMission is reset (§6.1).
	if(PlayerInfo[playerid][pOnMission] == MISSION_AIRPORT_ROBBERY)
		Mission_AirportCleanup(playerid);
	// Domestic Disturbance uses a standard SetPlayerCheckpoint (single destination) —
	// disable it on teardown so the marker never lingers (§6.3).
	if(PlayerInfo[playerid][pOnMission] == MISSION_DOMESTIC_DISTURBANCE && IsPlayerConnected(playerid))
		DisablePlayerCheckpoint(playerid);

	if(IsPlayerConnected(playerid)) DisablePlayerRaceCheckpoint(playerid);
	PlayerInfo[playerid][pOnMission] = -1;
	PlayerInfo[playerid][pMissionCheckpoint] = 0;
	PlayerInfo[playerid][pMissionRouteLen] = 0;
	PlayerInfo[playerid][pMissionVehicle] = -1;
	PlayerInfo[playerid][pMissionData] = 0;
	// M6 (stage 4) bespoke mission scratch.
	PlayerInfo[playerid][pMissionDeadline] = 0;
	PlayerInfo[playerid][pMissionStartHour] = 0;
	PlayerInfo[playerid][pMissionTargetCd] = 0;
	PlayerInfo[playerid][pMissionRaceId] = -1;
	PlayerInfo[playerid][pMissionDestX] = PlayerInfo[playerid][pMissionDestY] = PlayerInfo[playerid][pMissionDestZ] = 0.0;
	for(new i = 0; i < MISSION_MAX_CHECKPOINTS; i++)
	{
		PlayerInfo[playerid][pMissionRouteX][i] = 0.0;
		PlayerInfo[playerid][pMissionRouteY][i] = 0.0;
		PlayerInfo[playerid][pMissionRouteZ][i] = 0.0;
		g_VehTheftDelivered[playerid][i] = -1;	// M5 Vehicle Theft dedup — clear on any mission end
	}
	// Clear the pre-owned snapshot so it does not leak into a later run.
	for(new v = 0; v <= MAX_VEHICLES; v++) g_VehTheftPreOwned[playerid][v] = false;
	// Clear the target-mission distinct-target dedup set (Flower/Pickpocket/Sexual).
	for(new t = 0; t < MAX_PLAYERS; t++) g_MissionTargetHit[playerid][t] = false;
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

	// Civ/cop entry gate. Flower Delivery (§6.5) is the one CIVILIAN-typed row open to
	// BOTH cops and civilians (VERIFIED) — exempt it from the "cops can't take civ jobs" rule.
	if(MissionReg[missionId][mType] == MISSION_TYPE_COP && !IsCop(playerid))
		return SendClientMessage(playerid, COLOR_ERROR, "That mission is for law enforcement only.");
	if(MissionReg[missionId][mType] == MISSION_TYPE_CIVILIAN && IsCop(playerid) && missionId != MISSION_FLOWER_DELIVERY)
		return SendClientMessage(playerid, COLOR_ERROR, "Law enforcement cannot take civilian jobs.");

	// Drug Delivery requires an innocent (WL0) start (§6.4 VERIFIED). Paperboy (§6.11),
	// Illegal Immigrant Transport (§6.9) and Sexual Encounter (§6.15) are also innocent-only.
	if((missionId == MISSION_DRUG_DELIVERY || missionId == MISSION_PAPERBOY || missionId == MISSION_ILLEGAL_IMMIGRANT || missionId == MISSION_SEXUAL_ENCOUNTER) && GetPlayerWanted(playerid) > 0)
		return SendClientMessage(playerid, COLOR_ERROR, "You must be innocent (no wanted level) to start that mission.");

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
			case MISSION_VEHICLE_THEFT: format(err, sizeof(err), "You must be driving a (stolen) land vehicle to deliver it — steal a car first, then start the mission.");
			case MISSION_AIRPORT_ROBBERY: format(err, sizeof(err), "You must be in a truck, van or garbage truck at the airport to start the Airport Robbery.");
			case MISSION_COURIER_DELIVERY: format(err, sizeof(err), "You must be driving a land vehicle to pick up the courier cargo.");
			case MISSION_HOUSE_DELIVERY: format(err, sizeof(err), "You must be driving a land vehicle to load the house deliveries.");
			case MISSION_ILLEGAL_IMMIGRANT: format(err, sizeof(err), "You must be driving a land vehicle to transport the immigrants.");
			case MISSION_LAWN_MOWING: format(err, sizeof(err), "You must be on a Lawn Mower at the golf course to start Lawn Mowing.");
			case MISSION_TRACTOR: format(err, sizeof(err), "You must be on a Tractor at CrazyBob's Farm to start the Tractor Mission.");
			case MISSION_ROUTINE_PATROL: format(err, sizeof(err), "You must be in a police land vehicle to start Routine Patrol.");
			case MISSION_DOMESTIC_DISTURBANCE: format(err, sizeof(err), "You must be in a police land vehicle to respond to a Domestic Disturbance.");
			default: format(err, sizeof(err), "You need the right vehicle for that mission.");
		}
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}

	// All gates passed — start it. Bind the mission vehicle (for the exit abort).
	PlayerInfo[playerid][pOnMission] = missionId;
	PlayerInfo[playerid][pMissionCheckpoint] = 0;
	PlayerInfo[playerid][pMissionData] = 0;
	// Vehicle Theft delivers 5 DIFFERENT cars, so it is NOT bound to one vehicle
	// (pMissionVehicle is its last-delivered-car dedupe slot, starting at -1). Other
	// vehicle missions bind the single mission vehicle for the leave-vehicle abort.
	if(missionId == MISSION_VEHICLE_THEFT)
	{
		PlayerInfo[playerid][pMissionVehicle] = -1;
		// Clear the full-run delivered-set (all slots to -1 = empty).
		for(new i = 0; i < MISSION_MAX_CHECKPOINTS; i++) g_VehTheftDelivered[playerid][i] = -1;
		// Snapshot which vehicles this player already owns NOW, before the mission. The
		// own-car rule (§6.19) uses this snapshot, not Veh_IsOwnedBy at delivery time
		// (by then the player is last-driver of each delivered car, making every drop
		// look "owned"). Only vehicles owned BEFORE start are blocked.
		for(new v = 0; v <= MAX_VEHICLES; v++)
			g_VehTheftPreOwned[playerid][v] = Veh_IsOwnedBy(playerid, v) ? true : false;
	}
	else PlayerInfo[playerid][pMissionVehicle] = MissionReg[missionId][mNeedsVehicle] ? GetPlayerVehicleID(playerid) : -1;

	// Airport Robbery is a BESPOKE box-collection mechanic, not a race-CP route
	// (§6.1 — MMB / /box). Hand off to its custom starter and skip the route build.
	if(missionId == MISSION_AIRPORT_ROBBERY)
		return Mission_AirportStart(playerid);

	// ---- M6 (stage 4) bespoke hand-offs (not standard race-CP routes) -----------
	switch(missionId)
	{
		// Tractor Mission (§6.16) wraps the /farmjob crop system at CrazyBob's Farm. The
		// farm job owns its own field-CP loop; we just launch it and detach the mission
		// slot (the cooldown was validated above; stamp it now so it throttles as designed).
		case MISSION_TRACTOR:
		{
			PlayerInfo[playerid][pOnMission] = -1;	// detach — the farm job runs independently
			PlayerInfo[playerid][pMissionVehicle] = -1;
			Mission_SetCooldown(playerid, MISSION_TRACTOR);
			SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_GREEN"* Tractor Mission "COL_WHITE"— running CrazyBob's Farm crop route. Drive the field checkpoints; "COL_YELLOW"/cancelfarmjob"COL_WHITE" to stop.");
			CallLocalFunction("Farm_StartTractorJob", "d", playerid);
			return 1;
		}
		case MISSION_PAPERBOY:			return Mission_PaperboyStart(playerid);
		case MISSION_PICKPOCKET:		return Mission_TargetStart(playerid, MISSION_PICKPOCKET);
		case MISSION_SEXUAL_ENCOUNTER:	return Mission_TargetStart(playerid, MISSION_SEXUAL_ENCOUNTER);
		case MISSION_FLOWER_DELIVERY:	return Mission_TargetStart(playerid, MISSION_FLOWER_DELIVERY);
		case MISSION_RACE_CHALLENGE:	return Mission_RaceLauncher(playerid);
		case MISSION_DOMESTIC_DISTURBANCE:	return Mission_DomesticStart(playerid);
	}

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
	// Vehicle Theft — warn about the last-car WL10 rule (§6.19 CRITICAL warning).
	if(missionId == MISSION_VEHICLE_THEFT)
	{
		SendClientMessage(playerid, COLOR_ERROR, ""COL_RED"Steal each marked-type car and deliver it to the chop-shop. "COL_WHITE"Delivering the "COL_YELLOW"LAST"COL_WHITE" car instantly makes you "COL_RED"Most Wanted (WL10)"COL_WHITE" — save your fastest car for last.");
	}
	// Holdup Mission — warn about the escalating warrant across 3 stores (§6.7).
	if(missionId == MISSION_HOLDUP_MISSION)
	{
		new msg[160];
		format(msg, sizeof(msg), ""COL_RED"Rob "COL_WHITE"%d"COL_RED" stores on foot (min "COL_GREEN"$%d"COL_RED" each). Each holdup adds "COL_WHITE"+%d wanted"COL_RED" — expect heavy police attention.", MHOLDUP_STORES, MHOLDUP_MIN_PER_STORE, MHOLDUP_WANTED_PER_CP);
		SendClientMessage(playerid, COLOR_ERROR, msg);
	}
	// ---- M6 (stage 4) route-mission start warnings ------------------------------
	// Courier Delivery — smuggling warrant on pickup (§6.2), applied right now at start.
	if(missionId == MISSION_COURIER_DELIVERY)
	{
		GivePlayerWanted(playerid, MCOURIER_WANTED_START, "Courier smuggling (pickup)");
		SendClientMessage(playerid, COLOR_ERROR, ""COL_RED"You picked up smuggled cargo — you have a "COL_WHITE"smuggling warrant"COL_RED". Police will try to intercept. Deliver into the city.");
	}
	// House Delivery — some cargo is illegal (§6.8): +wanted on an illegal drop near a cop.
	if(missionId == MISSION_HOUSE_DELIVERY)
		SendClientMessage(playerid, COLOR_ERROR, ""COL_RED"Some of these deliveries are "COL_WHITE"illegal goods"COL_RED" — making an illegal drop within sight of a cop raises your wanted level.");
	// Illegal Immigrant — wanted issued ON COMPLETION, not pickup (§6.9). Night hides cops.
	if(missionId == MISSION_ILLEGAL_IMMIGRANT)
		SendClientMessage(playerid, COLOR_ERROR, ""COL_RED"Stay clear of police while transporting — you only receive a wanted level "COL_WHITE"on completion"COL_RED". Cops see less at night.");
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

	// Illegal Immigrant Transport (§6.9) — the wanted level is issued ON COMPLETION
	// (successful drop-off), NOT on pickup (VERIFIED). Apply it here, before cleanup.
	if(missionId == MISSION_ILLEGAL_IMMIGRANT)
	{
		GivePlayerWanted(playerid, MIMMIG_WANTED_COMPLETE, "Transported illegal immigrants");
		SendClientMessage(playerid, COLOR_ERROR, ""COL_RED"Delivery done — but transporting illegals earned you a "COL_WHITE"+"#MIMMIG_WANTED_COMPLETE" wanted level"COL_RED".");
	}

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

	// Race Challenge (§6.13) rides the race-CP callback too — its scoring is bespoke
	// (par-time / podium), so dispatch and return before the delivery-route logic.
	if(missionId == MISSION_RACE_CHALLENGE)
		return Mission_RaceOnCheckpoint(playerid);

	// Vehicle Theft is special: each of the 5 drops must be a DIFFERENT stolen car
	// (§6.19 "5 randomly-assigned vehicles"), so it is NOT bound to one vehicle. The
	// drop is scored only if the player is driving a land vehicle that is not the car
	// they just delivered, and the delivered car must NOT be one they already owned
	// before the mission (Veh_IsOwnedBy is reused — you cannot chop your own car).
	if(missionId == MISSION_VEHICLE_THEFT)
	{
		if(!IsPlayerInAnyVehicle(playerid) || GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
			return SendClientMessage(playerid, COLOR_ERROR, "Drive a stolen car into the chop-shop checkpoint to deliver it.");
		new veh = GetPlayerVehicleID(playerid);
		if(!Mission_IsLandVehicle(GetVehicleModel(veh)))
			return SendClientMessage(playerid, COLOR_ERROR, "Only land vehicles can be delivered to the chop-shop.");
		// §6.19 — cannot chop a car you already owned before the mission started.
		if(g_VehTheftPreOwned[playerid][veh])
			return SendClientMessage(playerid, COLOR_ERROR, "You can't chop a car you already owned — steal a DIFFERENT vehicle to deliver.");
		// Reject any car already delivered this run (full-set scan, not just the last
		// one — blocks the 2-car-alternation farm across all 5 drops).
		for(new i = 0; i < PlayerInfo[playerid][pMissionCheckpoint]; i++)
		{
			if(g_VehTheftDelivered[playerid][i] == veh)
				return SendClientMessage(playerid, COLOR_ERROR, "You already delivered this car — bring a DIFFERENT stolen vehicle.");
		}
		// Record this delivery in the set (pMissionCheckpoint is incremented AFTER this
		// block, so it correctly indexes 0..len-1 as the write slot here).
		g_VehTheftDelivered[playerid][PlayerInfo[playerid][pMissionCheckpoint]] = veh;
	}
	// Other vehicle missions: must still be in the ONE bound mission vehicle to score.
	else if(MissionReg[missionId][mNeedsVehicle])
	{
		if(!IsPlayerInAnyVehicle(playerid) || GetPlayerVehicleID(playerid) != PlayerInfo[playerid][pMissionVehicle])
		{
			SendClientMessage(playerid, COLOR_ERROR, "Get back in your mission vehicle to make the drop.");
			return 0;
		}
	}

	// Score this checkpoint.
	PlayerInfo[playerid][pMissionCheckpoint]++;
	new bool:isFinal = (PlayerInfo[playerid][pMissionCheckpoint] >= PlayerInfo[playerid][pMissionRouteLen]);
	new pay = MissionReg[missionId][mCpPay];

	// Mission-specific per-checkpoint PAY adjustment (small switch — extension point).
	switch(missionId)
	{
		case MISSION_HOLDUP_MISSION:
		{
			// Each store yields at least the $10k floor + a random register bonus
			// (§6.7 "min $10k each"; the take mirrors the /holdup register band).
			pay = MHOLDUP_CP_PAY + random(MHOLDUP_CP_BONUS_MAX + 1);
		}
	}

	if(pay > 0) Bank_GiveCash(playerid, pay);
	PlayerInfo[playerid][pMissionData] += pay;
	PlayerInfo[playerid][pScore] += 1;	// +1 score per checkpoint (§6 Score Guide)

	new string[160];
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
		case MISSION_VEHICLE_THEFT:
		{
			// §6.19 CRITICAL: the LAST car forces instant WL10 (Most Wanted). Prior cars
			// raise wanted moderately. Delivering the final (5th) car IS "entering the
			// last vehicle on the list" — we apply the max wanted on that final drop.
			if(isFinal)
			{
				SetPlayerWanted(playerid, MAX_WANTED, "Grand theft auto — Vehicle Theft final car");
				SendClientMessage(playerid, COLOR_ERROR, ""COL_RED"* LAST CAR DELIVERED — you are now "COL_WHITE"MOST WANTED (WL10)"COL_RED"! Every cop is coming for you.");
			}
			else GivePlayerWanted(playerid, MissionReg[missionId][mWantedPerCp], "Vehicle theft delivery");
		}
		case MISSION_HOLDUP_MISSION:
		{
			// +7 wanted per store holdup → cumulative high warrant across 3 (§6.7 / §5.2).
			GivePlayerWanted(playerid, MissionReg[missionId][mWantedPerCp], "Store holdup (mission)");
			// Record a holdup on the robbery history so the crowbar is confiscated on a
			// later arrest (robbery.inc owns the bitmask; reached via CallLocalFunction).
			CallLocalFunction("Robbery_MarkHoldupHistory", "d", playerid);
		}
		case MISSION_HOUSE_DELIVERY:
		{
			// §6.8 — some deliveries carry illegal cargo. A random subset of drops are
			// "illegal"; making one within sight of a cop raises wanted (no warrant unless
			// seen). Roll per-drop so each run's risk varies.
			if(random(100) < MHOUSEDEL_ILLEGAL_PCT && Mission_IsCopInSight(playerid))
			{
				GivePlayerWanted(playerid, MHOUSEDEL_WANTED_PER_CP, "Illegal house delivery seen by police");
				SendClientMessage(playerid, COLOR_ERROR, ""COL_RED"That parcel was contraband and a cop saw the drop — "COL_WHITE"+"#MHOUSEDEL_WANTED_PER_CP" wanted"COL_RED".");
			}
		}
		case MISSION_ILLEGAL_IMMIGRANT:
		{
			// §6.9 — wanted is issued ON COMPLETION (the final drop), NOT on pickup. The
			// final-drop warrant is applied in Mission_Complete's switch. Mid-route: a
			// stealth penalty only if a cop sees you AND it is day (night hides cops).
			if(!isFinal && !Mission_IsNight() && Mission_IsCopInSightRange(playerid, MIMMIG_COPSIGHT_DAY))
			{
				GivePlayerWanted(playerid, MIMMIG_WANTED_ENROUTE, "Spotted transporting illegal immigrants");
				SendClientMessage(playerid, COLOR_ERROR, ""COL_RED"A cop spotted your cargo in daylight — "COL_WHITE"+"#MIMMIG_WANTED_ENROUTE" wanted"COL_RED". Move at night to stay unseen.");
			}
		}
	}

	// Final checkpoint? Complete; else advance the route.
	if(isFinal)
		Mission_Complete(playerid);
	else
		Mission_NextCheckpoint(playerid);
	return 1;
}

//==============================================================================
//	Airport Robbery — bespoke box-collection mechanic (§6.1)
//==============================================================================
//	NOT a race-CP route. On start: WL7, a collect timer, and MAIRPORT_BOX_COUNT box
//	pickups scatter around the airport apron. The robber exits the truck/van and
//	grabs boxes on foot (MMB / /box) — each box banks nothing yet but raises the
//	delivery payout. When the collect timer expires, a hideout checkpoint appears;
//	drive the loot there within the deliver window to be paid (boxes → payout). The
//	whole state lives in the pAirport* session fields + the per-player box arrays.
//==============================================================================

// Per-player box pickup handles + a collected flag + the box coords (kept locally
// so the collect check needs no streamer position query) for the current run.
static STREAMER_TAG_PICKUP:g_AirportBox[MAX_PLAYERS][MAIRPORT_BOX_COUNT];
static bool:g_AirportBoxTaken[MAX_PLAYERS][MAIRPORT_BOX_COUNT];
static Float:g_AirportBoxPos[MAX_PLAYERS][MAIRPORT_BOX_COUNT][3];

// The airport apron centre the boxes scatter around (LS International — near the
// mission start marker). Boxes spawn in a MAIRPORT_BOX_SPREAD ring around this.
#define MAIRPORT_APRON_X		(1685.0)
#define MAIRPORT_APRON_Y		(-2333.0)
#define MAIRPORT_APRON_Z		(13.5)

// The hideout the loot is driven to after the collect timer (LS El Corona docks).
#define MAIRPORT_HIDEOUT_X		(2790.0)
#define MAIRPORT_HIDEOUT_Y		(-2415.0)
#define MAIRPORT_HIDEOUT_Z		(13.6)

// Destroy all of a player's box pickups + clear the box arrays. Safe to call twice.
FUNCTION Mission_AirportCleanup(playerid)
{
	for(new i = 0; i < MAIRPORT_BOX_COUNT; i++)
	{
		if(g_AirportBox[playerid][i] != STREAMER_TAG_PICKUP:INVALID_STREAMER_ID)
		{
			DestroyDynamicPickup(g_AirportBox[playerid][i]);
			g_AirportBox[playerid][i] = STREAMER_TAG_PICKUP:INVALID_STREAMER_ID;
		}
		g_AirportBoxTaken[playerid][i] = false;
	}
	if(IsPlayerConnected(playerid)) DisablePlayerCheckpoint(playerid);
	PlayerInfo[playerid][pAirportPhase] = AIRPHASE_NONE;
	PlayerInfo[playerid][pAirportBoxes] = 0;
	PlayerInfo[playerid][pAirportDeadline] = 0;
	PlayerInfo[playerid][pAirportVehicle] = -1;
	PlayerInfo[playerid][pAirportSite] = 0;
	return 1;
}

// Start the airport box run (called from Mission_Start after all gates pass). The
// caller already set pOnMission + pMissionVehicle. WL7, scatter boxes, arm the
// collect timer. Returns 1.
FUNCTION Mission_AirportStart(playerid)
{
	PlayerInfo[playerid][pAirportPhase] = AIRPHASE_COLLECT;
	PlayerInfo[playerid][pAirportBoxes] = 0;
	PlayerInfo[playerid][pAirportVehicle] = GetPlayerVehicleID(playerid);
	PlayerInfo[playerid][pAirportDeadline] = gettime() + MAIRPORT_COLLECT_SECS;

	// Scatter the box pickups in a ring around the apron.
	new spread = floatround(MAIRPORT_BOX_SPREAD);
	for(new i = 0; i < MAIRPORT_BOX_COUNT; i++)
	{
		new Float:bx = MAIRPORT_APRON_X + float(random(2 * spread) - spread);
		new Float:by = MAIRPORT_APRON_Y + float(random(2 * spread) - spread);
		new Float:bz = MAIRPORT_APRON_Z;
		g_AirportBoxPos[playerid][i][0] = bx;
		g_AirportBoxPos[playerid][i][1] = by;
		g_AirportBoxPos[playerid][i][2] = bz;
		g_AirportBox[playerid][i] = CreateDynamicPickup(MAIRPORT_BOX_PICKUP_MODEL, 1,
			bx, by, bz, -1, -1, playerid, 100.0);
		g_AirportBoxTaken[playerid][i] = false;
	}

	// Wanted while collecting (§6.1) — WL7.
	GivePlayerWanted(playerid, MAIRPORT_WANTED_START, "Airport robbery");

	new msg[192];
	format(msg, sizeof(msg), ""COL_GREEN"* Airport Robbery started! "COL_WHITE"Get out and grab boxes with "COL_YELLOW"/box"COL_WHITE" (or MMB) — you have "COL_YELLOW"%d seconds"COL_WHITE". The more boxes, the bigger the payout ("COL_GREEN"$%d"COL_WHITE"/box).",
		MAIRPORT_COLLECT_SECS, MAIRPORT_PAY_PER_BOX);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	SendClientMessage(playerid, COLOR_ERROR, ""COL_RED"You are WANTED while collecting — cops will intervene. "COL_YELLOW"/quitmission"COL_WHITE" to bail out.");
	GameTextForPlayer(playerid, "~r~Airport Robbery~n~~w~collect the boxes!", 4000, 5);

	// Nearby cops get a heads-up.
	new string[160], zoneName[MAX_ZONE_NAME];
	if(isNull(PlayerInfo[playerid][pZoneName])) format(zoneName, sizeof(zoneName), "the airport");
	else format(zoneName, sizeof(zoneName), "%s", PlayerInfo[playerid][pZoneName]);
	format(string, sizeof(string), ""COL_ORANGE"[AIRPORT ROBBERY] "COL_WHITE"%s (%d) is looting cargo at %s! Respond!", PlayerInfo[playerid][pUserName], playerid, zoneName);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || i == playerid) continue;
		if(IsCop(i)) SendClientMessage(i, COLOR_WANTED_ORANGE, string);
	}
	return 1;
}

// Collect the nearest uncollected box in reach (dispatched from /box or the MMB
// key). Returns 1 if a box was grabbed, 0 otherwise. On-foot only, collect phase.
FUNCTION Mission_AirportCollectBox(playerid)
{
	if(PlayerInfo[playerid][pOnMission] != MISSION_AIRPORT_ROBBERY) return 0;
	if(PlayerInfo[playerid][pAirportPhase] != AIRPHASE_COLLECT) return 0;
	if(IsPlayerInAnyVehicle(playerid)) return 0;	// must be on foot to grab a box
	if(gettime() < PlayerInfo[playerid][pBoxCollectCooldown]) return 0;

	new best = -1, Float:bestDist = MAIRPORT_BOX_RANGE + 1.0;
	for(new i = 0; i < MAIRPORT_BOX_COUNT; i++)
	{
		if(g_AirportBoxTaken[playerid][i]) continue;
		if(g_AirportBox[playerid][i] == STREAMER_TAG_PICKUP:INVALID_STREAMER_ID) continue;
		new Float:bx = g_AirportBoxPos[playerid][i][0];
		new Float:by = g_AirportBoxPos[playerid][i][1];
		new Float:bz = g_AirportBoxPos[playerid][i][2];
		if(!IsPlayerInRangeOfPoint(playerid, MAIRPORT_BOX_RANGE, bx, by, bz)) continue;
		new Float:d = GetPlayerDistanceFromPoint(playerid, bx, by, bz);
		if(d < bestDist) { bestDist = d; best = i; }
	}
	if(best == -1)
	{
		SendClientMessage(playerid, COLOR_ERROR, "There is no box within reach — walk onto a box marker first.");
		return 0;
	}

	// Grab it: remove the pickup, mark taken, bump the count.
	g_AirportBoxTaken[playerid][best] = true;
	DestroyDynamicPickup(g_AirportBox[playerid][best]);
	g_AirportBox[playerid][best] = STREAMER_TAG_PICKUP:INVALID_STREAMER_ID;
	PlayerInfo[playerid][pAirportBoxes]++;
	PlayerInfo[playerid][pBoxCollectCooldown] = gettime() + 1;	// tiny anti-spam gap

	new msg[128];
	format(msg, sizeof(msg), ""COL_GREEN"* Box collected! "COL_WHITE"%d/%d "COL_GREEN"(worth ~$%d so far). "COL_WHITE"Grab more before the timer runs out.",
		PlayerInfo[playerid][pAirportBoxes], MAIRPORT_BOX_COUNT, PlayerInfo[playerid][pAirportBoxes] * MAIRPORT_PAY_PER_BOX);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}

// Move to the deliver phase: destroy any remaining boxes, drop a hideout checkpoint,
// arm the deliver window.
static Mission_AirportBeginDeliver(playerid)
{
	for(new i = 0; i < MAIRPORT_BOX_COUNT; i++)
	{
		if(g_AirportBox[playerid][i] != STREAMER_TAG_PICKUP:INVALID_STREAMER_ID)
		{
			DestroyDynamicPickup(g_AirportBox[playerid][i]);
			g_AirportBox[playerid][i] = STREAMER_TAG_PICKUP:INVALID_STREAMER_ID;
		}
	}
	PlayerInfo[playerid][pAirportPhase] = AIRPHASE_DELIVER;
	PlayerInfo[playerid][pAirportDeadline] = gettime() + MAIRPORT_HIDEOUT_SECS;
	SetPlayerCheckpoint(playerid, MAIRPORT_HIDEOUT_X, MAIRPORT_HIDEOUT_Y, MAIRPORT_HIDEOUT_Z, MAIRPORT_HIDEOUT_CP_SIZE);

	new msg[176];
	format(msg, sizeof(msg), ""COL_GREEN"* Time's up! "COL_WHITE"You grabbed "COL_YELLOW"%d"COL_WHITE" box(es). Get back in your vehicle and reach the "COL_YELLOW"hideout"COL_WHITE" within "COL_YELLOW"%d seconds"COL_WHITE" to cash out.",
		PlayerInfo[playerid][pAirportBoxes], MAIRPORT_HIDEOUT_SECS);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	GameTextForPlayer(playerid, "~y~Drive to the hideout!", 4000, 5);
}

// Pay out the airport run on hideout delivery (boxes → payout, §6.1), stamp the
// cooldown, and clean up. No boxes = a token failure (nothing to fence).
static Mission_AirportDeliver(playerid)
{
	new boxes = PlayerInfo[playerid][pAirportBoxes];
	new pay = boxes * MAIRPORT_PAY_PER_BOX;
	if(pay > 0) pay += MAIRPORT_BONUS;		// flat completion bonus on top when something was hauled

	if(pay > 0) Bank_GiveCash(playerid, pay);
	PlayerInfo[playerid][pScore] += 1;		// +1 per completed robbery (§6.1 Score Guide)

	// Stamp the cooldown BEFORE clearing state (mirrors Mission_Complete).
	Mission_SetCooldown(playerid, MISSION_AIRPORT_ROBBERY);

	new msg[192];
	if(pay > 0) format(msg, sizeof(msg), ""COL_GREEN"* Airport Robbery COMPLETE! "COL_WHITE"You fenced "COL_YELLOW"%d"COL_WHITE" box(es) for "COL_GREEN"$%d"COL_WHITE" (incl. "COL_GREEN"$%d"COL_WHITE" bonus). +1 score.", boxes, pay, MAIRPORT_BONUS);
	else format(msg, sizeof(msg), ""COL_RED"* You reached the hideout with no boxes "COL_WHITE"— nothing to fence. +1 score.");
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	GameTextForPlayer(playerid, "~g~Robbery complete!", 4000, 4);

	Mission_ClearState(playerid);	// tears down airport state + resets pOnMission
	return 1;
}

// Per-player airport tick (dispatched from Mission_OnTick). Drives the collect /
// deliver phase deadlines, the vehicle guard, and the HUD.
FUNCTION Mission_AirportTick(playerid)
{
	new phase = PlayerInfo[playerid][pAirportPhase];
	if(phase == AIRPHASE_NONE) return 0;

	new left = PlayerInfo[playerid][pAirportDeadline] - gettime();

	if(phase == AIRPHASE_COLLECT)
	{
		// Collect window expired → move to the deliver phase.
		if(left <= 0) { Mission_AirportBeginDeliver(playerid); return 1; }
		// All boxes grabbed early → skip straight to delivery.
		if(PlayerInfo[playerid][pAirportBoxes] >= MAIRPORT_BOX_COUNT) { Mission_AirportBeginDeliver(playerid); return 1; }

		new gt[96];
		format(gt, sizeof(gt), "~r~AIRPORT ROBBERY~n~~w~boxes: ~y~%d~w~/%d   ~w~time: ~y~%d", PlayerInfo[playerid][pAirportBoxes], MAIRPORT_BOX_COUNT, left);
		GameTextForPlayer(playerid, gt, 1200, 5);
		return 1;
	}

	// Deliver phase.
	if(left <= 0)
	{
		// Blew the deliver window — the loot is lost (no payout, no cooldown stamp so
		// the player may retry once the mission CD elapses; abort message via Abort).
		Mission_Abort(playerid, "you ran out of time to reach the hideout");
		return 1;
	}
	// Reached the hideout (on foot or by vehicle — the truck may be abandoned)?
	if(IsPlayerInRangeOfPoint(playerid, MAIRPORT_HIDEOUT_CP_SIZE, MAIRPORT_HIDEOUT_X, MAIRPORT_HIDEOUT_Y, MAIRPORT_HIDEOUT_Z))
	{
		Mission_AirportDeliver(playerid);
		return 1;
	}
	new gt[80];
	format(gt, sizeof(gt), "~y~DELIVER LOOT~n~~w~boxes: ~y~%d   ~w~time: ~y~%d", PlayerInfo[playerid][pAirportBoxes], left);
	GameTextForPlayer(playerid, gt, 1200, 5);
	return 1;
}

//==============================================================================
//	M6 (stage 4) — Paperboy: ON FOOT, 4-MINUTE REAL-TIME limit (§6.11)
//==============================================================================
//	A race-CP route on foot, but the run is bounded by a REAL-TIME deadline
//	(gettime) on the shared tick, NOT the game-hour cooldown clock. Each drop pays
//	$500; when the timer expires (or all drops are made) the run ends and pays the
//	completion bonus scaled by deliveries. Reuses the route-CP machinery for markers.
//==============================================================================

FUNCTION Mission_PaperboyStart(playerid)
{
	Mission_BuildRoute(playerid, MISSION_PAPERBOY);
	Mission_PlaceRaceCP(playerid);
	Mission_ShowHUD(playerid);
	PlayerInfo[playerid][pMissionDeadline] = gettime() + MPAPERBOY_TIME_SECS;	// 4-min real-time

	new msg[192];
	format(msg, sizeof(msg), ""COL_GREEN"* Paperboy started! "COL_WHITE"Toss papers at the house checkpoints ON FOOT — "COL_GREEN"$%d"COL_WHITE"/delivery. You have "COL_YELLOW"%d seconds"COL_WHITE" (real time)!", MPAPERBOY_CP_PAY, MPAPERBOY_TIME_SECS);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	SendClientMessage(playerid, COLOR_SERVER_INFO, "Deliver as many as you can before the timer runs out. "COL_YELLOW"/quitmission"COL_WHITE" to stop.");
	return 1;
}

// Paperboy tick — real-time deadline. Ends the run when the 4-minute timer expires,
// paying the completion bonus for whatever was delivered (via Mission_Complete).
static Mission_PaperboyTick(playerid)
{
	new left = PlayerInfo[playerid][pMissionDeadline] - gettime();
	if(left <= 0)
	{
		// Time up. If at least one delivery was made, complete (pays the bonus); else
		// abort (no bonus). The per-drop $500 is already banked either way.
		if(PlayerInfo[playerid][pMissionCheckpoint] > 0)
		{
			// Fake the "final" bonus path without requiring the last route CP.
			SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_GREEN"* Paperboy time's up! "COL_WHITE"Cashing out your deliveries.");
			Mission_Complete(playerid);
		}
		else Mission_Abort(playerid, "you ran out of time with no deliveries");
		return 1;
	}
	new gt[96];
	format(gt, sizeof(gt), "~g~PAPERBOY~n~~w~delivered: ~y~%d~n~~w~time: ~y~%ds", PlayerInfo[playerid][pMissionCheckpoint], left);
	GameTextForPlayer(playerid, gt, 1200, 4);
	return 1;
}

//==============================================================================
//	M6 (stage 4) — target-based missions: Flower Delivery / Pickpocket / Sexual
//	Encounter (§6.5 / §6.12 / §6.15)
//==============================================================================
//	No race route. A game-hour WINDOW runs from start; the player acts on nearby
//	players via a command (/flowers, /pickpocket, /encounter). Distinct targets are
//	deduped in g_MissionTargetHit. Progress is tracked in pMissionCheckpoint (count)
//	and pMissionData (cash for Pickpocket). Completing the goal pays the bonus.
//==============================================================================

FUNCTION Mission_TargetStart(playerid, missionId)
{
	PlayerInfo[playerid][pMissionCheckpoint] = 0;		// targets hit / cash progress base
	PlayerInfo[playerid][pMissionData] = 0;				// running earnings this run
	PlayerInfo[playerid][pMissionStartHour] = Mission_NowGameHour();
	PlayerInfo[playerid][pMissionTargetCd] = 0;
	for(new t = 0; t < MAX_PLAYERS; t++) g_MissionTargetHit[playerid][t] = false;

	new msg[208];
	switch(missionId)
	{
		case MISSION_FLOWER_DELIVERY:
			format(msg, sizeof(msg), ""COL_GREEN"* Flower Delivery started! "COL_WHITE"Hand flowers to "COL_YELLOW"%d different players"COL_WHITE" with "COL_YELLOW"/flowers"COL_WHITE" (near them) within "COL_YELLOW"%d game hours"COL_WHITE".", MFLOWER_DELIVERIES, MFLOWER_WINDOW_HRS);
		case MISSION_PICKPOCKET:
			format(msg, sizeof(msg), ""COL_GREEN"* Pickpocket mission started! "COL_WHITE"Steal "COL_GREEN"$%d"COL_WHITE" total from players with "COL_YELLOW"/pickpocket"COL_WHITE" (near them) within "COL_YELLOW"%d game hours"COL_WHITE".", MPICKPOCKET_TARGET, MPICKPOCKET_WINDOW_HRS);
		case MISSION_SEXUAL_ENCOUNTER:
			format(msg, sizeof(msg), ""COL_GREEN"* Sexual Encounter started! "COL_WHITE"Complete "COL_YELLOW"%d encounters"COL_WHITE" with "COL_YELLOW"/encounter"COL_WHITE" (near players) within "COL_YELLOW"%d game hours"COL_WHITE".", MSEX_ENCOUNTERS, MSEX_WINDOW_HRS);
	}
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	SendClientMessage(playerid, COLOR_SERVER_INFO, "Monitor progress with "COL_YELLOW"/missions"COL_WHITE"; "COL_YELLOW"/quitmission"COL_WHITE" to abandon.");
	if(missionId == MISSION_PICKPOCKET && PlayerInfo[playerid][pSkill] == SKILL_PICKPOCKET)
		SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_GREEN"Your Pickpocket skill boosts your success rate and take.");
	if(missionId == MISSION_SEXUAL_ENCOUNTER && (PlayerInfo[playerid][pSkill] == SKILL_RAPIST || PlayerInfo[playerid][pSkill] == SKILL_PROSTITUTE))
		SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_GREEN"Your skill boosts your encounter success rate.");
	return 1;
}

// The game-hour window for a target mission has elapsed → abort (§6.5/§6.12/§6.15).
static Mission_TargetWindowHrs(missionId)
{
	switch(missionId)
	{
		case MISSION_FLOWER_DELIVERY: return MFLOWER_WINDOW_HRS;
		case MISSION_PICKPOCKET: return MPICKPOCKET_WINDOW_HRS;
		case MISSION_SEXUAL_ENCOUNTER: return MSEX_WINDOW_HRS;
	}
	return 0;
}

// Nearest valid living non-cop, non-NPC target in range (same interior/VW), not
// already counted this run, not the actor. -1 = none. Shared by all 3 target jobs.
static Mission_NearestTarget(playerid, Float:range)
{
	new best = -1, Float:bestDist = range + 1.0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(i == playerid || !IsPlayerConnected(i) || IsPlayerNPC(i) || !PlayerInfo[i][pLoggedIn]) continue;
		if(GetPlayerInterior(i) != GetPlayerInterior(playerid)) continue;
		if(GetPlayerVirtualWorld(i) != GetPlayerVirtualWorld(playerid)) continue;
		new Float:tx, Float:ty, Float:tz;
		GetPlayerPos(i, tx, ty, tz);
		new Float:dd = GetPlayerDistanceFromPoint(playerid, tx, ty, tz);
		if(dd <= range && dd < bestDist) { best = i; bestDist = dd; }
	}
	return best;
}

// /flowers — hand flowers to the nearest not-yet-served player (Flower Delivery §6.5).
FUNCTION Mission_FlowerDeliver(playerid)
{
	if(PlayerInfo[playerid][pOnMission] != MISSION_FLOWER_DELIVERY)
		return SendClientMessage(playerid, COLOR_ERROR, "You are not on a Flower Delivery. Start one at the Flower Delivery checkpoint or a Church.");
	if(gettime() < PlayerInfo[playerid][pMissionTargetCd])
		return SendClientMessage(playerid, COLOR_ERROR, "Slow down — you just handed out flowers.");
	new t = Mission_NearestTarget(playerid, MFLOWER_RANGE);
	if(t == -1) return SendClientMessage(playerid, COLOR_ERROR, "There is no fresh player nearby to give flowers to.");
	if(g_MissionTargetHit[playerid][t]) return SendClientMessage(playerid, COLOR_ERROR, "You already gave that player flowers — find someone new.");

	g_MissionTargetHit[playerid][t] = true;
	PlayerInfo[playerid][pMissionCheckpoint]++;
	PlayerInfo[playerid][pMissionTargetCd] = gettime() + 3;
	Bank_GiveCash(playerid, MFLOWER_PAY_PER);
	PlayerInfo[playerid][pMissionData] += MFLOWER_PAY_PER;

	new msg[144];
	format(msg, sizeof(msg), ""COL_GREEN"* Flowers delivered to %s! "COL_WHITE"%d/%d "COL_GREEN"(+$%d).", PlayerInfo[t][pUserName], PlayerInfo[playerid][pMissionCheckpoint], MFLOWER_DELIVERIES, MFLOWER_PAY_PER);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	SendClientMessage(t, COLOR_SERVER_INFO, ""COL_GREEN"* You received a bunch of flowers and a warm fuzzy feeling.");

	if(PlayerInfo[playerid][pMissionCheckpoint] >= MFLOWER_DELIVERIES)
		Mission_TargetComplete(playerid, MISSION_FLOWER_DELIVERY);
	return 1;
}

// /pickpocket — steal from the nearest not-yet-hit non-cop (Pickpocket mission §6.12).
FUNCTION Mission_PickpocketHit(playerid)
{
	if(PlayerInfo[playerid][pOnMission] != MISSION_PICKPOCKET)
		return SendClientMessage(playerid, COLOR_ERROR, "You are not on a Pickpocket mission. Start one at the Pickpocket checkpoint.");
	if(gettime() < PlayerInfo[playerid][pMissionTargetCd])
		return SendClientMessage(playerid, COLOR_ERROR, "Wait a moment before your next attempt.");
	new t = Mission_NearestTarget(playerid, MPICKPOCKET_RANGE);
	if(t == -1) return SendClientMessage(playerid, COLOR_ERROR, "There is no target within reach — get close to a player.");
	if(IsCop(t)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot pickpocket a police officer.");
	if(g_MissionTargetHit[playerid][t]) return SendClientMessage(playerid, COLOR_ERROR, "You already picked that player's pocket — pick a different mark.");

	PlayerInfo[playerid][pMissionTargetCd] = gettime() + MPICKPOCKET_CD_SECS;

	// Skill scales both success and take (§6.12 "Pickpocket skill most effective").
	new success = (PlayerInfo[playerid][pSkill] == SKILL_PICKPOCKET) ? MPICKPOCKET_SKILL_SUCCESS : MPICKPOCKET_BASE_SUCCESS;
	if(random(100) >= success)
	{
		SendClientMessage(playerid, COLOR_ERROR, ""COL_RED"You fumbled it — the mark noticed and pulled away.");
		SendClientMessage(t, COLOR_ERROR, ""COL_RED"Someone just tried to pick your pocket!");
		return 1;
	}
	new pct = (PlayerInfo[playerid][pSkill] == SKILL_PICKPOCKET) ? MPICKPOCKET_SKILL_PCT : MPICKPOCKET_BASE_PCT;
	new take = (PlayerInfo[t][pMoney] * pct) / 100;
	if(take <= 0) { SendClientMessage(playerid, COLOR_ERROR, "That mark had no cash on hand — try another."); return 1; }

	g_MissionTargetHit[playerid][t] = true;
	Bank_TakeCash(t, take);
	Bank_GiveCash(playerid, take);
	PlayerInfo[playerid][pMissionData] += take;
	PlayerInfo[playerid][pMissionCheckpoint]++;
	GivePlayerWanted(playerid, MPICKPOCKET_WANTED, "Pickpocketing (mission)");

	new msg[160];
	format(msg, sizeof(msg), ""COL_GREEN"* Lifted "COL_WHITE"$%d"COL_GREEN" from %s! "COL_WHITE"Run total "COL_GREEN"$%d"COL_WHITE"/$%d.", take, PlayerInfo[t][pUserName], PlayerInfo[playerid][pMissionData], MPICKPOCKET_TARGET);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	SendClientMessage(t, COLOR_ERROR, ""COL_RED"* You were pickpocketed and lost some cash!");

	if(PlayerInfo[playerid][pMissionData] >= MPICKPOCKET_TARGET)
		Mission_TargetComplete(playerid, MISSION_PICKPOCKET);
	return 1;
}

// /encounter — a Sexual Encounter attempt on the nearest not-yet-hit player (§6.15).
FUNCTION Mission_EncounterHit(playerid)
{
	if(PlayerInfo[playerid][pOnMission] != MISSION_SEXUAL_ENCOUNTER)
		return SendClientMessage(playerid, COLOR_ERROR, "You are not on a Sexual Encounter mission. Start one at a sex shop.");
	if(gettime() < PlayerInfo[playerid][pMissionTargetCd])
		return SendClientMessage(playerid, COLOR_ERROR, "Wait a moment before your next encounter.");
	new t = Mission_NearestTarget(playerid, MSEX_RANGE);
	if(t == -1) return SendClientMessage(playerid, COLOR_ERROR, "There is no willing player nearby.");
	if(g_MissionTargetHit[playerid][t]) return SendClientMessage(playerid, COLOR_ERROR, "You already had an encounter with that player — find someone new.");

	PlayerInfo[playerid][pMissionTargetCd] = gettime() + MSEX_CD_SECS;

	// Rapist / Prostitute skill boosts the per-encounter success chance (§6.15).
	new success = (PlayerInfo[playerid][pSkill] == SKILL_RAPIST || PlayerInfo[playerid][pSkill] == SKILL_PROSTITUTE) ? MSEX_SKILL_SUCCESS : MSEX_BASE_SUCCESS;
	if(random(100) >= success)
	{
		SendClientMessage(playerid, COLOR_ERROR, ""COL_RED"They turned you down. Try someone else.");
		return 1;
	}
	g_MissionTargetHit[playerid][t] = true;
	PlayerInfo[playerid][pMissionCheckpoint]++;
	Bank_GiveCash(playerid, MSEX_PAY_PER_ENCOUNTER);
	PlayerInfo[playerid][pMissionData] += MSEX_PAY_PER_ENCOUNTER;

	new msg[144];
	format(msg, sizeof(msg), ""COL_GREEN"* Encounter with %s "COL_WHITE"%d/%d "COL_GREEN"(+$%d).", PlayerInfo[t][pUserName], PlayerInfo[playerid][pMissionCheckpoint], MSEX_ENCOUNTERS, MSEX_PAY_PER_ENCOUNTER);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);

	if(PlayerInfo[playerid][pMissionCheckpoint] >= MSEX_ENCOUNTERS)
		Mission_TargetComplete(playerid, MISSION_SEXUAL_ENCOUNTER);
	return 1;
}

// Shared completion for a target mission: pay the completion bonus, stamp cooldown, end.
// FUNCTION (auto-forwarded) — the target-hit handlers above call it before its definition.
FUNCTION Mission_TargetComplete(playerid, missionId)
{
	new bonus = MissionReg[missionId][mBonus];
	if(bonus > 0) Bank_GiveCash(playerid, bonus);
	PlayerInfo[playerid][pMissionData] += bonus;
	PlayerInfo[playerid][pScore] += 1;
	new total = PlayerInfo[playerid][pMissionData];
	new name[40];
	format(name, sizeof(name), "%s", MissionReg[missionId][mName]);
	Mission_SetCooldown(playerid, missionId);
	Mission_ClearState(playerid);

	new string[176];
	format(string, sizeof(string), ""COL_GREEN"* %s COMPLETE! "COL_WHITE"Completion bonus "COL_GREEN"$%d"COL_WHITE". Total earned: "COL_GREEN"$%d"COL_WHITE". +1 score.", name, bonus, total);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	GameTextForPlayer(playerid, "~g~Mission complete!", 4000, 4);
	return 1;
}

//==============================================================================
//	M6 (stage 4) — Race Challenges (§6.13): /challenge race framework
//==============================================================================
//	Data-driven races (g_RaceReg). A picker dialog lists the shipped races; on
//	select, a race-CP route runs against a par time. First attempt at a given race
//	is free (per-mission cooldown = 0 first time); later attempts cost a fee. +1
//	score for finishing, +2 (bonus) for a podium (par-beat). Add races by appending
//	g_RaceReg rows — no framework change.
//==============================================================================

// Does the player's vehicle match the race category? (Bikes / boats / land.)
static Mission_RaceVehicleOK(playerid, category)
{
	if(!IsPlayerInAnyVehicle(playerid) || GetPlayerState(playerid) != PLAYER_STATE_DRIVER) return 0;
	new model = GetVehicleModel(GetPlayerVehicleID(playerid));
	switch(category)
	{
		case MRACE_CATEGORY_BOAT:	return !Mission_IsLandVehicle(model) && Mission_IsBoat(model);
		case MRACE_CATEGORY_BIKE:	return Mission_IsBike(model);
		default:					return Mission_IsLandVehicle(model);	// land
	}
}

static Mission_IsBoat(model)
{
	switch(model)
	{
		case 430, 446, 452, 453, 454, 472, 473, 484, 493, 595: return 1;
	}
	return 0;
}
static Mission_IsBike(model)
{
	switch(model)
	{
		case 448, 461, 462, 463, 468, 471, 481, 509, 510, 521, 522, 523, 581, 586: return 1;	// bikes/BMX/mopeds
	}
	return 0;
}

// Open the race picker dialog (/challenge — the Race Challenge launcher).
FUNCTION Mission_RaceLauncher(playerid)
{
	// Reset the tentative mission slot (Mission_Start set pOnMission = MISSION_RACE_CHALLENGE
	// before handing here) — the actual race is chosen in the dialog, so detach for now.
	PlayerInfo[playerid][pOnMission] = -1;

	new caption[1024], row[160];
	caption[0] = EOS;
	for(new r = 0; r < sizeof(g_RaceReg); r++)
	{
		new cat[8];
		switch(g_RaceReg[r][rCategory])
		{
			case MRACE_CATEGORY_BIKE: format(cat, sizeof(cat), "Bike");
			case MRACE_CATEGORY_BOAT: format(cat, sizeof(cat), "Boat");
			default: format(cat, sizeof(cat), "Land");
		}
		format(row, sizeof(row), "%s\t%s\t%d CPs\n", g_RaceReg[r][rName], cat, g_RaceReg[r][rCheckpoints]);
		strcat(caption, row);
	}
	ShowPlayerDialog(playerid, MRACE_SELECT_DIALOG, DIALOG_STYLE_TABLIST, "Race Challenges", caption, "Race", "Close");
	return 1;
}

// Race dialog resolution (routed from OnDialogResponse in CnR.pwn).
FUNCTION Mission_RaceOnDialog(playerid, response, listitem)
{
	if(!response) return 1;
	if(listitem < 0 || listitem >= sizeof(g_RaceReg)) return 1;
	Mission_RaceStart(playerid, listitem);
	return 1;
}

// Begin race `raceId`. First attempt is free; a repeat costs MRACE_RETRY_FEE (the
// per-mission game-hour cooldown doubles as the "have you run before" gate — if the
// stamp is non-zero the player has raced this session type before → charge the fee).
// FUNCTION (auto-forwarded) — Mission_RaceOnDialog, defined above, calls it.
FUNCTION Mission_RaceStart(playerid, raceId)
{
	if(Mission_IsOnMission(playerid))
		return SendClientMessage(playerid, COLOR_ERROR, "Finish or /quitmission your current mission first.");
	// Per-run cooldown (§6.13 — "1 game-hr cooldown between race runs"). The same stamp
	// doubles as the first-free / retry-fee flag below; here it gates repeat starts so
	// the race cannot be farmed as a guaranteed-profit money faucet.
	if(Mission_OnCooldown(playerid, MISSION_RACE_CHALLENGE))
	{
		new cderr[128];
		format(cderr, sizeof(cderr), "Race Challenge is on cooldown — try again in %d game hour(s).", Mission_CooldownLeft(playerid, MISSION_RACE_CHALLENGE));
		return SendClientMessage(playerid, COLOR_ERROR, cderr);
	}
	if(!Mission_RaceVehicleOK(playerid, g_RaceReg[raceId][rCategory]))
	{
		new err[128];
		switch(g_RaceReg[raceId][rCategory])
		{
			case MRACE_CATEGORY_BIKE: format(err, sizeof(err), "This is a BIKES-only race — get on a bike.");
			case MRACE_CATEGORY_BOAT: format(err, sizeof(err), "This is a BOATS-only race — get in a boat.");
			default: format(err, sizeof(err), "This is a LAND-vehicle race — get in a car.");
		}
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	// First attempt free (§6.13). We treat a set MISSION_RACE_CHALLENGE cooldown stamp
	// as "has raced before" → charge the retry fee.
	new bool:firstTime = (PlayerInfo[playerid][pMissionCooldown][MISSION_RACE_CHALLENGE] == 0);
	if(!firstTime)
	{
		if(Bank_GetCash(playerid) < MRACE_RETRY_FEE)
			return SendClientMessage(playerid, COLOR_ERROR, "You need $5,000 cash-in-hand for the race entry fee. First race is free — later runs cost $5,000.");
		Bank_TakeCash(playerid, MRACE_RETRY_FEE);
	}

	PlayerInfo[playerid][pOnMission] = MISSION_RACE_CHALLENGE;
	PlayerInfo[playerid][pMissionRaceId] = raceId;
	PlayerInfo[playerid][pMissionCheckpoint] = 0;
	PlayerInfo[playerid][pMissionData] = 0;
	PlayerInfo[playerid][pMissionVehicle] = GetPlayerVehicleID(playerid);
	PlayerInfo[playerid][pMissionDeadline] = gettime();	// race START time (par compared on finish)
	new len = g_RaceReg[raceId][rCheckpoints];
	if(len > MISSION_MAX_CHECKPOINTS) len = MISSION_MAX_CHECKPOINTS;
	PlayerInfo[playerid][pMissionRouteLen] = len;
	for(new i = 0; i < len; i++)
	{
		PlayerInfo[playerid][pMissionRouteX][i] = g_RaceReg[raceId][rRoute][i * 3 + 0];
		PlayerInfo[playerid][pMissionRouteY][i] = g_RaceReg[raceId][rRoute][i * 3 + 1];
		PlayerInfo[playerid][pMissionRouteZ][i] = g_RaceReg[raceId][rRoute][i * 3 + 2];
	}
	Mission_PlaceRaceCP(playerid);

	new msg[192];
	format(msg, sizeof(msg), ""COL_GREEN"* Race: "COL_WHITE"%s"COL_GREEN"! "COL_WHITE"Hit all "COL_YELLOW"%d"COL_WHITE" checkpoints. Beat "COL_YELLOW"%ds"COL_WHITE" for a podium bonus. %s", g_RaceReg[raceId][rName], len, g_RaceReg[raceId][rParSecs], firstTime ? ("(first attempt free)") : ("(entry fee paid)"));
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}

// A race checkpoint was reached (dispatched from Mission_OnCheckpoint for the race).
// FUNCTION (auto-forwarded) because Mission_OnCheckpoint, defined earlier, calls it.
FUNCTION Mission_RaceOnCheckpoint(playerid)
{
	PlayerInfo[playerid][pMissionCheckpoint]++;
	new bool:isFinal = (PlayerInfo[playerid][pMissionCheckpoint] >= PlayerInfo[playerid][pMissionRouteLen]);
	if(!isFinal)
	{
		Mission_PlaceRaceCP(playerid);
		new gt[80];
		format(gt, sizeof(gt), "~g~CHECKPOINT ~y~%d~w~/~y~%d", PlayerInfo[playerid][pMissionCheckpoint], PlayerInfo[playerid][pMissionRouteLen]);
		GameTextForPlayer(playerid, gt, 2000, 4);
		return 1;
	}
	// Finished — score + pay. Podium (par-beat) earns the extra bonus (§6.13 +2 / Top-10).
	new elapsed = gettime() - PlayerInfo[playerid][pMissionDeadline];
	new raceId = PlayerInfo[playerid][pMissionRaceId];
	new bool:podium = (elapsed <= g_RaceReg[raceId][rParSecs]);
	new pay = MRACE_FINISH_PAY;
	if(podium) pay += MRACE_PODIUM_BONUS;
	Bank_GiveCash(playerid, pay);
	PlayerInfo[playerid][pScore] += podium ? 2 : 1;		// +2 podium, +1 finish (§6.13 VERIFIED)

	new name[40];
	format(name, sizeof(name), "%s", g_RaceReg[raceId][rName]);
	Mission_SetCooldown(playerid, MISSION_RACE_CHALLENGE);	// marks "has raced before" (fee applies next time)
	Mission_ClearState(playerid);

	new msg[192];
	format(msg, sizeof(msg), ""COL_GREEN"* Race COMPLETE: "COL_WHITE"%s"COL_GREEN" in "COL_WHITE"%ds"COL_GREEN"! %s "COL_WHITE"Payout "COL_GREEN"$%d"COL_WHITE" (+%d score).", name, elapsed, podium ? (""COL_YELLOW"PODIUM!"COL_GREEN) : (""), pay, podium ? 2 : 1);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	GameTextForPlayer(playerid, podium ? ("~g~Podium finish!") : ("~g~Race finished!"), 4000, 4);
	return 1;
}

//==============================================================================
//	M6 (stage 4) — Domestic Disturbance (COP, §6.3): drive PD → a player-owned house
//==============================================================================
//	A single destination (a random player-owned house — reuses M5 housing coords via
//	House_RandomOwnedCoords). A real-time par: reach the house under MDOMESTIC_PAR_SECS
//	for the full speed bonus; the bonus decays to the base as time passes; a timeout
//	abandons the call. Bespoke single SetPlayerCheckpoint (not a multi-drop route).
//==============================================================================

FUNCTION Mission_DomesticStart(playerid)
{
	new Float:hx, Float:hy, Float:hz;
	// Reuse the M5 housing system for a real player-owned house destination (§6.3
	// "a player-owned house"). Falls back to a default LS point if no house is owned.
	// Direct call (housing.inc is included before missions.inc), so the float refs work.
	if(!House_RandomOwnedCoords(hx, hy, hz))
	{
		// No owned houses on the server yet — fall back to a fixed residential point so
		// the mission is still runnable (design allows this until houses exist).
		hx = 2145.0; hy = -1015.0; hz = 62.7;
		SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_WHITE"(No player-owned houses yet — dispatching you to a residential address.)");
	}
	PlayerInfo[playerid][pMissionDestX] = hx;
	PlayerInfo[playerid][pMissionDestY] = hy;
	PlayerInfo[playerid][pMissionDestZ] = hz;
	PlayerInfo[playerid][pMissionDeadline] = gettime() + MDOMESTIC_TIMEOUT_SECS;
	PlayerInfo[playerid][pMissionData] = gettime();		// reuse pMissionData as the START time for the speed bonus
	SetPlayerCheckpoint(playerid, hx, hy, hz, MDOMESTIC_CP_SIZE);

	SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_GREEN"* Domestic Disturbance! "COL_WHITE"A call came in — drive to the marked house "COL_YELLOW"as fast as you can"COL_WHITE". The faster you arrive, the bigger the bonus.");
	GameTextForPlayer(playerid, "~b~Domestic Disturbance~n~~w~respond to the house!", 4000, 5);
	return 1;
}

// Domestic Disturbance arrival — pay a speed-scaled bonus (§6.3 "faster = bigger").
static Mission_DomesticArrive(playerid)
{
	new elapsed = gettime() - PlayerInfo[playerid][pMissionData];	// pMissionData = start time
	new speedBonus;
	if(elapsed <= MDOMESTIC_PAR_SECS) speedBonus = MDOMESTIC_SPEED_MAX;
	else
	{
		// Linear decay from full bonus at par to zero at the timeout.
		new span = MDOMESTIC_TIMEOUT_SECS - MDOMESTIC_PAR_SECS;
		new over = elapsed - MDOMESTIC_PAR_SECS;
		if(over >= span) speedBonus = 0;
		else speedBonus = (MDOMESTIC_SPEED_MAX * (span - over)) / span;
	}
	new pay = MDOMESTIC_BASE_PAY + speedBonus;
	Bank_GiveCash(playerid, pay);
	PlayerInfo[playerid][pScore] += 1;

	if(IsPlayerConnected(playerid)) DisablePlayerCheckpoint(playerid);
	Mission_SetCooldown(playerid, MISSION_DOMESTIC_DISTURBANCE);
	Mission_ClearState(playerid);

	new msg[176];
	format(msg, sizeof(msg), ""COL_GREEN"* Domestic Disturbance handled in "COL_WHITE"%ds"COL_GREEN"! "COL_WHITE"Payout "COL_GREEN"$%d"COL_WHITE" (base $%d + speed $%d). +1 score.", elapsed, pay, MDOMESTIC_BASE_PAY, speedBonus);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	GameTextForPlayer(playerid, "~g~Disturbance handled!", 4000, 4);
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
		// Airport Robbery leaves streamed box pickups behind — destroy them.
		if(PlayerInfo[playerid][pOnMission] == MISSION_AIRPORT_ROBBERY)
			Mission_AirportCleanup(playerid);
		PlayerInfo[playerid][pOnMission] = -1;
		PlayerInfo[playerid][pMissionCheckpoint] = 0;
		PlayerInfo[playerid][pMissionRouteLen] = 0;
		PlayerInfo[playerid][pMissionVehicle] = -1;
		PlayerInfo[playerid][pMissionData] = 0;
		// M6 (stage 4) bespoke mission scratch.
		PlayerInfo[playerid][pMissionDeadline] = 0;
		PlayerInfo[playerid][pMissionStartHour] = 0;
		PlayerInfo[playerid][pMissionTargetCd] = 0;
		PlayerInfo[playerid][pMissionRaceId] = -1;
		PlayerInfo[playerid][pMissionDestX] = PlayerInfo[playerid][pMissionDestY] = PlayerInfo[playerid][pMissionDestZ] = 0.0;
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

	// Airport Robbery EXPECTS the player to leave the vehicle on foot to grab boxes
	// during the collect phase (§6.1); Vehicle Theft swaps between 5 DIFFERENT cars
	// (§6.19) — both would false-abort on the leave-vehicle rule, so exempt them.
	// Domestic Disturbance (§6.3) completes by reaching the house on the tick; a cop
	// stepping out at the destination must not false-abort it — exempt it too.
	if(missionId == MISSION_AIRPORT_ROBBERY || missionId == MISSION_VEHICLE_THEFT || missionId == MISSION_DOMESTIC_DISTURBANCE) return 0;

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

		new mid = PlayerInfo[playerid][pOnMission];

		// Airport Robbery runs its own bespoke box tick (collect/deliver phases +
		// deadlines), not the race-CP off-route guard — dispatch and skip the route logic.
		if(mid == MISSION_AIRPORT_ROBBERY)
		{
			Mission_AirportTick(playerid);
			continue;
		}
		// Paperboy (§6.11) — 4-minute REAL-TIME deadline on the shared tick.
		if(mid == MISSION_PAPERBOY)
		{
			Mission_PaperboyTick(playerid);
			continue;
		}
		// Domestic Disturbance (§6.3) — single-house destination + timeout, no route CP.
		if(mid == MISSION_DOMESTIC_DISTURBANCE)
		{
			new left = PlayerInfo[playerid][pMissionDeadline] - gettime();
			if(left <= 0) { Mission_Abort(playerid, "the disturbance call was abandoned"); continue; }
			if(IsPlayerInRangeOfPoint(playerid, MDOMESTIC_CP_SIZE, PlayerInfo[playerid][pMissionDestX], PlayerInfo[playerid][pMissionDestY], PlayerInfo[playerid][pMissionDestZ]))
				Mission_DomesticArrive(playerid);
			continue;
		}
		// Target-based missions (Flower/Pickpocket/Sexual) — game-hour window expiry.
		if(mid == MISSION_FLOWER_DELIVERY || mid == MISSION_PICKPOCKET || mid == MISSION_SEXUAL_ENCOUNTER)
		{
			new elapsed = Mission_NowGameHour() - PlayerInfo[playerid][pMissionStartHour];
			if(elapsed >= Mission_TargetWindowHrs(mid))
				Mission_Abort(playerid, "you ran out of time to reach the target");
			continue;
		}
		// Race Challenge (§6.13) — no time limit and no off-route abort (races legitimately
		// span long distances between checkpoints); the player quits via /quitmission.
		if(mid == MISSION_RACE_CHALLENGE) continue;

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
	// Widened 96->255 in M6 stage 4: the 19-mission roster packs up to 19 stamps of
	// up-to-9 chars + delimiters (~189) which overruns the old 96-char buffer.
	new packed[255], part[16];
	packed[0] = EOS;
	for(new i = 0; i < MISSION_COUNT; i++)
	{
		format(part, sizeof(part), "%d", PlayerInfo[playerid][pMissionCooldown][i]);
		strcat(packed, part);
		if(i != MISSION_COUNT - 1) strcat(packed, "|");
	}
	new string[352];
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
	new caption[2400], row[176];
	caption[0] = EOS;
	for(new m = 0; m < MISSION_COUNT; m++)
	{
		new status[48], payLabel[56];
		// Flower Delivery (§6.5) is open to BOTH cops and civilians — never show it as
		// role-locked. All other rows use the plain civ/cop entry gate.
		if(MissionReg[m][mType] == MISSION_TYPE_COP && !IsCop(playerid)) format(status, sizeof(status), "{888888}cops only");
		else if(MissionReg[m][mType] == MISSION_TYPE_CIVILIAN && IsCop(playerid) && m != MISSION_FLOWER_DELIVERY) format(status, sizeof(status), "{888888}civilians only");
		else if(Mission_OnCooldown(playerid, m)) format(status, sizeof(status), "{CC5555}%d game-hr cooldown", Mission_CooldownLeft(playerid, m));
		else if(MissionReg[m][mNeedsVehicle] && !Mission_VehicleOK(playerid, m)) format(status, sizeof(status), "{DDAA33}need vehicle");
		else format(status, sizeof(status), "{55CC55}READY");

		// Pay label varies by mission mechanic (route / box / target / race / speed).
		switch(m)
		{
			case MISSION_AIRPORT_ROBBERY:	format(payLabel, sizeof(payLabel), "$%d/box + $%d", MAIRPORT_PAY_PER_BOX, MissionReg[m][mBonus]);
			case MISSION_PAPERBOY:			format(payLabel, sizeof(payLabel), "$%d/drop (4-min real time)", MPAPERBOY_CP_PAY);
			case MISSION_PICKPOCKET:		format(payLabel, sizeof(payLabel), "steal $%d target + $%d", MPICKPOCKET_TARGET, MPICKPOCKET_COMPLETE_BONUS);
			case MISSION_SEXUAL_ENCOUNTER:	format(payLabel, sizeof(payLabel), "%d encounters + $%d", MSEX_ENCOUNTERS, MSEX_COMPLETE_BONUS);
			case MISSION_FLOWER_DELIVERY:	format(payLabel, sizeof(payLabel), "%d players + $%d", MFLOWER_DELIVERIES, MFLOWER_COMPLETE_BONUS);
			case MISSION_TRACTOR:			format(payLabel, sizeof(payLabel), "farm crop route");
			case MISSION_RACE_CHALLENGE:	format(payLabel, sizeof(payLabel), "race: $%d + podium $%d", MRACE_FINISH_PAY, MRACE_PODIUM_BONUS);
			case MISSION_DOMESTIC_DISTURBANCE:	format(payLabel, sizeof(payLabel), "$%d + speed bonus", MDOMESTIC_BASE_PAY);
			default:						format(payLabel, sizeof(payLabel), "$%d/cp + $%d", MissionReg[m][mCpPay], MissionReg[m][mBonus]);
		}

		format(row, sizeof(row), "%s\t%s\t%s\n", MissionReg[m][mName], payLabel, status);
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
		new missionId = PlayerInfo[playerid][pOnMission], string[208];
		// Airport Robbery has its own box/phase status, not a checkpoint N/M line.
		if(missionId == MISSION_AIRPORT_ROBBERY)
		{
			new left = PlayerInfo[playerid][pAirportDeadline] - gettime();
			if(left < 0) left = 0;
			format(string, sizeof(string), ""COL_WHITE"Active: "COL_GREEN"Airport Robbery"COL_WHITE" — %s, boxes "COL_YELLOW"%d"COL_WHITE", "COL_YELLOW"%ds"COL_WHITE" left. "COL_YELLOW"/box"COL_WHITE" to grab, "COL_YELLOW"/quitmission"COL_WHITE" to bail.",
				(PlayerInfo[playerid][pAirportPhase] == AIRPHASE_COLLECT) ? ("collecting") : ("delivering"), PlayerInfo[playerid][pAirportBoxes], left);
			return SendClientMessage(playerid, COLOR_SERVER_INFO, string);
		}
		// M6 (stage 4) bespoke status lines.
		switch(missionId)
		{
			case MISSION_PAPERBOY:
			{
				new left = PlayerInfo[playerid][pMissionDeadline] - gettime();
				if(left < 0) left = 0;
				format(string, sizeof(string), ""COL_WHITE"Active: "COL_GREEN"Paperboy"COL_WHITE" — delivered "COL_YELLOW"%d"COL_WHITE", "COL_YELLOW"%ds"COL_WHITE" left (real time). Toss papers at the house checkpoints on foot.", PlayerInfo[playerid][pMissionCheckpoint], left);
				return SendClientMessage(playerid, COLOR_SERVER_INFO, string);
			}
			case MISSION_FLOWER_DELIVERY:
			{
				format(string, sizeof(string), ""COL_WHITE"Active: "COL_GREEN"Flower Delivery"COL_WHITE" — "COL_YELLOW"%d"COL_WHITE"/"COL_YELLOW"%d"COL_WHITE" players. Use "COL_YELLOW"/flowers"COL_WHITE" near a player.", PlayerInfo[playerid][pMissionCheckpoint], MFLOWER_DELIVERIES);
				return SendClientMessage(playerid, COLOR_SERVER_INFO, string);
			}
			case MISSION_PICKPOCKET:
			{
				format(string, sizeof(string), ""COL_WHITE"Active: "COL_GREEN"Pickpocket"COL_WHITE" — stolen "COL_GREEN"$%d"COL_WHITE"/$%d. Use "COL_YELLOW"/pickpocket"COL_WHITE" near a player.", PlayerInfo[playerid][pMissionData], MPICKPOCKET_TARGET);
				return SendClientMessage(playerid, COLOR_SERVER_INFO, string);
			}
			case MISSION_SEXUAL_ENCOUNTER:
			{
				format(string, sizeof(string), ""COL_WHITE"Active: "COL_GREEN"Sexual Encounter"COL_WHITE" — "COL_YELLOW"%d"COL_WHITE"/"COL_YELLOW"%d"COL_WHITE". Use "COL_YELLOW"/encounter"COL_WHITE" near a player.", PlayerInfo[playerid][pMissionCheckpoint], MSEX_ENCOUNTERS);
				return SendClientMessage(playerid, COLOR_SERVER_INFO, string);
			}
			case MISSION_RACE_CHALLENGE:
			{
				new elapsed = gettime() - PlayerInfo[playerid][pMissionDeadline];
				format(string, sizeof(string), ""COL_WHITE"Active: "COL_GREEN"Race — %s"COL_WHITE" — CP "COL_YELLOW"%d"COL_WHITE"/"COL_YELLOW"%d"COL_WHITE", "COL_YELLOW"%ds"COL_WHITE" elapsed. "COL_YELLOW"/quitmission"COL_WHITE" to bail.", g_RaceReg[PlayerInfo[playerid][pMissionRaceId]][rName], PlayerInfo[playerid][pMissionCheckpoint], PlayerInfo[playerid][pMissionRouteLen], elapsed);
				return SendClientMessage(playerid, COLOR_SERVER_INFO, string);
			}
			case MISSION_DOMESTIC_DISTURBANCE:
			{
				new left = PlayerInfo[playerid][pMissionDeadline] - gettime();
				if(left < 0) left = 0;
				format(string, sizeof(string), ""COL_WHITE"Active: "COL_GREEN"Domestic Disturbance"COL_WHITE" — reach the marked house! "COL_YELLOW"%ds"COL_WHITE" before the call is abandoned.", left);
				return SendClientMessage(playerid, COLOR_SERVER_INFO, string);
			}
		}
		Mission_ShowHUD(playerid);
		format(string, sizeof(string), ""COL_WHITE"Active mission: "COL_GREEN"%s"COL_WHITE" — checkpoint "COL_YELLOW"%d"COL_WHITE"/"COL_YELLOW"%d"COL_WHITE", earned "COL_GREEN"$%d"COL_WHITE". "COL_YELLOW"/quitmission"COL_WHITE" to abandon.",
			MissionReg[missionId][mName], PlayerInfo[playerid][pMissionCheckpoint], PlayerInfo[playerid][pMissionRouteLen], PlayerInfo[playerid][pMissionData]);
		return SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	}
	return Mission_ShowLauncher(playerid);
}
alias:missions("mission", "work", "jobs", "mis")

// /challenge (§6.13) — open the Race Challenge picker (all players). Also startable
// from the /missions launcher row, but /challenge is the CB-canonical command.
CMD:challenge(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(Mission_IsOnMission(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Finish or /quitmission your current mission first.");
	if(Jail_IsJailed(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot race while jailed.");
	if(Mission_OnCooldown(playerid, MISSION_RACE_CHALLENGE))
	{
		new err[128];
		format(err, sizeof(err), "Race Challenge is on cooldown - try again in %d game hour(s).", Mission_CooldownLeft(playerid, MISSION_RACE_CHALLENGE));
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	return Mission_RaceLauncher(playerid);
}
alias:challenge("challenges", "race")

// /flowers (§6.5) — hand flowers to the nearest player during a Flower Delivery.
CMD:flowers(playerid, params[])
{
	#pragma unused params
	return Mission_FlowerDeliver(playerid);
}

// /pickpocket (§6.12) — pickpocket the nearest player during a Pickpocket mission.
CMD:pickpocket(playerid, params[])
{
	#pragma unused params
	return Mission_PickpocketHit(playerid);
}
alias:pickpocket("pp")

// /encounter (§6.15) — a Sexual Encounter attempt on the nearest player.
CMD:encounter(playerid, params[])
{
	#pragma unused params
	return Mission_EncounterHit(playerid);
}

// /box (§6.1) — collect the nearest airport-robbery box in reach on foot. Also
// triggered by the Middle Mouse Button (KEY_ACTION) in OnPlayerKeyStateChange.
CMD:box(playerid, params[])
{
	#pragma unused params
	if(PlayerInfo[playerid][pOnMission] != MISSION_AIRPORT_ROBBERY)
		return SendClientMessage(playerid, COLOR_ERROR, "You are not on an Airport Robbery. Start one at the airport in a truck/van/garbage truck.");
	if(PlayerInfo[playerid][pAirportPhase] != AIRPHASE_COLLECT)
		return SendClientMessage(playerid, COLOR_ERROR, "The collection phase is over — drive the loot to the hideout.");
	Mission_AirportCollectBox(playerid);
	return 1;
}

CMD:quitmission(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(!Mission_IsOnMission(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You are not on a mission.");
	Mission_Abort(playerid, "");
	return 1;
}
alias:quitmission("cancelmission", "stopmission", "cancel")
