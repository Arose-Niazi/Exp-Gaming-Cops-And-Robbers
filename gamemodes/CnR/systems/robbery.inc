/*

	Full robbery system + crowbar for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 5 (stage 2) - design §5 (the robberies system - THE spec):
	  §5.1 shoplifting, §5.2 store-holdup UPGRADE (crowbar bonus + cop interrupt),
	  §5.3 bank robbery, §5.4 casino robbery, §5.5 house robbery (shell - depends on
	  M5 stage-3 housing), §5.6 special/major robberies (12-min hideout window),
	  §5.7 the crowbar (clothing item, dual effect, confiscated on arrest).
	Grounding: docs/research/crazybobs-site-full.md (§Robberies and Crowbar -
	VERIFIED numbers) + design §5.8 / §11.2 / §11.4 (DB additions).

	This is the M5 robbery framework. It UPGRADES the M4 base store holdup
	(holdup.inc) with the crowbar bonus (shorter clerk wait + higher per-second
	register take) and a cop mid-holdup interrupt (/stoprobbery or shooting the
	robber ends it), and adds the other robbery types on top:

	  - SAFE-TYPE robberies (bank/casino/special): a two-phase sequence driven by a
	    per-player 1s crack countdown (pRobTimer) then a gettime()-deadline carry
	    window checked on the shared GameModeClock tick (Robbery_OnTick - NO new
	    global timer for the 12-minute window). Crack the safe at the site
	    checkpoint, then deliver it to a RANDOM hideout checkpoint before the
	    deadline. Payout on delivery via the Bank_* API.
	  - The BANK robbery physically MOVES money: it drains random ONLINE players'
	    BANK balances (max $100k each) with Bank_Take and hands it to the robber
	    with Bank_GiveCash - victims lose exactly what the robber gains. Bank-insured
	    victims (an active LifeInsurance/HealthInsurance policy) are refunded 75% of
	    their loss from a gov/void source (this is the ONLY minted money by design,
	    a deliberate insurance subsidy - not a dupe of the robber's take).
	  - SHOPLIFTING: a low-risk on-foot item steal at a store clerk (small cash, +2).

	Crowbar (§5.7): pCrowbarEquipped (LSplayers.CrowbarEquipped) is a clothing item
	bought for $40,000 via /clotheswear (crowbar entry) or /buycrowbar. While
	equipped it shortens the clerk wait + safe-crack time and raises the register
	take + safe-crack success. On ARREST it is confiscated if the robber's
	RobberyHistory (LSplayers.RobberyHistory, ROB_* bitmask) carries a robbery flag
	(Robbery_OnArrest, hooked from Jail_SendToJail) - UNLESS the robber was killed
	by a cop (lethal force keeps the crowbar, Robbery_OnPlayerDeath).

	Public API:
		Robbery_Init()                       : create bank/casino/special site checkpoints + map icons (OnGameModeInit)
		Robbery_OnTick()                     : shared GameModeClock 1s tick - carry-window deadline enforcement
		Robbery_OnArrest(playerid)           : arrest hook - abort a running rob + confiscate crowbar (from jail.inc)
		Robbery_OnPlayerDeath(playerid, killerid) : death hook - abort a running rob; keep crowbar if killed by a cop (§5.7)
		Robbery_OnDisconnect(playerid)       : kill the crack timer on disconnect
		Robbery_OnEnterCP(playerid, STREAMER_TAG_CP:cpid) : dynamic-CP hook (from OnPlayerEnterDynamicCP)
		Robbery_HasHistory(playerid)         : bool - RobberyHistory carries any ROB_* flag
		Robbery_CrowbarShortenSecs(playerid, base) : safe-crack seconds after the crowbar reduction
		Holdup_CrowbarEquipped(playerid)     : bool the holdup upgrade reads for its wait/rate bonus

	Commands (crowbar buy/remove UI moved to systems/clothes.inc in M6 - this module
	keeps only the crowbar EFFECT + the Robbery_BuyCrowbar/Robbery_RemoveCrowbar publics
	the clothes menu calls):
		/shoplift                            (civ, on-foot)  §5.1 - low-risk store item steal
		/bankrob (/robbank)                  (civ)           §5.3 - start a bank-safe crack at a bank checkpoint
		/casinorob                           (civ)           §5.4 - start a casino-safe crack
		/robbery                             (civ)           §5.6 - start a special/major-site safe crack (dispatch by checkpoint)
		/houserob (/hrob /hsrob)             (civ)           §5.5 - break into the nearest OWNED house (owner must be online); money moves house->robber, no mint (housing.inc)
		/stoprobbery (/stoprob)              (cop, on-foot)  interrupt a nearby robber mid-holdup/crack (§5.2)

	Functions Being Called:
		misc:    IsCop, IsCivilian, IsPlayerInRangeOfPoint (native)
		wanted:  GivePlayerWanted, IsPlayerSpawnProtected
		jail:    Jail_IsJailed, Jail_SavePlayer
		bank:    Bank_TakeCash, Bank_GiveCash, Bank_Take, Bank_GetBalance
		server:  SaveBankRobCooldowns, ServerInfo[sBankRobLast*]
*/

//==============================================================================
//	Robbery site tables (checkpoints) - design §5.3/§5.4/§5.6
//==============================================================================

// A robbery site: the crack checkpoint the player starts the robbery at. `kind`
// is ROBKIND_BANK/CASINO/SPECIAL; `branch` is the bank cooldown branch (0=LS,
// 1=SF, 2=LV) for banks, ignored otherwise. Coords from the design/research.
enum e_RobSite
{
	Float:rsX,
	Float:rsY,
	Float:rsZ,
	rsKind,				// ROBKIND_*
	rsBranch,			// bank cooldown branch 0/1/2 (banks only, else -1)
	rsName[40]
};

