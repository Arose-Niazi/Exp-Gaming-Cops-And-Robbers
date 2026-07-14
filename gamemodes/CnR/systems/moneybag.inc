/*

	Money bag (hidden "lost Mafia money") for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 4 (stage 2) - design §7 (/moneybag) + economy.md §5 (money bag =
	hidden lost Mafia money the first finder keeps).

	One hidden money bag exists at a time. It sits at a random spawn point; the
	first player to reach it (grab the pickup or /moneybag while close) is paid a
	random reward (random(MONEYBAG_REWARD_MIN..MAX)) via Bank_GiveCash + 1 score,
	the server is told, and the bag relocates. If nobody finds it for
	MONEYBAG_TIMEOUT_SECS the bag relocates on its own (no reward).

	/moneybag: not carrying (you never "carry" it in this model) -> a COARSE zone
	hint - warmer/colder by distance - so players hunt it down. The pickup itself
	is the grab; /moneybag is the compass. /addmoneybag (scripter) forces a bag
	at a chosen/random location with a set/random reward for testing/events.

	The timeout is driven off the game-clock 1-second tick (Moneybag_OnTick), NOT
	a new global timer, matching the project's no-new-timer rule.

	Credits:
		economy.md §5 for the money-bag mechanic grounding.

	Public API:
		Moneybag_Spawn(Float:x, Float:y, Float:z, reward)  : place a bag (reward<=0 => random)
		Moneybag_SpawnRandom()                             : place a bag at a random spot, random reward
		Moneybag_Exists()                                  : bool - is a bag active
		Moneybag_OnPickup(playerid, pickupid)              : hook from OnPlayerPickUpDynamicPickup
		Moneybag_OnTick()                                  : hook from GameModeClock() (1s) - timeout
		Moneybag_Cleanup()                                 : destroy the pickup (OnGameModeExit)

	Commands:
		/moneybag (/mb)   (player)   zone hint to the active bag (warmer/colder)
		/addmoneybag      (scripter) spawn a bag (chosen/random location + reward)

	Functions Being Called:
		bank: Bank_GiveCash
*/

//==============================================================================
//	State
//==============================================================================

static gMoneybagPickup = -1;			// active dynamic pickup id (-1 = none)
static Float:gMoneybagX;
static Float:gMoneybagY;
static Float:gMoneybagZ;
static gMoneybagReward = 0;				// reward paid to the finder
static gMoneybagSpawnTime = 0;			// gettime() the bag was placed (timeout base)

// Candidate spawn points (Los Santos, open reachable ground) [OUR DESIGN].
static const Float:gMoneybagSpots[][3] =
{
	{ 1479.0,  -1734.0,  13.5 },		// near the PD / downtown
	{ 2035.0,  -1417.0,  17.2 },		// hospital
	{ 1421.0,  -1046.0,  25.7 },		// bank / commerce
	{ 2447.0,  -1971.0,  13.5 },		// Grove Street
	{ 1930.0,  -2050.0,  13.5 },		// LS airport road
	{ 1544.0,  -1675.0,  16.2 },		// City Hall
	{ 2200.0,  -1160.0,  25.9 },		// Mulholland
	{ 1830.0,  -1130.0,  24.0 }			// Vinewood-ish
};

//==============================================================================
//	Internal helpers
//==============================================================================

// A random reward inside the design band.
static Moneybag_RandomReward()
	return MONEYBAG_REWARD_MIN + random(MONEYBAG_REWARD_MAX - MONEYBAG_REWARD_MIN + 1);

// Destroy the current pickup (if any) without announcing.
static Moneybag_Clear()
{
	if(gMoneybagPickup != -1)
	{
		DestroyDynamicPickup(gMoneybagPickup);
		gMoneybagPickup = -1;
	}
	return 1;
}

//==============================================================================
//	Spawn / relocate (design §7)
//==============================================================================

FUNCTION Moneybag_Exists()
	return (gMoneybagPickup != -1);

// Place a bag at a specific point with a specific reward (reward<=0 => random).
FUNCTION Moneybag_Spawn(Float:x, Float:y, Float:z, reward)
{
	Moneybag_Clear();
	gMoneybagX = x;
	gMoneybagY = y;
	gMoneybagZ = z;
	gMoneybagReward = (reward > 0) ? reward : Moneybag_RandomReward();
	gMoneybagSpawnTime = gettime();
	gMoneybagPickup = CreateDynamicPickup(MONEYBAG_MODEL, 1, x, y, z, 0, 0, -1, 100.0);
	return 1;
}

// Place a bag at a random candidate spot with a random reward.
FUNCTION Moneybag_SpawnRandom()
{
	new pick = random(sizeof(gMoneybagSpots));
	return Moneybag_Spawn(gMoneybagSpots[pick][0], gMoneybagSpots[pick][1], gMoneybagSpots[pick][2], 0);
}

