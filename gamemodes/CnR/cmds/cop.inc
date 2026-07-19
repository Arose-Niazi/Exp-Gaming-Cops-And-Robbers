/*

	Cop command suite for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 3 (stage 2) - design §3.1 (minimal cop set) + §2.4 (ticket->warrant
	escalation, arrest clears wanted).

	Cop-only verbs that drive the arrest->jail loop: /arrest, /ticket, /vc, /su,
	/backup, /report+/clr, /suspects+/warrants+/mostwanted, /jaillist, /parole,
	/accept+/refuse (bribe resolution), /refill (PD) and /donut. Gates are IsCop
	+ on-foot per the spec (cannot arrest in-vehicle; targets must be on foot).
	Builds entirely on the stage-1 wanted API and the stage-2 jail system.

	Credits:
		crime-cop-loop.md §4 for the cop mechanic grounding.

	Commands (gate in brackets):
		/arrest (/ar)                    (cop, on-foot) nearest WL6+ on foot in range
		/ticket (/tk)                    (cop, on-foot) nearest WL1-5 in range
		/vc (/visualcontact)             (cop) pause a suspect's decay + announce
		/su <id> <crime>                 (cop) mark a suspect + raise WL (serious)
		/backup (/bk)                    (cop) purple marker + broadcast to cops
		/report (/rp) <id> <reason>      (cop) raise a suspect's WL (serious)
		/clr (/cancellastreport)         (cop) undo last /report
		/copmsg (/cm) <text>             (cop) PM to all cops
		/suspects (/sus)                 (cop) list WL1-5
		/warrants (/war)                 (cop) list WL6+
		/mostwanted (/mw)                (cop) list WL9-10
		/jaillist (/jl)                  (cop) jailed players + time + bail
		/parole (/pl) <id>               (cop) free a served-minimum inmate
		/accept (/ac) /refuse (/ref)     (cop) resolve an offered bribe
		/refill (/rf)                    (cop, at PD) HP/armour/weapons on points
		/donut                           (cop) +50% HP, carry up to 6

	Functions Being Called:
		misc:     IsCop
		wanted:   GetPlayerWanted, GetWantedTier, GivePlayerWanted, SetVisualContact
		jail:     Jail_SendToJail, Jail_Release, Jail_IsJailed
		messages: (cop broadcast helper below)
*/

//==============================================================================
//	Internal helpers
//==============================================================================

// Broadcast to every on-line cop (mirrors the messages.inc SendMessageTo* set).
static Cop_Broadcast(colour, const text[])
{
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || !PlayerInfo[i][pLoggedIn]) continue;
		if(IsCop(i)) SendClientMessage(i, colour, text);
	}
	return 1;
}

// Nearest on-foot suspect to `copid` inside `range` whose wanted level is within
// [wlMin, wlMax]. Returns the player id or -1. Skips DM-zone / spawn-protected
// / jailed targets so cops can't act on protected players.
static Cop_NearestSuspect(copid, Float:range, wlMin, wlMax)
{
	new Float:cx, Float:cy, Float:cz, best = -1;
	new Float:bestDist = range + 1.0;
	GetPlayerPos(copid, cx, cy, cz);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(i == copid || !IsPlayerConnected(i) || IsPlayerNPC(i) || !PlayerInfo[i][pLoggedIn]) continue;
		if(IsCop(i)) continue;
		if(PlayerInfo[i][pInDMZone] || Jail_IsJailed(i)) continue;
		new wl = GetPlayerWanted(i);
		if(wl < wlMin || wl > wlMax) continue;
		if(!IsPlayerInRangeOfPoint(i, range, cx, cy, cz)) continue;
		new Float:d = GetPlayerDistanceFromPoint(i, cx, cy, cz);
		if(d < bestDist) { bestDist = d; best = i; }
	}
	return best;
}

// A player is "on foot" for arrest/ticket purposes when not in any vehicle.
static bool:Cop_OnFoot(playerid)
	return (!IsPlayerInAnyVehicle(playerid));

// Stage B (§3.5) - the cop rank-title ladder (0-10 Recruit->Commissioner). Shared
// by /d, /coprank, and the arrest promotion message. index = pCopRank.
static const g_CopRankTitle[][] =
{
	"Recruit", "Officer", "Officer II", "Officer III", "Senior Officer",
	"Sergeant", "Lieutenant", "Captain", "Commander", "Deputy Chief", "Commissioner"
};

// Stage B (§3.5) - a short label for the current /refill weapon kit (tiers unlock
// at ranks 3 Shotgun / 6 MP5 / 9 M4, see Refill_OnDialogResponse). Used by /coprank
// + the promotion message. Returns a string literal.
Cop_RefillTierLabel(rank)
{
	new label[40];
	if(rank >= 9) format(label, sizeof(label), "Deagle + Shotgun + MP5 + M4");
	else if(rank >= 6) format(label, sizeof(label), "Deagle + Shotgun + MP5");
	else if(rank >= 3) format(label, sizeof(label), "Deagle + Shotgun");
	else format(label, sizeof(label), "Deagle");
	return label;
}

//==============================================================================
//	/arrest (/ar) - design §3.1 / §4.2
//==============================================================================