// Only LS sites are live now (CITY_ == LS); SF/LV coords are kept for when those
// cities go live (§5.3 lists all 7 banks / 5 casinos). We create checkpoints for
// every listed site - out-of-city ones simply never get walked into.
static const g_RobSites[][e_RobSite] =
{
	// --- Banks (§5.3 - 7 total; the 3 city branches carry a cooldown branch) ---
	{ 620.250,  -1258.000, 13.500, ROBKIND_BANK,    0, "Los Santos Bank" },
	{ -2021.000, 460.250,  35.000, ROBKIND_BANK,    1, "San Fierro Bank" },
	{ 2470.000,  2258.750, 10.500, ROBKIND_BANK,    2, "Las Venturas Bank" },
	// --- Casinos (§5.4) ---
	{ 2137.500,  1678.500, 10.800, ROBKIND_CASINO, -1, "Caligula's Palace" },
	{ 2015.000,  1017.000, 10.800, ROBKIND_CASINO, -1, "Four Dragons Casino" },
	// --- Special / major robberies (§5.6) ---
	{ 1374.000, -1291.000, 13.500, ROBKIND_SPECIAL, -1, "CrazyBob's Place (LS)" },
	{ -1699.000, 1036.000, 45.000, ROBKIND_SPECIAL, -1, "SF Federal Mint" }
};

// Hideouts (§5.3/§5.6 - deliver the safe to a RANDOM one). Spread across the map.
static const Float:g_RobHideouts[][3] =
{
	{ 1638.000, -2237.000, 13.500 },	// LS Ocean Docks
	{ 2790.000,  -2415.000, 13.600 },	// LS El Corona docks
	{ 661.000,   1855.000,  6.500 },	// LV freight yard
	{ -1832.000, -160.000,  15.400 },	// SF Doherty
	{ 1425.000,  248.000,   19.500 }	// Red County barn
};

// Live checkpoint handles for every site (created in Robbery_Init).
static STREAMER_TAG_CP:g_RobSiteCP[sizeof(g_RobSites)];

// Bank map icon handles (created in Robbery_Init for bank sites; INVALID_STREAMER_ID for others).
static STREAMER_TAG_MAP_ICON:g_RobSiteIcon[sizeof(g_RobSites)];

// Per-site real-time cooldown (gettime() a site may be robbed again). Banks ALSO
// carry a persisted per-branch cooldown in server_data (§5.3); casino/special use
// this session-only stamp (§5.4/§5.6 [OUR DESIGN]).
static g_RobSiteCooldown[sizeof(g_RobSites)];

//==============================================================================
//	Small helpers
//==============================================================================

// Does the player's RobberyHistory carry ANY robbery flag? Drives the crowbar
// confiscation-on-arrest rule (§5.7).
FUNCTION Robbery_HasHistory(playerid)
	return (PlayerInfo[playerid][pRobberyHistory] & ROB_ANY_MASK) ? 1 : 0;

// The holdup upgrade (holdup.inc) reads this to decide its clerk-wait/register
// bonus (§5.2/§5.7). Kept here so the crowbar item stays owned by this module.
FUNCTION Holdup_CrowbarEquipped(playerid)
	return PlayerInfo[playerid][pCrowbarEquipped] ? 1 : 0;

// Safe-crack seconds after the crowbar reduction (never below 5s). §5.7.
FUNCTION Robbery_CrowbarShortenSecs(playerid, base)
{
	new secs = base;
	if(PlayerInfo[playerid][pCrowbarEquipped]) secs -= CROWBAR_SAFE_REDUCTION;
	if(secs < 5) secs = 5;
	return secs;
}

// Set a robbery-history flag + persist it (so the crowbar-confiscation rule and
// the "committed this type" record survive a relog). §5.7/§5.8.
static Robbery_FlagHistory(playerid, flag)
{
	if(!(PlayerInfo[playerid][pRobberyHistory] & flag))
	{
		PlayerInfo[playerid][pRobberyHistory] |= flag;
		Jail_SavePlayer(playerid);	// shared LSplayers UPDATE (no competing query)
	}
}

// Public wrapper the M4 store-holdup (holdup.inc, included earlier) reaches via
// CallLocalFunction when a /holdup begins, so the store holdup shares the crowbar
// confiscation history bitmask (§5.2/§5.7).
FUNCTION Robbery_MarkHoldupHistory(playerid)
{
	Robbery_FlagHistory(playerid, ROB_HOLDUP);
	return 1;
}

// The shared crime gate (mirrors holdup.inc): cop/jailed/spawn-protected/DM-zone
// all block a robbery. Returns 1 if BLOCKED (and messages the player), 0 if clear.
static Robbery_CrimeGateBlocked(playerid)
{
	if(!PlayerInfo[playerid][pLoggedIn]) { SendClientMessage(playerid, COLOR_ERROR, "You must be logged in."); return 1; }
	if(IsCop(playerid)) { SendClientMessage(playerid, COLOR_ERROR, "Law enforcement cannot commit robberies."); return 1; }
	if(Jail_IsJailed(playerid)) { SendClientMessage(playerid, COLOR_ERROR, "You cannot do that while you are in jail."); return 1; }
	if(IsPlayerSpawnProtected(playerid)) { SendClientMessage(playerid, COLOR_ERROR, "You cannot commit crimes while spawn-protected. Wait a moment."); return 1; }
	if(PlayerInfo[playerid][pInDMZone]) { SendClientMessage(playerid, COLOR_ERROR, "You cannot commit crimes inside a DM zone."); return 1; }
	return 0;
}

