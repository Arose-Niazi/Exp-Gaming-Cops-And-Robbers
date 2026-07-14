/*

	Kidnap crime for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 5 (stage 4) - design §4.4 (kidnap/holdup spec) + docs/research/
	{crazybobs-site-full.md §Kidnapper, primary-sources.md §Kidnapper} (VERIFIED:
	ransom ≤$50,000/victim; at the hideout steal 20-80% of victim on-hand cash;
	small bonus on grab; kidnapping ALWAYS gives a warrant - +1 per civilian,
	+6 per cop kidnapped; kidnappers disguise as Drivers via /fakeskill).

	The kidnap crime: /kidnap (/kd) a nearby non-cop. You force them into your
	vehicle (if you are driving) or hold them on foot; a hideout checkpoint appears
	for YOU. Drive/lead them to the hideout to steal 20-80% of their on-hand cash
	(plus the ransom is the cap on what they can pay out to buy freedom early). The
	grab always raises your wanted (a warrant) and pays a small bonus. The victim
	is freed by:
	  - a cop /uncuff-ing them within reach (frees + no cash lost),
	  - paying the ransom with /payransom (frees, ransom moves victim->kidnapper),
	  - escaping (straying KIDNAP_ESCAPE_DIST from the kidnapper),
	  - the kidnapper's death / arrest / disconnect (all release the victim),
	  - the victim's own death / disconnect (§4.4: a victim who quits still loses
	    the ransom to the kidnapper - the crime already "paid").

	This is a session crime (no DB): pKidnapVictim / pKidnappedBy in pInfo, reset
	on death / disconnect / arrest. It respects the shared crime gate (not a cop,
	not jailed, not spawn-protected, not in a DM zone) exactly like /rob and /rape.

	>>> M6 EXTENSION POINT - the Kidnapper SKILL (§4.4) scales the ransom and lets
	    the kidnapper appear as a Driver (/fakeskill). Skills are set at City Hall
	    in M6; until pSkill can be SKILL_KIDNAPPER, the BASE ransom applies and
	    /fakeskill is a cosmetic session flag. grep "M6" for the hook. <<<

	Credits:
		crazybobs-site-full.md §Kidnapper + primary-sources.md for the numbers.

	Public API:
		Kidnap_IsVictim(playerid)              : bool - is this player currently kidnapped
		Kidnap_FreeVictim(victimid, const why[], bool:tellKidnapper) : release a held victim
		Kidnap_OnPlayerDeath(playerid, killerid): death hook - release on either side (§4.4)
		Kidnap_OnDisconnect(playerid)          : disconnect hook - release + victim-quit payout (§4.4)
		Kidnap_OnArrest(playerid)              : arrest hook - a jailed kidnapper releases the victim
		Kidnap_OnTick()                        : GameModeClock 1s tick - hideout delivery + escape guard

	Commands:
		/kidnap (/kd) [id] [ransom]  (civ, on-foot) grab a nearby non-cop
		/release (/rel)              (kidnapper)     let your victim go (no payout)
		/ransom [amount]             (kidnapper)     re-set the ransom on your held victim
		/payransom (/pr)             (victim)        pay the ransom to free yourself early
		/uncuff [id]                 (cop, on-foot)  free a kidnapped victim near you
		/fakeskill                   (civ)           disguise flag - appear as a Driver (§4.4, M6 skill cosmetic)

	Functions Being Called:
		misc:    IsCop, IsCivilian, UpdateMoney
		wanted:  GivePlayerWanted, IsPlayerSpawnProtected
		jail:    Jail_IsJailed
		bank:    (cash moves use pMoney + UpdateMoney directly - hand-cash, mirrors /rob)
*/

//==============================================================================
//	Kidnap hideout (design §4.4 - "drive the victim to a hideout")
//==============================================================================

// The LS kidnap hideout the kidnapper delivers the victim to for the cash steal.
// A quiet spot in East LS (matches the robbery hideout style - reachable by road).
#define KIDNAP_HIDEOUT_X		(2585.0)
#define KIDNAP_HIDEOUT_Y		(-1310.0)
#define KIDNAP_HIDEOUT_Z		(6.0)