CMD:arrest(playerid, params[])
{
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can arrest.");
	if(Jail_IsJailed(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot do that right now.");
	if(!Cop_OnFoot(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You must be on foot to arrest a suspect.");

	// Optional explicit target; otherwise the nearest warranted suspect.
	new targetid = -1;
	if(!isNull(params))
	{
		if(sscanf(params, "d", targetid)) return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /arrest [playerid] (or blank for nearest)");
		if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || IsCop(targetid))
			return SendClientMessage(playerid, COLOR_ERROR, "That is not a valid suspect.");
		if(GetPlayerWanted(targetid) < WANTED_WARRANT_MIN)
			return SendClientMessage(playerid, COLOR_ERROR, "You can only arrest a warranted suspect (WL6+). Use /ticket for lower levels.");
		if(!Cop_OnFoot(targetid))
			return SendClientMessage(playerid, COLOR_ERROR, "You cannot arrest someone in a vehicle - stop the car first.");
		new Float:cx, Float:cy, Float:cz;
		GetPlayerPos(playerid, cx, cy, cz);
		if(!IsPlayerInRangeOfPoint(targetid, ARREST_RANGE, cx, cy, cz))
			return SendClientMessage(playerid, COLOR_ERROR, "That suspect is too far away to arrest.");
	}
	else
	{
		targetid = Cop_NearestSuspect(playerid, ARREST_RANGE, WANTED_WARRANT_MIN, MAX_WANTED);
		if(targetid == -1) return SendClientMessage(playerid, COLOR_ERROR, "No warranted suspect (WL6+) on foot within range.");
		if(!Cop_OnFoot(targetid)) return SendClientMessage(playerid, COLOR_ERROR, "The nearest suspect is in a vehicle - stop the car first.");
	}

	new wl = GetPlayerWanted(targetid);
	new bool:serious = (PlayerInfo[targetid][pSeriousCrimes] > 0);

	// Jail_SendToJail disarms, clears wanted (§2.4), teleports + arms the timer.
	// It returns 0 without jailing if the target is admin-jailed (admin-jail wins)
	// or no longer valid - in that case do NOT pay the cop or broadcast a false arrest.
	if(!Jail_SendToJail(targetid, wl, "Arrested by police", serious))
		return SendClientMessage(playerid, COLOR_ERROR, "That suspect cannot be arrested right now.");

	Dispatch_SetLastSuspect(playerid, targetid);	// Stage B (§3.3) - $sus source

	// Cop reward: +1 score + WL-scaled cash (§3.1). Cop rank climbs with arrests.
	new bonus = wl * ARREST_CASH_PER_WL;
	PlayerInfo[playerid][pScore] += ARREST_SCORE;
	PlayerInfo[playerid][pMoney] += bonus;
	UpdateMoney(playerid);
	// M7 - arrest cash feeds the GOVERNMENT stock pool (§10.2, factor 0.1).
	StockMarket_UpdateEarnings(E_STOCK_GOVERNMENT, bonus, STOCK_FACTOR_GOV);
	new oldRank = PlayerInfo[playerid][pCopRank];
	if(PlayerInfo[playerid][pCopRank] < 10) PlayerInfo[playerid][pCopRank]++;
	if(PlayerInfo[playerid][pRefillPoints] < REFILL_POINTS_MAX) PlayerInfo[playerid][pRefillPoints]++;

	// Stage B (§3.5) - a promotion milestone message at ranks that unlock a new
	// /refill weapon tier (3=Shotgun, 6=MP5, 9=M4). Pure UX; the rank is already
	// persisted by the disconnect/state save (CopRank column). Announced once, only
	// when the rank actually advanced this arrest.
	new newRank = PlayerInfo[playerid][pCopRank];
	if(newRank > oldRank && (newRank == 3 || newRank == 6 || newRank == 9))
	{
		new promo[176];
		format(promo, sizeof(promo), ""COL_GREEN"* Promoted to %s! "COL_WHITE"Your /refill now issues: "COL_YELLOW"%s"COL_WHITE".", g_CopRankTitle[newRank], Cop_RefillTierLabel(newRank));
		SendClientMessage(playerid, COLOR_SERVER_INFO, promo);
	}

	new string[160];
	format(string, sizeof(string), ""COL_GREEN"* Arrest: "COL_WHITE"you jailed %s (%d) at WL%d. "COL_GREEN"+$%d, +%d score.", PlayerInfo[targetid][pUserName], targetid, wl, bonus, ARREST_SCORE);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);

	format(string, sizeof(string), ""COL_ORANGE"[ARREST] "COL_WHITE"%s (%d) was arrested by %s (%d) at WL%d.", PlayerInfo[targetid][pUserName], targetid, PlayerInfo[playerid][pUserName], playerid, wl);
	Cop_Broadcast(COLOR_WANTED_ORANGE, string);
	return 1;
}
alias:arrest("ar")

//==============================================================================
//	/ticket (/tk) - design §3.1 / §4.3
//==============================================================================

CMD:ticket(playerid, params[])
{
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can issue tickets.");
	if(Jail_IsJailed(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot do that right now.");
	if(!Cop_OnFoot(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You must be on foot to ticket a suspect.");

	new targetid = -1;
	if(!isNull(params))
	{
		if(sscanf(params, "d", targetid)) return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /ticket [playerid] (or blank for nearest)");
		if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || IsCop(targetid))
			return SendClientMessage(playerid, COLOR_ERROR, "That is not a valid suspect.");
		new wl = GetPlayerWanted(targetid);
		if(wl < WANTED_SUSPECT_MIN) return SendClientMessage(playerid, COLOR_ERROR, "You cannot ticket an innocent (WL0) player.");
		if(wl >= WANTED_WARRANT_MIN) return SendClientMessage(playerid, COLOR_ERROR, "That player is warranted (WL6+) - use /arrest, not /ticket.");
		new Float:cx, Float:cy, Float:cz;
		GetPlayerPos(playerid, cx, cy, cz);
		if(!IsPlayerInRangeOfPoint(targetid, TICKET_RANGE, cx, cy, cz))
			return SendClientMessage(playerid, COLOR_ERROR, "That suspect is too far away to ticket.");
	}
	else
	{
		targetid = Cop_NearestSuspect(playerid, TICKET_RANGE, WANTED_SUSPECT_MIN, WANTED_WARRANT_MIN - 1);
		if(targetid == -1) return SendClientMessage(playerid, COLOR_ERROR, "No suspect (WL1-5) within range to ticket.");
	}

	if(PlayerInfo[targetid][pTicketAmount] > 0)
		return SendClientMessage(playerid, COLOR_ERROR, "That suspect already has an outstanding ticket.");

	Dispatch_SetLastSuspect(playerid, targetid);	// Stage B (§3.3) - $sus source

	// Issue the ticket: fine + escalation clock (§2.4). Cop collects +$500 +1.
	PlayerInfo[targetid][pTicketAmount] = TICKET_FINE;
	PlayerInfo[targetid][pTicketByCop] = playerid;
	PlayerInfo[targetid][pTicketExpiry] = gettime() + TICKET_PAY_SECS;
	PlayerInfo[playerid][pScore] += TICKET_SCORE;
	PlayerInfo[playerid][pMoney] += TICKET_FINE;
	// M7 - ticket fines feed the GOVERNMENT stock pool (§10.2, factor 0.1).
	StockMarket_UpdateEarnings(E_STOCK_GOVERNMENT, TICKET_FINE, STOCK_FACTOR_GOV);
	UpdateMoney(playerid);
	if(PlayerInfo[playerid][pRefillPoints] < REFILL_POINTS_MAX) PlayerInfo[playerid][pRefillPoints]++;

	new string[176];
	format(string, sizeof(string), ""COL_GREEN"* Ticket: "COL_WHITE"you fined %s (%d) $%d. "COL_GREEN"+$%d, +%d score.", PlayerInfo[targetid][pUserName], targetid, TICKET_FINE, TICKET_FINE, TICKET_SCORE);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	format(string, sizeof(string), ""COL_YELLOW"* You were ticketed $%d by %s (%d). "COL_WHITE"Pay with "COL_YELLOW"/payticket"COL_WHITE" within %d seconds or a warrant will be issued.", TICKET_FINE, PlayerInfo[playerid][pUserName], playerid, TICKET_PAY_SECS);
	SendClientMessage(targetid, COLOR_WANTED_YELLOW, string);
	return 1;
}
alias:ticket("tk")

// /payticket - the ticketed suspect settles the fine (clears the escalation).
CMD:payticket(playerid, params[])
{
	#pragma unused params
	if(PlayerInfo[playerid][pTicketAmount] <= 0) return SendClientMessage(playerid, COLOR_ERROR, "You have no outstanding ticket.");
	if(PlayerInfo[playerid][pMoney] < PlayerInfo[playerid][pTicketAmount])
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot afford to pay your ticket.");
	new fine = PlayerInfo[playerid][pTicketAmount];
	PlayerInfo[playerid][pMoney] -= fine;
	UpdateMoney(playerid);
	PlayerInfo[playerid][pTicketAmount] = 0;
	PlayerInfo[playerid][pTicketByCop] = -1;
	PlayerInfo[playerid][pTicketExpiry] = 0;
	// Paying a ticket knocks a level off the suspect tier (rewards compliance).
	if(GetPlayerWanted(playerid) > 0) SetPlayerWanted(playerid, GetPlayerWanted(playerid) - 1, "");
	new string[96];
	format(string, sizeof(string), "You paid your $%d ticket. Stay out of trouble.", fine);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	return 1;
}
alias:payticket("pt")

//==============================================================================
//	/visualcontact (/vc) - pause a suspect's WL decay (design §2.4 / §3.1)
//==============================================================================

CMD:vc(playerid, params[])
{
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can report visual contact.");
	new targetid;
	if(!isNull(params))
	{
		if(sscanf(params, "d", targetid)) return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /vc [playerid] (or blank for nearest suspect)");
		if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || IsCop(targetid))
			return SendClientMessage(playerid, COLOR_ERROR, "That is not a valid suspect.");
		new Float:cx, Float:cy, Float:cz;
		GetPlayerPos(playerid, cx, cy, cz);
		if(!IsPlayerInRangeOfPoint(targetid, VC_RANGE, cx, cy, cz))
			return SendClientMessage(playerid, COLOR_ERROR, "That suspect is too far away for visual contact.");
	}
	else targetid = Cop_NearestSuspect(playerid, VC_RANGE, WANTED_SUSPECT_MIN, MAX_WANTED);
	if(targetid == -1 || targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid))
		return SendClientMessage(playerid, COLOR_ERROR, "No suspect found to report visual contact on.");
	if(GetPlayerWanted(targetid) < WANTED_SUSPECT_MIN)
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not a suspect.");
	if(!SetVisualContact(playerid, targetid))
		return SendClientMessage(playerid, COLOR_ERROR, "Could not lock visual contact on that player.");
	Dispatch_SetLastSuspect(playerid, targetid);	// Stage B (§3.3) - $sus source

	new string[144];
	format(string, sizeof(string), "You have visual contact on %s (%d) - their wanted decay is frozen.", PlayerInfo[targetid][pUserName], targetid);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	format(string, sizeof(string), ""COL_ORANGE"[PURSUIT] "COL_WHITE"%s (%d) has visual contact on %s (%d) (WL%d).", PlayerInfo[playerid][pUserName], playerid, PlayerInfo[targetid][pUserName], targetid, GetPlayerWanted(targetid));
	Cop_Broadcast(COLOR_WANTED_ORANGE, string);
	SendClientMessage(targetid, COLOR_ERROR, "A cop has visual contact on you - your wanted level will not decay while pursued.");
	return 1;
}
alias:vc("visualcontact", "vcontact")

//==============================================================================
//	/su <id> <crime> - mark a suspect + raise their WL (design §3.1)
//==============================================================================

CMD:su(playerid, params[])
{
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can mark suspects.");
	new targetid, crime[48];
	if(sscanf(params, "ds[48]", targetid, crime))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /su [playerid] [crime]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || IsCop(targetid))
		return SendClientMessage(playerid, COLOR_ERROR, "That is not a valid suspect.");
	if(targetid == playerid) return SendClientMessage(playerid, COLOR_ERROR, "You cannot mark yourself.");

	// Mark + pursue: raise WL for the observed crime and freeze their decay.
	new bool:added = (GivePlayerWanted(targetid, 2, crime) != 0);
	SetVisualContact(playerid, targetid);
	Dispatch_SetLastSuspect(playerid, targetid);	// Stage B (§3.3) - $sus source
	if(added) PlayerInfo[targetid][pSeriousCrimes]++;	// only when a wanted was actually added

	new string[160];
	format(string, sizeof(string), "You marked %s (%d) as a suspect for: %s (now WL%d).", PlayerInfo[targetid][pUserName], targetid, crime, GetPlayerWanted(targetid));
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	format(string, sizeof(string), ""COL_ORANGE"[SUSPECT] "COL_WHITE"%s (%d) marked %s (%d): %s.", PlayerInfo[playerid][pUserName], playerid, PlayerInfo[targetid][pUserName], targetid, crime);
	Cop_Broadcast(COLOR_WANTED_ORANGE, string);
	return 1;
}

//==============================================================================
//	/report (/rp) + /clr - cop crime report + undo (design §2.4)
//==============================================================================

static gLastReport[MAX_PLAYERS] = { -1, ... };	// last suspect a cop reported
static gLastReportAmt[MAX_PLAYERS];				// WL added by that report (for /clr)

// Reset the per-connection /report undo state (statics can't be cleared from
// ClearPlayerVariables). Called from OnPlayerDisconnect, mirroring Jail_OnDisconnect.
FUNCTION Cop_OnDisconnect(playerid)
{
	gLastReport[playerid] = -1;
	gLastReportAmt[playerid] = 0;
	return 1;
}

CMD:report(playerid, params[])
{
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can report suspects.");
	new targetid, reason[48];
	if(sscanf(params, "ds[48]", targetid, reason))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /report [playerid] [serious-crime reason]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || IsCop(targetid))
		return SendClientMessage(playerid, COLOR_ERROR, "That is not a valid suspect.");
	if(targetid == playerid) return SendClientMessage(playerid, COLOR_ERROR, "You cannot report yourself.");

	new before = GetPlayerWanted(targetid);
	GivePlayerWanted(targetid, 3, reason);	// serious report bump
	new gained = GetPlayerWanted(targetid) - before;
	if(gained <= 0)
		return SendClientMessage(playerid, COLOR_ERROR, "That suspect is protected right now (safe zone / spawn protection) - no report filed.");
	PlayerInfo[targetid][pSeriousCrimes]++;
	gLastReport[playerid] = targetid;
	gLastReportAmt[playerid] = gained;
	Dispatch_SetLastSuspect(playerid, targetid);	// Stage B (§3.3) - $sus source

	// Stage B (§3.3) - the reason may carry 10-codes / quick-strings (CB contract).
	new expanded[176];
	Cop_Expand10Code(expanded, reason, playerid);

	new string[224];
	format(string, sizeof(string), "You reported %s (%d) for: %s (WL %d -> %d).", PlayerInfo[targetid][pUserName], targetid, expanded, before, GetPlayerWanted(targetid));
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	format(string, sizeof(string), ""COL_ORANGE"[REPORT] "COL_WHITE"%s (%d) reported %s (%d): %s.", PlayerInfo[playerid][pUserName], playerid, PlayerInfo[targetid][pUserName], targetid, expanded);
	Cop_Broadcast(COLOR_WANTED_ORANGE, string);
	return 1;
}
alias:report("rp")

CMD:clr(playerid, params[])
{
	#pragma unused params
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can cancel reports.");
	new targetid = gLastReport[playerid];
	if(targetid == -1 || !IsPlayerConnected(targetid))
		return SendClientMessage(playerid, COLOR_ERROR, "You have no recent report to cancel.");
	// Restore the WL added by the last report and undo the serious flag.
	new restore = GetPlayerWanted(targetid) - gLastReportAmt[playerid];
	if(restore < 0) restore = 0;
	SetPlayerWanted(targetid, restore, "");
	if(PlayerInfo[targetid][pSeriousCrimes] > 0) PlayerInfo[targetid][pSeriousCrimes]--;
	gLastReport[playerid] = -1;
	gLastReportAmt[playerid] = 0;

	new string[128];
	format(string, sizeof(string), "You cancelled your last report on %s (%d). Their wanted level was restored.", PlayerInfo[targetid][pUserName], targetid);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	return 1;
}
alias:clr("cancellastreport", "cancelreport")

//==============================================================================
//	/backup (/bk) - purple marker + broadcast (design §3.1 / §4.1)
//==============================================================================

CMD:backup(playerid, params[])
{
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can request backup.");
	new Float:x, Float:y, Float:z, zoneName[MAX_ZONE_NAME];
	GetPlayerPos(playerid, x, y, z);
	format(zoneName, sizeof(zoneName), "%s", PlayerInfo[playerid][pZoneName]);

	// Expand any 10-codes + quick-strings in the optional message (§3.3).
	new note[176];
	if(!isNull(params)) Cop_Expand10Code(note, params, playerid);
	else note[0] = '\0';

	// Purple backup marker for every other cop (§2.1 backup colour).
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || !PlayerInfo[i][pLoggedIn]) continue;
		if(!IsCop(i) || i == playerid) continue;
		SetPlayerMarkerForPlayer(i, playerid, COLOR_BACKUP_PURPLE);
	}

	new string[224];
	format(string, sizeof(string), ""COL_PURPLE"[BACKUP] "COL_WHITE"%s (%d) is requesting backup near %s%s%s.", PlayerInfo[playerid][pUserName], playerid, isNull(zoneName) ? ("their location") : (zoneName), isNull(note) ? ("") : (" - "), isNull(note) ? ("") : (note));
	Cop_Broadcast(COLOR_BACKUP_PURPLE, string);
	SendClientMessage(playerid, COLOR_SERVER_INFO, "Backup requested - nearby cops see your purple marker.");

	// Stage B (§3.2) - fire a 10-78 (Backup needed) dispatch alert server-wide.
	new dtext[144];
	format(dtext, sizeof(dtext), "Backup needed - %s (%d)%s%s", PlayerInfo[playerid][pUserName], playerid, isNull(note) ? ("") : (" - "), isNull(note) ? ("") : (note));
	Dispatch_Alert(playerid, DISPATCH_HIGH, "10-78", dtext, DISPATCH_ALL);

	// Auto-clear the marker after the window so it doesn't linger.
	SetTimerEx("Cop_ClearBackupMarker", BACKUP_MARKER_SECS * 1000, false, "d", playerid);
	return 1;
}
alias:backup("bk")