// Alert all cops (radar-range heads-up mirrors the holdup announce). §5 alarm.
static Robbery_AlertCops(playerid, const what[])
{
	new string[160], zoneName[MAX_ZONE_NAME];
	if(isNull(PlayerInfo[playerid][pZoneName])) format(zoneName, sizeof(zoneName), "the city");
	else format(zoneName, sizeof(zoneName), "%s", PlayerInfo[playerid][pZoneName]);
	format(string, sizeof(string), ""COL_ORANGE"[ROBBERY] "COL_WHITE"%s (%d) - %s near %s! Respond!", PlayerInfo[playerid][pUserName], playerid, what, zoneName);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || i == playerid) continue;
		if(IsCop(i)) SendClientMessage(i, COLOR_WANTED_ORANGE, string);
	}
}

//==============================================================================
//	Site setup (checkpoints + map icons) - OnGameModeInit
//==============================================================================

FUNCTION Robbery_Init()
{
	for(new i = 0; i < sizeof(g_RobSites); i++)
	{
		g_RobSiteCP[i] = CreateDynamicCP(g_RobSites[i][rsX], g_RobSites[i][rsY], g_RobSites[i][rsZ],
			(g_RobSites[i][rsKind] == ROBKIND_BANK) ? BANKROB_CP_SIZE : CASINOROB_CP_SIZE, -1, -1, -1, 100.0);
		// A robbery blip on the map for banks (so robbers can find a branch). Store the
		// handle so Robbery_Cleanup can destroy it on OnGameModeExit.
		if(g_RobSites[i][rsKind] == ROBKIND_BANK)
			g_RobSiteIcon[i] = CreateDynamicMapIcon(g_RobSites[i][rsX], g_RobSites[i][rsY], g_RobSites[i][rsZ], BANKROB_ICON_MODEL, 0, -1, -1, -1, 200.0, MAPICON_GLOBAL);
		else
			g_RobSiteIcon[i] = STREAMER_TAG_MAP_ICON:INVALID_STREAMER_ID;
	}
	return 1;
}

// Destroy all robbery-site checkpoints and bank map icons on gamemode exit.
// Mirrors Moneybag_Cleanup - called from OnGameModeExit.
FUNCTION Robbery_Cleanup()
{
	for(new i = 0; i < sizeof(g_RobSites); i++)
	{
		if(DestroyDynamicCP(g_RobSiteCP[i])) {}
		if(DestroyDynamicMapIcon(g_RobSiteIcon[i])) {}
	}
	return 1;
}

// Which site (index) is this player standing at? Returns -1 if none in range.
static Robbery_SiteAt(playerid, kindFilter)
{
	for(new i = 0; i < sizeof(g_RobSites); i++)
	{
		if(kindFilter != -1 && g_RobSites[i][rsKind] != kindFilter) continue;
		if(GetPlayerInterior(playerid) != 0) continue;	// sites are world-0 exteriors
		if(IsPlayerInRangeOfPoint(playerid, (g_RobSites[i][rsKind] == ROBKIND_BANK) ? BANKROB_CP_SIZE + 1.0 : CASINOROB_CP_SIZE + 1.0, g_RobSites[i][rsX], g_RobSites[i][rsY], g_RobSites[i][rsZ]))
			return i;
	}
	return -1;
}

//==============================================================================
//	Bank-cooldown branch helpers (server_data.BankRobLast*) - §5.3
//==============================================================================

static Robbery_BranchStamp(branch)
{
	switch(branch)
	{
		case 0: return ServerInfo[sBankRobLastLS];
		case 1: return ServerInfo[sBankRobLastSF];
		case 2: return ServerInfo[sBankRobLastLV];
	}
	return 0;
}

static Robbery_SetBranchStamp(branch, stamp)
{
	switch(branch)
	{
		case 0: ServerInfo[sBankRobLastLS] = stamp;
		case 1: ServerInfo[sBankRobLastSF] = stamp;
		case 2: ServerInfo[sBankRobLastLV] = stamp;
	}
	SaveBankRobCooldowns();
}

//==============================================================================
//	Safe-type robbery sequence (crack -> carry -> deliver) - §5.3/§5.4/§5.6
//==============================================================================

// Abort a running safe-type robbery (any phase). announce prints the failure.
static Robbery_Abort(playerid, bool:announce, const why[])
{
	if(PlayerInfo[playerid][pRobTimer] != -1)
	{
		KillTimer(PlayerInfo[playerid][pRobTimer]);
		PlayerInfo[playerid][pRobTimer] = -1;
	}
	if(PlayerInfo[playerid][pRobStage] == ROBSTAGE_NONE) return 0;
	PlayerInfo[playerid][pRobStage] = ROBSTAGE_NONE;
	PlayerInfo[playerid][pRobCrackLeft] = 0;
	PlayerInfo[playerid][pRobDeadline] = 0;
	if(IsPlayerConnected(playerid))
	{
		DisablePlayerCheckpoint(playerid);
		if(announce && !isNull(why))
		{
			new msg[144];
			format(msg, sizeof(msg), ""COL_RED"* Robbery failed: "COL_WHITE"%s", why);
			SendClientMessage(playerid, COLOR_ERROR, msg);
		}
	}
	return 1;
}

// Move to the carry phase: pick a random hideout, set the 12-min deadline, and
// point the player at it with a checkpoint (§5.3/§5.6).
static Robbery_BeginCarry(playerid)
{
	PlayerInfo[playerid][pRobStage] = ROBSTAGE_CARRYING;
	PlayerInfo[playerid][pRobHideout] = random(sizeof(g_RobHideouts));
	PlayerInfo[playerid][pRobDeadline] = gettime() + ((PlayerInfo[playerid][pRobKind] == ROBKIND_BANK) ? BANKROB_HIDEOUT_SECS : ROB_HIDEOUT_SECS);

	// Extra wanted for carrying the safe (§5.3 escalation -> WL9-10).
	GivePlayerWanted(playerid, (PlayerInfo[playerid][pRobKind] == ROBKIND_BANK) ? BANKROB_WANTED_CARRY : ROB_WANTED_CARRY, "Carrying a robbery safe");

	new h = PlayerInfo[playerid][pRobHideout];
	SetPlayerCheckpoint(playerid, g_RobHideouts[h][0], g_RobHideouts[h][1], g_RobHideouts[h][2], 4.0);

	new mins = ((PlayerInfo[playerid][pRobKind] == ROBKIND_BANK) ? BANKROB_HIDEOUT_SECS : ROB_HIDEOUT_SECS) / 60;
	new msg[160];
	format(msg, sizeof(msg), ""COL_GREEN"* Safe cracked! "COL_WHITE"Get the safe to the marked hideout within "COL_YELLOW"%d minutes"COL_WHITE" - the cops are on you!", mins);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	Robbery_AlertCops(playerid, "is fleeing with a cracked safe");
}