//==============================================================================
//	Internal helpers
//==============================================================================

// Is this player currently being held kidnapped by someone?
FUNCTION Kidnap_IsVictim(playerid)
	return (PlayerInfo[playerid][pKidnappedBy] != -1);

// M6 (stage 1) - the Kidnapper SKILL ransom-scaling hook (§4.4). A player who chose
// SKILL_KIDNAPPER at City Hall can demand a higher ransom ceiling; everyone else is
// capped at the base KIDNAP_RANSOM_MAX. pSkill is set by systems/skills.inc (/skill).
static Kidnap_RansomMaxFor(playerid)
	return (PlayerInfo[playerid][pSkill] == SKILL_KIDNAPPER)
		? (KIDNAP_RANSOM_MAX * KIDNAP_SKILL_RANSOM_MULT)
		: KIDNAP_RANSOM_MAX;

// A valid non-cop kidnap victim: connected, logged-in, human, not the actor, not
// a cop, not jailed, not already kidnapped, not spawn-protected / DM-zoned.
static bool:Kidnap_ValidVictim(playerid, targetid)
{
	if(targetid == INVALID_PLAYER_ID || targetid == playerid) return false;
	if(!IsPlayerConnected(targetid) || IsPlayerNPC(targetid) || !PlayerInfo[targetid][pLoggedIn]) return false;
	if(IsCop(targetid)) return false;
	if(Jail_IsJailed(targetid)) return false;
	if(Kidnap_IsVictim(targetid)) return false;
	if(IsPlayerSpawnProtected(targetid) || PlayerInfo[targetid][pInDMZone]) return false;
	return true;
}

// Nearest valid non-cop kidnap victim to `playerid` within `range`. Blank-target
// convenience for /kidnap. Returns -1 if none.
static Kidnap_NearestVictim(playerid, Float:range)
{
	new Float:x, Float:y, Float:z, best = -1, Float:bestDist = range + 1.0;
	GetPlayerPos(playerid, x, y, z);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!Kidnap_ValidVictim(playerid, i)) continue;
		if(!IsPlayerInRangeOfPoint(i, range, x, y, z)) continue;
		new Float:d = GetPlayerDistanceFromPoint(i, x, y, z);
		if(d < bestDist) { bestDist = d; best = i; }
	}
	return best;
}

// Free a held victim, clearing BOTH sides of the link + the victim's hideout CP.
// `why` is shown to the victim; if `tellKidnapper` the kidnapper is told too.
FUNCTION Kidnap_FreeVictim(victimid, const why[], tellKidnapper)
{
	new kidid = PlayerInfo[victimid][pKidnappedBy];
	PlayerInfo[victimid][pKidnappedBy] = -1;
	if(IsPlayerConnected(victimid))
	{
		DisablePlayerCheckpoint(victimid);
		if(!isNull(why))
		{
			new msg[144];
			format(msg, sizeof(msg), ""COL_GREEN"* You are free! "COL_WHITE"%s", why);
			SendClientMessage(victimid, COLOR_SERVER_INFO, msg);
		}
	}
	if(kidid != -1 && PlayerInfo[kidid][pKidnapVictim] == victimid)
	{
		PlayerInfo[kidid][pKidnapVictim] = -1;
		PlayerInfo[kidid][pKidnapRansom] = 0;
		if(tellKidnapper && IsPlayerConnected(kidid))
			SendClientMessage(kidid, COLOR_ERROR, ""COL_RED"* Your victim got away."COL_WHITE);
	}
	return 1;
}