FUNCTION Cop_ClearBackupMarker(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	// Re-apply the standard wanted/cop colour, replacing the purple marker.
	Wanted_ApplyColour(playerid);
	return 1;
}

//==============================================================================
//	Department radio (§3.1) - the cop chat channel + mute toggle
//==============================================================================

// Broadcast one department-channel line to cops who have the channel ON. Mirrors
// Cop_Broadcast but honours the pCopChatOn mute toggle (§3.1).
static Cop_DeptBroadcast(colour, const text[])
{
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || !PlayerInfo[i][pLoggedIn]) continue;
		if(!IsCop(i) || !PlayerInfo[i][pCopChatOn]) continue;
		SendClientMessage(i, colour, text);
	}
	return 1;
}

// Shared /d //copmsg body: expand 10-codes + quick-strings, then fan the
// [DEPT] {rank} Name: <text> line to the department channel (§3.1/§3.3).
static Cop_SendDept(playerid, const rawtext[])
{
	new expanded[176], string[224];
	Cop_Expand10Code(expanded, rawtext, playerid);
	format(string, sizeof(string), ""COL_LIGHTBLUE"[DEPT] "COL_WHITE"%s %s: %s", g_CopRankTitle[PlayerInfo[playerid][pCopRank]], PlayerInfo[playerid][pUserName], expanded);
	Cop_DeptBroadcast(COLOR_COP_BLIP, string);
	// A cop with the channel muted still sees their own outgoing line.
	if(!PlayerInfo[playerid][pCopChatOn]) SendClientMessage(playerid, COLOR_COP_BLIP, string);
	return 1;
}