// Per-second crack countdown (pRobTimer). When it lapses, move to carry phase.
FUNCTION Robbery_CrackTick(playerid)
{
	if(PlayerInfo[playerid][pRobStage] != ROBSTAGE_CRACKING) return 0;
	if(!IsPlayerConnected(playerid)) { Robbery_Abort(playerid, false, ""); return 0; }

	// Must stay on foot at the site while cracking.
	new s = PlayerInfo[playerid][pRobSite];
	if(IsPlayerInAnyVehicle(playerid) ||
		!IsPlayerInRangeOfPoint(playerid, ((g_RobSites[s][rsKind] == ROBKIND_BANK) ? BANKROB_CP_SIZE : CASINOROB_CP_SIZE) + 2.0, g_RobSites[s][rsX], g_RobSites[s][rsY], g_RobSites[s][rsZ]))
	{
		Robbery_Abort(playerid, true, "you left the safe.");
		return 0;
	}

	PlayerInfo[playerid][pRobCrackLeft]--;
	if(PlayerInfo[playerid][pRobCrackLeft] <= 0)
	{
		if(PlayerInfo[playerid][pRobTimer] != -1) { KillTimer(PlayerInfo[playerid][pRobTimer]); PlayerInfo[playerid][pRobTimer] = -1; }
		// Success roll (§5.7 - a crowbar RAISES the chance of a successful crack).
		// Base ROB_CRACK_BASE_SUCCESS%, +CROWBAR_SUCCESS_BONUS% with a crowbar.
		new chance = ROB_CRACK_BASE_SUCCESS + (PlayerInfo[playerid][pCrowbarEquipped] ? CROWBAR_SUCCESS_BONUS : 0);
		if(random(100) >= chance)
		{
			Robbery_Abort(playerid, true, "the safe jammed - the crack failed. Try again.");
			return 1;
		}
		Robbery_BeginCarry(playerid);
		return 1;
	}

	new gt[64];
	format(gt, sizeof(gt), "~r~ROBBERY~n~~w~cracking safe... %d", PlayerInfo[playerid][pRobCrackLeft]);
	GameTextForPlayer(playerid, gt, 1200, 5);
	return 1;
}

// Start a safe-type robbery at site `s`. Sets WL6, arms the crack countdown.
static Robbery_StartCrack(playerid, s)
{
	new baseSecs;
	switch(g_RobSites[s][rsKind])
	{
		case ROBKIND_BANK:    baseSecs = BANKROB_CRACK_SECS;
		case ROBKIND_CASINO:  baseSecs = CASINOROB_CRACK_SECS;
		default:              baseSecs = SPECIALROB_CRACK_SECS;
	}
	new secs = Robbery_CrowbarShortenSecs(playerid, baseSecs);

	PlayerInfo[playerid][pRobStage] = ROBSTAGE_CRACKING;
	PlayerInfo[playerid][pRobKind] = g_RobSites[s][rsKind];
	PlayerInfo[playerid][pRobSite] = s;
	PlayerInfo[playerid][pRobCrackLeft] = secs;
	PlayerInfo[playerid][pRobDeadline] = 0;

	// Warrant issued when the robbery begins (§5 general principle); WL6 start.
	GivePlayerWanted(playerid, (g_RobSites[s][rsKind] == ROBKIND_BANK) ? BANKROB_WANTED_START : ROB_WANTED_START, "Robbery in progress");

	if(PlayerInfo[playerid][pRobTimer] != -1) KillTimer(PlayerInfo[playerid][pRobTimer]);
	PlayerInfo[playerid][pRobTimer] = SetTimerEx("Robbery_CrackTick", 1000, true, "d", playerid);

	new msg[176];
	format(msg, sizeof(msg), ""COL_RED"* ROBBERY! "COL_WHITE"Cracking the %s safe - hold position for "COL_YELLOW"~%d seconds"COL_WHITE".%s", g_RobSites[s][rsName], secs, PlayerInfo[playerid][pCrowbarEquipped] ? " (crowbar: faster)" : "");
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	Robbery_AlertCops(playerid, "is cracking a safe");
}

//==============================================================================
//	Payout on hideout delivery - §5.3 (money MOVES), §5.4/§5.6 (server yield)
//==============================================================================

