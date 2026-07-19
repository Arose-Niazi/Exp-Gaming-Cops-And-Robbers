/*

	Store holdup (base) for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 4 (stage 2) - design §5.2 (store holdup base) + §4.4 + §2.3 (holdup
	= set +7 wanted) + docs/research/crazybobs-site-full.md (§Store Holdups).

	The BASE store holdup. On foot near a store / 24-7 clerk actor, /holdup sets
	the robber's wanted to +HOLDUP_WANTED (7 - Felon tier, into Warrant range),
	makes the clerk wait ~HOLDUP_CLERK_WAIT seconds, then the register pays out
	$HOLDUP_RATE_MIN..HOLDUP_RATE_MAX per second for HOLDUP_PAYOUT_SECS seconds
	(via Bank_GiveCash). Each completed holdup = +1 score. A per-player cooldown
	(HOLDUP_STORE_CD) throttles it. Blocked if cop / jailed / spawn-protected / in
	a DM zone (the shared crime gate).

	The whole sequence is driven by a per-player SetTimerEx tick (mirrors the jail
	countdown pattern) - the register pays once per second and stops if the robber
	dies, moves too far from the store, or disconnects.

	>>> M5 EXTENSION POINT - the FULL robbery system + crowbar bonus lands in M5.
	    §5.7: a crowbar reduces the clerk wait by a couple of seconds AND raises
	    the per-second payout. Other robbery types (banks, houses, ATMs), the
	    Version-23 alarm delay, and cop mid-holdup interrupt/arrest also land in
	    M5. This module is deliberately minimal; grep "TODO(M5)" for the hooks. <<<

	Credits:
		crazybobs-site-full.md §Store Holdups for the mechanic grounding.

	Public API:
		Holdup_OnPlayerDeath(playerid)      : abort a running holdup on death
		Holdup_OnDisconnect(playerid)       : kill the holdup timer on disconnect
		Holdup_Tick(playerid)               : per-player sequence tick (public timer)

	Commands:
		/holdup (/hup /storerob /robstore)  (civ, on-foot) rob a nearby store clerk

	Functions Being Called:
		misc:    IsCop, IsPlayerInRangeOfPoint (native)
		wanted:  GivePlayerWanted, IsPlayerSpawnProtected
		jail:    Jail_IsJailed
		bank:    Bank_GiveCash
		actors:  ActorInfo[] (store-clerk proximity)
*/

//==============================================================================
//	Store-clerk actor gate (design §5.2 - eligible store types)
//==============================================================================

// Actor types that count as a robbable store clerk (ActorTypeName index order in
// server/actors.inc): 24-7, Sex Shop, Bar, Pizza, Burger, Chicken, Ammunation,
// RPC Bar, Hippie. (Bank/Casino/PD/etc. are NOT store holdups - those are M5.)
static bool:Holdup_IsStoreActorType(type)
{
	switch(type)
	{
		case 4, 5, 6, 7, 8, 9, 14, 17, 23: return true;	// 24-7/Sex/Bar/Pizza/Burger/Chicken/RPC/Ammu/Hippie
	}
	return false;
}

// Find the nearest robbable store-clerk actor to the player (same interior/VW,
// within HOLDUP_ACTOR_RANGE). Returns the ActorInfo slot index, or -1 if none.
// NOTE (M5): no longer static - the M5 robbery module (/shoplift, robbery.inc)
// reuses this store-clerk proximity gate.
Holdup_NearestStore(playerid)
{
	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	new best = -1, Float:bestDist = HOLDUP_ACTOR_RANGE + 1.0;
	for(new i = 0; i < MAX_ACTORS_USED; i++)
	{
		if(ActorInfo[i][ActorAID] == INVALID_ACTOR_ID) continue;
		if(!Holdup_IsStoreActorType(ActorInfo[i][ActorType])) continue;
		if(GetPlayerInterior(playerid) != ActorInfo[i][ActorInt]) continue;
		if(GetPlayerVirtualWorld(playerid) != ActorInfo[i][ActorVW]) continue;
		new Float:d = GetPlayerDistanceFromPoint(playerid, ActorInfo[i][ActorSpawnX], ActorInfo[i][ActorSpawnY], ActorInfo[i][ActorSpawnZ]);
		if(d <= HOLDUP_ACTOR_RANGE && d < bestDist) { bestDist = d; best = i; }
	}
	return best;
}