CMD:d(playerid, params[])
{
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can use the department channel.");
	if(isNull(params)) return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /d [message]  (10-codes + $loc/$sus/$veh/$time supported)");
	return Cop_SendDept(playerid, params);
}
alias:d("dept", "copchat")

// /copmsg (/cm) routes to the same department channel (§3.1).
CMD:copmsg(playerid, params[])
{
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can use the cop channel.");
	if(isNull(params)) return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /copmsg [message]  (routes to /d)");
	return Cop_SendDept(playerid, params);
}
alias:copmsg("cm")

CMD:copchaton(playerid, params[])
{
	#pragma unused params
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can toggle the department channel.");
	PlayerInfo[playerid][pCopChatOn] = true;
	Jail_SavePlayer(playerid);	// persist the toggle (shared LSplayers UPDATE)
	SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_GREEN"* Department channel ON."COL_WHITE" You receive /d chatter and dispatch alerts.");
	return 1;
}

CMD:copchatoff(playerid, params[])
{
	#pragma unused params
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can toggle the department channel.");
	PlayerInfo[playerid][pCopChatOn] = false;
	Jail_SavePlayer(playerid);
	SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_ORANGE"* Department channel OFF."COL_WHITE" You still receive Most-Wanted (critical) alerts. Use /copchaton to re-enable.");
	return 1;
}

//==============================================================================
//	Suspect / warrant / most-wanted / jail lists (design §3.1 / §4.1)
//==============================================================================

