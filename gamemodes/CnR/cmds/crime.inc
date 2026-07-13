/*

	Player crime command suite for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 3 (stage 3) — design §4 (crime commands) + §5 (/jumpkick, /lock,
	/unlock).

	The civilian side of the core loop: /rob a nearby player's cash (skill-capped),
	/rape a nearby target (spreads STDs via std.inc), /takedrugs from the held
	stash (buzz/overdose handled inside drugs.inc), /jumpkick a player in front,
	and /lock / /unlock the vehicle you are driving. /usecondom equips a carried
	condom for the next rape defence. Every crime is gated identically — not a cop,
	not jailed, not spawn-protected, not in a DM zone, on-foot proximity, and a
	per-player cooldown — and calls GivePlayerWanted so the wanted/cop/jail loop
	responds. Builds on the stage-1 wanted API, the stage-2 jail system and the
	std.inc / drugs.inc modules (their public API is used here, not reimplemented).

	Credits:
		crime-cop-loop.md §6 for the crime/drug/STD triangle grounding.

	Commands (gate in brackets):
		/rob (/rb) [id]        (civ, on-foot) rob nearest non-cop's cash, skill-capped
		/rape (/ra) [id]       (civ, on-foot) chance-based, may infect nearest non-cop
		/usecondom (/uc)       (civ) prime a carried condom (reduces infect chance)
		/takedrugs (/td) [g]   (civ) consume held drugs (buzz/overdose in drugs.inc)
		/jumpkick              (civ, on-foot) melee a player directly in front <2m
		/lock (/lk)            (any, driving) lock the vehicle you are driving
		/unlock (/ulk)         (any, driving) unlock the vehicle you are driving

	Functions Being Called:
		misc:    IsCop, UpdateMoney
		wanted:  GivePlayerWanted, IsPlayerSpawnProtected
		jail:    Jail_IsJailed
		std:     STD_GiveRandom, STD_Has, STD_Count
		drugs:   Drug_Consume, DrugCarryCap

	Deferred to later milestones (noted at each site):
		- Drug acquisition (buy/grow/deal) → economy/dealer + farming (M6/M7); no
		  §4 testing-grant path is specified, so /takedrugs only spends drugs the
		  player already carries (0 today until the dealer/shop system lands).
		- Condom / chastity-belt / cure purchases → /shop + medic economy (M4/M6).
		- Owned-vehicle + house locking (co-owners, Car Jacker defeat, persistence)
		  → M5; /lock here only affects the vehicle you are actively driving.
*/

//==============================================================================
//	Internal helpers
//==============================================================================

// Session-only lock state for the driven-vehicle /lock (M5 owns real ownership
// + persistence). Indexed by vehicleid; reset on nothing today (a re-created
// vehicle keeps the array clear because CreateVehicle reuses ids rarely and the
// full ownership model lands in M5).
static bool:gVehicleLocked[MAX_VEHICLES + 1];

// Shared crime gate — a civilian who can currently commit a crime. Rejects cops,
// jailed players, spawn-protected players and DM-zone players with a message so
// every crime handler reads identically (§2.5 / §4). Returns 1 if allowed.
static bool:Crime_CanCommit(playerid)
{
	if(IsCop(playerid))
	{
		SendClientMessage(playerid, COLOR_ERROR, "Law enforcement cannot commit crimes.");
		return false;
	}
	if(Jail_IsJailed(playerid))
	{
		SendClientMessage(playerid, COLOR_ERROR, "You cannot do that while you are in jail.");
		return false;
	}
	if(IsPlayerSpawnProtected(playerid))
	{
		SendClientMessage(playerid, COLOR_ERROR, "You cannot commit crimes while spawn-protected. Wait a moment.");
		return false;
	}
	if(PlayerInfo[playerid][pInDMZone])
	{
		SendClientMessage(playerid, COLOR_ERROR, "You cannot commit crimes inside a DM zone.");
		return false;
	}
	return true;
}

