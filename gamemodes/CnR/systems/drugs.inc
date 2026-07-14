/*

	Drug system for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 3 (stage 3) — design §4.3 (/takedrugs, buzz stages, overdose, bad
	drugs) + the STD-drain stall (§4.2 — drugs buy time until cured).

	Owns the drug consumption side of the crime/drug/STD triangle. Drugs heal
	over time (a small heal every DRUG_HEAL_TICK_SECS while buzzed), stall the STD
	drain (std.inc checks pDrugEffectUntil), and can go wrong: >60g at once is an
	overdose (slow death cancelled by the M4 adrenaline pill), and a random dose
	is "bad" (falling anim, buzz reset, chance of an STD, -1 score). Carry limits
	are skill-gated (Drug Dealer 2500g base, 5000g with a drug bag; others 500g);
	the skill enum + drug bag are set in M6, so DrugCarryCap() reads the base cap
	for everyone today (§16 #2).

	Credits:
		crime-cop-loop.md §6.2 for the drug mechanic grounding.

	Public API (called by cmds/crimes.inc + CnR.pwn):
		DrugCarryCap(playerid)                   : carry cap in grams (skill-gated)
		Drug_GiveGrams(playerid, grams)          : add grams (clamped to the cap)
		Drug_OnGameModeClockTick()               : hook from GameModeClock() (1s) — heal/OD
		Drug_ClearOverdose(playerid, const r[])  : cancel an overdose (adrenaline, M4)
		Drug_OnPlayerDeath(playerid)             : reset buzz/OD on death
		Drug_SavePlayer(playerid)                : persist (disconnect)

	Functions Being Called:
		std:     STD_GiveRandom
*/

//==============================================================================
//	Carry cap (design §4.3 — skill-gated, base cap today)
//==============================================================================

// Drug Dealer holds 5000g, everyone else 500g. pSkill is set at City Hall in
// M6; until then everyone reads the base cap. Parameterised so the M6 skill work
// only has to define the skill id.
FUNCTION DrugCarryCap(playerid)
{
	// Drug Dealers get the bigger cap; a drug bag (M6 refill-point acquisition)
	// doubles it (§16 #2). pSkill/pHasDrugBag are M6 data, so today this reads the
	// base cap for everyone.
	if(PlayerInfo[playerid][pSkill] == SKILL_DRUGDEALER)
		return PlayerInfo[playerid][pHasDrugBag] ? DRUG_DEALER_WITH_BAG : DRUG_DEALER_BASE;
	return DRUG_CARRY_MAX;
}

// Add grams to the player's stash, clamped to their carry cap. Returns the
// grams actually added.
FUNCTION Drug_GiveGrams(playerid, grams)
{
	if(grams <= 0) return 0;
	new cap = DrugCarryCap(playerid);
	new room = cap - PlayerInfo[playerid][pDrugs];
	if(room <= 0) return 0;
	if(grams > room) grams = room;
	PlayerInfo[playerid][pDrugs] += grams;
	return grams;
}

//==============================================================================
//	Buzz stage helper (design §4.3 — Low/Medium/High/Very High)
//==============================================================================

// Map an intake amount to a buzz stage (more grams = higher buzz = faster heal).
static Drug_BuzzStageFor(grams)
{
	if(grams >= 45) return BUZZ_VERYHIGH;
	if(grams >= 30) return BUZZ_HIGH;
	if(grams >= 15) return BUZZ_MEDIUM;
	if(grams >= 1)  return BUZZ_LOW;
	return BUZZ_NONE;
}

static Drug_BuzzName(stage)
{
	static ret[12];
	switch(stage)
	{
		case BUZZ_LOW:      format(ret, sizeof(ret), "Low");
		case BUZZ_MEDIUM:   format(ret, sizeof(ret), "Medium");
		case BUZZ_HIGH:     format(ret, sizeof(ret), "High");
		case BUZZ_VERYHIGH: format(ret, sizeof(ret), "Very High");
		default:            format(ret, sizeof(ret), "None");
	}
	return ret;
}