// Shared list builder: rows for players whose WL is within [wlMin, wlMax].
static Cop_ShowWantedList(playerid, wlMin, wlMax, const title[])
{
	new list[1600], row[128], count = 0, zone[MAX_ZONE_NAME];
	strcat(list, "{FFFF00}Suspect\tWL\tZone\n");
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || !PlayerInfo[i][pLoggedIn]) continue;
		if(IsCop(i)) continue;
		new wl = GetPlayerWanted(i);
		if(wl < wlMin || wl > wlMax) continue;
		if(isNull(PlayerInfo[i][pZoneName])) format(zone, sizeof(zone), "unknown");
		else format(zone, sizeof(zone), "%s", PlayerInfo[i][pZoneName]);
		format(row, sizeof(row), "{FFFFFF}%s (%d)\t%d\t%s\n", PlayerInfo[i][pUserName], i, wl, zone);
		strcat(list, row);
		count++;
	}
	if(!count)
	{
		new msg[96];
		format(msg, sizeof(msg), "No players in that wanted range right now.");
		return SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	}
	ShowPlayerDialog(playerid, COP_INFO_DIALOG, DIALOG_STYLE_TABLIST_HEADERS, title, list, "Okay", "");
	return 1;
}

CMD:suspects(playerid)
{
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can view suspects.");
	return Cop_ShowWantedList(playerid, WANTED_SUSPECT_MIN, WANTED_WARRANT_MIN - 1, "Suspects (WL 1-5)");
}
alias:suspects("sus", "wanted", "wan")

CMD:warrants(playerid)
{
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can view warrants.");
	return Cop_ShowWantedList(playerid, WANTED_WARRANT_MIN, MAX_WANTED, "Warrants (WL 6+)");
}
alias:warrants("war", "warrant")

CMD:mostwanted(playerid)
{
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can view the most-wanted list.");
	return Cop_ShowWantedList(playerid, WANTED_MOSTWANTED_MIN, MAX_WANTED, "Most Wanted (WL 9-10)");
}
alias:mostwanted("mw", "most")

CMD:jaillist(playerid)
{
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can view the jail list.");
	new list[1600], row[128], count = 0;
	strcat(list, "{FFFF00}Inmate\tRemaining\tBail\n");
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || !Jail_IsJailed(i)) continue;
		new left = PlayerInfo[i][pJailUntil] - gettime();
		if(left < 0) left = 0;
		format(row, sizeof(row), "{FFFFFF}%s (%d)\t%d:%02d\t$%d\n", PlayerInfo[i][pUserName], i, left / 60, left % 60, PlayerInfo[i][pJailBail]);
		strcat(list, row);
		count++;
	}
	if(!count) return SendClientMessage(playerid, COLOR_SERVER_INFO, "No players are currently jailed.");
	ShowPlayerDialog(playerid, COP_INFO_DIALOG, DIALOG_STYLE_TABLIST_HEADERS, "Jailed Players", list, "Okay", "");
	return 1;
}
alias:jaillist("jl")

//==============================================================================
//	/parole (/pl) - free a served-minimum inmate (design §3.1 / §5)
//==============================================================================

CMD:parole(playerid, params[])
{
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can parole inmates.");
	new targetid;
	if(sscanf(params, "d", targetid))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /parole [playerid]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || !Jail_IsJailed(targetid))
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not in jail.");
	if((gettime() - PlayerInfo[targetid][pJailStart]) < JAIL_MIN_SERVE_SECS)
	{
		new err[112];
		format(err, sizeof(err), "That inmate must serve at least %d seconds before parole.", JAIL_MIN_SERVE_SECS);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	new string[128];
	format(string, sizeof(string), "%s (%d) paroled you - you are free to go.", PlayerInfo[playerid][pUserName], playerid);
	SendClientMessage(targetid, COLOR_SERVER_INFO, string);
	format(string, sizeof(string), "You paroled %s (%d).", PlayerInfo[targetid][pUserName], targetid);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	Jail_Release(targetid, "paroled", true);
	return 1;
}
alias:parole("pl")

//==============================================================================
//	/accept (/ac) + /refuse (/ref) - resolve an offered bribe (design §3.2 / §5)
//==============================================================================

CMD:accept(playerid, params[])
{
	#pragma unused params
	// M6 (stage 3) - /accept doubles as the duel-challenge acceptor (§6.4). If a
	// pending duel offer was resolved, stop here; else fall through to bribe accept.
	// arenas.inc is included after this module, so it is reached via CallLocalFunction.
	if(CallLocalFunction("Duel_TryAccept", "d", playerid)) return 1;
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can accept a bribe.");
	new briber = PlayerInfo[playerid][pBribeFrom];
	if(briber == -1 || !IsPlayerConnected(briber) || PlayerInfo[briber][pBribeTo] != playerid || !Jail_IsJailed(briber))
	{
		PlayerInfo[playerid][pBribeFrom] = -1;
		return SendClientMessage(playerid, COLOR_ERROR, "You have no pending bribe to accept.");
	}
	new amount = PlayerInfo[briber][pBribeAmount];
	if(PlayerInfo[briber][pMoney] < amount)
	{
		SendClientMessage(playerid, COLOR_ERROR, "The briber can no longer afford that amount.");
		PlayerInfo[briber][pBribeTo] = -1; PlayerInfo[briber][pBribeAmount] = 0; PlayerInfo[playerid][pBribeFrom] = -1;
		return 1;
	}
	// Money changes hands; sentence is cut and (if it lapses) they walk.
	PlayerInfo[briber][pMoney] -= amount;
	UpdateMoney(briber);
	PlayerInfo[playerid][pMoney] += amount;
	UpdateMoney(playerid);
	PlayerInfo[briber][pJailUntil] -= BRIBE_CUT_SECS;
	if(PlayerInfo[briber][pJailBail] > BRIBE_CUT_SECS * BAIL_PER_SEC) PlayerInfo[briber][pJailBail] -= BRIBE_CUT_SECS * BAIL_PER_SEC;
	else PlayerInfo[briber][pJailBail] = 0;

	new string[144];
	format(string, sizeof(string), "You accepted %s (%d)'s $%d bribe - their sentence was cut.", PlayerInfo[briber][pUserName], briber, amount);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	PlayerInfo[briber][pBribeTo] = -1; PlayerInfo[briber][pBribeAmount] = 0; PlayerInfo[playerid][pBribeFrom] = -1;

	if((PlayerInfo[briber][pJailUntil] - gettime()) <= 0)
	{
		SendClientMessage(briber, COLOR_SERVER_INFO, ""COL_GREEN"Your bribe was accepted - you walk free.");
		Jail_Release(briber, "bribe accepted", true);
	}
	else
	{
		new left = PlayerInfo[briber][pJailUntil] - gettime();
		format(string, sizeof(string), "Your bribe was accepted - %d:%02d shaved off your sentence.", BRIBE_CUT_SECS / 60, BRIBE_CUT_SECS % 60);
		SendClientMessage(briber, COLOR_SERVER_INFO, string);
		#pragma unused left
	}
	return 1;
}
alias:accept("ac")