// Pay a finder and relocate the bag. Shared by the pickup grab and /moneybag
// when the player is already standing on it.
static Moneybag_Claim(playerid)
{
	new reward = gMoneybagReward;
	Bank_GiveCash(playerid, reward);
	PlayerInfo[playerid][pScore] += 1;

	new string[160];
	format(string, sizeof(string), ""COL_YELLOW"[MONEY BAG] "COL_WHITE"%s (%d) found the lost money bag and pocketed "COL_GREEN"$%d"COL_WHITE"!", PlayerInfo[playerid][pUserName], playerid, reward);
	SendClientMessageToAll(COLOR_SERVER_INFO, string);
	SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_GREEN"* You grabbed the money bag! "COL_WHITE"+1 score. A new bag has been hidden.");

	// Relocate to a fresh random spot with a fresh reward.
	Moneybag_SpawnRandom();
	return 1;
}

//==============================================================================
//	Pickup grab - hooked from OnPlayerPickUpDynamicPickup (design §7)
//==============================================================================

FUNCTION Moneybag_OnPickup(playerid, pickupid)
{
	if(gMoneybagPickup == -1 || pickupid != gMoneybagPickup) return 0;
	if(!PlayerInfo[playerid][pLoggedIn] || IsPlayerNPC(playerid)) return 0;
	Moneybag_Claim(playerid);
	return 1;
}

//==============================================================================
//	Timeout tick - hooked from GameModeClock() (1s, no new timer, design §7)
//==============================================================================

FUNCTION Moneybag_OnTick()
{
	if(gMoneybagPickup == -1) return 0;
	if((gettime() - gMoneybagSpawnTime) < MONEYBAG_TIMEOUT_SECS) return 0;
	// Unclaimed too long - quietly relocate (no reward).
	Moneybag_SpawnRandom();
	SendClientMessageToAll(COLOR_SERVER_INFO, ""COL_YELLOW"[MONEY BAG] "COL_WHITE"The hidden money bag moved to a new location. Use "COL_GREEN"/moneybag"COL_WHITE" to hunt it.");
	return 1;
}

//==============================================================================
//	Cleanup - OnGameModeExit
//==============================================================================

FUNCTION Moneybag_Cleanup()
	return Moneybag_Clear();

//==============================================================================
//	Command - /moneybag (/mb) : coarse zone hint (warmer/colder) (design §7)
//==============================================================================

CMD:moneybag(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(gMoneybagPickup == -1)
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "There is no money bag hidden right now. One appears from time to time (or a scripter can /addmoneybag).");

	// Distance to the bag (same coarse hint whether on foot or in a vehicle).
	new Float:d = GetPlayerDistanceFromPoint(playerid, gMoneybagX, gMoneybagY, gMoneybagZ);

	// Standing on it -> claim it (covers players who walk over without the pickup
	// streaming in, e.g. different interior/VW edge - keep it forgiving).
	if(d <= MONEYBAG_PICKUP_RANGE)
	{
		Moneybag_Claim(playerid);
		return 1;
	}

	new string[144];
	if(d <= MONEYBAG_HINT_WARM)
		format(string, sizeof(string), ""COL_GREEN"* You are VERY WARM "COL_WHITE"- the money bag is almost under your feet. Keep looking!");
	else if(d <= MONEYBAG_HINT_HOT)
		format(string, sizeof(string), ""COL_GREEN"* You are WARM "COL_WHITE"- the money bag is close by.");
	else if(d <= MONEYBAG_HINT_MILD)
		format(string, sizeof(string), ""COL_YELLOW"* You are COLD "COL_WHITE"- the money bag is somewhere in this part of the city.");
	else
		format(string, sizeof(string), ""COL_RED"* You are FREEZING "COL_WHITE"- the money bag is far from here. Head across the city.");
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	return 1;
}
alias:moneybag("mb")

//==============================================================================
//	Command - /addmoneybag (scripter) : spawn a bag (chosen/random) (design §8.3)
//==============================================================================

CMD:addmoneybag(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_SCRIPTER) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);

	// USAGE: /addmoneybag [here|random] [reward|0 for random]
	new where[8], reward;
	if(sscanf(params, "s[8]D(0)", where, reward))
	{
		SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /addmoneybag [here|random] [reward (0 = random)]");
		return 1;
	}

	new string[160];
	if(!strcmp(where, "here", true))
	{
		new Float:x, Float:y, Float:z;
		GetPlayerPos(playerid, x, y, z);
		Moneybag_Spawn(x, y, z, reward);
		format(string, sizeof(string), "%s (%d) placed a money bag at their position (reward $%d).", PlayerInfo[playerid][pUserName], playerid, gMoneybagReward);
	}
	else
	{
		// Random spot; if a reward was given, honour it, else random.
		Moneybag_SpawnRandom();
		if(reward > 0) gMoneybagReward = reward;
		format(string, sizeof(string), "%s (%d) placed a money bag at a random location (reward $%d).", PlayerInfo[playerid][pUserName], playerid, gMoneybagReward);
	}
	SendMessageToAdmins(COLOR_ADMIN_INFO, string);
	SendClientMessage(playerid, COLOR_SERVER_INFO, "Money bag spawned. Players can hunt it with /moneybag.");
	SendClientMessageToAll(COLOR_SERVER_INFO, ""COL_YELLOW"[MONEY BAG] "COL_WHITE"A money bag has been hidden somewhere in the city! Use "COL_GREEN"/moneybag"COL_WHITE" to hunt it down.");
	return 1;
}
