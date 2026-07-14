/*

	Jail system for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 3 (stage 2) — design §3.2 (jail: sentence, bail, bribe, appeal,
	escape, breakout) + §2.4 (arrest clears wanted).

	This owns the cop-jail state (distinct from the M2 admin-jail; admin-jail
	always takes precedence — see Jail_IsBusy / the spawn re-jail order in
	CnR.pwn). Sends an arrested/re-jailed suspect to the real LSPD (Pershing
	Square) cell interior in an isolated virtual world, disarms them, arms a
	release timer with a visible countdown, and spawns them back outside on
	release. Persistence mirrors the AdminJailUntil pattern: JailUntil is an
	absolute unix time in LSplayers, re-applied on spawn (loading_data.inc loads
	it, OnPlayerSpawn re-jails), saved on disconnect.

	Credits:
		crime-cop-loop.md §5 for the jail mechanic grounding.

	Public API (called by cmds/cop.inc + CnR.pwn):
		Jail_SendToJail(suspectid, wantedAtArrest, const reason[], bool:serious)
		Jail_IsJailed(playerid)
		Jail_Release(playerid, const reason[], bool:spawnOutside)
		Jail_ComputeSentence(wantedAtArrest, serious)   : sentence in seconds
		Jail_OnPlayerSpawn(playerid)                    : re-jail check (spawn)
		Jail_SavePlayer(playerid)                       : persist (disconnect)
		Jail_OnDisconnect(playerid)                     : kill timer + persist

	Functions Being Called:
		misc:     IsCop
		wanted:   ClearPlayerWanted, SetPlayerWanted, GetPlayerWanted
		messages: SendMessageToModerators
*/

//==============================================================================
//	Internal helpers
//==============================================================================

// Admin-jail always wins over cop-jail; a busy player can't be cop-jailed and
// cop-jail commands short-circuit if the player is admin-jailed.
static bool:Jail_AdminBusy(playerid)
	return PlayerInfo[playerid][pAdminJailed];

// Seconds remaining on the current sentence (clamped >= 0).
static Jail_SecsLeft(playerid)
{
	new left = PlayerInfo[playerid][pJailUntil] - gettime();
	return (left < 0) ? 0 : left;
}

// Clear any pending appeal on the appellant + release every seated juror.
static Jail_ClearAppeal(playerid)
{
	if(PlayerInfo[playerid][pAppealJurors] <= 0 && !PlayerInfo[playerid][pAppealYes] && !PlayerInfo[playerid][pAppealNo]) return 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || !PlayerInfo[i][pOnJury]) continue;
		if(PlayerInfo[i][pJuryFor] != playerid) continue;
		PlayerInfo[i][pOnJury] = false;
		PlayerInfo[i][pJuryFor] = -1;
		SendClientMessage(i, COLOR_SERVER_INFO, "The appeal you were on a jury for has ended.");
	}
	PlayerInfo[playerid][pAppealYes] = PlayerInfo[playerid][pAppealNo] = PlayerInfo[playerid][pAppealJurors] = 0;
	return 1;
}

//==============================================================================
//	Sentence formula (design §3.2)
//==============================================================================

// pJailTime = JAIL_BASE + wantedAtArrest*JAIL_PER_WL + seriousCrimes*JAIL_PER_SERIOUS.
FUNCTION Jail_ComputeSentence(wantedAtArrest, serious)
{
	new secs = JAIL_BASE_SECS + (wantedAtArrest * JAIL_PER_WL_SECS) + (serious * JAIL_PER_SERIOUS_SECS);
	if(secs < JAIL_BASE_SECS) secs = JAIL_BASE_SECS;
	return secs;
}

//==============================================================================
//	Placement / release
//==============================================================================

// Teleport + freeze a suspect into the LSPD cell and disarm them. mins-style
// re-arm is folded in via pJailUntil (already an absolute unix time).
static Jail_Place(playerid)
{
	SetPlayerInterior(playerid, JAIL_INT);
	SetPlayerVirtualWorld(playerid, JAIL_VW);
	SetPlayerPos(playerid, JAIL_X, JAIL_Y, JAIL_Z);
	SetPlayerFacingAngle(playerid, JAIL_A);
	SetCameraBehindPlayer(playerid);
	ResetPlayerWeapons(playerid);
	SetPlayerHealth(playerid, 100.0);
	SetPlayerArmour(playerid, 0.0);
	return 1;
}