CMD:refuse(playerid, params[])
{
	#pragma unused params
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can refuse a bribe.");
	new briber = PlayerInfo[playerid][pBribeFrom];
	if(briber == -1 || !IsPlayerConnected(briber) || PlayerInfo[briber][pBribeTo] != playerid || !Jail_IsJailed(briber))
	{
		PlayerInfo[playerid][pBribeFrom] = -1;
		return SendClientMessage(playerid, COLOR_ERROR, "You have no pending bribe to refuse.");
	}
	// Refusal: time + bail up, -1 score, charged with attempted bribery.
	PlayerInfo[briber][pJailUntil] += BRIBE_REFUSE_ADD_SECS;
	PlayerInfo[briber][pJailBail] += BRIBE_REFUSE_ADD_SECS * BAIL_PER_SEC;
	if(PlayerInfo[briber][pScore] > 0) PlayerInfo[briber][pScore]--;

	new string[144];
	format(string, sizeof(string), "You refused %s (%d)'s bribe - their sentence was extended.", PlayerInfo[briber][pUserName], briber);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	format(string, sizeof(string), ""COL_RED"Your bribe was REFUSED."COL_WHITE" +%d:%02d to your sentence, -1 score (attempted bribery).", BRIBE_REFUSE_ADD_SECS / 60, BRIBE_REFUSE_ADD_SECS % 60);
	SendClientMessage(briber, COLOR_ERROR, string);
	PlayerInfo[briber][pBribeTo] = -1; PlayerInfo[briber][pBribeAmount] = 0; PlayerInfo[playerid][pBribeFrom] = -1;
	return 1;
}
alias:refuse("ref")

//==============================================================================
//	/refill (/rf) - PD health/armour/weapons on points (design §3.1 / §4.3)
//==============================================================================