//==============================================================================
//	Sequence control
//==============================================================================

// End a running holdup (release the state + timer). announce=1 prints the take.
static Holdup_End(playerid, bool:announce)
{
	if(PlayerInfo[playerid][pHoldupTimer] != -1)
	{
		KillTimer(PlayerInfo[playerid][pHoldupTimer]);
		PlayerInfo[playerid][pHoldupTimer] = -1;
	}
	if(!PlayerInfo[playerid][pHoldingUp]) return 0;
	PlayerInfo[playerid][pHoldingUp] = false;

	if(announce && IsPlayerConnected(playerid))
	{
		if(PlayerInfo[playerid][pHoldupTake] > 0)
		{
			PlayerInfo[playerid][pScore] += 1;	// each completed robbery = +1 score (§5)
			// Stage A (FEATURE-EXPANSION-PLAN §1.5): a holdup that actually paid out
			// counts as a success - climb the grind counter + persist. Rob_RecordSuccess
			// lives in robbery.inc (included after this module) - reach it via CallLocalFunction.
			CallLocalFunction("Rob_RecordSuccess", "d", playerid);
			new string[144];
			format(string, sizeof(string), ""COL_GREEN"* Holdup complete: "COL_WHITE"you cleared "COL_GREEN"$%d"COL_WHITE" from the register. +1 score. Get out before the cops arrive!", PlayerInfo[playerid][pHoldupTake]);
			SendClientMessage(playerid, COLOR_SERVER_INFO, string);
		}
		else SendClientMessage(playerid, COLOR_ERROR, "The holdup was interrupted - you got nothing.");
	}
	PlayerInfo[playerid][pHoldupStage] = 0;
	PlayerInfo[playerid][pHoldupTake] = 0;
	return 1;
}

// Per-player sequence tick (1s). Stage 0 = clerk wait counting up to
// HOLDUP_CLERK_WAIT; then stages 1..HOLDUP_PAYOUT_SECS pay the register.
FUNCTION Holdup_Tick(playerid)
{
	if(!PlayerInfo[playerid][pHoldingUp]) return 0;
	if(!IsPlayerConnected(playerid)) { Holdup_End(playerid, false); return 0; }

	// Abort if the robber left the store (moved out of range of the locked spot)
	// or got into a vehicle (must stay on foot at the counter).
	if(IsPlayerInAnyVehicle(playerid) ||
		!IsPlayerInRangeOfPoint(playerid, HOLDUP_ACTOR_RANGE + 3.0, PlayerInfo[playerid][pHoldupX], PlayerInfo[playerid][pHoldupY], PlayerInfo[playerid][pHoldupZ]))
	{
		SendClientMessage(playerid, COLOR_ERROR, "You left the store - the holdup is off.");
		Holdup_End(playerid, true);
		return 0;
	}

	PlayerInfo[playerid][pHoldupStage]++;

	// M5 (§5.7) - a crowbar shortens the clerk wait by a couple of seconds AND
	// raises the per-second register take. Holdup_CrowbarEquipped lives in
	// robbery.inc (which owns the crowbar item); it is included after this module
	// but the FUNCTION public is resolvable via CallLocalFunction at call time.
	new bool:crowbar = (CallLocalFunction("Holdup_CrowbarEquipped", "d", playerid) != 0);
	new clerkWait = HOLDUP_CLERK_WAIT;
	if(crowbar) clerkWait -= CROWBAR_WAIT_REDUCTION;
	if(clerkWait < 3) clerkWait = 3;

	// Clerk still opening the register (stage <= wait seconds).
	if(PlayerInfo[playerid][pHoldupStage] < clerkWait)
	{
		new left = clerkWait - PlayerInfo[playerid][pHoldupStage];
		new gt[48];
		format(gt, sizeof(gt), "~r~HOLDUP~n~~w~clerk opening register... %d", left);
		GameTextForPlayer(playerid, gt, 1200, 5);
		return 1;
	}

	// Register open - pay a random per-second amount for HOLDUP_PAYOUT_SECS. A
	// crowbar raises the rate band (§5.7 higher register take).
	new payingSecond = PlayerInfo[playerid][pHoldupStage] - clerkWait + 1;	// 1..PAYOUT_SECS
	new rateMin = crowbar ? CROWBAR_RATE_MIN : HOLDUP_RATE_MIN;
	new rateMax = crowbar ? CROWBAR_RATE_MAX : HOLDUP_RATE_MAX;
	new take = rateMin + random(rateMax - rateMin + 1);
	Bank_GiveCash(playerid, take);
	if(PlayerInfo[playerid][pHoldupTake] <= MAX_MONEY - take) PlayerInfo[playerid][pHoldupTake] += take;

	new msg[128];
	format(msg, sizeof(msg), ""COL_GREEN"* Register: "COL_WHITE"+$%d"COL_GREEN" (total $%d)."COL_WHITE, take, PlayerInfo[playerid][pHoldupTake]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);

	if(payingSecond >= HOLDUP_PAYOUT_SECS)
	{
		Holdup_End(playerid, true);
		return 1;
	}
	return 1;
}