//==============================================================================
//	Consumption core (design §4.3) — used by /takedrugs and click-mode
//==============================================================================

// Consume `grams` of drugs. Enforces the max-single (overdose) rule, rolls for
// bad drugs, and starts/extends the buzz that heals + stalls the STD drain.
// Returns 1 if the dose was consumed, 0 on a hard error (no drugs / bad params).
FUNCTION Drug_Consume(playerid, grams)
{
	if(!IsPlayerConnected(playerid) || !PlayerInfo[playerid][pLoggedIn]) return 0;
	if(grams <= 0) return 0;
	if(PlayerInfo[playerid][pDrugs] < grams)
	{
		SendClientMessage(playerid, COLOR_ERROR, "You do not have that many grams of drugs on you.");
		return 0;
	}

	// Deduct the stash first (whatever happens next, the drugs are gone).
	PlayerInfo[playerid][pDrugs] -= grams;

	new string[144];

	// Bad-drugs roll (§4.3): falling anim, buzz reset, chance of an STD, -1 score.
	if(random(100) < DRUG_BADCHANCE)
	{
		ApplyAnimation(playerid, "PED", "KO_shot_stom", 4.1, false, false, false, false, 0, true);
		PlayerInfo[playerid][pDrugBuzzStage] = BUZZ_NONE;
		PlayerInfo[playerid][pDrugEffectUntil] = 0;
		if(PlayerInfo[playerid][pScore] > 0) PlayerInfo[playerid][pScore]--;
		SendClientMessage(playerid, COLOR_ERROR, ""COL_RED"* Bad drugs!"COL_WHITE" You collapse, your buzz is gone and you lost a point.");
		if(random(100) < DRUG_BAD_STD_CHANCE) STD_GiveRandom(playerid, "bad drugs");
		return 1;
	}

	// Overdose: a single intake over the max slowly kills (§4.3).
	if(grams > DRUG_MAX_SINGLE)
	{
		PlayerInfo[playerid][pDrugOverdose] = true;
		PlayerInfo[playerid][pDrugBuzzStage] = BUZZ_NONE;
		PlayerInfo[playerid][pDrugEffectUntil] = 0;
		format(string, sizeof(string), ""COL_RED"* OVERDOSE! "COL_WHITE"You took %dg at once (max %dg). You are dying slowly — an Adrenaline pill (/ad) can save you.", grams, DRUG_MAX_SINGLE);
		SendClientMessage(playerid, COLOR_DRUG, string);
		return 1;
	}

	// Normal dose: start/extend the buzz (heals + stalls the STD drain).
	new addSecs = grams * DRUG_EFFECT_SECS_PER_G;
	new base = (PlayerInfo[playerid][pDrugEffectUntil] > gettime()) ? PlayerInfo[playerid][pDrugEffectUntil] : gettime();
	PlayerInfo[playerid][pDrugEffectUntil] = base + addSecs;
	new newStage = Drug_BuzzStageFor(grams);
	if(newStage > PlayerInfo[playerid][pDrugBuzzStage]) PlayerInfo[playerid][pDrugBuzzStage] = newStage;
	PlayerInfo[playerid][pDrugHealCounter] = 0;

	format(string, sizeof(string), ""COL_DRUG"* You took %dg. "COL_WHITE"Buzz: "COL_DRUG"%s"COL_WHITE" — you heal over time. Stash: %dg.", grams, Drug_BuzzName(PlayerInfo[playerid][pDrugBuzzStage]), PlayerInfo[playerid][pDrugs]);
	SendClientMessage(playerid, COLOR_DRUG, string);
	return 1;
}

//==============================================================================
//	Overdose cancel (design §4.3 — adrenaline pill, wired in M4)
//==============================================================================

FUNCTION Drug_ClearOverdose(playerid, const reason[])
{
	if(!PlayerInfo[playerid][pDrugOverdose]) return 0;
	PlayerInfo[playerid][pDrugOverdose] = false;
	if(!isNull(reason))
	{
		new string[96];
		format(string, sizeof(string), ""COL_GREEN"* Overdose stopped "COL_WHITE"(%s).", reason);
		SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	}
	return 1;
}

