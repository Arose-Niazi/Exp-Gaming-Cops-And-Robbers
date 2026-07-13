/*

	STD (disease) system for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 3 (stage 3) — design §4.2 (STD tiers, HP drain, cures).

	Owns the disease side of the rape/drug/STD triangle. STDs live in a bitmask
	(pSTD) with five tiers (Chlamydia..Mary Lou); each infected tier drains HP on
	a slow timer and kills if untreated. Drugs stall the drain (a buzzed player
	takes no STD damage — the drug system sets pDrugEffectUntil). Cures: condoms
	reduce the infection chance at /rape time (not a cure), the Adrenaline pill
	(/ad, M4) fully heals + cures all, and a Public Medic can force-cure (M6).

	Credits:
		crime-cop-loop.md §6.1 for the STD mechanic grounding.

	Public API (called by cmds/crimes.inc + CnR.pwn):
		STD_Has(playerid, stdBit)                : is this specific STD carried?
		STD_HasAny(playerid)                     : bool — any STD at all
		STD_Count(playerid)                      : number of tiers infected (0..MAX_STD_TYPES)
		STD_Give(playerid, stdBit, const src[])  : infect with one STD tier
		STD_GiveRandom(playerid, const src[])    : infect with a random not-yet-held tier
		STD_CureAll(playerid, const reason[])    : clear all STDs (adrenaline / medic)
		STD_NameFor(stdBit)                      : readable tier name
		STD_OnGameModeClockTick()                : hook from GameModeClock() (1s) — HP drain
		STD_SavePlayer(playerid)                 : persist (disconnect)

	Functions Being Called:
		misc:    (none — self-contained)
*/

//==============================================================================
//	Tier table (bit -> readable name), ordered mildest..worst
//==============================================================================

static const gStdBits[MAX_STD_TYPES] = { STD_CHLAMYDIA, STD_GONORRHEA, STD_SYPHILIS, STD_HERPES, STD_MARYLOU };
static const gStdNames[MAX_STD_TYPES][] = { "Chlamydia", "Gonorrhea", "Syphilis", "Herpes", "Mary Lou" };

// Readable name for a single STD bit (returns "an STD" if the bit is unknown).
// Plain stock (not a FUNCTION public): publics may not return arrays, and this
// is only called within this module (STD_Give), so no cross-module forward is
// needed.
stock STD_NameFor(stdBit)
{
	static ret[16];
	for(new i = 0; i < MAX_STD_TYPES; i++)
	{
		if(gStdBits[i] == stdBit) { format(ret, sizeof(ret), "%s", gStdNames[i]); return ret; }
	}
	format(ret, sizeof(ret), "an STD");
	return ret;
}

//==============================================================================
//	Query helpers (design §4.2)
//==============================================================================

FUNCTION STD_Has(playerid, stdBit)
	return (PlayerInfo[playerid][pSTD] & stdBit) ? 1 : 0;

FUNCTION STD_HasAny(playerid)
	return (PlayerInfo[playerid][pSTD] != 0) ? 1 : 0;

FUNCTION STD_Count(playerid)
{
	new n = 0;
	for(new i = 0; i < MAX_STD_TYPES; i++)
		if(PlayerInfo[playerid][pSTD] & gStdBits[i]) n++;
	return n;
}

//==============================================================================
//	Infection / cure (design §4.2)
//==============================================================================

// Infect a player with one specific STD tier. No-op if they already carry it.
FUNCTION STD_Give(playerid, stdBit, const src[])
{
	if(!IsPlayerConnected(playerid) || !PlayerInfo[playerid][pLoggedIn]) return 0;
	if(PlayerInfo[playerid][pSTD] & stdBit) return 0;	// already infected with this tier
	PlayerInfo[playerid][pSTD] |= stdBit;

	new string[144];
	format(string, sizeof(string), ""COL_DISEASE"* You have contracted %s%s%s. "COL_WHITE"It drains your health — cure it with an Adrenaline pill (/ad) or a Public Medic.",
		STD_NameFor(stdBit), isNull(src) ? ("") : (" from "), isNull(src) ? ("") : (src));
	SendClientMessage(playerid, COLOR_DISEASE, string);
	return 1;
}

