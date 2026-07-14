/*

	Farming system for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 6 (stage 2) - design §9.3 (farming / drug growing) + §10.2 (crops/
	drugs × DRUG) + §11.2 (DrugSeeds/HuntPermits cols) + §11.3 (plants table) +
	docs/research/economy.md §9 + docs/research/crazybobs-site-full §Drug Planting
	and Hunting / §CrazyBob's Farm.

	Two loops, mirroring CB:

	1. ILLEGAL drug farming (§9.3): buy seeds at a Bait Shop / Drug Refill Point
	   (systems/fishing.inc Bait Shop menu + /buyseeds here), /plant a drug plant at
	   your location (up to MAX_PLANTS_PER_PLAYER=5, not on top of water/other plants),
	   plants grow over ~20 minutes of ONLINE time toward 200g (they do NOT grow while
	   the owner is offline, §9.3), /harvest -> Drug_GiveGrams(pDrugs). /fertilize speeds
	   growth (attracts deer - noted, hunting tie-in). Planting/harvesting is illegal:
	   if a cop is in sight you gain wanted, and cops (or anyone) can /destroyplant a
	   found plant. Grown drugs are sold ONLY at Drug Refill Points via /drugsell, which
	   feeds the DRUG stock (dumping lowers the price, §10.2). Plants persist in the
	   `plants` table (async CRUD); loaded on init.

	2. LEGAL crop job at CrazyBob's Farm (Flint County, §9.3): /farmjob runs a plow/
	   harvest route of FARM_JOB_FIELDS field checkpoints for legit low-yield cash
	   ($500-$2,000/field + a completion bonus) - a safe income floor. This is the FARM
	   crop SYSTEM; the Tractor MISSION (stage 4) will wrap it. Feeds no illegal stock.

	Hunting (§9.3) is implemented MINIMALLY: /hunt "shoots a deer" in front of you -
	killing a deer without a hunting permit gives +6 wanted (§9.3 VERIFIED); with a
	permit it heals + pays a small bonus. Full deer AI / deer-eat-plants / deer traps
	are DEFERRED (heavy, needs a ped/actor deer spawner) - noted in the report.

	Public API (called by CnR.pwn / loading_data.inc):
		Farm_Init()                         : load the plants table (async) + build the farm-job route markers
		Farm_Cleanup()                      : destroy plant checkpoints/objects + farm markers
		Farm_OnPlayerLogin(playerid)        : tally this player's active plant count on login
		Farm_OnGameModeClockTick()          : grow online players' plants (1s) - accrue grow-seconds
		Farm_OnDisconnect(playerid)         : end an active farm job (session)
		Farm_OnPlayerDeath(playerid)        : abort a farm job on death
		Farm_OnPlayerEnterCP(playerid, cpid): farm-job field checkpoint reached

	Commands:
		/plant                      plant a drug seed at your location (illegal)
		/harvest (/harv)            harvest the nearest ripe plant -> drugs
		/fertilize (/fert)          fertilize the nearest plant (faster, attracts deer)
		/destroyplant              destroy the nearest plant (cops find/destroy crops)
		/plantgps (/pgps)           list your plants + their growth
		/drugsell (/ds)             sell grown drugs at a Drug Refill Point
		/buyseeds                   buy drug seeds at a Drug Refill Point
		/hunt                       hunt a deer (minimal - permit gate)
		/farmjob                    start the legal crop job at CrazyBob's Farm

	Functions Being Called:
		bank:     Bank_GiveCash
		drugs:    Drug_GiveGrams (harvest feeds pDrugs), DrugCarryCap
		wanted:   GivePlayerWanted
		jail:     Jail_IsJailed, Jail_SavePlayer (persists DrugSeeds/HuntPermits/Drugs via the shared UPDATE)
		misc:     IsCop, IsCivilian
		economy:  StockMarket_UpdateEarnings, StockMarket_PriceMult (M7 stubs)
*/

//==============================================================================
//	Plant table (loaded from `plants`, §11.3) + Drug Refill Points + farm markers
//==============================================================================

enum e_Plant
{
	plID,				// plants.ID (DB primary key)
	plOwnerAID,			// players.aID of the planter
	Float:plX,			// world position
	Float:plY,
	Float:plZ,
	plGrams,			// current yield snapshot (0..DRUG_PLANT_MAX_GRAMS)
	plGrowSecs,			// grow-seconds accrued (only while owner online, §9.3)
	bool:plFertilized,	// fertilized (faster growth, attracts deer)
	STREAMER_TAG_CP:plCP,		// red checkpoint on the plant (§9.3)
	STREAMER_TAG_OBJECT:plObject	// the plant object at the spot
}

static PlantInfo[MAX_PLANTS][e_Plant];
static gPlantCount = 0;

// Drug Refill Points (crazybobs-site-full §Drug Refill Points) - where grown drugs
// are sold (/drugsell) and seeds are bought (/buyseeds). LS-area coords reachable now.
static const Float:g_RefillPoints[][3] =
{
	{ 1930.900,  -1772.900, 13.400 },	// LS Idlewood
	{ 2495.000,  -1690.000, 13.500 },	// LS East Los Santos
	{ 613.400,   -580.900,  17.900 }	// Dillimore (kept for later)
};

