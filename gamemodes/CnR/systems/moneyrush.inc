/*

	Money rush (cash-rain event) for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 4 (stage 2) — design §7 (/moneyrush) + §8.2 (/startmoneyrush owner)
	+ economy.md §6.2 (money rush).

	An owner-started cash-rain event. /startmoneyrush (owner) scatters
	MONEYRUSH_PICKUPS dynamic pickups around an arena; players /moneyrush (/mr) to
	join, then run around grabbing cash. Each pickup is worth random(1k..25k) paid
	via Bank_GiveCash. PvP is allowed during the rush (no wanted for kills between
	participants is NOT enforced here — the wanted system already suppresses inside
	DM zones; the rush is an open free-for-all by design). A per-event leaderboard
	tracks who grabbed the most (pMoneyRushEarned). The event runs for
	MONEYRUSH_DURATION seconds then ends automatically (or an owner force-ends it),
	with a MONEYRUSH_COOLDOWN before the next can start. All timing is driven off
	the game-clock 1-second tick (Moneyrush_OnTick), NOT a new global timer.

	Grabbed pickups respawn for the duration (the streamer pickup stays), so the
	arena keeps raining cash until the timer ends; all pickups are destroyed on
	end. Only joined participants are paid (others just see the pickups).

	Credits:
		economy.md §6.2 for the money-rush mechanic grounding.

	Public API:
		Moneyrush_Start(byWho)         : start the event (owner /startmoneyrush)
		Moneyrush_ForceEnd(byWho)      : end early (owner /startmoneyrush stop)
		Moneyrush_IsActive()           : bool
		Moneyrush_OnPickup(playerid, pickupid) : hook from OnPlayerPickUpDynamicPickup
		Moneyrush_OnTick()             : hook from GameModeClock() (1s) — duration/cooldown
		Moneyrush_OnDisconnect(playerid) : clear a leaving participant's join flag
		Moneyrush_Cleanup()            : destroy all pickups (OnGameModeExit)

	Commands:
		/moneyrush (/mr)  (player)  join the active event (§7)

	Functions Being Called:
		bank: Bank_GiveCash
*/

//==============================================================================
//	State
//==============================================================================

static bool:gRushActive = false;
static gRushEndsAt = 0;					// gettime() the active rush ends
static gRushCooldownUntil = 0;			// gettime() the next rush may start
static gRushPickups[MONEYRUSH_PICKUPS];	// dynamic pickup ids (-1 = unused)

//==============================================================================
//	Init — mark all pickup slots empty (arrays default to 0, not -1)
//==============================================================================

FUNCTION Moneyrush_Init()
{
	for(new i = 0; i < MONEYRUSH_PICKUPS; i++) gRushPickups[i] = -1;
	return 1;
}

//==============================================================================
//	Internal helpers
//==============================================================================

static Moneyrush_DestroyPickups()
{
	for(new i = 0; i < MONEYRUSH_PICKUPS; i++)
	{
		if(gRushPickups[i] != -1)
		{
			DestroyDynamicPickup(gRushPickups[i]);
			gRushPickups[i] = -1;
		}
	}
	return 1;
}

// Is `pickupid` one of ours? Returns the slot index or -1.
static Moneyrush_SlotOf(pickupid)
{
	for(new i = 0; i < MONEYRUSH_PICKUPS; i++)
		if(gRushPickups[i] == pickupid) return i;
	return -1;
}

// Announce + crown the top earner, clear all join flags, wipe leaderboard.
static Moneyrush_Finish(const how[])
{
	new best = -1, bestAmount = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(PlayerInfo[i][pInMoneyRush] && PlayerInfo[i][pMoneyRushEarned] > bestAmount)
		{
			bestAmount = PlayerInfo[i][pMoneyRushEarned];
			best = i;
		}
	}

	new string[160];
	format(string, sizeof(string), ""COL_YELLOW"[MONEY RUSH] "COL_WHITE"The cash rain has ended (%s).", how);
	SendClientMessageToAll(COLOR_SERVER_INFO, string);
	if(best != -1)
	{
		format(string, sizeof(string), ""COL_YELLOW"[MONEY RUSH] "COL_WHITE"Top grabber: "COL_GREEN"%s (%d)"COL_WHITE" with "COL_GREEN"$%d"COL_WHITE"!", PlayerInfo[best][pUserName], best, bestAmount);
		SendClientMessageToAll(COLOR_SERVER_INFO, string);
	}

	// Clear participation + leaderboard.
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i)) continue;
		PlayerInfo[i][pInMoneyRush] = false;
		PlayerInfo[i][pMoneyRushEarned] = 0;
	}
	return 1;
}

//==============================================================================
//	Start / end (design §7 / §8.2)
//==============================================================================

FUNCTION Moneyrush_IsActive()
	return gRushActive;

FUNCTION Moneyrush_Start(byWho)
{
	if(gRushActive) return 0;						// already running
	if(gettime() < gRushCooldownUntil) return 0;	// still cooling down

	// Scatter the pickups around the arena centre.
	for(new i = 0; i < MONEYRUSH_PICKUPS; i++)
	{
		new Float:ox = float(random(2 * floatround(MONEYRUSH_SPREAD))) - MONEYRUSH_SPREAD;
		new Float:oy = float(random(2 * floatround(MONEYRUSH_SPREAD))) - MONEYRUSH_SPREAD;
		gRushPickups[i] = CreateDynamicPickup(MONEYRUSH_PICKUP_MODEL, 1, MONEYRUSH_X + ox, MONEYRUSH_Y + oy, MONEYRUSH_Z, 0, 0, -1, 300.0);
	}

	gRushActive = true;
	gRushEndsAt = gettime() + MONEYRUSH_DURATION;

	new string[176];
	format(string, sizeof(string), ""COL_YELLOW"[MONEY RUSH] "COL_WHITE"A cash-rain event has started! Type "COL_GREEN"/moneyrush"COL_WHITE" (/mr) to join and grab the falling cash at Glen Park. Lasts %d seconds!", MONEYRUSH_DURATION);
	SendClientMessageToAll(COLOR_SERVER_INFO, string);
	#pragma unused byWho
	return 1;
}