//==============================================================================
//	Death / disconnect hooks
//==============================================================================

// Dying mid-holdup aborts it (the robber keeps whatever the register already
// paid - it's cash-in-hand, at risk to the medical fee / robbery like any cash).
FUNCTION Holdup_OnPlayerDeath(playerid)
{
	if(PlayerInfo[playerid][pHoldingUp]) Holdup_End(playerid, false);
	return 1;
}

FUNCTION Holdup_OnDisconnect(playerid)
{
	if(PlayerInfo[playerid][pHoldupTimer] != -1)
	{
		KillTimer(PlayerInfo[playerid][pHoldupTimer]);
		PlayerInfo[playerid][pHoldupTimer] = -1;
	}
	PlayerInfo[playerid][pHoldingUp] = false;
	return 1;
}

// M5 (§5.2) - a cop mid-holdup interrupt ends the holdup with nothing (the
// robbery.inc /stoprobbery command calls this for a nearby robber). Also called
// if the robber is shot while holding up (Holdup_OnPlayerDeath already aborts on
// death). The robber keeps whatever the register already paid - it's cash at risk.
FUNCTION Holdup_CopInterrupt(playerid)
{
	if(!PlayerInfo[playerid][pHoldingUp]) return 0;
	SendClientMessage(playerid, COLOR_ERROR, ""COL_RED"* The police interrupted your holdup!"COL_WHITE);
	Holdup_End(playerid, false);
	return 1;
}

//==============================================================================
//	Command - /holdup (/hup /storerob /robstore) (design §5.2)
//==============================================================================