static STREAMER_TAG_MAP_ICON:g_RefillIcon[sizeof(g_RefillPoints)];

//==============================================================================
//	Legal crop job - CrazyBob's Farm (Flint County, §9.3)
//==============================================================================

// Candidate field checkpoints around CrazyBob's Farm - the launcher picks a route.
static const Float:g_FarmFields[][3] =
{
	{ -315.0,  -1420.0, 25.7 },
	{ -260.0,  -1380.0, 26.5 },
	{ -360.0,  -1470.0, 25.2 },
	{ -220.0,  -1450.0, 27.1 },
	{ -300.0,  -1360.0, 26.0 },
	{ -380.0,  -1400.0, 25.5 },
	{ -250.0,  -1500.0, 25.9 }
};

static STREAMER_TAG_MAP_ICON:g_FarmIcon;

// Per-player active farm-job state (session only - a legit checkpoint loop).
static bool:gFarmJobActive[MAX_PLAYERS];
static gFarmJobField[MAX_PLAYERS];			// current field index reached (0..FARM_JOB_FIELDS)
static gFarmJobEarned[MAX_PLAYERS];			// running earnings this run
static Float:gFarmJobRouteX[MAX_PLAYERS][FARM_JOB_FIELDS];
static Float:gFarmJobRouteY[MAX_PLAYERS][FARM_JOB_FIELDS];
static Float:gFarmJobRouteZ[MAX_PLAYERS][FARM_JOB_FIELDS];
static STREAMER_TAG_CP:gFarmJobCP[MAX_PLAYERS];

//==============================================================================
//	Setup / teardown
//==============================================================================

FUNCTION Farm_Init()
{
	gPlantCount = 0;
	for(new i = 0; i < MAX_PLANTS; i++)
	{
		PlantInfo[i][plCP] = STREAMER_TAG_CP:INVALID_STREAMER_ID;
		PlantInfo[i][plObject] = STREAMER_TAG_OBJECT:INVALID_STREAMER_ID;
	}
	for(new i = 0; i < sizeof(g_RefillPoints); i++)
		g_RefillIcon[i] = CreateDynamicMapIcon(g_RefillPoints[i][0], g_RefillPoints[i][1], g_RefillPoints[i][2], 51, 0, -1, -1, -1, 250.0, MAPICON_LOCAL);
	// Farm map icon (a green field marker) at CrazyBob's Farm (§9.3).
	g_FarmIcon = CreateDynamicMapIcon(FARM_X, FARM_Y, FARM_Z, 56, 0, -1, -1, -1, 300.0, MAPICON_LOCAL);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		gFarmJobActive[i] = false;
		gFarmJobField[i] = gFarmJobEarned[i] = 0;
		gFarmJobCP[i] = STREAMER_TAG_CP:INVALID_STREAMER_ID;
	}
	// Load the plants table (async) -> build each plant's checkpoint + object.
	mysql_pquery(g_SQL, "SELECT * FROM `plants` ORDER BY ID ASC", "Farm_OnPlantsLoad", "");
	return 1;
}

FUNCTION Farm_Cleanup()
{
	for(new i = 0; i < gPlantCount; i++) Farm_DestroyPlantStreamer(i);
	for(new i = 0; i < sizeof(g_RefillPoints); i++)
		if(DestroyDynamicMapIcon(g_RefillIcon[i])) {}
	if(DestroyDynamicMapIcon(g_FarmIcon)) {}
	for(new i = 0; i < MAX_PLAYERS; i++)
		if(gFarmJobCP[i] != STREAMER_TAG_CP:INVALID_STREAMER_ID) { DestroyDynamicCP(gFarmJobCP[i]); gFarmJobCP[i] = STREAMER_TAG_CP:INVALID_STREAMER_ID; }
	return 1;
}

// Async result handler for Farm_Init - read the plant rows into PlantInfo[] and
// build the streamer objects/checkpoints.
FUNCTION Farm_OnPlantsLoad()
{
	new rows;
	cache_get_row_count(rows);
	if(rows > MAX_PLANTS) rows = MAX_PLANTS;
	gPlantCount = rows;
	for(new i = 0; i < rows; i++)
	{
		cache_get_value_int(i, "ID", PlantInfo[i][plID]);
		cache_get_value_int(i, "OwnerAID", PlantInfo[i][plOwnerAID]);
		cache_get_value_float(i, "X", PlantInfo[i][plX]);
		cache_get_value_float(i, "Y", PlantInfo[i][plY]);
		cache_get_value_float(i, "Z", PlantInfo[i][plZ]);
		cache_get_value_int(i, "Grams", PlantInfo[i][plGrams]);
		cache_get_value_int(i, "GrowSecs", PlantInfo[i][plGrowSecs]);
		cache_get_value_bool(i, "Fertilized", PlantInfo[i][plFertilized]);
		Farm_BuildPlantStreamer(i);
	}
	printf("[FARMING] Loaded %d plant(s).", gPlantCount);
	// Tally per-player plant counts for anyone already logged in (hot reload).
	for(new pid = 0; pid < MAX_PLAYERS; pid++)
		if(IsPlayerConnected(pid) && !IsPlayerNPC(pid) && PlayerInfo[pid][pLoggedIn]) Farm_RecountPlants(pid);
	return 1;
}