// A valid non-cop crime victim: connected, logged-in, human, not the actor, not
// a cop, not jailed and not spawn-protected / DM-zoned (protected players are off
// limits). Used to validate an explicit /rob|/rape [id] target.
static bool:Crime_ValidVictim(playerid, targetid)
{
	if(targetid == INVALID_PLAYER_ID || targetid == playerid) return false;
	if(!IsPlayerConnected(targetid) || IsPlayerNPC(targetid) || !PlayerInfo[targetid][pLoggedIn]) return false;
	if(IsCop(targetid)) return false;
	if(Jail_IsJailed(targetid)) return false;
	if(IsPlayerSpawnProtected(targetid) || PlayerInfo[targetid][pInDMZone]) return false;
	return true;
}

// Nearest valid non-cop victim to `playerid` within `range` (on foot only). The
// blank-target convenience for /rob and /rape (§4.1 / §4.2). Returns -1 if none.
static Crime_NearestVictim(playerid, Float:range)
{
	new Float:x, Float:y, Float:z, best = -1, Float:bestDist = range + 1.0;
	GetPlayerPos(playerid, x, y, z);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!Crime_ValidVictim(playerid, i)) continue;
		if(IsPlayerInAnyVehicle(i)) continue;					// on-foot targets only
		if(!IsPlayerInRangeOfPoint(i, range, x, y, z)) continue;
		new Float:d = GetPlayerDistanceFromPoint(i, x, y, z);
		if(d < bestDist) { bestDist = d; best = i; }
	}
	return best;
}

// Rob cash cap for a player's skill tier (§4.1). Base $50k today; the M6 skill
// work only has to set pSkill for the higher caps to unlock (data change only).
static Crime_RobCapFor(playerid)
{
	switch(PlayerInfo[playerid][pSkill])
	{
		case SKILL_PICKPOCKET: return ROB_CAP_PICKPOCKET;	// $100k, very high success
		case SKILL_CONARTIST:  return ROB_CAP_CONARTIST;	// $500k, moderate success
	}
	return ROB_CAP_BASE;									// SKILL_NONE / any other tier
}

// Rob success chance % for a player's skill tier (§4.1 — Pickpocket very high,
// Con Artist moderate, everyone else the base rate).
static Crime_RobSuccessFor(playerid)
{
	switch(PlayerInfo[playerid][pSkill])
	{
		case SKILL_PICKPOCKET: return 90;	// "very high success"
		case SKILL_CONARTIST:  return 60;	// "moderate success"
	}
	return 70;								// base
}

//==============================================================================
//	/rob (/rb) — rob a player's cash-in-hand (design §4.1)
//==============================================================================