CMD:refill(playerid, params[])
{
	#pragma unused params
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can refill.");
	if(Jail_IsJailed(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot do that right now.");
	// Must be at a PD / police property (§3.1 - refill happens at the station).
	if(!PlayerInfo[playerid][pInPoliceProperty])
		return SendClientMessage(playerid, COLOR_ERROR, "You must be at a police station to refill.");
	if(gettime() < PlayerInfo[playerid][pRefillCooldown])
	{
		new err[96];
		format(err, sizeof(err), "Refill on cooldown - try again in %d seconds.", PlayerInfo[playerid][pRefillCooldown] - gettime());
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	if(PlayerInfo[playerid][pRefillPoints] <= 0)
		return SendClientMessage(playerid, COLOR_ERROR, "You have no Refill Points - earn them doing cop work (arrests/tickets).");

	new caption[256], item[80];
	format(item, sizeof(item), "Health (+full)\t%d pts\n", REFILL_HP_COST);
	strcat(caption, item);
	format(item, sizeof(item), "Armour (+full)\t%d pts\n", REFILL_ARMOUR_COST);
	strcat(caption, item);
	format(item, sizeof(item), "Weapon package (rank-gated)\t%d pts\n", REFILL_WEAPON_COST);
	strcat(caption, item);
	new title[64];
	format(title, sizeof(title), "Refill - %d point(s) available", PlayerInfo[playerid][pRefillPoints]);
	ShowPlayerDialog(playerid, REFILL_DIALOG, DIALOG_STYLE_TABLIST, title, caption, "Buy", "Cancel");
	return 1;
}
alias:refill("rf")

// Dialog resolution for /refill (routed from OnDialogResponse in CnR.pwn).
FUNCTION Refill_OnDialogResponse(playerid, response, listitem)
{
	if(!response) return 1;
	if(!IsCop(playerid) || !PlayerInfo[playerid][pInPoliceProperty]) return 1;
	new cost, applied[64];
	switch(listitem)
	{
		case 0:
		{
			cost = REFILL_HP_COST;
			if(PlayerInfo[playerid][pRefillPoints] < cost) return SendClientMessage(playerid, COLOR_ERROR, "Not enough Refill Points.");
			SetPlayerHealth(playerid, 100.0);
			format(applied, sizeof(applied), "Health topped up to 100.");
		}
		case 1:
		{
			cost = REFILL_ARMOUR_COST;
			if(PlayerInfo[playerid][pRefillPoints] < cost) return SendClientMessage(playerid, COLOR_ERROR, "Not enough Refill Points.");
			SetPlayerArmour(playerid, 100.0);
			format(applied, sizeof(applied), "Armour topped up to 100.");
		}
		case 2:
		{
			cost = REFILL_WEAPON_COST;
			if(PlayerInfo[playerid][pRefillPoints] < cost) return SendClientMessage(playerid, COLOR_ERROR, "Not enough Refill Points.");
			// Weapon tier gated by cop rank (§3.1): higher rank = stronger kit.
			GivePlayerWeapon(playerid, 24, 100);	// Deagle (all ranks)
			if(PlayerInfo[playerid][pCopRank] >= 3) GivePlayerWeapon(playerid, 25, 50);	// Shotgun
			if(PlayerInfo[playerid][pCopRank] >= 6) GivePlayerWeapon(playerid, 29, 200);	// MP5
			if(PlayerInfo[playerid][pCopRank] >= 9) GivePlayerWeapon(playerid, 31, 300);	// M4
			format(applied, sizeof(applied), "Weapon package issued (cop rank %d).", PlayerInfo[playerid][pCopRank]);
		}
		default: return 1;
	}
	PlayerInfo[playerid][pRefillPoints] -= cost;
	PlayerInfo[playerid][pRefillCooldown] = gettime() + REFILL_COOLDOWN_SECS;
	new string[128];
	format(string, sizeof(string), "%s (%d point(s) left)", applied, PlayerInfo[playerid][pRefillPoints]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	return 1;
}

//==============================================================================
//	/donut - +50% HP, carry up to 6 (design §3.1 / §4.1)
//==============================================================================

CMD:donut(playerid, params[])
{
	#pragma unused params
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can eat duty donuts.");
	if(Jail_IsJailed(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot do that right now.");
	if(PlayerInfo[playerid][pDonutCarry] <= 0)
		return SendClientMessage(playerid, COLOR_ERROR, "You have no donuts - pick some up at a donut shop.");
	new Float:hp;
	GetPlayerHealth(playerid, hp);
	hp += DONUT_HEAL_PCT;
	if(hp > 100.0) hp = 100.0;
	SetPlayerHealth(playerid, hp);
	PlayerInfo[playerid][pDonutCarry]--;
	new string[96];
	format(string, sizeof(string), "You ate a donut (+%.0f%% HP). %d left.", DONUT_HEAL_PCT, PlayerInfo[playerid][pDonutCarry]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	return 1;
}

//==============================================================================
//	Stage B (§3.4) - /respond (/yes) : respond to the last dispatch call
//==============================================================================

// Fires a 10-23 (responding) dispatch + sets a RED call checkpoint on the last
// dispatch origin (g_LastDispatchPos, set by Dispatch_Alert). Cop-only.
CMD:respond(playerid, params[])
{
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can respond to dispatch.");
	if(g_LastDispatchPos[0] == 0.0 && g_LastDispatchPos[1] == 0.0 && g_LastDispatchPos[2] == 0.0)
		return SendClientMessage(playerid, COLOR_ERROR, "There is no active dispatch call to respond to.");

	// Optional message on the response line (10-codes + quick-strings supported).
	new note[176];
	if(!isNull(params)) Cop_Expand10Code(note, params, playerid);
	else note[0] = '\0';

	// Red call checkpoint on the last dispatch origin so the responder can drive to it.
	SetPlayerCheckpoint(playerid, g_LastDispatchPos[0], g_LastDispatchPos[1], g_LastDispatchPos[2], RESPOND_CP_RADIUS);

	new dtext[224];
	format(dtext, sizeof(dtext), "%s (%d) responding%s%s", PlayerInfo[playerid][pUserName], playerid, isNull(note) ? ("") : (" - "), isNull(note) ? ("") : (note));
	// Location-less line (the responder's own position isn't the call site) - use -1
	// origin so the "- zone" tail reads "the city" and the checkpoint stays intact.
	Dispatch_Alert(-1, DISPATCH_NORMAL, "10-23", dtext, DISPATCH_ALL);
	SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_GREEN"* Responding. "COL_WHITE"A red call checkpoint marks the last dispatch location.");
	return 1;
}
alias:respond("yes")

//==============================================================================
//	Stage B (§3.4) - /robberies (/robs) : in-progress robberies/holdups
//==============================================================================

// Scan every player for an active holdup or safe-type robbery/carry and list the
// type + zone + suspect. Reads the real robbery/holdup state vars (pHoldingUp,
// pRobStage/pRobKind). Cop-only.
CMD:robberies(playerid, params[])
{
	#pragma unused params
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can view active robberies.");
	new list[1600], row[128], count = 0, zone[MAX_ZONE_NAME], what[40];
	strcat(list, "{FFFF00}Type\tSuspect\tZone\n");
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || !PlayerInfo[i][pLoggedIn]) continue;
		if(!PlayerInfo[i][pHoldingUp] && PlayerInfo[i][pRobStage] == ROBSTAGE_NONE) continue;

		// Classify the in-progress crime by its real state.
		if(PlayerInfo[i][pHoldingUp]) format(what, sizeof(what), "Store Holdup");
		else if(PlayerInfo[i][pRobStage] == ROBSTAGE_CARRYING) format(what, sizeof(what), "Carrying Safe");
		else
		{
			switch(PlayerInfo[i][pRobKind])
			{
				case ROBKIND_BANK:   format(what, sizeof(what), "Bank Robbery");
				case ROBKIND_CASINO: format(what, sizeof(what), "Casino Robbery");
				default:             format(what, sizeof(what), "Robbery");
			}
		}
		if(isNull(PlayerInfo[i][pZoneName])) format(zone, sizeof(zone), "unknown");
		else format(zone, sizeof(zone), "%s", PlayerInfo[i][pZoneName]);
		format(row, sizeof(row), "{FFFFFF}%s\t%s (%d)\t%s\n", what, PlayerInfo[i][pUserName], i, zone);
		strcat(list, row);
		count++;
	}
	if(!count) return SendClientMessage(playerid, COLOR_SERVER_INFO, "No robberies or holdups are in progress right now.");
	ShowPlayerDialog(playerid, COP_INFO_DIALOG, DIALOG_STYLE_TABLIST_HEADERS, "Robberies In Progress", list, "Okay", "");
	return 1;
}
alias:robberies("robs")

//==============================================================================
//	Stage B (§3.4) - /freeze (/fr /pullover /pu) : order a WANTED suspect to stop
//==============================================================================

// Order the nearest WANTED (WL1+) suspect within range to comply: a brief
// FREEZE_SECS control lock (TogglePlayerControllable), never on an innocent
// (anti-abuse), both parties messaged. The lift is scheduled by a one-shot timer.
CMD:freeze(playerid, params[])
{
	#pragma unused params
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can order a suspect to stop.");
	if(Jail_IsJailed(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot do that right now.");

	// Nearest suspect WL1+ within FREEZE_RANGE (Cop_NearestSuspect already skips
	// cops/DM-zone/jailed/protected targets). Wanted-only is enforced by the wlMin.
	new targetid = Cop_NearestSuspect(playerid, FREEZE_RANGE, WANTED_SUSPECT_MIN, MAX_WANTED);
	if(targetid == -1) return SendClientMessage(playerid, COLOR_ERROR, "No wanted suspect within range to order to stop.");

	// Brief comply-stop. TogglePlayerControllable(false) freezes; the one-shot timer
	// releases them after FREEZE_SECS so it is a "pull over" beat, not a lockdown.
	TogglePlayerControllable(targetid, false);
	SetTimerEx("Cop_UnfreezeSuspect", FREEZE_SECS * 1000, false, "d", targetid);

	new string[144];
	format(string, sizeof(string), ""COL_ORANGE"* You ordered %s (%d) to STOP. "COL_WHITE"They are frozen for %d seconds - close in and /arrest or /ticket.", PlayerInfo[targetid][pUserName], targetid, FREEZE_SECS);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	format(string, sizeof(string), ""COL_RED"* FREEZE! "COL_WHITE"%s (%d) ordered you to stop - you are held for %d seconds. Comply or flee once released.", PlayerInfo[playerid][pUserName], playerid, FREEZE_SECS);
	SendClientMessage(targetid, COLOR_ERROR, string);
	Dispatch_SetLastSuspect(playerid, targetid);	// $sus source
	return 1;
}
alias:freeze("fr", "pullover", "pu")

// One-shot unfreeze (from the /freeze timer). Guards a disconnect/jail race so it
// never re-enables control on a jailed suspect (jail freezes them separately).
FUNCTION Cop_UnfreezeSuspect(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	if(Jail_IsJailed(playerid)) return 0;	// jail owns the freeze if they were arrested meanwhile
	TogglePlayerControllable(playerid, true);
	return 1;
}

//==============================================================================
//	Stage B (§3.4) - /crimes (/crime) : a suspect's last 5 crimes
//==============================================================================

// Show a target's last 5 crimes from their pCrimeLog ring buffer (written by
// GivePlayerWanted). Cop-only. Blank target = nearest suspect, else an explicit id.
CMD:crimes(playerid, params[])
{
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can pull a rap sheet.");
	new targetid;
	if(!isNull(params))
	{
		if(sscanf(params, "d", targetid)) return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /crimes [playerid] (or blank for nearest suspect)");
		if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || IsPlayerNPC(targetid))
			return SendClientMessage(playerid, COLOR_ERROR, "That is not a valid player.");
	}
	else
	{
		targetid = Cop_NearestSuspect(playerid, VC_RANGE, WANTED_SUSPECT_MIN, MAX_WANTED);
		if(targetid == -1) return SendClientMessage(playerid, COLOR_ERROR, "No suspect nearby - specify an id: /crimes [playerid].");
	}

	if(PlayerInfo[targetid][pCrimeLogCount] <= 0)
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "That player has no recorded crimes this session.");

	new list[512], row[128], title[64];
	format(title, sizeof(title), "Last crimes - %s (%d)", PlayerInfo[targetid][pUserName], targetid);
	strcat(list, "{FFFF00}When\tCrime\n");
	// Walk the ring newest-first: head points at the NEXT write slot, so the most
	// recent entry is at head-1. Show up to 5.
	new shown = (PlayerInfo[targetid][pCrimeLogCount] < 5) ? PlayerInfo[targetid][pCrimeLogCount] : 5;
	for(new k = 0; k < shown; k++)
	{
		new idx = (PlayerInfo[targetid][pCrimeLogHead] - 1 - k + 5) % 5;
		new ago = gettime() - PlayerInfo[targetid][pCrimeLog][idx];
		if(ago < 0) ago = 0;
		format(row, sizeof(row), "{FFFFFF}%d:%02d ago\t%s\n", ago / 60, ago % 60, g_CrimeLogWhat[targetid][idx]);
		strcat(list, row);
	}
	ShowPlayerDialog(playerid, COP_INFO_DIALOG, DIALOG_STYLE_TABLIST_HEADERS, title, list, "Okay", "");
	return 1;
}
alias:crimes("crime")

//==============================================================================
//	Stage B (§3.4) - /vehrepair (/vehfix) : repair the cop's own PD vehicle
//==============================================================================

// A cop on foot next to their own last-driven vehicle repairs it for
// COP_REPAIR_COST (Bank_TakeCash + RepairVehicle). Cop-only.
CMD:vehrepair(playerid, params[])
{
	#pragma unused params
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can buy a vehicle repair kit.");
	if(Jail_IsJailed(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot do that right now.");
	if(IsPlayerInAnyVehicle(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Get out of the vehicle first, then stand next to it to repair it.");

	new vid = PlayerInfo[playerid][pLastVehicle];
	if(vid == INVALID_VEHICLE_ID || !IsValidVehicle(vid))
		return SendClientMessage(playerid, COLOR_ERROR, "You have no recent vehicle to repair. Drive one first.");

	new Float:vx, Float:vy, Float:vz;
	GetVehiclePos(vid, vx, vy, vz);
	if(!IsPlayerInRangeOfPoint(playerid, VEHREPAIR_RANGE, vx, vy, vz))
		return SendClientMessage(playerid, COLOR_ERROR, "You must be standing next to your vehicle to repair it.");

	if(PlayerInfo[playerid][pMoney] < COP_REPAIR_COST)
	{
		new err[112];
		format(err, sizeof(err), "A repair kit costs "COL_GREEN"$%d"COL_WHITE" cash - you cannot afford it.", COP_REPAIR_COST);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	if(!Bank_TakeCash(playerid, COP_REPAIR_COST)) return SendClientMessage(playerid, COLOR_ERROR, "Payment failed.");

	RepairVehicle(vid);
	new string[112];
	format(string, sizeof(string), ""COL_GREEN"* Vehicle repaired "COL_WHITE"($%d)"COL_GREEN"."COL_WHITE" Full health + fixed panels.", COP_REPAIR_COST);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	return 1;
}
alias:vehrepair("vehfix")

//==============================================================================
//	Stage B (§3.5) - /coprank : own rank, arrests-to-next, current /refill tier
//==============================================================================

// Read-only: show pCopRank (0-10 Recruit->Commissioner), the next weapon-tier
// unlock, and the current /refill weapon kit. No state change.
CMD:coprank(playerid, params[])
{
	#pragma unused params
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement have a cop rank.");
	new rank = PlayerInfo[playerid][pCopRank], string[176];
	format(string, sizeof(string), ""COL_LIGHTBLUE"* Cop Rank: "COL_WHITE"%d - %s"COL_LIGHTBLUE".", rank, g_CopRankTitle[rank]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	format(string, sizeof(string), "Current /refill weapon kit: "COL_YELLOW"%s"COL_WHITE".", Cop_RefillTierLabel(rank));
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);

	// Next weapon-tier unlock (ranks 3/6/9). Each arrest = +1 rank (max 10).
	new nextTier = -1;
	if(rank < 3) nextTier = 3;
	else if(rank < 6) nextTier = 6;
	else if(rank < 9) nextTier = 9;
	if(nextTier != -1)
	{
		format(string, sizeof(string), "Next weapon-tier unlock at rank "COL_YELLOW"%d (%s)"COL_WHITE" - "COL_YELLOW"%d"COL_WHITE" more arrest(s).", nextTier, g_CopRankTitle[nextTier], nextTier - rank);
		SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	}
	else SendClientMessage(playerid, COLOR_SERVER_INFO, "You have unlocked every /refill weapon tier.");
	return 1;
}

//==============================================================================
//	Ticket -> warrant escalation (design §2.4) - hooked from GameModeClock()
//==============================================================================

// A ticketed suspect who doesn't /payticket in TICKET_PAY_SECS jumps to WL6.
// Runs on the 1-second clock (no new global timer), alongside the wanted decay.
FUNCTION Cop_OnGameModeClockTick()
{
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || !PlayerInfo[i][pLoggedIn]) continue;
		if(PlayerInfo[i][pTicketAmount] <= 0 || PlayerInfo[i][pTicketExpiry] == 0) continue;
		if(gettime() < PlayerInfo[i][pTicketExpiry]) continue;

		// Unpaid ticket lapsed -> warrant.
		PlayerInfo[i][pTicketAmount] = 0;
		PlayerInfo[i][pTicketByCop] = -1;
		PlayerInfo[i][pTicketExpiry] = 0;
		if(GetPlayerWanted(i) < TICKET_ESCALATE_WL) SetPlayerWanted(i, TICKET_ESCALATE_WL, "Unpaid ticket -> warrant");
		SendClientMessage(i, COLOR_ERROR, ""COL_RED"* Your unpaid ticket escalated to a WARRANT."COL_WHITE" Cops can now arrest you.");
	}
	return 1;
}