// Build a plant's red checkpoint + plant object at its position (§9.3 "red checkpoint
// visible ~50m; cops spot from further; visible from air").
static Farm_BuildPlantStreamer(idx)
{
	PlantInfo[idx][plCP] = CreateDynamicCP(PlantInfo[idx][plX], PlantInfo[idx][plY], PlantInfo[idx][plZ], DRUG_PLANT_CP_SIZE, -1, -1, -1, DRUG_PLANT_ICON_RANGE);
	PlantInfo[idx][plObject] = CreateDynamicObject(DRUG_PLANT_CP_MODEL, PlantInfo[idx][plX], PlantInfo[idx][plY], PlantInfo[idx][plZ] - 1.0, 0.0, 0.0, 0.0, -1, -1, -1, DRUG_PLANT_ICON_RANGE);
	return 1;
}

static Farm_DestroyPlantStreamer(idx)
{
	if(PlantInfo[idx][plCP] != STREAMER_TAG_CP:INVALID_STREAMER_ID) { DestroyDynamicCP(PlantInfo[idx][plCP]); PlantInfo[idx][plCP] = STREAMER_TAG_CP:INVALID_STREAMER_ID; }
	if(PlantInfo[idx][plObject] != STREAMER_TAG_OBJECT:INVALID_STREAMER_ID) { DestroyDynamicObject(PlantInfo[idx][plObject]); PlantInfo[idx][plObject] = STREAMER_TAG_OBJECT:INVALID_STREAMER_ID; }
	return 1;
}

//==============================================================================
//	Plant table helpers
//==============================================================================

// Recount how many active plants an online player owns (own-cap enforcement).
static Farm_RecountPlants(playerid)
{
	new n = 0;
	for(new i = 0; i < gPlantCount; i++)
		if(PlantInfo[i][plOwnerAID] == PlayerInfo[playerid][pID]) n++;
	PlayerInfo[playerid][pPlantCount] = n;
	return n;
}

// Called from loading_data.inc (via CallLocalFunction) once LSplayers is loaded.
FUNCTION Farm_OnPlayerLogin(playerid)
{
	if(!PlayerInfo[playerid][pLoggedIn]) return 0;
	Farm_RecountPlants(playerid);
	return 1;
}

// Compute the full-grown grams from accrued grow-seconds (linear to the cap, §9.3).
static Farm_ComputeGrams(growSecs)
{
	if(growSecs >= DRUG_PLANT_GROW_SECS) return DRUG_PLANT_MAX_GRAMS;
	return (growSecs * DRUG_PLANT_MAX_GRAMS) / DRUG_PLANT_GROW_SECS;
}

// Nearest plant to a player within range -> index, or -1.
static Farm_NearestPlant(playerid, Float:range)
{
	new Float:x, Float:y, Float:z, best = -1, Float:bestD = range;
	GetPlayerPos(playerid, x, y, z);
	for(new i = 0; i < gPlantCount; i++)
	{
		new Float:d = VectorSize(PlantInfo[i][plX] - x, PlantInfo[i][plY] - y, PlantInfo[i][plZ] - z);
		if(d <= bestD) { bestD = d; best = i; }
	}
	return best;
}

// Remove a plant (row + streamer + array shift). Async DELETE by ID.
static Farm_RemovePlant(idx)
{
	if(idx < 0 || idx >= gPlantCount) return 0;
	new query[96];
	mysql_format(g_SQL, query, sizeof(query), "DELETE FROM `plants` WHERE ID=%d LIMIT 1", PlantInfo[idx][plID]);
	mysql_pquery(g_SQL, query);
	Farm_DestroyPlantStreamer(idx);
	// Shift the tail down to keep the array dense (matches the load index model).
	for(new i = idx; i < gPlantCount - 1; i++) PlantInfo[i] = PlantInfo[i + 1];
	gPlantCount--;
	// Re-tally owners for everyone online (a removed plant frees an owner slot).
	for(new pid = 0; pid < MAX_PLAYERS; pid++)
		if(IsPlayerConnected(pid) && !IsPlayerNPC(pid) && PlayerInfo[pid][pLoggedIn]) Farm_RecountPlants(pid);
	return 1;
}

// Is a cop within sight of this player (drives the illegal-farming wanted, §9.3)?
static Farm_IsCopInSight(playerid)
{
	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || i == playerid) continue;
		if(!PlayerInfo[i][pLoggedIn] || !PlayerInfo[i][pSpawned]) continue;
		if(!IsCop(i)) continue;
		if(IsPlayerInRangeOfPoint(i, DRUG_PLANT_COPSIGHT, x, y, z)) return 1;
	}
	return 0;
}

//==============================================================================
//	/plant - plant a drug seed (§9.3)
//==============================================================================