// Bank rob payout: drain random ONLINE players' BANK balances (max $100k each),
// hand the total to the robber, and refund 75% to insured victims (gov/void).
// Money physically MOVES: victims lose exactly what the robber gains; the only
// minted money is the insured refund, a deliberate insurance subsidy (§5.3).
static Robbery_PayoutBank(playerid)
{
	new total = 0, victims = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(i == playerid || !IsPlayerConnected(i) || IsPlayerNPC(i) || !PlayerInfo[i][pLoggedIn]) continue;
		new bal = Bank_GetBalance(i);
		if(bal <= 0) continue;

		// A small random slice of this account, capped at $100k (§5.3 VERIFIED).
		new take = bal / 4 + random(bal / 4 + 1);	// ~25-50% of the balance
		if(take > BANKROB_MAX_PER_ACCT) take = BANKROB_MAX_PER_ACCT;
		if(take > bal) take = bal;
		if(take <= 0) continue;

		// Respect the full-server yield ceiling (§5.3 ~$500k-1M).
		if(total + take > BANKROB_YIELD_MAX) take = BANKROB_YIELD_MAX - total;
		if(take <= 0) break;

		// MOVE the money: victim's bank -> robber's cash-in-hand. No mint.
		if(!Bank_Take(i, take)) continue;
		total += take;
		victims++;

		// Bank-insured victims get 75% of their loss back (gov/void source, §5.3).
		new bool:insured = (PlayerInfo[i][pLifeInsurance] > 0 || PlayerInfo[i][pHealthInsExpiry] > gettime());
		new refund = 0;
		if(insured)
		{
			refund = (take / 100) * BANKROB_INSURED_PCT;
			if(refund > 0) Bank_Give(i, refund);	// minted subsidy by design (insurance)
		}

		new vmsg[176];
		if(insured) format(vmsg, sizeof(vmsg), ""COL_RED"* Bank robbery! "COL_WHITE"$%d"COL_RED" was taken from your account - insurance refunded "COL_GREEN"$%d"COL_WHITE".", take, refund);
		else format(vmsg, sizeof(vmsg), ""COL_RED"* Bank robbery! "COL_WHITE"$%d"COL_RED" was stolen from your bank account. Get insured to recover losses."COL_WHITE, take);
		SendClientMessage(i, COLOR_VICTIM, vmsg);

		Jail_SavePlayer(i);	// persist the victim's new balance immediately
		if(total >= BANKROB_YIELD_MAX) break;
	}

	if(total > 0)
	{
		Bank_GiveCash(playerid, total);
		new rmsg[128];
		format(rmsg, sizeof(rmsg), ""COL_GREEN"* You drained "COL_WHITE"%d"COL_GREEN" bank account(s) for "COL_WHITE"$%d"COL_GREEN" total."COL_WHITE, victims, total);
		SendClientMessage(playerid, COLOR_SERVER_INFO, rmsg);
	}
	return total;
}

// Casino/special payout: a scaling server-yield (no victim accounts) via Bank_*.
static Robbery_PayoutYield(playerid, kind)
{
	new lo, hi;
	if(kind == ROBKIND_CASINO) { lo = CASINOROB_YIELD_MIN; hi = CASINOROB_YIELD_MAX; }
	else { lo = SPECIALROB_YIELD_MIN; hi = SPECIALROB_YIELD_MAX; }

	// Scale with the online population (§5 - payouts scale with server size).
	new online = 0;
	for(new i = 0; i < MAX_PLAYERS; i++) if(IsPlayerConnected(i) && !IsPlayerNPC(i)) online++;
	new span = hi - lo;
	new pay = lo + random(span + 1);
	pay = pay / 2 + (pay / 2) * (online > 20 ? 20 : online) / 20;	// half fixed, half population-scaled
	if(pay < lo) pay = lo;

	Bank_GiveCash(playerid, pay);
	return pay;
}

// The player reached the hideout carrying a safe - pay out + finish. §5.3/§5.6.
static Robbery_Deliver(playerid)
{
	new kind = PlayerInfo[playerid][pRobKind], take = 0, histFlag = ROB_SPECIAL;
	switch(kind)
	{
		case ROBKIND_BANK:
		{
			take = Robbery_PayoutBank(playerid);
			histFlag = ROB_BANK;
			// Close the robbed branch for the cooldown window (§5.3 banks close).
			new s = PlayerInfo[playerid][pRobSite];
			if(g_RobSites[s][rsBranch] != -1) Robbery_SetBranchStamp(g_RobSites[s][rsBranch], gettime() + BANKROB_BRANCH_CD);
		}
		case ROBKIND_CASINO: { take = Robbery_PayoutYield(playerid, ROBKIND_CASINO);  histFlag = ROB_CASINO;  g_RobSiteCooldown[PlayerInfo[playerid][pRobSite]] = gettime() + ROB_SITE_CD; }
		default:             { take = Robbery_PayoutYield(playerid, ROBKIND_SPECIAL); histFlag = ROB_SPECIAL; g_RobSiteCooldown[PlayerInfo[playerid][pRobSite]] = gettime() + ROB_SITE_CD; }
	}

	// Finish the sequence + record the history flag + score.
	if(PlayerInfo[playerid][pRobTimer] != -1) { KillTimer(PlayerInfo[playerid][pRobTimer]); PlayerInfo[playerid][pRobTimer] = -1; }
	PlayerInfo[playerid][pRobStage] = ROBSTAGE_NONE;
	PlayerInfo[playerid][pRobDeadline] = 0;
	DisablePlayerCheckpoint(playerid);
	PlayerInfo[playerid][pScore] += 1;	// each completed robbery = +1 score (§5)
	Robbery_FlagHistory(playerid, histFlag);

	new msg[176];
	if(take > 0) format(msg, sizeof(msg), ""COL_GREEN"* Robbery complete! "COL_WHITE"You cracked the safe for "COL_GREEN"$%d"COL_WHITE". +1 score. Lie low!", take);
	else format(msg, sizeof(msg), ""COL_GREEN"* Robbery complete! "COL_WHITE"but there was nothing worth taking. +1 score.");
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
}

//==============================================================================
//	Shared GameModeClock tick - carry-window deadline (NO new global timer) §5.6
//==============================================================================