// Persist the current jail row (absolute JailUntil + bail/reason/chained).
// This is the single per-player LSplayers disconnect/state UPDATE — M4 (stage 1)
// folded the bank/tax/insurance columns in here (BankMoney/TaxOwed/LifeInsurance/
// HealthInsExpiry) rather than adding a second competing UPDATE. Any subsystem
// that mutates those fields (bank.inc) just writes PlayerInfo and this save picks
// them up on disconnect/release; bank.inc also calls this directly after a
// deposit/withdraw so the money core is durable without a separate query.
FUNCTION Jail_SavePlayer(playerid)
{
	if(!PlayerInfo[playerid][pLoggedIn]) return 0;
	new string[832];	// headroom for Money/Bank/Tax + RefillPoints + LottoNumber + Crowbar/RobHistory + HouseSpawnID + FightStyle + fishing/farming cols + a 64-char (escaped) JailReason
	mysql_format(g_SQL, string, sizeof(string),
		"UPDATE "TABLE_PLAYERS_C" SET Money=%d, BankMoney=%d, TaxOwed=%d, LifeInsurance=%d, HealthInsExpiry=%d, Score=%d, CopRank=%d, RefillPoints=%d, SeriousCrimes=%d, Jailed=%d, JailUntil=%d, Bail=%d, JailReason='%e', EscapeChained=%d, LottoNumber=%d, CrowbarEquipped=%d, RobberyHistory=%d, HouseSpawnID=%d, FightStyle=%d, FishPermits=%d, HuntPermits=%d, DrugSeeds=%d, HasFishingRod=%d, HasFishCooler=%d, FishSalesPermit=%d, CoolerCount=%d, CoolerWeight=%d, CoolerValue=%d WHERE aID=%d LIMIT 1",
		PlayerInfo[playerid][pMoney],
		PlayerInfo[playerid][pBankMoney], PlayerInfo[playerid][pTaxOwed], PlayerInfo[playerid][pLifeInsurance], PlayerInfo[playerid][pHealthInsExpiry],
		PlayerInfo[playerid][pScore], PlayerInfo[playerid][pCopRank], PlayerInfo[playerid][pRefillPoints], PlayerInfo[playerid][pSeriousCrimes],
		PlayerInfo[playerid][pJailed] ? 1 : 0, PlayerInfo[playerid][pJailUntil], PlayerInfo[playerid][pJailBail],
		PlayerInfo[playerid][pJailReason], PlayerInfo[playerid][pEscapeChained] ? 1 : 0, PlayerInfo[playerid][pLottoNumber],
		PlayerInfo[playerid][pCrowbarEquipped] ? 1 : 0, PlayerInfo[playerid][pRobberyHistory], PlayerInfo[playerid][pHouseSpawn], PlayerInfo[playerid][pFightStyle],
		// M6 (stage 2) — fishing & farming persistence (design §9.2/§9.3/§11.2).
		PlayerInfo[playerid][pFishPermits], PlayerInfo[playerid][pHuntPermits], PlayerInfo[playerid][pDrugSeeds],
		PlayerInfo[playerid][pHasFishingRod] ? 1 : 0, PlayerInfo[playerid][pHasFishCooler] ? 1 : 0, PlayerInfo[playerid][pHasFishSalesPermit] ? 1 : 0,
		PlayerInfo[playerid][pCoolerCount], PlayerInfo[playerid][pCoolerWeight], PlayerInfo[playerid][pCoolerValue],
		PlayerInfo[playerid][pID]);
	mysql_pquery(g_SQL, string);
	return 1;
}