// Infect with a random tier the player does not already carry. Returns the bit
// given (0 if the player already carries every tier).
FUNCTION STD_GiveRandom(playerid, const src[])
{
	if(!IsPlayerConnected(playerid)) return 0;
	// Collect the tiers not yet held.
	new avail[MAX_STD_TYPES], count = 0;
	for(new i = 0; i < MAX_STD_TYPES; i++)
		if(!(PlayerInfo[playerid][pSTD] & gStdBits[i])) avail[count++] = gStdBits[i];
	if(count == 0) return 0;	// fully infected already
	new pick = avail[random(count)];
	STD_Give(playerid, pick, src);
	return pick;
}

// Clear every STD (adrenaline pill / Public Medic force-cure).
FUNCTION STD_CureAll(playerid, const reason[])
{
	if(PlayerInfo[playerid][pSTD] == 0) return 0;
	PlayerInfo[playerid][pSTD] = 0;
	PlayerInfo[playerid][pSTDDrainCounter] = 0;
	if(!isNull(reason))
	{
		new string[96];
		format(string, sizeof(string), ""COL_GREEN"* All diseases cured "COL_WHITE"(%s).", reason);
		SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	}
	return 1;
}

//==============================================================================
//	HP drain — hooked from GameModeClock() (1-second timer, design §4.2)
//==============================================================================

// Runs once per real second (no new global timer). Infected players lose HP
// scaled by how many tiers they carry, unless a drug buzz is stalling the drain
// (§4.3 — drugs counteract the STD drain to buy time until cured). Draining to
// 0 HP kills the untreated carrier.
FUNCTION STD_OnGameModeClockTick()
{
	for(new playerid = 0; playerid < MAX_PLAYERS; playerid++)
	{
		if(!IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) continue;
		if(!PlayerInfo[playerid][pLoggedIn] || !PlayerInfo[playerid][pSpawned]) continue;
		if(PlayerInfo[playerid][pSTD] == 0) continue;
		// Jailed/admin-jailed players are frozen — don't drain them into a death loop.
		if(PlayerInfo[playerid][pJailed] || PlayerInfo[playerid][pAdminJailed]) continue;
		// A live drug buzz stalls the drain (§4.3).
		if(PlayerInfo[playerid][pDrugEffectUntil] != 0 && gettime() < PlayerInfo[playerid][pDrugEffectUntil]) continue;

		if(++PlayerInfo[playerid][pSTDDrainCounter] < STD_DRAIN_SECS) continue;
		PlayerInfo[playerid][pSTDDrainCounter] = 0;

		new Float:hp;
		GetPlayerHealth(playerid, hp);
		new Float:drain = STD_Count(playerid) * STD_DRAIN_PER_TIER;
		hp -= drain;
		if(hp <= 0.0)
		{
			SetPlayerHealth(playerid, 0.0);	// disease death (OnPlayerDeath clears STDs)
			SendClientMessage(playerid, COLOR_DISEASE, ""COL_RED"* Your untreated disease killed you."COL_WHITE" Carry drugs or find a cure next time.");
		}
		else
		{
			SetPlayerHealth(playerid, hp);
			// Occasional nag so the player knows why they're bleeding out.
			if(random(4) == 0)
			{
				new string[96];
				format(string, sizeof(string), ""COL_DISEASE"* Disease is draining your health "COL_WHITE"(%d infection%s). Cure it soon.", STD_Count(playerid), (STD_Count(playerid) == 1) ? ("") : ("s"));
				SendClientMessage(playerid, COLOR_DISEASE, string);
			}
		}
	}
	return 1;
}

//==============================================================================
//	Persistence (disconnect) — LSplayers.STDs / Condoms / ChastityBelt
//==============================================================================

FUNCTION STD_SavePlayer(playerid)
{
	if(!PlayerInfo[playerid][pLoggedIn]) return 0;
	new string[144];
	mysql_format(g_SQL, string, sizeof(string),
		"UPDATE "TABLE_PLAYERS_C" SET STDs=%d, Condoms=%d, ChastityBelt=%d WHERE aID=%d LIMIT 1",
		PlayerInfo[playerid][pSTD], PlayerInfo[playerid][pCondoms], PlayerInfo[playerid][pChastityBelt] ? 1 : 0, PlayerInfo[playerid][pID]);
	mysql_pquery(g_SQL, string);
	return 1;
}

// Death clears the disease state (a fresh spawn is clean); called from
// OnPlayerDeath in CnR.pwn.
FUNCTION STD_OnPlayerDeath(playerid)
{
	PlayerInfo[playerid][pSTD] = 0;
	PlayerInfo[playerid][pSTDDrainCounter] = 0;
	return 1;
}