FUNCTION Moneyrush_ForceEnd(byWho)
{
	if(!gRushActive) return 0;
	Moneyrush_DestroyPickups();
	gRushActive = false;
	gRushCooldownUntil = gettime() + MONEYRUSH_COOLDOWN;
	Moneyrush_Finish("stopped by an owner");
	#pragma unused byWho
	return 1;
}

//==============================================================================
//	Pickup grab — hooked from OnPlayerPickUpDynamicPickup (design §7)
//==============================================================================

FUNCTION Moneyrush_OnPickup(playerid, pickupid)
{
	if(!gRushActive) return 0;
	new slot = Moneyrush_SlotOf(pickupid);
	if(slot == -1) return 0;
	if(!PlayerInfo[playerid][pLoggedIn] || IsPlayerNPC(playerid)) return 0;

	// Only joined participants get paid (others see the pickups but don't earn).
	if(!PlayerInfo[playerid][pInMoneyRush])
	{
		SendClientMessage(playerid, COLOR_SERVER_INFO, "Type "COL_GREEN"/moneyrush"COL_WHITE" to join the cash rain and start earning!");
		return 1;
	}

	new cash = MONEYRUSH_CASH_MIN + random(MONEYRUSH_CASH_MAX - MONEYRUSH_CASH_MIN + 1);
	Bank_GiveCash(playerid, cash);
	if(PlayerInfo[playerid][pMoneyRushEarned] <= MAX_MONEY - cash)
		PlayerInfo[playerid][pMoneyRushEarned] += cash;

	new string[128];
	format(string, sizeof(string), ""COL_GREEN"* Cash grabbed: "COL_WHITE"+$%d"COL_GREEN"! "COL_WHITE"Rush total: "COL_GREEN"$%d"COL_WHITE".", cash, PlayerInfo[playerid][pMoneyRushEarned]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);

	// Respawn this pickup at a fresh scatter so the arena keeps raining.
	DestroyDynamicPickup(gRushPickups[slot]);
	new Float:ox = float(random(2 * floatround(MONEYRUSH_SPREAD))) - MONEYRUSH_SPREAD;
	new Float:oy = float(random(2 * floatround(MONEYRUSH_SPREAD))) - MONEYRUSH_SPREAD;
	gRushPickups[slot] = CreateDynamicPickup(MONEYRUSH_PICKUP_MODEL, 1, MONEYRUSH_X + ox, MONEYRUSH_Y + oy, MONEYRUSH_Z, 0, 0, -1, 300.0);
	return 1;
}

//==============================================================================
//	Duration tick — hooked from GameModeClock() (1s, no new timer, design §7)
//==============================================================================

FUNCTION Moneyrush_OnTick()
{
	if(!gRushActive) return 0;
	if(gettime() < gRushEndsAt) return 0;
	// Time's up — end the rush and start the cooldown.
	Moneyrush_DestroyPickups();
	gRushActive = false;
	gRushCooldownUntil = gettime() + MONEYRUSH_COOLDOWN;
	Moneyrush_Finish("time up");
	return 1;
}

//==============================================================================
//	Disconnect / cleanup hooks
//==============================================================================

FUNCTION Moneyrush_OnDisconnect(playerid)
{
	PlayerInfo[playerid][pInMoneyRush] = false;
	PlayerInfo[playerid][pMoneyRushEarned] = 0;
	return 1;
}

FUNCTION Moneyrush_Cleanup()
	return Moneyrush_DestroyPickups();

//==============================================================================
//	Command — /moneyrush (/mr) : join the active event (design §7)
//==============================================================================

CMD:moneyrush(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(!gRushActive)
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "There is no money-rush event running right now. An owner starts one with /startmoneyrush.");
	// Jailed players stay in their cell — joining would teleport them out.
	if(Jail_IsJailed(playerid) || PlayerInfo[playerid][pAdminJailed])
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot join the money rush while you are in jail.");
	if(PlayerInfo[playerid][pInMoneyRush])
	{
		new held[128];
		format(held, sizeof(held), "You are already in the rush (earned "COL_GREEN"$%d"COL_WHITE" so far). Grab the cash pickups at Glen Park!", PlayerInfo[playerid][pMoneyRushEarned]);
		return SendClientMessage(playerid, COLOR_SERVER_INFO, held);
	}

	PlayerInfo[playerid][pInMoneyRush] = true;
	PlayerInfo[playerid][pMoneyRushEarned] = 0;
	// Drop the joiner into the arena so they can start grabbing.
	SetPlayerInterior(playerid, 0);
	SetPlayerVirtualWorld(playerid, 0);
	SetPlayerPos(playerid, MONEYRUSH_X + float(random(20)) - 10.0, MONEYRUSH_Y + float(random(20)) - 10.0, MONEYRUSH_Z);
	SetCameraBehindPlayer(playerid);
	SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_GREEN"* You joined the money rush! "COL_WHITE"Grab as much cash as you can before the timer runs out. PvP is ON.");
	return 1;
}
alias:moneyrush("mr")