// Per-second countdown tick (visible timer) — fires the release when it lapses.
FUNCTION Jail_CountdownTick(playerid)
{
	if(!IsPlayerConnected(playerid) || !PlayerInfo[playerid][pJailed]) return 0;

	// Admin-jail takes over: suspend the cop-jail countdown/HUD silently.
	if(Jail_AdminBusy(playerid)) return 0;

	new left = Jail_SecsLeft(playerid);
	if(left <= 0)
	{
		Jail_Release(playerid, "Sentence served", true);
		return 1;
	}
	new gt[24];
	format(gt, sizeof(gt), "~r~JAIL~n~~w~%d:%02d", left / 60, left % 60);
	GameTextForPlayer(playerid, gt, 1200, 5);
	return 1;
}

// Arm (or re-arm) the countdown timer for the remaining sentence.
static Jail_ArmTimer(playerid)
{
	if(PlayerInfo[playerid][pJailTimer] != -1)
	{
		KillTimer(PlayerInfo[playerid][pJailTimer]);
		PlayerInfo[playerid][pJailTimer] = -1;
	}
	PlayerInfo[playerid][pJailTimer] = SetTimerEx("Jail_CountdownTick", 1000, true, "d", playerid);
	return 1;
}

// Core entry: jail a suspect for a computed sentence. Clears their wanted level
// (arrest/takedown/escape recapture all pass through here) and pauses nothing —
// they are out of the pursuit loop. `serious` bumps the seriousCrime count.
FUNCTION Jail_SendToJail(suspectid, wantedAtArrest, const reason[], serious)
{
	if(!IsPlayerConnected(suspectid) || !PlayerInfo[suspectid][pLoggedIn]) return 0;
	if(Jail_AdminBusy(suspectid)) return 0;	// admin-jail precedence

	if(serious) PlayerInfo[suspectid][pSeriousCrimes]++;

	new secs = Jail_ComputeSentence(wantedAtArrest, PlayerInfo[suspectid][pSeriousCrimes]);
	PlayerInfo[suspectid][pJailed] = true;
	PlayerInfo[suspectid][pJailStart] = gettime();
	PlayerInfo[suspectid][pJailUntil] = gettime() + secs;
	PlayerInfo[suspectid][pJailBail] = secs * BAIL_PER_SEC;
	format(PlayerInfo[suspectid][pJailReason], 64, "%s", isNull(reason) ? ("Arrested") : (reason));

	// Reset any stale bribe/appeal state from a previous stint.
	PlayerInfo[suspectid][pBribeTo] = -1;
	PlayerInfo[suspectid][pBribeAmount] = 0;
	Jail_ClearAppeal(suspectid);

	// Arrest clears wanted (§2.4).
	ClearPlayerWanted(suspectid, "");

	Jail_Place(suspectid);
	Jail_ArmTimer(suspectid);
	// M5 (stage 2) — arrest confiscates the crowbar if the suspect's RobberyHistory
	// carries a robbery flag (§5.7). This is the ARREST path — it always confiscates
	// (the "killed by a cop keeps the crowbar" exception lives in the death path).
	// robbery.inc is included after jail.inc, so reach it via CallLocalFunction; it
	// aborts any running safe-crack, clears the flag, and calls Jail_SavePlayer.
	CallLocalFunction("Robbery_OnArrest", "d", suspectid);
	// M5 (stage 4) — a jailed kidnapper releases any victim they were holding (§4.4).
	// kidnap.inc is included after jail.inc, so reach it via CallLocalFunction.
	CallLocalFunction("Kidnap_OnArrest", "d", suspectid);

	new string[144];
	format(string, sizeof(string), ""COL_RED"* You have been jailed: %s. "COL_WHITE"Sentence "COL_YELLOW"%d:%02d"COL_WHITE", bail "COL_GREEN"$%d"COL_WHITE".", PlayerInfo[suspectid][pJailReason], secs / 60, secs % 60, PlayerInfo[suspectid][pJailBail]);
	SendClientMessage(suspectid, COLOR_ERROR, string);
	SendClientMessage(suspectid, COLOR_SERVER_INFO, "Ways out: "COL_WHITE"/bail (/pb)  /appeal  /bribe  /escape (/esc)"COL_SERVER_INFO".");
	return 1;
}

FUNCTION Jail_IsJailed(playerid)
	return (PlayerInfo[playerid][pJailed] && !Jail_AdminBusy(playerid));