CMD:rob(playerid, params[])
{
	if(!Crime_CanCommit(playerid)) return 1;
	if(IsPlayerInAnyVehicle(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You must be on foot to rob someone.");
	if(gettime() < PlayerInfo[playerid][pRobCooldown])
	{
		new err[96];
		format(err, sizeof(err), "You are still catching your breath — try again in %d seconds.", PlayerInfo[playerid][pRobCooldown] - gettime());
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}

	// Optional explicit target; blank picks the nearest on-foot non-cop (§4.1).
	new targetid = -1;
	if(!isNull(params))
	{
		if(sscanf(params, "u", targetid)) return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /rob [playerid] (or blank for nearest)");
		if(!Crime_ValidVictim(playerid, targetid))
			return SendClientMessage(playerid, COLOR_ERROR, "That is not a valid person to rob.");
		if(IsPlayerInAnyVehicle(targetid))
			return SendClientMessage(playerid, COLOR_ERROR, "You cannot rob someone in a vehicle.");
		new Float:x, Float:y, Float:z;
		GetPlayerPos(playerid, x, y, z);
		if(!IsPlayerInRangeOfPoint(targetid, ROB_RANGE, x, y, z))
			return SendClientMessage(playerid, COLOR_ERROR, "That person is too far away to rob.");
	}
	else
	{
		targetid = Crime_NearestVictim(playerid, ROB_RANGE);
		if(targetid == -1) return SendClientMessage(playerid, COLOR_ERROR, "There is nobody close enough to rob.");
	}

	// The victim must still be robbable this cycle (per-victim cooldown, §4.1).
	if(gettime() < PlayerInfo[targetid][pRobbedUntil])
		return SendClientMessage(playerid, COLOR_ERROR, "That person was robbed too recently — try someone else.");

	// Only cash-in-hand is robbable (bank cash is safe, §4.1).
	if(PlayerInfo[targetid][pMoney] <= 0)
	{
		SendClientMessage(playerid, COLOR_ERROR, "That person has no cash on them to steal.");
		// Still start the robber cooldown so failed attempts aren't spammable.
		PlayerInfo[playerid][pRobCooldown] = gettime() + ROB_CD;
		return 1;
	}

	// Cooldowns start regardless of the success roll below.
	PlayerInfo[playerid][pRobCooldown] = gettime() + ROB_CD;
	PlayerInfo[targetid][pRobbedUntil] = gettime() + ROB_CD;

	// Success roll (skill-gated, §4.1). A failed rob still lifts wanted — you
	// were caught with your hand in someone's pocket.
	if(random(100) >= Crime_RobSuccessFor(playerid))
	{
		SendClientMessage(playerid, COLOR_ERROR, ""COL_RED"* The robbery failed! "COL_WHITE"They shrugged you off and you got nothing.");
		SendClientMessage(targetid, COLOR_WANTED_YELLOW, ""COL_YELLOW"* Someone just tried to rob you "COL_WHITE"but you fought them off.");
		GivePlayerWanted(playerid, ROB_WANTED, "Attempted robbery");
		return 1;
	}

	// Take a random slice of the victim's on-hand cash, capped by the skill tier.
	new pct = ROB_PCT_MIN + random(ROB_PCT_MAX - ROB_PCT_MIN + 1);	// ROB_PCT_MIN..ROB_PCT_MAX
	new amount = (PlayerInfo[targetid][pMoney] * pct) / 100;
	if(amount < 1) amount = 1;
	new cap = Crime_RobCapFor(playerid);
	if(amount > cap) amount = cap;
	if(amount > PlayerInfo[targetid][pMoney]) amount = PlayerInfo[targetid][pMoney];

	PlayerInfo[targetid][pMoney] -= amount;
	UpdateMoney(targetid);
	PlayerInfo[playerid][pMoney] += amount;
	UpdateMoney(playerid);
	PlayerInfo[playerid][pScore] += ROB_SCORE;

	new string[144];
	format(string, sizeof(string), ""COL_GREEN"* Robbery: "COL_WHITE"you stole "COL_GREEN"$%d"COL_WHITE" from %s (%d). +%d score.", amount, PlayerInfo[targetid][pUserName], targetid, ROB_SCORE);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	format(string, sizeof(string), ""COL_RED"* You were robbed! "COL_WHITE"%s (%d) took "COL_RED"$%d"COL_WHITE" of your cash.", PlayerInfo[playerid][pUserName], playerid, amount);
	SendClientMessage(targetid, COLOR_ERROR, string);

	// Wanted for the completed crime (§2.3 — +2, attributed).
	GivePlayerWanted(playerid, ROB_WANTED, "Robbery of a player");
	return 1;
}
alias:rob("rb")

//==============================================================================
//	/rape (/ra) — proximity, chance-based, may infect (design §4.2)
//==============================================================================

CMD:rape(playerid, params[])
{
	if(!Crime_CanCommit(playerid)) return 1;
	if(IsPlayerInAnyVehicle(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You must be on foot to do that.");
	if(gettime() < PlayerInfo[playerid][pRapeCooldown])
	{
		new err[96];
		format(err, sizeof(err), "You cannot do that again yet — wait %d seconds.", PlayerInfo[playerid][pRapeCooldown] - gettime());
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}

	new targetid = -1;
	if(!isNull(params))
	{
		if(sscanf(params, "u", targetid)) return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /rape [playerid] (or blank for nearest)");
		if(!Crime_ValidVictim(playerid, targetid))
			return SendClientMessage(playerid, COLOR_ERROR, "That is not a valid target.");
		if(IsPlayerInAnyVehicle(targetid))
			return SendClientMessage(playerid, COLOR_ERROR, "You cannot do that to someone in a vehicle.");
		new Float:x, Float:y, Float:z;
		GetPlayerPos(playerid, x, y, z);
		if(!IsPlayerInRangeOfPoint(targetid, RAPE_RANGE, x, y, z))
			return SendClientMessage(playerid, COLOR_ERROR, "That target is too far away.");
	}
	else
	{
		targetid = Crime_NearestVictim(playerid, RAPE_RANGE);
		if(targetid == -1) return SendClientMessage(playerid, COLOR_ERROR, "There is nobody close enough.");
	}

	// The cooldown starts on the attempt (Rapist skill shortens it in M6).
	PlayerInfo[playerid][pRapeCooldown] = gettime() + RAPE_CD;

	// A chastity belt prevents the act outright (breakable — M6 owns the break
	// roll; today it simply blocks, per §4.2).
	if(PlayerInfo[targetid][pChastityBelt])
	{
		SendClientMessage(playerid, COLOR_ERROR, ""COL_RED"* They are wearing a chastity belt "COL_WHITE"— you cannot rape them.");
		SendClientMessage(targetid, COLOR_SERVER_INFO, "Your chastity belt protected you from a rape attempt.");
		return 1;
	}

	// Success roll (§4.2). Rapist skill + carrying several STDs push toward ~100%.
	new chance = RAPE_SUCCESS_BASE;
	if(PlayerInfo[playerid][pSkill] == SKILL_RAPIST) chance += 30;
	chance += STD_Count(playerid) * 10;						// several STDs → near-certain
	if(chance > 100) chance = 100;
	if(random(100) >= chance)
	{
		SendClientMessage(playerid, COLOR_ERROR, ""COL_RED"* Your attempt failed "COL_WHITE"— they got away.");
		SendClientMessage(targetid, COLOR_WANTED_YELLOW, "Someone tried to assault you but you escaped.");
		GivePlayerWanted(playerid, RAPE_WANTED, "Attempted rape");
		return 1;
	}

	// Deals damage even through a condom (§4.2).
	new Float:hp;
	GetPlayerHealth(targetid, hp);
	hp -= RAPE_DAMAGE;
	if(hp < 0.0) hp = 0.0;
	SetPlayerHealth(targetid, hp);

	// Infection roll — a condom drastically cuts the chance (§4.2). A Rapist can
	// pass a tier they don't personally carry; STD_GiveRandom picks a tier the
	// victim doesn't already have.
	new infectChance = (PlayerInfo[targetid][pCondoms] > 0) ? RAPE_INFECT_CONDOM : RAPE_INFECT_BASE;
	if(PlayerInfo[playerid][pSkill] == SKILL_RAPIST) infectChance += 20;
	new bool:infected = false;
	if(random(100) < infectChance)
	{
		// Consume one primed condom on a defended hit (it did its job / broke).
		if(PlayerInfo[targetid][pCondoms] > 0) PlayerInfo[targetid][pCondoms]--;
		if(STD_GiveRandom(targetid, "a rape") != 0) infected = true;
	}
	else if(PlayerInfo[targetid][pCondoms] > 0)
	{
		// Condom held — spend it and spare the infection.
		PlayerInfo[targetid][pCondoms]--;
		SendClientMessage(targetid, COLOR_SERVER_INFO, "Your condom protected you from infection this time.");
	}

	PlayerInfo[playerid][pScore] += RAPE_SCORE;

	new string[144];
	format(string, sizeof(string), ""COL_GREEN"* You raped %s (%d). "COL_WHITE"+%d score%s.", PlayerInfo[targetid][pUserName], targetid, RAPE_SCORE, infected ? (" — you infected them") : (""));
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	format(string, sizeof(string), ""COL_RED"* You were raped by %s (%d)."COL_WHITE, PlayerInfo[playerid][pUserName], playerid);
	SendClientMessage(targetid, COLOR_ERROR, string);

	// Wanted +2, +1 more if it infected (§2.3).
	GivePlayerWanted(playerid, RAPE_WANTED + (infected ? RAPE_WANTED_INFECT : 0), "Rape");
	return 1;
}
alias:rape("ra")

//==============================================================================
//	/usecondom (/uc) — prime a carried condom for the next rape (design §4.2)
//==============================================================================

// Condoms reduce the infection chance at /rape time (they are consumed there,
// not a cure). Buying condoms is deferred to the /shop / medic economy (M4/M6);
// this only reports what the player already carries.
CMD:usecondom(playerid, params[])
{
	#pragma unused params
	if(PlayerInfo[playerid][pCondoms] <= 0)
		return SendClientMessage(playerid, COLOR_ERROR, "You have no condoms — buy some at a 24/7, hospital or medic (coming with the shop system).");
	new string[128];
	format(string, sizeof(string), ""COL_GREEN"* You have %d condom(s) ready. "COL_WHITE"They cut your infection chance if you are raped.", PlayerInfo[playerid][pCondoms]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	return 1;
}
alias:usecondom("uc")

//==============================================================================
//	/takedrugs (/td) — consume the held drug stash (design §4.3)
//==============================================================================

// Buzz stages, healing, overdose (>60g) and bad-drug rolls all live in
// drugs.inc; this command only validates the amount and forwards to Drug_Consume.
// Buying/growing drugs is deferred to the dealer/shop + farming milestones
// (M6/M7); §4 specifies no testing-grant path, so pDrugs is 0 until then.
CMD:takedrugs(playerid, params[])
{
	// No jail/cop/DM-zone gate: taking drugs is not itself a wanted-raising crime,
	// and drugs are deliberately usable in jail (§5 — the cell is a rape hotspot,
	// so inmates carry drugs to survive the STD drain).
	if(PlayerInfo[playerid][pDrugs] <= 0)
		return SendClientMessage(playerid, COLOR_ERROR, "You have no drugs on you. Buy or grow some first (dealer/shop system coming soon).");

	new grams;
	if(sscanf(params, "d", grams))
	{
		// No amount → take the whole stash (click-mode is an F-driven client
		// feature deferred to M6; the single-shot dose keeps /td useful today).
		grams = PlayerInfo[playerid][pDrugs];
		if(grams > DRUG_MAX_SINGLE) grams = DRUG_MAX_SINGLE;	// avoid an accidental overdose on a blank /td
	}
	if(grams <= 0) return SendClientMessage(playerid, COLOR_ERROR, "Enter a positive number of grams.");
	if(grams > PlayerInfo[playerid][pDrugs])
		return SendClientMessage(playerid, COLOR_ERROR, "You do not have that many grams on you.");

	// Drug_Consume deducts the stash, rolls bad-drugs, handles overdose + buzz.
	Drug_Consume(playerid, grams);
	return 1;
}
alias:takedrugs("td")

//==============================================================================
//	/jumpkick — melee a player directly in front (design §5 / jobs §7.1)
//==============================================================================

CMD:jumpkick(playerid, params[])
{
	#pragma unused params
	if(Jail_IsJailed(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot do that while you are in jail.");
	if(IsPlayerInAnyVehicle(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "You must be on foot to jump-kick.");
	if(gettime() < PlayerInfo[playerid][pJumpkickCooldown])
	{
		new err[96];
		format(err, sizeof(err), "You cannot jump-kick again yet — wait %d seconds.", PlayerInfo[playerid][pJumpkickCooldown] - gettime());
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	PlayerInfo[playerid][pJumpkickCooldown] = gettime() + JUMPKICK_CD;

	// Kick animation regardless of a hit (the move always plays).
	ApplyAnimation(playerid, "KUNG_FU", "kickgroin", 4.1, false, false, false, false, 0, true);

	// Find the nearest player within reach in front of us. We approximate "in
	// front" with a short-range proximity check (a full facing-cone check is not
	// worth the trig here — JUMPKICK_RANGE is 2m).
	new Float:x, Float:y, Float:z, targetid = -1, Float:bestDist = JUMPKICK_RANGE + 1.0;
	GetPlayerPos(playerid, x, y, z);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(i == playerid || !IsPlayerConnected(i) || IsPlayerNPC(i) || !PlayerInfo[i][pLoggedIn]) continue;
		if(IsPlayerInAnyVehicle(i)) continue;
		if(IsPlayerSpawnProtected(i) || Jail_IsJailed(i)) continue;
		if(!IsPlayerInRangeOfPoint(i, JUMPKICK_RANGE, x, y, z)) continue;
		new Float:d = GetPlayerDistanceFromPoint(i, x, y, z);
		if(d < bestDist) { bestDist = d; targetid = i; }
	}
	if(targetid == -1)
	{
		SendClientMessage(playerid, COLOR_SERVER_INFO, "You jump-kick the air — nobody was in front of you.");
		return 1;
	}

	// Land the kick: small melee damage.
	new Float:hp;
	GetPlayerHealth(targetid, hp);
	hp -= JUMPKICK_DAMAGE;
	if(hp < 0.0) hp = 0.0;
	SetPlayerHealth(targetid, hp);

	new string[128];
	format(string, sizeof(string), ""COL_YELLOW"* You jump-kicked %s (%d)."COL_WHITE, PlayerInfo[targetid][pUserName], targetid);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	format(string, sizeof(string), ""COL_RED"* %s (%d) jump-kicked you!"COL_WHITE, PlayerInfo[playerid][pUserName], playerid);
	SendClientMessage(targetid, COLOR_ERROR, string);

	// Small wanted only when it actually damages a non-cop (§5). Cops jump-kicking
	// or the DM-zone/spawn-protect suppression is handled inside GivePlayerWanted.
	if(!IsCop(playerid)) GivePlayerWanted(playerid, 1, "Jump-kick assault");
	return 1;
}

//==============================================================================
//	/lock (/lk) + /unlock (/ulk) — the vehicle you are driving (design §5)
//==============================================================================

// M3 scope: lock/unlock the doors of the vehicle you are currently DRIVING for
// every other player (SetVehicleParamsForPlayer). Full owned-vehicle + house
// locking (co-owners, Car Jacker defeat, persistence) is deferred to M5.
static Crime_SetVehicleLock(playerid, bool:locked)
{
	if(!IsPlayerInAnyVehicle(playerid))
		return SendClientMessage(playerid, COLOR_ERROR, "You must be driving a vehicle to do that.");
	if(GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
		return SendClientMessage(playerid, COLOR_ERROR, "Only the driver can lock or unlock the vehicle.");

	new vehicleid = GetPlayerVehicleID(playerid);
	if(vehicleid < 1 || vehicleid > MAX_VEHICLES) return 0;

	gVehicleLocked[vehicleid] = locked;

	// Apply the per-player door lock for everyone except the driver so they can
	// still drive it (open.mp SetVehicleParamsForPlayer: objective, doors).
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(i == playerid) { SetVehicleParamsForPlayer(vehicleid, i, 0, 0); continue; }	// driver's doors open
		SetVehicleParamsForPlayer(vehicleid, i, 0, locked ? 1 : 0);
	}

	SendClientMessage(playerid, COLOR_SERVER_INFO, locked ?
		(""COL_GREEN"* Vehicle locked."COL_WHITE" Others cannot enter it.") :
		(""COL_GREEN"* Vehicle unlocked."COL_WHITE" Others can enter it again."));
	return 1;
}

CMD:lock(playerid, params[])
{
	#pragma unused params
	return Crime_SetVehicleLock(playerid, true);
}
alias:lock("lk")

CMD:unlock(playerid, params[])
{
	#pragma unused params
	return Crime_SetVehicleLock(playerid, false);
}
alias:unlock("ulk")