CMD:holdup(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");

	// Shared crime gate: cops can't rob, and jailed/spawn-protected/DM-zone block.
	if(IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Law enforcement cannot hold up stores.");
	if(Jail_IsJailed(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot do that while you are in jail.");
	if(IsPlayerSpawnProtected(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot commit crimes while spawn-protected. Wait a moment.");
	if(PlayerInfo[playerid][pInDMZone]) return SendClientMessage(playerid, COLOR_ERROR, "You cannot commit crimes inside a DM zone.");
	if(IsPlayerInAnyVehicle(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You must be on foot at the counter to hold up a store.");
	if(PlayerInfo[playerid][pHoldingUp]) return SendClientMessage(playerid, COLOR_ERROR, "You are already holding up this store.");

	if(gettime() < PlayerInfo[playerid][pHoldupCooldown])
	{
		new err[112];
		format(err, sizeof(err), "You cannot hold up another store yet - wait %d seconds.", PlayerInfo[playerid][pHoldupCooldown] - gettime());
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}

	// Must be looking at / next to a robbable store clerk (24-7, bar, ammu, ...).
	new store = Holdup_NearestStore(playerid);
	if(store == -1)
		return SendClientMessage(playerid, COLOR_ERROR, "You must be standing at a store clerk (24-7, bar, ammunation, food shop, sex shop) to hold it up.");

	// Stage A (FEATURE-EXPANSION-PLAN §1.3): an entry success roll the base holdup
	// previously lacked - base 60 -> cap 92 on the grind curve. On a FAIL the clerk
	// refuses / trips a silent alarm: the holdup aborts before the clerk-wait, no
	// register payout, a +2 "Attempted holdup" wanted, and only a shorter
	// HOLDUP_FAIL_CD retry cooldown (a grind-tax, not the full HOLDUP_STORE_CD lockout).
	if(random(100) >= Rob_SuccessChance(60, 92, PlayerInfo[playerid][pSuccessfulRobberies]))
	{
		PlayerInfo[playerid][pHoldupCooldown] = gettime() + HOLDUP_FAIL_CD;
		GivePlayerWanted(playerid, 2, "Attempted holdup");
		PlayerInfo[playerid][pFailedRobberies]++;
		Jail_SavePlayer(playerid);
		SendClientMessage(playerid, COLOR_ERROR, ""COL_RED"* Holdup failed! "COL_WHITE"The clerk refused and hit the silent alarm - you got nothing. Try again shortly.");
		return 1;
	}

	// Start the sequence. Lock the store position for the leave-the-zone check.
	GetPlayerPos(playerid, PlayerInfo[playerid][pHoldupX], PlayerInfo[playerid][pHoldupY], PlayerInfo[playerid][pHoldupZ]);
	PlayerInfo[playerid][pHoldingUp] = true;
	PlayerInfo[playerid][pHoldupStage] = 0;
	PlayerInfo[playerid][pHoldupTake] = 0;
	PlayerInfo[playerid][pHoldupCooldown] = gettime() + HOLDUP_STORE_CD;

	// Instantly +7 wanted (Felon - into Warrant range). §5.2 VERIFIED.
	GivePlayerWanted(playerid, HOLDUP_WANTED, "Store holdup");
	// M5 (§5.7) - record a holdup on the robbery history so the crowbar is
	// confiscated on a later arrest (robbery.inc owns the bitmask + persistence).
	CallLocalFunction("Robbery_MarkHoldupHistory", "d", playerid);

	if(PlayerInfo[playerid][pHoldupTimer] != -1) KillTimer(PlayerInfo[playerid][pHoldupTimer]);
	PlayerInfo[playerid][pHoldupTimer] = SetTimerEx("Holdup_Tick", 1000, true, "d", playerid);

	new string[160];
	format(string, sizeof(string), ""COL_RED"* HOLD-UP! "COL_WHITE"You pull a gun on the clerk - wait "COL_YELLOW"~%d seconds"COL_WHITE" for the register to open. Stay at the counter!", HOLDUP_CLERK_WAIT);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);

	// Nearby cops get a heads-up (the crime announce in GivePlayerWanted already
	// notified radar-range cops; this is the store-specific line).
	new zoneName[MAX_ZONE_NAME];
	if(isNull(PlayerInfo[playerid][pZoneName])) format(zoneName, sizeof(zoneName), "the city");
	else format(zoneName, sizeof(zoneName), "%s", PlayerInfo[playerid][pZoneName]);
	format(string, sizeof(string), ""COL_ORANGE"[HOLDUP] "COL_WHITE"%s (%d) is robbing a store near %s! Respond!", PlayerInfo[playerid][pUserName], playerid, zoneName);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || i == playerid) continue;
		if(IsCop(i)) SendClientMessage(i, COLOR_WANTED_ORANGE, string);
	}

	// Stage B (§3.2) - a store holdup fires a 10-130 (Robbery/Hold Up) dispatch,
	// server-wide. dispatch.inc is included after this module, so reach it via
	// CallLocalFunction. The [DISPATCH] line is the high-signal event feed layered
	// on top of the [HOLDUP]/[CRIME] proximity announces above.
	new dtext[128];
	format(dtext, sizeof(dtext), "in progress - %s (%d)", PlayerInfo[playerid][pUserName], playerid);
	CallLocalFunction("Dispatch_Alert", "ddssf", playerid, DISPATCH_NORMAL, "10-130", dtext, DISPATCH_ALL);
	return 1;
}
alias:holdup("hup", "storerob", "robstore")