// Free a player from jail: clear state, stop the timer, persist, and (if asked)
// spawn them back outside the PD.
FUNCTION Jail_Release(playerid, const reason[], spawnOutside)
{
	if(PlayerInfo[playerid][pJailTimer] != -1)
	{
		KillTimer(PlayerInfo[playerid][pJailTimer]);
		PlayerInfo[playerid][pJailTimer] = -1;
	}
	PlayerInfo[playerid][pJailed] = false;
	PlayerInfo[playerid][pJailUntil] = 0;
	PlayerInfo[playerid][pJailStart] = 0;
	PlayerInfo[playerid][pJailBail] = 0;
	// Reset the serious-crime count on sentence completion so it cannot grow
	// unbounded across stints or permanently block /appeal.
	PlayerInfo[playerid][pSeriousCrimes] = 0;
	strdel(PlayerInfo[playerid][pJailReason], 0, 64);
	PlayerInfo[playerid][pBribeTo] = -1;
	PlayerInfo[playerid][pBribeAmount] = 0;
	Jail_ClearAppeal(playerid);

	if(IsPlayerConnected(playerid))
	{
		if(spawnOutside)
		{
			SetPlayerInterior(playerid, 0);
			SetPlayerVirtualWorld(playerid, 0);
			SetPlayerPos(playerid, JAIL_RELEASE_X, JAIL_RELEASE_Y, JAIL_RELEASE_Z);
			SetPlayerFacingAngle(playerid, JAIL_RELEASE_A);
			SetCameraBehindPlayer(playerid);
			SetPlayerHealth(playerid, 100.0);
		}
		if(!isNull(reason))
		{
			new string[96];
			format(string, sizeof(string), "You are free to go (%s).", reason);
			SendClientMessage(playerid, COLOR_SERVER_INFO, string);
		}
	}
	Jail_SavePlayer(playerid);
	return 1;
}

//==============================================================================
//	Persistence hooks (spawn re-jail / disconnect save)
//==============================================================================

// Re-apply a still-running sentence on (re)spawn (mirrors the admin-jail path in
// OnPlayerSpawn). Admin-jail precedence: CnR.pwn checks admin-jail first, so we
// only re-place when the player is NOT admin-jailed.
FUNCTION Jail_OnPlayerSpawn(playerid)
{
	if(Jail_AdminBusy(playerid)) return 0;
	if(PlayerInfo[playerid][pJailUntil] > gettime())
	{
		PlayerInfo[playerid][pJailed] = true;
		if(PlayerInfo[playerid][pJailStart] == 0) PlayerInfo[playerid][pJailStart] = gettime();
		Jail_Place(playerid);
		Jail_ArmTimer(playerid);
		new string[128], left = Jail_SecsLeft(playerid), rsn[64];
		if(isNull(PlayerInfo[playerid][pJailReason])) format(rsn, sizeof(rsn), "Arrested");
		else format(rsn, sizeof(rsn), "%s", PlayerInfo[playerid][pJailReason]);
		format(string, sizeof(string), ""COL_RED"* Jail sentence resumed: %s "COL_WHITE"(%d:%02d left).", rsn, left / 60, left % 60);
		SendClientMessage(playerid, COLOR_ERROR, string);
		return 1;
	}
	// Sentence lapsed while offline — clear the stale row.
	if(PlayerInfo[playerid][pJailUntil] != 0 || PlayerInfo[playerid][pJailed])
	{
		PlayerInfo[playerid][pJailed] = false;
		PlayerInfo[playerid][pJailUntil] = 0;
		Jail_SavePlayer(playerid);
	}
	return 0;
}

// Disconnect: kill the timer and persist the remaining absolute JailUntil.
FUNCTION Jail_OnDisconnect(playerid)
{
	if(PlayerInfo[playerid][pJailTimer] != -1)
	{
		KillTimer(PlayerInfo[playerid][pJailTimer]);
		PlayerInfo[playerid][pJailTimer] = -1;
	}
	// A departing appellant/juror shouldn't strand the other side.
	Jail_ClearAppeal(playerid);
	if(PlayerInfo[playerid][pOnJury])
	{
		PlayerInfo[playerid][pOnJury] = false;
		PlayerInfo[playerid][pJuryFor] = -1;
	}
	if(PlayerInfo[playerid][pLoggedIn]) Jail_SavePlayer(playerid);
	return 1;
}