// Alert cops that a kidnapping is under way (mirrors the holdup/robbery announce).
static Kidnap_AlertCops(playerid, victimid)
{
	new string[176], zoneName[MAX_ZONE_NAME];
	if(isNull(PlayerInfo[playerid][pZoneName])) format(zoneName, sizeof(zoneName), "the city");
	else format(zoneName, sizeof(zoneName), "%s", PlayerInfo[playerid][pZoneName]);
	format(string, sizeof(string), ""COL_ORANGE"[10-47 KIDNAP] "COL_WHITE"%s (%d) has kidnapped %s (%d) near %s! Rescue them (/uncuff).",
		PlayerInfo[playerid][pUserName], playerid, PlayerInfo[victimid][pUserName], victimid, zoneName);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || i == playerid) continue;
		if(IsCop(i)) SendClientMessage(i, COLOR_WANTED_ORANGE, string);
	}
}

// The cash steal at the hideout (§4.4): take 20-80% of the victim's on-hand cash,
// clamped so the take never exceeds the ransom cap on it (the ransom is the
// bargaining amount) is NOT applied here - the hideout steal is the full 20-80%
// slice; /payransom is the alternate, capped, buy-your-freedom path. Money moves
// victim -> kidnapper (hand cash, mirrors /rob) - no minting.
static Kidnap_StealAtHideout(kidid, victimid)
{
	new pct = KIDNAP_STEAL_MIN_PCT + random(KIDNAP_STEAL_MAX_PCT - KIDNAP_STEAL_MIN_PCT + 1);
	new amount = (PlayerInfo[victimid][pMoney] * pct) / 100;
	if(amount < 0) amount = 0;
	if(amount > PlayerInfo[victimid][pMoney]) amount = PlayerInfo[victimid][pMoney];

	if(amount > 0)
	{
		PlayerInfo[victimid][pMoney] -= amount;
		UpdateMoney(victimid);
		PlayerInfo[kidid][pMoney] += amount;
		UpdateMoney(kidid);
	}
	PlayerInfo[kidid][pScore] += KIDNAP_SCORE;

	new msg[176];
	format(msg, sizeof(msg), ""COL_GREEN"* Ransom collected! "COL_WHITE"You stripped %s (%d) of "COL_GREEN"$%d"COL_WHITE" (%d%%) at the hideout. +%d score.",
		PlayerInfo[victimid][pUserName], victimid, amount, pct, KIDNAP_SCORE);
	SendClientMessage(kidid, COLOR_SERVER_INFO, msg);
	format(msg, sizeof(msg), ""COL_RED"* You were robbed at the hideout! "COL_WHITE"%s (%d) took "COL_RED"$%d"COL_WHITE" of your cash.",
		PlayerInfo[kidid][pUserName], kidid, amount);
	SendClientMessage(victimid, COLOR_ERROR, msg);

	// The steal ends the kidnap - the victim is dumped at the hideout.
	Kidnap_FreeVictim(victimid, "The kidnapper dumped you at the hideout.", 0);
	return amount;
}

//==============================================================================
//	GameModeClock 1s tick - hideout delivery + escape guard (NO new timer, §4.4)
//==============================================================================

FUNCTION Kidnap_OnTick()
{
	for(new kidid = 0; kidid < MAX_PLAYERS; kidid++)
	{
		if(!IsPlayerConnected(kidid) || IsPlayerNPC(kidid)) continue;
		new victimid = PlayerInfo[kidid][pKidnapVictim];
		if(victimid == -1) continue;

		// Defensive: a broken link (victim gone / no longer points back) -> clear.
		if(!IsPlayerConnected(victimid) || PlayerInfo[victimid][pKidnappedBy] != kidid)
		{
			PlayerInfo[kidid][pKidnapVictim] = -1;
			PlayerInfo[kidid][pKidnapRansom] = 0;
			continue;
		}

		// A jailed kidnapper can no longer hold anyone (belt-and-braces to the arrest hook).
		if(Jail_IsJailed(kidid)) { Kidnap_FreeVictim(victimid, "Your kidnapper was arrested.", 1); continue; }

		// Escape: the victim strayed too far from the kidnapper (§4.4). Both must be
		// in the same interior/VW for the distance to count - a warp apart frees them.
		if(GetPlayerInterior(kidid) != GetPlayerInterior(victimid) ||
			GetPlayerVirtualWorld(kidid) != GetPlayerVirtualWorld(victimid))
		{
			Kidnap_FreeVictim(victimid, "You slipped away from your kidnapper.", 1);
			continue;
		}
		new Float:vx, Float:vy, Float:vz;
		GetPlayerPos(kidid, vx, vy, vz);
		if(!IsPlayerInRangeOfPoint(victimid, KIDNAP_ESCAPE_DIST, vx, vy, vz))
		{
			Kidnap_FreeVictim(victimid, "You escaped your kidnapper - you got too far away!", 1);
			continue;
		}

		// Reached the hideout WITH the victim in tow -> steal 20-80% on-hand (§4.4).
		if(IsPlayerInRangeOfPoint(kidid, KIDNAP_HIDEOUT_RANGE, KIDNAP_HIDEOUT_X, KIDNAP_HIDEOUT_Y, KIDNAP_HIDEOUT_Z) &&
			IsPlayerInRangeOfPoint(victimid, KIDNAP_HIDEOUT_RANGE + 8.0, KIDNAP_HIDEOUT_X, KIDNAP_HIDEOUT_Y, KIDNAP_HIDEOUT_Z))
		{
			Kidnap_StealAtHideout(kidid, victimid);
			continue;
		}
	}
	return 1;
}