//==============================================================================
//	Heal / overdose tick — hooked from GameModeClock() (1-second, design §4.3)
//==============================================================================

FUNCTION Drug_OnGameModeClockTick()
{
	for(new playerid = 0; playerid < MAX_PLAYERS; playerid++)
	{
		if(!IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) continue;
		if(!PlayerInfo[playerid][pLoggedIn] || !PlayerInfo[playerid][pSpawned]) continue;

		// Overdose slow-death (§4.3) — bleeds HP until an adrenaline pill cancels it.
		if(PlayerInfo[playerid][pDrugOverdose])
		{
			new Float:hp;
			GetPlayerHealth(playerid, hp);
			hp -= DRUG_OD_DMG_PER_SEC;
			if(hp <= 0.0)
			{
				SetPlayerHealth(playerid, 0.0);	// OD death — OnPlayerDeath resets the flag
				SendClientMessage(playerid, COLOR_DRUG, ""COL_RED"* You overdosed and died."COL_WHITE" Never take more than 60g at once.");
			}
			else SetPlayerHealth(playerid, hp);
			continue;	// no healing while overdosing
		}

		// Active buzz: small heal on the heal-tick interval (§4.3).
		if(PlayerInfo[playerid][pDrugEffectUntil] == 0) continue;
		if(gettime() >= PlayerInfo[playerid][pDrugEffectUntil])
		{
			// Buzz just ended — clear the stage.
			PlayerInfo[playerid][pDrugEffectUntil] = 0;
			PlayerInfo[playerid][pDrugBuzzStage] = BUZZ_NONE;
			PlayerInfo[playerid][pDrugHealCounter] = 0;
			continue;
		}
		if(++PlayerInfo[playerid][pDrugHealCounter] < DRUG_HEAL_TICK_SECS) continue;
		PlayerInfo[playerid][pDrugHealCounter] = 0;
		new Float:hp;
		GetPlayerHealth(playerid, hp);
		if(hp < 100.0)
		{
			// Higher buzz heals a touch faster.
			new Float:heal = DRUG_HEAL_PER_TICK * float(PlayerInfo[playerid][pDrugBuzzStage]);
			if(heal < DRUG_HEAL_PER_TICK) heal = DRUG_HEAL_PER_TICK;
			hp += heal;
			if(hp > 100.0) hp = 100.0;
			SetPlayerHealth(playerid, hp);
		}
	}
	return 1;
}

//==============================================================================
//	Death / persistence hooks
//==============================================================================

// Dying resets the buzz/overdose (a fresh spawn is clean). Called from
// OnPlayerDeath in CnR.pwn.
FUNCTION Drug_OnPlayerDeath(playerid)
{
	PlayerInfo[playerid][pDrugOverdose] = false;
	PlayerInfo[playerid][pDrugBuzzStage] = BUZZ_NONE;
	PlayerInfo[playerid][pDrugEffectUntil] = 0;
	PlayerInfo[playerid][pDrugHealCounter] = 0;
	PlayerInfo[playerid][pDrugClickMode] = false;
	return 1;
}

FUNCTION Drug_SavePlayer(playerid)
{
	if(!PlayerInfo[playerid][pLoggedIn]) return 0;
	new string[128];
	// M6 (stage 1) — Skill (set at City Hall via /skill) and HasDrugBag (bought via
	// /clotheswear, doubles the Drug Dealer carry cap) live in the crime/drug domain,
	// so they persist through this same crime/drug UPDATE (no competing query).
	mysql_format(g_SQL, string, sizeof(string), "UPDATE "TABLE_PLAYERS_C" SET Drugs=%d, Skill=%d, HasDrugBag=%d WHERE aID=%d LIMIT 1", PlayerInfo[playerid][pDrugs], PlayerInfo[playerid][pSkill], PlayerInfo[playerid][pHasDrugBag] ? 1 : 0, PlayerInfo[playerid][pID]);
	mysql_pquery(g_SQL, string);
	return 1;
}