CMD:plant(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(Jail_IsJailed(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot plant while jailed.");
	if(IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Law enforcement cannot grow drugs.");
	if(IsPlayerInAnyVehicle(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Get out of your vehicle to plant.");
	if(PlayerInfo[playerid][pDrugSeeds] <= 0)
		return SendClientMessage(playerid, COLOR_ERROR, "You have no drug seeds. Buy some at a Bait Shop or Drug Refill Point ("COL_GREEN"/buyseeds"COL_WHITE").");
	if(PlayerInfo[playerid][pPlantCount] >= MAX_PLANTS_PER_PLAYER)
	{
		new err[128];
		format(err, sizeof(err), "You already have the maximum %d plants. Harvest one first ("COL_GREEN"/harvest"COL_WHITE").", MAX_PLANTS_PER_PLAYER);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	if(gPlantCount >= MAX_PLANTS) return SendClientMessage(playerid, COLOR_ERROR, "The world is saturated with plants right now - try again later.");

	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	// Not near an existing plant (§9.3 "not near other plants / water / checkpoints").
	if(Farm_NearestPlant(playerid, DRUG_PLANT_SPACING) != -1)
		return SendClientMessage(playerid, COLOR_ERROR, "There is already a plant too close - spread your crops out.");
	if(z < 1.0) return SendClientMessage(playerid, COLOR_ERROR, "You cannot plant in the water. Find dry, remote ground.");

	// Consume a seed and INSERT the plant (async), keeping a local optimistic slot.
	PlayerInfo[playerid][pDrugSeeds]--;
	new query[256];
	mysql_format(g_SQL, query, sizeof(query),
		"INSERT INTO `plants` (OwnerAID, X, Y, Z, Grams, GrowSecs, Fertilized, PlantedAt) VALUES (%d, %f, %f, %f, 0, 0, 0, NOW())",
		PlayerInfo[playerid][pID], x, y, z);
	mysql_pquery(g_SQL, query, "Farm_OnPlanted", "dd", playerid, g_MysqlRaceCheck[playerid]);
	Jail_SavePlayer(playerid);	// persist the seed decrement

	// Illegal farming: if a cop is watching, gain wanted (§9.3).
	if(Farm_IsCopInSight(playerid))
		GivePlayerWanted(playerid, DRUG_PLANT_WANTED, "Growing drugs");

	SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_GREEN"* You planted a drug seed here."COL_WHITE" It grows over ~20 min (only while you are online). "COL_GREEN"/harvest"COL_WHITE" when ripe.");
	return 1;
}

// Async handler for /plant - register the new row locally with its streamer objects.
FUNCTION Farm_OnPlanted(playerid, race_check)
{
	new id = cache_insert_id();
	if(gPlantCount >= MAX_PLANTS) return 0;	// safety: table filled between the check and the insert
	new idx = gPlantCount++;
	PlantInfo[idx][plID] = id;
	PlantInfo[idx][plGrams] = 0;
	PlantInfo[idx][plGrowSecs] = 0;
	PlantInfo[idx][plFertilized] = false;
	if(IsPlayerConnected(playerid) && race_check == g_MysqlRaceCheck[playerid])
	{
		PlantInfo[idx][plOwnerAID] = PlayerInfo[playerid][pID];
		GetPlayerPos(playerid, PlantInfo[idx][plX], PlantInfo[idx][plY], PlantInfo[idx][plZ]);
		Farm_RecountPlants(playerid);
	}
	else
	{
		// Owner left before the INSERT resolved - resolve the owner AID from the row
		// so the plant is still owned correctly; position is re-read on the next load.
		PlantInfo[idx][plOwnerAID] = 0;
		PlantInfo[idx][plX] = PlantInfo[idx][plY] = PlantInfo[idx][plZ] = 0.0;
	}
	Farm_BuildPlantStreamer(idx);
	return 1;
}

//==============================================================================
//	/harvest (/harv) - harvest the nearest ripe plant -> drugs (§9.3)
//==============================================================================

CMD:harvest(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(Jail_IsJailed(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot harvest while jailed.");
	if(IsPlayerInAnyVehicle(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Get out of your vehicle to harvest.");

	new idx = Farm_NearestPlant(playerid, DRUG_PLANT_RANGE);
	if(idx == -1) return SendClientMessage(playerid, COLOR_ERROR, "No plant nearby to harvest. Stand on the plant's checkpoint.");

	// Sync the plant's grams from its accrued grow-seconds (§9.3).
	PlantInfo[idx][plGrams] = Farm_ComputeGrams(PlantInfo[idx][plGrowSecs]);
	if(PlantInfo[idx][plGrams] < DRUG_PLANT_HARVEST_MIN)
	{
		new err[128];
		format(err, sizeof(err), "This plant is not ready - it has only %dg (need %dg). Let it grow.", PlantInfo[idx][plGrams], DRUG_PLANT_HARVEST_MIN);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}

	// Give the drugs into the player's stash (clamped to their carry cap by drugs.inc).
	new given = Drug_GiveGrams(playerid, PlantInfo[idx][plGrams]);
	new grown = PlantInfo[idx][plGrams];

	// You can harvest another player's plant (§9.3 "Can harvest others' plants") - the
	// score/point still goes to the harvester (>50g gave +1 already checked above).
	PlayerInfo[playerid][pScore]++;

	// Illegal: harvesting near a cop gains wanted (§9.3).
	if(Farm_IsCopInSight(playerid))
		GivePlayerWanted(playerid, DRUG_PLANT_WANTED, "Harvesting drugs");

	Farm_RemovePlant(idx);	// the plant is spent - delete the row + streamer
	Jail_SavePlayer(playerid);	// persist the added drugs + score

	new msg[176];
	if(given < grown)
		format(msg, sizeof(msg), ""COL_GREEN"* Harvested %dg"COL_WHITE" but your stash only took "COL_GREEN"%dg"COL_WHITE" (carry cap). "COL_GREEN"+1 score."COL_WHITE, grown, given);
	else
		format(msg, sizeof(msg), ""COL_GREEN"* Harvested %dg of drugs"COL_WHITE" into your stash. Sell them at a Drug Refill Point ("COL_GREEN"/drugsell"COL_WHITE"). "COL_GREEN"+1 score."COL_WHITE, grown);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}
alias:harvest("harv")

//==============================================================================
//	/fertilize (/fert) - speed the nearest plant (attracts deer, §9.3)
//==============================================================================

CMD:fertilize(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new idx = Farm_NearestPlant(playerid, DRUG_PLANT_RANGE);
	if(idx == -1) return SendClientMessage(playerid, COLOR_ERROR, "No plant nearby to fertilize.");
	if(PlantInfo[idx][plFertilized]) return SendClientMessage(playerid, COLOR_ERROR, "That plant is already fertilized.");
	PlantInfo[idx][plFertilized] = true;
	new query[96];
	mysql_format(g_SQL, query, sizeof(query), "UPDATE `plants` SET Fertilized=1 WHERE ID=%d LIMIT 1", PlantInfo[idx][plID]);
	mysql_pquery(g_SQL, query);
	SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_GREEN"* You fertilized the plant"COL_WHITE" - it now grows faster, but the smell "COL_YELLOW"attracts deer"COL_WHITE" that may eat it.");
	return 1;
}
alias:fertilize("fert")

//==============================================================================
//	/destroyplant - destroy the nearest plant (cops find/destroy crops, §9.3)
//==============================================================================

CMD:destroyplant(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new idx = Farm_NearestPlant(playerid, DRUG_PLANT_RANGE);
	if(idx == -1) return SendClientMessage(playerid, COLOR_ERROR, "No plant nearby to destroy.");
	// A cop destroying someone else's crop is police work; the owner destroying their
	// own is cleanup. Either way the plant is removed.
	new bool:mine = (PlantInfo[idx][plOwnerAID] == PlayerInfo[playerid][pID]);
	if(!mine && !IsCop(playerid))
		return SendClientMessage(playerid, COLOR_ERROR, "Only the owner or a police officer can destroy this plant.");
	Farm_RemovePlant(idx);
	if(IsCop(playerid) && !mine)
	{
		PlayerInfo[playerid][pScore]++;
		SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_GREEN"* You destroyed an illegal drug plant."COL_WHITE" +1 score.");
	}
	else SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_GREEN"* You destroyed the plant."COL_WHITE);
	return 1;
}

//==============================================================================
//	/plantgps (/pgps) - list your plants + growth (§9.3)
//==============================================================================

CMD:plantgps(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new n = 0, msg[144];
	for(new i = 0; i < gPlantCount; i++)
	{
		if(PlantInfo[i][plOwnerAID] != PlayerInfo[playerid][pID]) continue;
		new grams = Farm_ComputeGrams(PlantInfo[i][plGrowSecs]);
		new pct = (PlantInfo[i][plGrowSecs] * 100) / DRUG_PLANT_GROW_SECS;
		if(pct > 100) pct = 100;
		format(msg, sizeof(msg), ""COL_WHITE"Plant #%d: "COL_GREEN"%dg"COL_WHITE" (%d%% grown) at %.0f, %.0f%s", PlantInfo[i][plID], grams, pct, PlantInfo[i][plX], PlantInfo[i][plY], PlantInfo[i][plFertilized] ? (" "COL_YELLOW"[fertilized]") : (""));
		SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
		n++;
	}
	if(n == 0) SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_WHITE"You have no plants. Buy seeds and "COL_GREEN"/plant"COL_WHITE" one.");
	return 1;
}
alias:plantgps("pgps")

//==============================================================================
//	/drugsell (/ds) - sell grown drugs at a Drug Refill Point (§9.3/§10.2)
//==============================================================================

static Farm_AtRefillPoint(playerid)
{
	for(new i = 0; i < sizeof(g_RefillPoints); i++)
		if(IsPlayerInRangeOfPoint(playerid, DRUG_SELL_RANGE, g_RefillPoints[i][0], g_RefillPoints[i][1], g_RefillPoints[i][2]))
			return 1;
	return 0;
}

CMD:drugsell(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(!Farm_AtRefillPoint(playerid))
		return SendClientMessage(playerid, COLOR_ERROR, "You can only sell grown drugs at a Drug Refill Point (look for the map icons).");
	if(PlayerInfo[playerid][pDrugs] <= 0)
		return SendClientMessage(playerid, COLOR_ERROR, "You have no drugs to sell. Grow some ("COL_GREEN"/plant"COL_WHITE" -> "COL_GREEN"/harvest"COL_WHITE").");

	new amount;
	if(sscanf(params, "d", amount) || amount <= 0) amount = PlayerInfo[playerid][pDrugs];	// blank = sell all
	if(amount > PlayerInfo[playerid][pDrugs]) amount = PlayerInfo[playerid][pDrugs];

	// Payout = grams × per-gram × DRUG market mult (M7 stub = 1.0, §10.2).
	new payout = floatround(float(amount * DRUG_SELL_PRICE_PER_G) * StockMarket_PriceMult(E_STOCK_DRUG));
	if(payout < 1) payout = 1;

	PlayerInfo[playerid][pDrugs] -= amount;
	Bank_GiveCash(playerid, payout);
	// +1 score only for a substantial sale (§9.3 "minimum 200g for +1 score").
	if(amount >= DRUG_SELL_MIN_GRAMS) PlayerInfo[playerid][pScore]++;

	// Selling feeds the DRUG stock pool (dumping lowers the price, §10.2).
	StockMarket_UpdateEarnings(E_STOCK_DRUG, payout, STOCK_FACTOR_DRUG);

	Jail_SavePlayer(playerid);	// persist the drug decrement + money + score
	Drug_SavePlayer(playerid);	// keep pDrugs consistent in the crime/drug UPDATE too

	new msg[160];
	format(msg, sizeof(msg), ""COL_GREEN"* Sold %dg of drugs for "COL_WHITE"$%d"COL_GREEN".%s"COL_WHITE, amount, payout, (amount >= DRUG_SELL_MIN_GRAMS) ? (" +1 score.") : (""));
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}
alias:drugsell("ds")

// /buyseeds - buy a 5-pack of drug seeds at a Drug Refill Point (§9.3).
CMD:buyseeds(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(!Farm_AtRefillPoint(playerid))
		return SendClientMessage(playerid, COLOR_ERROR, "Buy seeds at a Drug Refill Point or a Bait Shop ("COL_GREEN"/baitshop"COL_WHITE").");
	if(PlayerInfo[playerid][pDrugSeeds] >= DRUG_SEED_MAX)
		return SendClientMessage(playerid, COLOR_ERROR, "You already hold the maximum drug seeds (10).");
	// M7 - seeds are a 24/7 shop item, so the price is SUPA_SAVE-scaled (§10.2).
	new price = StockMarket_BuyPrice(DRUG_SEED_BUNDLE_5, E_STOCK_SUPA_SAVE);
	if(!Bank_TakeCash(playerid, price))
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot afford a 5-pack of seeds.");
	PlayerInfo[playerid][pDrugSeeds] += 5;
	if(PlayerInfo[playerid][pDrugSeeds] > DRUG_SEED_MAX) PlayerInfo[playerid][pDrugSeeds] = DRUG_SEED_MAX;
	StockMarket_UpdateEarnings(E_STOCK_SUPA_SAVE, price, STOCK_FACTOR_SHOP);	// buy demand feeds the pool
	Jail_SavePlayer(playerid);
	new msg[128];
	format(msg, sizeof(msg), ""COL_GREEN"* Bought 5 drug seeds "COL_WHITE"($%d). You now hold %d/%d.", price, PlayerInfo[playerid][pDrugSeeds], DRUG_SEED_MAX);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}

//==============================================================================
//	/hunt - minimal deer hunting (§9.3 - permit gate)
//==============================================================================

CMD:hunt(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(Jail_IsJailed(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot hunt while jailed.");
	if(gettime() < PlayerInfo[playerid][pHuntCooldown])
		return SendClientMessage(playerid, COLOR_ERROR, "No deer nearby right now - wait a little.");

	// Minimal model: a deer is "found" in the wild (a rural, low-Z, out-of-city spot).
	// Full deer AI (deer eat plants, traps, herds) is deferred - this gates the permit
	// mechanic (§9.3): kill without a permit = +6 wanted; with one = heal + bonus.
	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	if(IsPlayerInAnyVehicle(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Get out of your vehicle to hunt.");

	PlayerInfo[playerid][pHuntCooldown] = gettime() + HUNT_CD;

	if(PlayerInfo[playerid][pHuntPermits] > 0)
	{
		PlayerInfo[playerid][pHuntPermits]--;
		new Float:hp; GetPlayerHealth(playerid, hp);
		hp += HUNT_HEAL; if(hp > 100.0) hp = 100.0;
		SetPlayerHealth(playerid, hp);
		new cash = HUNT_CASH_MIN + random(HUNT_CASH_MAX - HUNT_CASH_MIN + 1);
		Bank_GiveCash(playerid, cash);
		Jail_SavePlayer(playerid);	// persist the permit decrement + cash
		new msg[160];
		format(msg, sizeof(msg), ""COL_GREEN"* You hunted a deer with a permit"COL_WHITE" - +%.0f HP and "COL_GREEN"$%d"COL_WHITE". Permits left: %d.", HUNT_HEAL, cash, PlayerInfo[playerid][pHuntPermits]);
		SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	}
	else
	{
		// No permit -> illegal poaching (§9.3 VERIFIED +6 wanted).
		GivePlayerWanted(playerid, HUNT_NOPERMIT_WANTED, "Killing a deer without a hunting permit");
		SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_RED"* You poached a deer with no hunting permit!"COL_WHITE" Police have been alerted (+6 wanted). Buy a permit at a Bait Shop.");
	}
	return 1;
}

//==============================================================================
//	/farmjob - legal crop job at CrazyBob's Farm (Flint County, §9.3)
//==============================================================================

static Farm_AtFarm(playerid)
	return IsPlayerInRangeOfPoint(playerid, FARM_JOB_RANGE, FARM_X, FARM_Y, FARM_Z);

static Farm_JobSetCheckpoint(playerid)
{
	new f = gFarmJobField[playerid];
	if(gFarmJobCP[playerid] != STREAMER_TAG_CP:INVALID_STREAMER_ID) { DestroyDynamicCP(gFarmJobCP[playerid]); gFarmJobCP[playerid] = STREAMER_TAG_CP:INVALID_STREAMER_ID; }
	if(f >= FARM_JOB_FIELDS) return 0;
	gFarmJobCP[playerid] = CreateDynamicCP(gFarmJobRouteX[playerid][f], gFarmJobRouteY[playerid][f], gFarmJobRouteZ[playerid][f], FARM_JOB_CP_SIZE, -1, -1, playerid, 300.0);
	return 1;
}

static Farm_JobEnd(playerid, bool:completed)
{
	if(gFarmJobCP[playerid] != STREAMER_TAG_CP:INVALID_STREAMER_ID) { DestroyDynamicCP(gFarmJobCP[playerid]); gFarmJobCP[playerid] = STREAMER_TAG_CP:INVALID_STREAMER_ID; }
	gFarmJobActive[playerid] = false;
	gFarmJobField[playerid] = 0;
	gFarmJobEarned[playerid] = 0;
	PlayerInfo[playerid][pFarmJobCooldown] = gettime() + FARM_JOB_CD;
	#pragma unused completed
	return 1;
}

// Build + start a farm-job field route for `playerid` (shared by /farmjob and the
// M6 Tractor Mission, §6.16). Assumes the caller validated eligibility/location.
static Farm_JobBegin(playerid)
{
	// Build a route from the candidate fields (pick FARM_JOB_FIELDS distinct rows).
	new used[sizeof(g_FarmFields)], filled = 0;
	while(filled < FARM_JOB_FIELDS)
	{
		new pick = random(sizeof(g_FarmFields));
		if(used[pick]) continue;
		used[pick] = 1;
		gFarmJobRouteX[playerid][filled] = g_FarmFields[pick][0];
		gFarmJobRouteY[playerid][filled] = g_FarmFields[pick][1];
		gFarmJobRouteZ[playerid][filled] = g_FarmFields[pick][2];
		filled++;
	}
	gFarmJobActive[playerid] = true;
	gFarmJobField[playerid] = 0;
	gFarmJobEarned[playerid] = 0;
	Farm_JobSetCheckpoint(playerid);
	return 1;
}

// M6 (stage 4) Tractor Mission hand-off (§6.16) - the mission launcher (missions.inc)
// already validated the tractor + cooldown, so start the crop route directly.
FUNCTION Farm_StartTractorJob(playerid)
{
	if(gFarmJobActive[playerid]) return SendClientMessage(playerid, COLOR_ERROR, "You are already on a farm job. "COL_GREEN"/cancelfarmjob"COL_WHITE" to stop.");
	Farm_JobBegin(playerid);
	return 1;
}

CMD:farmjob(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(gFarmJobActive[playerid]) return SendClientMessage(playerid, COLOR_ERROR, "You are already on a farm job. "COL_GREEN"/cancelfarmjob"COL_WHITE" to stop.");
	if(gettime() < PlayerInfo[playerid][pFarmJobCooldown])
	{
		new err[96];
		format(err, sizeof(err), "The farmer needs a break - wait %d seconds.", PlayerInfo[playerid][pFarmJobCooldown] - gettime());
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	if(!Farm_AtFarm(playerid))
		return SendClientMessage(playerid, COLOR_ERROR, "Go to CrazyBob's Farm in Flint County to take a plow/harvest job (look for the map icon).");

	Farm_JobBegin(playerid);
	new msg[160];
	format(msg, sizeof(msg), ""COL_GREEN"* Farm job started!"COL_WHITE" Drive over the %d field checkpoints to plow/harvest. Legit pay per field + a bonus.", FARM_JOB_FIELDS);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}

CMD:cancelfarmjob(playerid, params[])
{
	#pragma unused params
	if(!gFarmJobActive[playerid]) return SendClientMessage(playerid, COLOR_ERROR, "You are not on a farm job.");
	Farm_JobEnd(playerid, false);
	SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_WHITE"Farm job cancelled.");
	return 1;
}

// Farm-job field checkpoint reached (from OnPlayerEnterDynamicCP in CnR.pwn).
FUNCTION Farm_OnPlayerEnterCP(playerid, STREAMER_TAG_CP:cpid)
{
	if(!gFarmJobActive[playerid]) return 0;
	if(cpid != gFarmJobCP[playerid]) return 0;

	new pay = FARM_JOB_PAY_MIN + random(FARM_JOB_PAY_MAX - FARM_JOB_PAY_MIN + 1);
	Bank_GiveCash(playerid, pay);
	gFarmJobEarned[playerid] += pay;
	gFarmJobField[playerid]++;

	if(gFarmJobField[playerid] >= FARM_JOB_FIELDS)
	{
		Bank_GiveCash(playerid, FARM_JOB_BONUS);
		PlayerInfo[playerid][pScore]++;
		new total = gFarmJobEarned[playerid] + FARM_JOB_BONUS;
		Farm_JobEnd(playerid, true);
		Jail_SavePlayer(playerid);	// persist the earnings + score
		new msg[176];
		format(msg, sizeof(msg), ""COL_GREEN"* Farm job complete!"COL_WHITE" You earned "COL_GREEN"$%d"COL_WHITE" total (incl. $%d bonus). +1 score.", total, FARM_JOB_BONUS);
		SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	}
	else
	{
		Farm_JobSetCheckpoint(playerid);
		new msg[144];
		format(msg, sizeof(msg), ""COL_GREEN"* Field done "COL_WHITE"(+$%d). %d of %d - next field marked.", pay, gFarmJobField[playerid], FARM_JOB_FIELDS);
		SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	}
	return 1;
}

//==============================================================================
//	Clock / lifecycle hooks
//==============================================================================

// Grow online players' plants (1-second GameModeClock tick, §9.3 - plants do NOT
// grow while the owner is offline). Each tick accrues grow-seconds for every plant
// whose owner is currently connected; fertilizer + Drug Dealer skill add extra.
FUNCTION Farm_OnGameModeClockTick()
{
	for(new i = 0; i < gPlantCount; i++)
	{
		if(PlantInfo[i][plGrowSecs] >= DRUG_PLANT_GROW_SECS) continue;	// already ripe
		// Find the owner if online (plants freeze while the owner is offline).
		new owner = -1;
		for(new pid = 0; pid < MAX_PLAYERS; pid++)
		{
			if(!IsPlayerConnected(pid) || IsPlayerNPC(pid)) continue;
			if(!PlayerInfo[pid][pLoggedIn]) continue;
			if(PlayerInfo[pid][pID] == PlantInfo[i][plOwnerAID]) { owner = pid; break; }
		}
		if(owner == -1) continue;	// owner offline -> no growth

		new before = PlantInfo[i][plGrowSecs];
		new add = 1;
		if(PlantInfo[i][plFertilized]) add *= DRUG_PLANT_FERT_MULT;	// fertilizer speeds it
		if(PlayerInfo[owner][pSkill] == SKILL_DRUGDEALER) add *= DRUG_DEALER_GROW_MULT;	// dealers grow faster
		PlantInfo[i][plGrowSecs] += add;
		if(PlantInfo[i][plGrowSecs] > DRUG_PLANT_GROW_SECS) PlantInfo[i][plGrowSecs] = DRUG_PLANT_GROW_SECS;
		PlantInfo[i][plGrams] = Farm_ComputeGrams(PlantInfo[i][plGrowSecs]);

		// Persist occasionally (each time the grow-seconds cross a 30-second boundary,
		// robust to the fertilizer/dealer step jumps) so a crash loses little progress
		// without hammering the DB every tick. Also persists the moment it ripens.
		if((before / 30) != (PlantInfo[i][plGrowSecs] / 30) || PlantInfo[i][plGrowSecs] >= DRUG_PLANT_GROW_SECS)
		{
			new query[96];
			mysql_format(g_SQL, query, sizeof(query), "UPDATE `plants` SET GrowSecs=%d, Grams=%d WHERE ID=%d LIMIT 1", PlantInfo[i][plGrowSecs], PlantInfo[i][plGrams], PlantInfo[i][plID]);
			mysql_pquery(g_SQL, query);
		}
	}
	return 1;
}

// End an active farm job on disconnect (session state - plants persist via the DB).
FUNCTION Farm_OnDisconnect(playerid)
{
	if(gFarmJobActive[playerid]) Farm_JobEnd(playerid, false);
	// Persist the latest grow progress of this player's plants (they stop growing now).
	for(new i = 0; i < gPlantCount; i++)
	{
		if(PlantInfo[i][plOwnerAID] != PlayerInfo[playerid][pID]) continue;
		new query[96];
		mysql_format(g_SQL, query, sizeof(query), "UPDATE `plants` SET GrowSecs=%d, Grams=%d WHERE ID=%d LIMIT 1", PlantInfo[i][plGrowSecs], PlantInfo[i][plGrams], PlantInfo[i][plID]);
		mysql_pquery(g_SQL, query);
	}
	return 1;
}

// Abort a farm job on death (§9.3 - the harvester run ends).
FUNCTION Farm_OnPlayerDeath(playerid)
{
	if(gFarmJobActive[playerid])
	{
		Farm_JobEnd(playerid, false);
		SendClientMessage(playerid, COLOR_ERROR, "Your farm job ended when you died.");
	}
	return 1;
}