//==============================================================================
//	Death / disconnect / arrest hooks
//==============================================================================

// Death on EITHER side releases the link. If the KIDNAPPER dies, the victim goes
// free (nothing lost). If the VICTIM dies in captivity, the kidnapper keeps the
// hold cleared but earned the grab bonus already (§4.4 "small bonus if the victim
// dies in the hideout after the timer") - no extra mint here, kept simple.
FUNCTION Kidnap_OnPlayerDeath(playerid, killerid)
{
	#pragma unused killerid
	// This player was a kidnapper -> free their victim.
	if(PlayerInfo[playerid][pKidnapVictim] != -1)
		Kidnap_FreeVictim(PlayerInfo[playerid][pKidnapVictim], "Your kidnapper is dead - you are free.", 0);
	// This player was a victim -> break the link (their kidnapper loses the hold).
	if(PlayerInfo[playerid][pKidnappedBy] != -1)
	{
		new kidid = PlayerInfo[playerid][pKidnappedBy];
		PlayerInfo[playerid][pKidnappedBy] = -1;
		if(IsPlayerConnected(playerid)) DisablePlayerCheckpoint(playerid);
		if(kidid != -1 && PlayerInfo[kidid][pKidnapVictim] == playerid)
		{
			PlayerInfo[kidid][pKidnapVictim] = -1;
			PlayerInfo[kidid][pKidnapRansom] = 0;
			if(IsPlayerConnected(kidid)) SendClientMessage(kidid, COLOR_ERROR, ""COL_RED"* Your victim died in captivity."COL_WHITE);
		}
	}
	return 1;
}

// Disconnect. §4.4: a VICTIM who quits while kidnapped still loses the ransom to
// the kidnapper (the crime already paid) - we hand the kidnapper the set ransom
// (capped to what the victim can back with cash+bank is not tracked at quit; we
// pay the ransom outright as the "still paid" bonus, a small deliberate subsidy).
FUNCTION Kidnap_OnDisconnect(playerid)
{
	// Kidnapper leaving -> free their victim cleanly.
	if(PlayerInfo[playerid][pKidnapVictim] != -1)
	{
		new victimid = PlayerInfo[playerid][pKidnapVictim];
		PlayerInfo[playerid][pKidnapVictim] = -1;
		PlayerInfo[playerid][pKidnapRansom] = 0;
		if(IsPlayerConnected(victimid) && PlayerInfo[victimid][pKidnappedBy] == playerid)
		{
			PlayerInfo[victimid][pKidnappedBy] = -1;
			DisablePlayerCheckpoint(victimid);
			SendClientMessage(victimid, COLOR_SERVER_INFO, ""COL_GREEN"* Your kidnapper disconnected - you are free."COL_WHITE);
		}
	}
	// Victim leaving -> the kidnapper is still paid the ransom (§4.4).
	if(PlayerInfo[playerid][pKidnappedBy] != -1)
	{
		new kidid = PlayerInfo[playerid][pKidnappedBy];
		new ransom = PlayerInfo[playerid][pKidnapRansom];
		PlayerInfo[playerid][pKidnappedBy] = -1;
		if(kidid != -1 && IsPlayerConnected(kidid) && PlayerInfo[kidid][pKidnapVictim] == playerid)
		{
			PlayerInfo[kidid][pKidnapVictim] = -1;
			PlayerInfo[kidid][pKidnapRansom] = 0;
			if(ransom > 0)
			{
				Bank_GiveCash(kidid, ransom);
				new msg[144];
				format(msg, sizeof(msg), ""COL_GREEN"* Your victim fled the server "COL_WHITE"- the ransom "COL_GREEN"$%d"COL_WHITE" was paid anyway.", ransom);
				SendClientMessage(kidid, COLOR_SERVER_INFO, msg);
			}
		}
	}
	return 1;
}