FUNCTION Robbery_OnTick()
{
	for(new playerid = 0; playerid < MAX_PLAYERS; playerid++)
	{
		if(!IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) continue;
		if(PlayerInfo[playerid][pRobStage] != ROBSTAGE_CARRYING) continue;

		// Deadline blown - the safe is lost (§5.6 12-minute window).
		if(gettime() >= PlayerInfo[playerid][pRobDeadline])
		{
			Robbery_Abort(playerid, true, "you ran out of time to reach the hideout.");
			continue;
		}
		// Reached the hideout on foot? (checkpoint proximity - belt-and-braces to
		// OnPlayerEnterCheckpoint so a missed callback still resolves the delivery.)
		new h = PlayerInfo[playerid][pRobHideout];
		if(!IsPlayerInAnyVehicle(playerid) && IsPlayerInRangeOfPoint(playerid, 4.0, g_RobHideouts[h][0], g_RobHideouts[h][1], g_RobHideouts[h][2]))
			Robbery_Deliver(playerid);
	}
	return 1;
}

//==============================================================================
//	Checkpoint / hook entry points
//==============================================================================

// Standard (non-race) checkpoint entry - the hideout delivery is a plain
// SetPlayerCheckpoint, so this resolves it. Hooked from OnPlayerEnterCheckpoint.
FUNCTION Robbery_OnPlayerEnterCheckpoint(playerid)
{
	if(PlayerInfo[playerid][pRobStage] == ROBSTAGE_CARRYING)
	{
		new h = PlayerInfo[playerid][pRobHideout];
		if(IsPlayerInRangeOfPoint(playerid, 5.0, g_RobHideouts[h][0], g_RobHideouts[h][1], g_RobHideouts[h][2]))
		{
			Robbery_Deliver(playerid);
			return 1;
		}
	}
	return 0;
}

// Dynamic-CP entry (site checkpoints). Just a hint - the actual start is the
// /bankrob //casinorob //robbery command (site proximity is re-checked there).
FUNCTION Robbery_OnEnterCP(playerid, STREAMER_TAG_CP:cpid)
{
	for(new i = 0; i < sizeof(g_RobSites); i++)
	{
		if(g_RobSiteCP[i] != cpid) continue;
		if(IsCop(playerid) || PlayerInfo[playerid][pRobStage] != ROBSTAGE_NONE) return 1;
		new cmdName[24];
		switch(g_RobSites[i][rsKind])
		{
			case ROBKIND_BANK:   format(cmdName, sizeof(cmdName), "/bankrob");
			case ROBKIND_CASINO: format(cmdName, sizeof(cmdName), "/casinorob");
			default:             format(cmdName, sizeof(cmdName), "/robbery");
		}
		new msg[144];
		format(msg, sizeof(msg), ""COL_ORANGE"* %s. "COL_WHITE"Type "COL_YELLOW"%s"COL_WHITE" to start the robbery (you'll be wanted!).", g_RobSites[i][rsName], cmdName);
		SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
		return 1;
	}
	return 0;
}

//==============================================================================
//	Arrest / death / disconnect hooks - crowbar confiscation (§5.7)
//==============================================================================

// Arrest hook (from Jail_SendToJail). Aborts any running robbery and confiscates
// the crowbar if RobberyHistory carries a robbery flag (§5.7). The ARREST path
// always confiscates - the "killed by a cop keeps it" exception is death-only.
FUNCTION Robbery_OnArrest(playerid)
{
	Robbery_Abort(playerid, false, "");
	if(PlayerInfo[playerid][pCrowbarEquipped] && Robbery_HasHistory(playerid))
	{
		PlayerInfo[playerid][pCrowbarEquipped] = false;
		SendClientMessage(playerid, COLOR_ERROR, ""COL_RED"* Arrested: "COL_WHITE"your crowbar was confiscated as evidence.");
	}
	// Clear the recent-robbery history on a completed arrest (fresh slate; the
	// crowbar is already gone if it applied). Persist via the shared UPDATE.
	PlayerInfo[playerid][pRobberyHistory] = 0;
	Jail_SavePlayer(playerid);
	return 1;
}

// Death hook (from OnPlayerDeath). Aborts a running robbery. §5.7 exception: if
// the robber was KILLED BY A COP (lethal force), the crowbar is NOT confiscated.
// Any OTHER death (civilian kill, environment, suicide) also keeps the crowbar -
// only a lawful ARREST confiscates it - so here we only abort the sequence.
FUNCTION Robbery_OnPlayerDeath(playerid, killerid)
{
	#pragma unused killerid
	Robbery_Abort(playerid, false, "");
	return 1;
}

FUNCTION Robbery_OnDisconnect(playerid)
{
	if(PlayerInfo[playerid][pRobTimer] != -1)
	{
		KillTimer(PlayerInfo[playerid][pRobTimer]);
		PlayerInfo[playerid][pRobTimer] = -1;
	}
	PlayerInfo[playerid][pRobStage] = ROBSTAGE_NONE;
	return 1;
}

//==============================================================================
//	The crowbar (clothing item) - buy / remove path (§5.7)
//==============================================================================

// Shared crowbar purchase path. $40,000 via Bank_TakeCash. The item's UI (the
// /clotheswear menu entry, /buycrowbar, /crowbar) lives in systems/clothes.inc as
// of M6 (the clothes system) - this stays here because the crowbar's crime EFFECT
// (holdup/safe-crack readers, arrest confiscation) is owned by this module. Clothes
// calls this public to buy+equip so /buycrowbar and the clothes menu are unified.
FUNCTION Robbery_BuyCrowbar(playerid)
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(PlayerInfo[playerid][pCrowbarEquipped]) return SendClientMessage(playerid, COLOR_ERROR, "You already have a crowbar equipped. Use /crowbar to put it away.");
	if(PlayerInfo[playerid][pMoney] < CROWBAR_PRICE)
	{
		new err[128];
		format(err, sizeof(err), "A crowbar costs "COL_GREEN"$%d"COL_WHITE" cash - you cannot afford it.", CROWBAR_PRICE);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	if(!Bank_TakeCash(playerid, CROWBAR_PRICE)) return SendClientMessage(playerid, COLOR_ERROR, "Payment failed.");

	PlayerInfo[playerid][pCrowbarEquipped] = true;
	Jail_SavePlayer(playerid);	// persist the equip flag (§5.8)

	new msg[176];
	format(msg, sizeof(msg), ""COL_GREEN"* Crowbar equipped "COL_WHITE"($%d)"COL_GREEN". "COL_WHITE"It shortens holdup/robbery time and raises your register take (see robberies). Lost on arrest.", CROWBAR_PRICE);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}