//==============================================================================
//	Ways out — jailed-player commands (design §3.2)
//==============================================================================

// 1) /bail (/pb /paybail) — pay bail to leave early.
CMD:bail(playerid, params[])
{
	#pragma unused params
	if(!Jail_IsJailed(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You are not in jail.");
	if((gettime() - PlayerInfo[playerid][pJailStart]) < JAIL_MIN_SERVE_SECS)
	{
		new e[112];
		format(e, sizeof(e), "You must serve at least %d seconds before you can post bail.", JAIL_MIN_SERVE_SECS);
		return SendClientMessage(playerid, COLOR_ERROR, e);
	}
	if(PlayerInfo[playerid][pMoney] < PlayerInfo[playerid][pJailBail])
	{
		new string[96];
		format(string, sizeof(string), "You cannot afford bail ($%d). Wait to be released or /parole'd.", PlayerInfo[playerid][pJailBail]);
		return SendClientMessage(playerid, COLOR_ERROR, string);
	}
	PlayerInfo[playerid][pMoney] -= PlayerInfo[playerid][pJailBail];
	UpdateMoney(playerid);
	Jail_Release(playerid, "bail paid", true);
	return 1;
}
alias:bail("pb", "paybail")

// 2) /appeal — jury of online players votes /innocent (release) or /guilty.
CMD:appeal(playerid, params[])
{
	#pragma unused params
	if(!Jail_IsJailed(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You are not in jail.");
	if(PlayerInfo[playerid][pAppealJurors] > 0) return SendClientMessage(playerid, COLOR_ERROR, "You already have an appeal in progress.");
	if(PlayerInfo[playerid][pSeriousCrimes] > 0)
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot appeal a serious (violent) charge.");
	if(Jail_SecsLeft(playerid) < APPEAL_MIN_LEFT_SECS)
	{
		new string[96];
		format(string, sizeof(string), "You cannot appeal with under %d seconds left — just serve it.", APPEAL_MIN_LEFT_SECS);
		return SendClientMessage(playerid, COLOR_ERROR, string);
	}

	// Empanel up to JURY_SIZE free online players (not the appellant, not jailed).
	new jurors = 0, string[144];
	for(new i = 0; i < MAX_PLAYERS && jurors < JURY_SIZE; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || i == playerid) continue;
		if(!PlayerInfo[i][pLoggedIn] || PlayerInfo[i][pOnJury]) continue;
		if(Jail_IsJailed(i) || Jail_AdminBusy(i)) continue;
		PlayerInfo[i][pOnJury] = true;
		PlayerInfo[i][pJuryFor] = playerid;
		jurors++;
		format(string, sizeof(string), ""COL_YELLOW"[JURY] "COL_WHITE"%s (%d) appeals their jail sentence. Vote "COL_GREEN"/innocent"COL_WHITE" or "COL_RED"/guilty"COL_WHITE".", PlayerInfo[playerid][pUserName], playerid);
		SendClientMessage(i, COLOR_SERVER_INFO, string);
	}
	// Record the tally counters BEFORE the min-jury check so a rollback's
	// Jail_ClearAppeal guard (pAppealJurors > 0) actually releases seated jurors.
	PlayerInfo[playerid][pAppealYes] = PlayerInfo[playerid][pAppealNo] = 0;
	PlayerInfo[playerid][pAppealJurors] = jurors;
	if(jurors < APPEAL_MIN_JURY)
	{
		// Roll back the partial empanelment.
		Jail_ClearAppeal(playerid);
		return SendClientMessage(playerid, COLOR_ERROR, "Not enough free players online to form a jury. Try again later.");
	}
	format(string, sizeof(string), "Your appeal is before a jury of %d. Awaiting their votes...", jurors);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	return 1;
}
alias:appeal("jailappeal")

// Tally one juror's vote; resolve the appeal once a majority is reached or all
// summoned jurors have voted.
static Appeal_Vote(jurorid, bool:innocent)
{
	if(!PlayerInfo[jurorid][pOnJury]) return SendClientMessage(jurorid, COLOR_ERROR, "You are not on a jury right now.");
	new appellant = PlayerInfo[jurorid][pJuryFor];
	if(appellant == -1 || !IsPlayerConnected(appellant) || !Jail_IsJailed(appellant))
	{
		PlayerInfo[jurorid][pOnJury] = false;
		PlayerInfo[jurorid][pJuryFor] = -1;
		return SendClientMessage(jurorid, COLOR_ERROR, "That appeal is no longer active.");
	}

	// Consume this juror's seat.
	PlayerInfo[jurorid][pOnJury] = false;
	PlayerInfo[jurorid][pJuryFor] = -1;
	if(innocent) PlayerInfo[appellant][pAppealYes]++;
	else PlayerInfo[appellant][pAppealNo]++;

	new string[128];
	format(string, sizeof(string), "Vote recorded (%s). Tally: "COL_GREEN"%d innocent"COL_WHITE" / "COL_RED"%d guilty"COL_WHITE".", innocent ? ("innocent") : ("guilty"), PlayerInfo[appellant][pAppealYes], PlayerInfo[appellant][pAppealNo]);
	SendClientMessage(jurorid, COLOR_SERVER_INFO, string);

	new total = PlayerInfo[appellant][pAppealJurors];
	new need = (total / 2) + 1;	// simple majority
	new yes = PlayerInfo[appellant][pAppealYes], no = PlayerInfo[appellant][pAppealNo];

	if(yes >= need)
	{
		SendClientMessage(appellant, COLOR_SERVER_INFO, ""COL_GREEN"The jury found you INNOCENT."COL_WHITE" You are released.");
		Jail_Release(appellant, "appeal upheld", true);
		return 1;
	}
	if(no >= need || (yes + no) >= total)
	{
		SendClientMessage(appellant, COLOR_ERROR, ""COL_RED"The jury found you GUILTY."COL_WHITE" You will serve your sentence.");
		Jail_ClearAppeal(appellant);
		return 1;
	}
	return 1;
}

CMD:innocent(playerid)
	return Appeal_Vote(playerid, true);
CMD:guilty(playerid)
	return Appeal_Vote(playerid, false);

// 3) /bribe [amount] — offer cash to a nearby cop who /accept or /refuse (cop
//    side lives in cmds/cop.inc). Most Wanted cannot bribe (they arrive here
//    with wanted already cleared, but the block is on the escape-chained/serious
//    path — spec: WL10 can't bribe → we block escape-chained, matching the
//    "already escaped/most-wanted" intent, plus a live cop must be near).
CMD:bribe(playerid, params[])
{
	if(!Jail_IsJailed(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You are not in jail.");
	if(PlayerInfo[playerid][pEscapeChained])
		return SendClientMessage(playerid, COLOR_ERROR, "You escaped once — you are chained and cannot bribe.");
	new amount;
	if(sscanf(params, "d", amount))
	{
		new usage[96];
		format(usage, sizeof(usage), "USAGE: /bribe [amount $%d-$%d]", BRIBE_MIN, BRIBE_MAX);
		return SendClientMessage(playerid, COLOR_SERVER_INFO, usage);
	}
	if(amount < BRIBE_MIN || amount > BRIBE_MAX)
	{
		new err[96];
		format(err, sizeof(err), "Bribe must be between $%d and $%d.", BRIBE_MIN, BRIBE_MAX);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	if(PlayerInfo[playerid][pMoney] < amount) return SendClientMessage(playerid, COLOR_ERROR, "You cannot afford that bribe.");
	if(PlayerInfo[playerid][pBribeTo] != -1) return SendClientMessage(playerid, COLOR_ERROR, "You already have a bribe pending — wait for the cop to respond.");

	// Find the nearest online cop (jail has cops walking the block; range is
	// generous since the jailed player is teleported into the isolated cell VW).
	new copid = -1;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || !PlayerInfo[i][pLoggedIn]) continue;
		if(!IsCop(i)) continue;
		copid = i;
		break;
	}
	if(copid == -1) return SendClientMessage(playerid, COLOR_ERROR, "No cop is online to accept a bribe right now.");

	PlayerInfo[playerid][pBribeTo] = copid;
	PlayerInfo[playerid][pBribeAmount] = amount;
	PlayerInfo[copid][pBribeFrom] = playerid;

	new string[144];
	format(string, sizeof(string), "You offered "COL_GREEN"$%d"COL_WHITE" to %s (%d). Awaiting their decision.", amount, PlayerInfo[copid][pUserName], copid);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	format(string, sizeof(string), ""COL_YELLOW"[BRIBE] "COL_WHITE"%s (%d) offers you "COL_GREEN"$%d"COL_WHITE" to walk free. "COL_YELLOW"/accept"COL_WHITE" or "COL_RED"/refuse"COL_WHITE".", PlayerInfo[playerid][pUserName], playerid, amount);
	SendClientMessage(copid, COLOR_SERVER_INFO, string);
	return 1;
}

// 4) /escape (/esc) — instant WL10 + escape state; re-caught escapees are chained.
CMD:escape(playerid, params[])
{
	#pragma unused params
	if(!Jail_IsJailed(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You are not in jail.");
	if(PlayerInfo[playerid][pEscapeChained])
		return SendClientMessage(playerid, COLOR_ERROR, "You are chained to your cell — you cannot escape again this sentence.");

	// Mark the escape state, free them outside, and light them up at WL10.
	PlayerInfo[playerid][pEscapeChained] = true;
	Jail_Release(playerid, "", true);	// no "free to go" message — it's a break-out
	SetPlayerWanted(playerid, ESCAPE_WL, "Jail Escape");

	new string[144];
	format(string, sizeof(string), ""COL_RED"* JAIL ESCAPE: "COL_WHITE"%s (%d) has broken out of jail and is now "COL_RED"MOST WANTED"COL_WHITE"!", PlayerInfo[playerid][pUserName], playerid);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(IsCop(i)) SendClientMessage(i, COLOR_WANTED_DARKORANGE, string);
	}
	SendClientMessage(playerid, COLOR_ERROR, "You escaped jail — you are Most Wanted. If re-caught you will be chained.");
	return 1;
}
alias:escape("esc")

// 5) /breakout (/bo) — free another jailed player you are close to. The rescuer
//    must be a free civilian near the PD; the freed player keeps their wanted
//    status but leaves the cell (chained, so they can't re-escape).
CMD:breakout(playerid, params[])
{
	if(Jail_IsJailed(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot break someone out while jailed yourself.");
	if(Jail_AdminBusy(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot do that right now.");
	if(IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Cops cannot break inmates out.");
	new targetid;
	if(sscanf(params, "d", targetid))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /breakout [playerid]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || !Jail_IsJailed(targetid))
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not in jail.");

	// Proximity check against the jailed target's ACTUAL position, and require the
	// rescuer to be in the same virtual world (IsPlayerInRangeOfPoint ignores VW).
	new Float:x, Float:y, Float:z;
	GetPlayerPos(targetid, x, y, z);
	if(GetPlayerVirtualWorld(playerid) != GetPlayerVirtualWorld(targetid) ||
		!IsPlayerInRangeOfPoint(playerid, BREAKOUT_RANGE, x, y, z))
		return SendClientMessage(playerid, COLOR_ERROR, "You must be right next to the jailed player to break them out.");

	PlayerInfo[targetid][pEscapeChained] = true;
	Jail_Release(targetid, "broken out", true);
	SetPlayerWanted(targetid, ESCAPE_WL, "Jail Break");

	new string[144];
	format(string, sizeof(string), ""COL_RED"* JAIL BREAK: "COL_WHITE"%s (%d) broke %s (%d) out of jail!", PlayerInfo[playerid][pUserName], playerid, PlayerInfo[targetid][pUserName], targetid);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(IsCop(i)) SendClientMessage(i, COLOR_WANTED_DARKORANGE, string);
	}
	SendClientMessage(playerid, COLOR_SERVER_INFO, "You broke a player out of jail. Both of you are now wanted.");
	// The rescuer picks up wanted heat for springing an inmate.
	GivePlayerWanted(playerid, WANTED_WARRANT_MIN, "Aiding a jailbreak");
	return 1;
}
alias:breakout("bo", "brk")