// A jailed kidnapper (arrest hook, from Jail_SendToJail) releases their victim.
FUNCTION Kidnap_OnArrest(playerid)
{
	if(PlayerInfo[playerid][pKidnapVictim] != -1)
		Kidnap_FreeVictim(PlayerInfo[playerid][pKidnapVictim], "Your kidnapper was arrested.", 1);
	// If the arrested player was themselves a victim, break that too (unlikely - a
	// kidnapped player is not arrestable mid-hold, but keep the state clean).
	if(PlayerInfo[playerid][pKidnappedBy] != -1)
	{
		new kidid = PlayerInfo[playerid][pKidnappedBy];
		PlayerInfo[playerid][pKidnappedBy] = -1;
		if(IsPlayerConnected(playerid)) DisablePlayerCheckpoint(playerid);
		if(kidid != -1 && PlayerInfo[kidid][pKidnapVictim] == playerid)
		{
			PlayerInfo[kidid][pKidnapVictim] = -1;
			PlayerInfo[kidid][pKidnapRansom] = 0;
		}
	}
	return 1;
}

//==============================================================================
//	Command - /kidnap (/kd) [id] [ransom] (design §4.4)
//==============================================================================

CMD:kidnap(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");

	// Shared crime gate (mirrors /rob //rape //holdup): cop/jailed/spawn-protect/DM-zone.
	if(IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Law enforcement cannot kidnap people.");
	if(Jail_IsJailed(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot do that while you are in jail.");
	if(IsPlayerSpawnProtected(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot commit crimes while spawn-protected. Wait a moment.");
	if(PlayerInfo[playerid][pInDMZone]) return SendClientMessage(playerid, COLOR_ERROR, "You cannot commit crimes inside a DM zone.");

	if(PlayerInfo[playerid][pKidnapVictim] != -1)
		return SendClientMessage(playerid, COLOR_ERROR, "You are already holding a victim. /release them or drive them to the hideout first.");
	if(Kidnap_IsVictim(playerid))
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot kidnap someone while you are being kidnapped yourself.");

	if(gettime() < PlayerInfo[playerid][pKidnapCooldown])
	{
		new err[112];
		format(err, sizeof(err), "You cannot kidnap again yet - wait %d seconds.", PlayerInfo[playerid][pKidnapCooldown] - gettime());
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}

	// Parse the optional target + optional ransom: sscanf "u D(default)" - the target
	// is optional (blank -> nearest), the ransom is optional (defaults if omitted).
	new targetid = INVALID_PLAYER_ID, ransom = KIDNAP_RANSOM_DEFAULT;
	if(!isNull(params))
	{
		if(sscanf(params, "uD(25000)", targetid, ransom))	// D default = KIDNAP_RANSOM_DEFAULT
			return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /kidnap [playerid] [ransom] (blank picks the nearest person)");
	}
	if(targetid == INVALID_PLAYER_ID) targetid = -1;
	if(targetid == -1) targetid = Kidnap_NearestVictim(playerid, KIDNAP_RANGE);
	if(targetid == -1) return SendClientMessage(playerid, COLOR_ERROR, "There is nobody close enough to kidnap.");
	if(!Kidnap_ValidVictim(playerid, targetid))
		return SendClientMessage(playerid, COLOR_ERROR, "That is not a valid person to kidnap.");

	// Proximity check for an explicit target too.
	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	if(!IsPlayerInRangeOfPoint(targetid, KIDNAP_RANGE, x, y, z))
		return SendClientMessage(playerid, COLOR_ERROR, "That person is too far away to grab.");

	// Both must be in the same interior/VW to start a hold.
	if(GetPlayerInterior(playerid) != GetPlayerInterior(targetid) || GetPlayerVirtualWorld(playerid) != GetPlayerVirtualWorld(targetid))
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot reach that person.");

	// Clamp the ransom to the skill-scaled cap (§4.4). Base players cap at $50,000;
	// a Kidnapper (SKILL_KIDNAPPER, set via /skill in skills.inc) can demand more -
	// this is the M6 "ransom scaling" hook now live.
	new ransomMax = Kidnap_RansomMaxFor(playerid);
	if(ransom < 0) ransom = 0;
	if(ransom > ransomMax) ransom = ransomMax;

	// Commit the grab.
	PlayerInfo[playerid][pKidnapCooldown] = gettime() + KIDNAP_CD;
	PlayerInfo[playerid][pKidnapVictim] = targetid;
	PlayerInfo[playerid][pKidnapRansom] = ransom;
	PlayerInfo[targetid][pKidnappedBy] = playerid;

	// Force the victim into your vehicle if you are driving (a free passenger seat),
	// otherwise they are held on foot (must stay within KIDNAP_ESCAPE_DIST of you).
	if(IsPlayerInAnyVehicle(playerid) && GetPlayerState(playerid) == PLAYER_STATE_DRIVER)
	{
		new veh = GetPlayerVehicleID(playerid);
		PutPlayerInVehicle(targetid, veh, 1);	// passenger seat 1
	}

	// Point the KIDNAPPER at the hideout with a checkpoint (drive the victim there).
	SetPlayerCheckpoint(playerid, KIDNAP_HIDEOUT_X, KIDNAP_HIDEOUT_Y, KIDNAP_HIDEOUT_Z, 5.0);

	// Small grab bonus (§4.4 "small bonus on kidnap"). A Kidnapper (SKILL_KIDNAPPER)
	// earns the larger bonus - part of the M6 kidnapper-skill activation.
	Bank_GiveCash(playerid, (PlayerInfo[playerid][pSkill] == SKILL_KIDNAPPER) ? KIDNAP_SKILL_GRAB_BONUS : KIDNAP_GRAB_BONUS);

	// Always a warrant (§4.4): +1 per civilian, +6 per cop kidnapped. The victim is
	// never a cop here (the gate blocks it), so this is the civilian value - the cop
	// branch is kept for completeness / future UC-cop grabs.
	new wanted = IsCop(targetid) ? KIDNAP_WANTED_COP : KIDNAP_WANTED_CIV;
	GivePlayerWanted(playerid, wanted, "Kidnapping");

	new msg[192];
	format(msg, sizeof(msg), ""COL_RED"* KIDNAP! "COL_WHITE"You grabbed %s (%d). Ransom set to "COL_GREEN"$%d"COL_WHITE". Drive them to the "COL_YELLOW"hideout checkpoint"COL_WHITE" to strip their cash (%d-%d%%).",
		PlayerInfo[targetid][pUserName], targetid, ransom, KIDNAP_STEAL_MIN_PCT, KIDNAP_STEAL_MAX_PCT);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	SendClientMessage(playerid, COLOR_SERVER_INFO, "Keep them close - if they get too far away they escape. "COL_YELLOW"/release"COL_WHITE" to let them go.");

	format(msg, sizeof(msg), ""COL_RED"* You have been KIDNAPPED by %s (%d)! "COL_WHITE"Ransom: "COL_GREEN"$%d"COL_WHITE". "COL_YELLOW"/payransom"COL_WHITE" to buy your freedom, or wait for a cop to "COL_YELLOW"/uncuff"COL_WHITE" you.",
		PlayerInfo[playerid][pUserName], playerid, ransom);
	SendClientMessage(targetid, COLOR_ERROR, msg);

	Kidnap_AlertCops(playerid, targetid);
	return 1;
}
alias:kidnap("kd")

//==============================================================================
//	Command - /release (/rel) (kidnapper lets the victim go, no payout)
//==============================================================================

CMD:release(playerid, params[])
{
	#pragma unused params
	if(PlayerInfo[playerid][pKidnapVictim] == -1)
		return SendClientMessage(playerid, COLOR_ERROR, "You are not holding anyone.");
	new victimid = PlayerInfo[playerid][pKidnapVictim];
	Kidnap_FreeVictim(victimid, "Your kidnapper let you go.", 0);
	SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_GREEN"* You released your victim."COL_WHITE);
	return 1;
}
alias:release("rel")

//==============================================================================
//	Command - /ransom [amount] (re-set the ransom on the held victim)
//==============================================================================

CMD:ransom(playerid, params[])
{
	if(PlayerInfo[playerid][pKidnapVictim] == -1)
		return SendClientMessage(playerid, COLOR_ERROR, "You are not holding anyone to ransom.");
	new amount, ransomMax = Kidnap_RansomMaxFor(playerid);	// skill-scaled cap (§4.4 - Kidnapper skill)
	if(sscanf(params, "d", amount))
	{
		new msg[128];
		format(msg, sizeof(msg), "USAGE: /ransom [amount]  (current ransom: $%d, max $%d)", PlayerInfo[playerid][pKidnapRansom], ransomMax);
		return SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	}
	if(amount < 0) amount = 0;
	if(amount > ransomMax) amount = ransomMax;
	PlayerInfo[playerid][pKidnapRansom] = amount;

	new victimid = PlayerInfo[playerid][pKidnapVictim], msg[128];
	format(msg, sizeof(msg), ""COL_GREEN"* Ransom updated to "COL_WHITE"$%d"COL_GREEN"."COL_WHITE, amount);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	if(IsPlayerConnected(victimid))
	{
		format(msg, sizeof(msg), ""COL_RED"* Your kidnapper set the ransom to "COL_GREEN"$%d"COL_RED". "COL_YELLOW"/payransom"COL_WHITE" to pay it.", amount);
		SendClientMessage(victimid, COLOR_ERROR, msg);
	}
	return 1;
}

//==============================================================================
//	Command - /payransom (/pr) (victim pays the ransom to free themselves early)
//==============================================================================

// The ransom moves victim -> kidnapper (cash + bank, mirrors a normal fee), then
// the victim is freed. Capped at KIDNAP_RANSOM_MAX by the grab/ransom commands.
CMD:payransom(playerid, params[])
{
	#pragma unused params
	if(!Kidnap_IsVictim(playerid))
		return SendClientMessage(playerid, COLOR_ERROR, "You are not being held for ransom.");
	new kidid = PlayerInfo[playerid][pKidnappedBy];
	new ransom = PlayerInfo[playerid][pKidnapRansom];
	if(ransom <= 0)
		return SendClientMessage(playerid, COLOR_ERROR, "There is no ransom set - you cannot buy your freedom (wait for a cop or escape).");

	// The victim pays cash-first, then bank (mirrors the medical-fee debit order).
	if((PlayerInfo[playerid][pMoney] + PlayerInfo[playerid][pBankMoney]) < ransom)
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot afford the ransom.");
	new fromCash = PlayerInfo[playerid][pMoney];
	if(fromCash > ransom) fromCash = ransom;
	if(fromCash > 0) Bank_TakeCash(playerid, fromCash);
	new fromBank = ransom - fromCash;
	if(fromBank > 0) Bank_Take(playerid, fromBank);
	// The ransom moves to the kidnapper's cash-in-hand (no mint).
	if(kidid != -1 && IsPlayerConnected(kidid)) Bank_GiveCash(kidid, ransom);

	new msg[144];
	format(msg, sizeof(msg), ""COL_GREEN"* Ransom paid "COL_WHITE"($%d)"COL_GREEN". You are released."COL_WHITE, ransom);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	if(kidid != -1 && IsPlayerConnected(kidid))
	{
		format(msg, sizeof(msg), ""COL_GREEN"* Your victim paid the ransom "COL_WHITE"$%d"COL_GREEN" and was released."COL_WHITE, ransom);
		SendClientMessage(kidid, COLOR_SERVER_INFO, msg);
		PlayerInfo[kidid][pScore] += KIDNAP_SCORE;
	}
	Kidnap_FreeVictim(playerid, "You paid the ransom.", 0);
	return 1;
}
alias:payransom("pr")

//==============================================================================
//	Command - /uncuff [id] (a cop frees a kidnapped victim near them, §4.4)
//==============================================================================

CMD:uncuff(playerid, params[])
{
	if(!IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only law enforcement can free a kidnapped person.");

	new targetid = -1;
	if(!isNull(params) && sscanf(params, "u", targetid)) return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /uncuff [playerid] (blank frees the nearest kidnapped person)");

	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	if(targetid == -1)
	{
		// Nearest kidnapped victim in reach.
		new best = -1, Float:bestDist = KIDNAP_RANGE + 2.0;
		for(new i = 0; i < MAX_PLAYERS; i++)
		{
			if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
			if(!Kidnap_IsVictim(i)) continue;
			if(!IsPlayerInRangeOfPoint(i, KIDNAP_RANGE + 2.0, x, y, z)) continue;
			new Float:d = GetPlayerDistanceFromPoint(i, x, y, z);
			if(d < bestDist) { bestDist = d; best = i; }
		}
		targetid = best;
	}
	if(targetid == -1 || !IsPlayerConnected(targetid)) return SendClientMessage(playerid, COLOR_ERROR, "There is nobody kidnapped near you to free.");
	if(!Kidnap_IsVictim(targetid)) return SendClientMessage(playerid, COLOR_ERROR, "That person is not kidnapped.");
	if(!IsPlayerInRangeOfPoint(targetid, KIDNAP_RANGE + 2.0, x, y, z)) return SendClientMessage(playerid, COLOR_ERROR, "You are too far from them.");

	new kidid = PlayerInfo[targetid][pKidnappedBy];
	Kidnap_FreeVictim(targetid, "A police officer freed you.", 1);
	PlayerInfo[playerid][pScore] += 1;	// +1 cop score for a rescue [OUR DESIGN]

	new msg[160];
	format(msg, sizeof(msg), ""COL_GREEN"* You freed %s (%d) from their kidnapper. +1 score."COL_WHITE, PlayerInfo[targetid][pUserName], targetid);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	if(kidid != -1 && IsPlayerConnected(kidid))
	{
		format(msg, sizeof(msg), ""COL_ORANGE"* A cop rescued your victim! "COL_WHITE"Get away before they arrest you.");
		SendClientMessage(kidid, COLOR_ERROR, msg);
	}
	return 1;
}

//==============================================================================
//	Command - /fakeskill (kidnapper disguise - appear as a Driver, §4.4)
//==============================================================================

// §4.4 / research: Kidnappers disguise as Drivers. The full skill-name spoof is an
// M6 skill feature; here /fakeskill toggles a session flag other systems can read
// (no visual/name change is applied in M5 - skins/skill names land in M6).
CMD:fakeskill(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	PlayerInfo[playerid][pFakeSkillDriver] = !PlayerInfo[playerid][pFakeSkillDriver];
	if(PlayerInfo[playerid][pFakeSkillDriver])
		SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_GREEN"* Disguise ON "COL_WHITE"- you now appear as a Driver to others (full skin/name spoof lands with skills in M6).");
	else
		SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_GREEN"* Disguise OFF "COL_WHITE"- your real profession is shown again.");
	return 1;
}