// Put the crowbar away (unequip). §5.7 remove path - called from /crowbar (clothes.inc).
FUNCTION Robbery_RemoveCrowbar(playerid)
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(!PlayerInfo[playerid][pCrowbarEquipped]) return SendClientMessage(playerid, COLOR_ERROR, "You do not have a crowbar equipped. Buy one with /clotheswear.");
	PlayerInfo[playerid][pCrowbarEquipped] = false;
	Jail_SavePlayer(playerid);
	SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_GREEN"* Crowbar put away."COL_WHITE" Re-equip it with "COL_YELLOW"/clotheswear"COL_WHITE".");
	return 1;
}

//==============================================================================
//	§5.1 Shoplifting - /shoplift (low-risk store item steal)
//==============================================================================

CMD:shoplift(playerid, params[])
{
	#pragma unused params
	if(Robbery_CrimeGateBlocked(playerid)) return 1;
	if(IsPlayerInAnyVehicle(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You must be on foot in a store to shoplift.");
	if(gettime() < PlayerInfo[playerid][pShopliftCooldown])
	{
		new err[112];
		format(err, sizeof(err), "You just shoplifted - wait %d seconds before trying again.", PlayerInfo[playerid][pShopliftCooldown] - gettime());
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	// Must be at a store clerk (reuse the holdup store-actor gate).
	if(Holdup_NearestStore(playerid) == -1)
		return SendClientMessage(playerid, COLOR_ERROR, "You must be standing at a store clerk (24-7, food shop, clothes store, ...) to shoplift.");

	PlayerInfo[playerid][pShopliftCooldown] = gettime() + SHOPLIFT_CD;

	// Low-risk: small chance of getting caught (a bump to wanted); otherwise a
	// petty item, resold for a little cash. Small/no wanted per §5.1.
	new loot = SHOPLIFT_CASH_MIN + random(SHOPLIFT_CASH_MAX - SHOPLIFT_CASH_MIN + 1);
	Bank_GiveCash(playerid, loot);
	GivePlayerWanted(playerid, SHOPLIFT_WANTED, "Shoplifting");	// +2 (suppressed in safe zones)
	Robbery_FlagHistory(playerid, ROB_HOLDUP);	// petty theft counts toward the crowbar rule

	new msg[144];
	format(msg, sizeof(msg), ""COL_GREEN"* Shoplifted! "COL_WHITE"You pocketed an item worth "COL_GREEN"$%d"COL_WHITE" and slipped out.", loot);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}

//==============================================================================
//	§5.3 Bank robbery - /bankrob (safe crack at a bank checkpoint)
//==============================================================================

CMD:bankrob(playerid, params[])
{
	#pragma unused params
	if(Robbery_CrimeGateBlocked(playerid)) return 1;
	if(IsPlayerInAnyVehicle(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You must be on foot at the bank to rob it.");
	if(PlayerInfo[playerid][pRobStage] != ROBSTAGE_NONE || PlayerInfo[playerid][pHoldingUp])
		return SendClientMessage(playerid, COLOR_ERROR, "You are already in the middle of a robbery.");

	new s = Robbery_SiteAt(playerid, ROBKIND_BANK);
	if(s == -1) return SendClientMessage(playerid, COLOR_ERROR, "You must be standing at a bank robbery checkpoint. Follow the bank map icons.");

	// Branch cooldown: banks close for a while after a rob (§5.3).
	new branch = g_RobSites[s][rsBranch];
	if(branch != -1 && gettime() < Robbery_BranchStamp(branch))
	{
		new left = (Robbery_BranchStamp(branch) - gettime() + 59) / 60;
		new err[128];
		format(err, sizeof(err), "This bank is closed after a recent robbery - try again in ~%d minutes.", left);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}

	Robbery_StartCrack(playerid, s);
	return 1;
}
alias:bankrob("robbank")

//==============================================================================
//	§5.4 Casino robbery - /casinorob
//==============================================================================

CMD:casinorob(playerid, params[])
{
	#pragma unused params
	if(Robbery_CrimeGateBlocked(playerid)) return 1;
	if(IsPlayerInAnyVehicle(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You must be on foot in the casino to rob it.");
	if(PlayerInfo[playerid][pRobStage] != ROBSTAGE_NONE || PlayerInfo[playerid][pHoldingUp])
		return SendClientMessage(playerid, COLOR_ERROR, "You are already in the middle of a robbery.");

	new s = Robbery_SiteAt(playerid, ROBKIND_CASINO);
	if(s == -1) return SendClientMessage(playerid, COLOR_ERROR, "You must be standing at a casino robbery checkpoint.");
	if(gettime() < g_RobSiteCooldown[s])
	{
		new left = (g_RobSiteCooldown[s] - gettime() + 59) / 60;
		new err[112];
		format(err, sizeof(err), "This casino was just robbed - try again in ~%d minutes.", left);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	Robbery_StartCrack(playerid, s);
	return 1;
}

//==============================================================================
//	§5.6 Special / major robberies - /robbery (dispatch by checkpoint)
//==============================================================================

CMD:robbery(playerid, params[])
{
	#pragma unused params
	if(Robbery_CrimeGateBlocked(playerid)) return 1;
	if(IsPlayerInAnyVehicle(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You must be on foot at the robbery site.");
	if(PlayerInfo[playerid][pRobStage] != ROBSTAGE_NONE || PlayerInfo[playerid][pHoldingUp])
		return SendClientMessage(playerid, COLOR_ERROR, "You are already in the middle of a robbery.");

	// /robbery works at special sites (and, as a convenience, casinos).
	new s = Robbery_SiteAt(playerid, ROBKIND_SPECIAL);
	if(s == -1) s = Robbery_SiteAt(playerid, ROBKIND_CASINO);
	if(s == -1) return SendClientMessage(playerid, COLOR_ERROR, "There is no robbery target here. Find a special-robbery or casino checkpoint.");
	if(gettime() < g_RobSiteCooldown[s])
	{
		new left = (g_RobSiteCooldown[s] - gettime() + 59) / 60;
		new err[112];
		format(err, sizeof(err), "This site was just robbed - try again in ~%d minutes.", left);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	Robbery_StartCrack(playerid, s);
	return 1;
}

//==============================================================================
//	§5.5 House robbery - /houserob (wired into the M5 stage-3 housing system)
//==============================================================================

// Break into the nearest OWNED house at the player's position (§5.5). Gates: crime
// gate (cop/jailed/spawn-protected/DM-zone), on foot, per-robber cooldown, a house
// entrance within range, and - the CB rule - the OWNER must be ONLINE. The break-in
// rolls against the house's super lock / pet defenses (House_RobSteal, in housing.inc,
// included before this module). On a SUCCESSFUL break-in the house's stored money
// MOVES into the robber's hand (Bank_GiveCash) - the house loses exactly what the
// robber gains (NO minting). Either way a warrant (+6) is issued, the ROB_HOUSE flag
// is set, the house alarm fires (owner + cops), and the cooldown is stamped.
CMD:houserob(playerid, params[])
{
	#pragma unused params
	if(Robbery_CrimeGateBlocked(playerid)) return 1;
	if(IsPlayerInAnyVehicle(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You must be on foot at the house door to rob it.");

	// Per-robber cooldown (§5.5 [OUR DESIGN]).
	if(gettime() < PlayerInfo[playerid][pHouseRobCooldown])
	{
		new err[128];
		format(err, sizeof(err), "You just attempted a house robbery - wait %d seconds before the next one.", PlayerInfo[playerid][pHouseRobCooldown] - gettime());
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}

	// Nearest OWNED house entrance in range (unowned/for-sale houses have nothing to
	// steal, so they are not targets). House_NearestOwned lives in housing.inc.
	new house = House_NearestOwned(playerid, HOUSEROB_RANGE);
	if(house == -1) return SendClientMessage(playerid, COLOR_ERROR, "There is no owned house here to rob. Stand at the door of a player-owned house.");

	// You cannot rob your own house.
	if(House_OwnerAID(house) == PlayerInfo[playerid][pID])
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot rob your own house.");

	// CB rule: the owner MUST be online to rob their house (§5.5 VERIFIED).
	new ownerid;
	if(!House_IsOwnerOnline(house, ownerid))
		return SendClientMessage(playerid, COLOR_ERROR, "You can only rob a house while its owner is online.");

	// Commit: stamp the cooldown, issue the warrant (+6), flag ROB_HOUSE history, and
	// run the break-in. House_RobSteal rolls success vs super-lock/pets, fires the
	// alarm, and MOVES the house safe money to the robber on success (no mint).
	PlayerInfo[playerid][pHouseRobCooldown] = gettime() + HOUSEROB_CD;
	GivePlayerWanted(playerid, HOUSEROB_WANTED, "House robbery");	// +6 warrant (§5.5)
	Robbery_FlagHistory(playerid, ROB_HOUSE);						// crowbar-confiscation + record (§5.7/§5.8)
	House_RobSteal(playerid, house);								// steal (money moves, no mint) or get foiled
	return 1;
}
alias:houserob("hrob", "hsrob")

//==============================================================================
//	§5.2 Cop mid-robbery interrupt - /stoprobbery (minimal interrupt)
//==============================================================================

// A cop on foot near a robber can interrupt a running holdup OR safe-crack. This
// is the scripted "cop enters and presses MMB" interrupt (§5.2). It ends the
// robbery with nothing; the cop should then /arrest the (now warranted) robber.
CMD:stoprobbery(playerid, params[])
{
	#pragma unused params
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can stop a robbery.");
	if(IsPlayerInAnyVehicle(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You must be on foot to interrupt a robbery.");

	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	new target = -1;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(i == playerid || !IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(!PlayerInfo[i][pHoldingUp] && PlayerInfo[i][pRobStage] == ROBSTAGE_NONE) continue;
		if(GetPlayerInterior(playerid) != GetPlayerInterior(i) || GetPlayerVirtualWorld(playerid) != GetPlayerVirtualWorld(i)) continue;
		if(IsPlayerInRangeOfPoint(i, HOLDUP_ACTOR_RANGE + 2.0, x, y, z)) { target = i; break; }
	}
	if(target == -1) return SendClientMessage(playerid, COLOR_ERROR, "No robbery in progress next to you.");

	// End whichever robbery is running on the target.
	if(PlayerInfo[target][pHoldingUp]) Holdup_CopInterrupt(target);
	if(PlayerInfo[target][pRobStage] != ROBSTAGE_NONE) Robbery_Abort(target, true, "the police interrupted your robbery!");

	new msg[144];
	format(msg, sizeof(msg), ""COL_ORANGE"* You interrupted %s (%d)'s robbery! "COL_WHITE"Arrest them before they flee.", PlayerInfo[target][pUserName], target);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	format(msg, sizeof(msg), ""COL_RED"* A cop interrupted your robbery - you got nothing! "COL_WHITE"Run or you'll be arrested.");
	SendClientMessage(target, COLOR_ERROR, msg);
	return 1;
}
alias:stoprobbery("stoprob")
