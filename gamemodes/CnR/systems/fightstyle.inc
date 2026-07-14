/*

	Fighting styles system for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 6 (stage 1) - design §9.8 (fighting styles) + jobs-progression §2.2
	(gym / fighting styles) + crazybobs-site-full §Gym.

	CB: "Visit any of the three gyms and change your fighting style! There are three
	options to choose from. Found in Ganton, Garcia, and Redsands East." We expose the
	three visually-distinct melee styles (Boxing / Kung-Fu / Knee-head) at gym markers
	in Ganton (LS), Garcia (SF), Redsands East (LV) for a small fee (FIGHTSTYLE_FEE).

	The chosen style persists (LSplayers.FightStyle, default 4=NORMAL) and is re-applied
	with SetPlayerFightingStyle on every spawn (FightStyle_ApplyOnSpawn). It ties into
	/jumpkick (M4 melee) - a heavier style lands harder-looking kicks.

	Public API:
		FightStyle_Init()                                : create the 3 gym markers/map icons (OnGameModeInit)
		FightStyle_Cleanup()                             : destroy them (OnGameModeExit)
		FightStyle_ApplyOnSpawn(playerid)                : re-apply the saved style on spawn (§9.8)
		FightStyle_OnDialogResponse(playerid, dialogid, response, listitem) : /fightstyle resolution

	Commands:
		/fightstyle (/fs /gym)  (on-foot at a gym)  buy / set a GTA fighting style

	Functions Being Called:
		bank:   Bank_TakeCash
		jail:   Jail_SavePlayer (persists FightStyle via the shared LSplayers UPDATE)
*/

//==============================================================================
//	Style table + gym markers (§9.8 / jobs §2.2)
//==============================================================================

// A choosable fighting style: its SA-MP FIGHTING_STYLE_* id + display name. CB
// offered 3 options; we expose the 3 distinct melee styles plus a free "Normal"
// reset row so a player can revert to the FightStyle default.
enum e_FightStyleDef
{
	fsId,				// FIGHTING_STYLE_* id
	fsName[16],			// display name
	fsFee				// fee to switch to this style ($) - Normal is free
};

// Style ids are stored as plain ints in this table (and in pFightStyle) - the
// SetPlayerFightingStyle call tags them with FIGHT_STYLE: at the call site. The raw
// values match the SA-MP ids: NORMAL 4, BOXING 5, KUNGFU 6, KNEEHEAD 7 (§9.8).
static const g_FightStyles[][e_FightStyleDef] =
{
	{ 4, "Normal",    0 },				// FIGHT_STYLE_NORMAL - reset to default (free)
	{ 5, "Boxing",    FIGHTSTYLE_FEE },	// FIGHT_STYLE_BOXING
	{ 6, "Kung-Fu",   FIGHTSTYLE_FEE },	// FIGHT_STYLE_KUNGFU
	{ 7, "Knee-head", FIGHTSTYLE_FEE }	// FIGHT_STYLE_KNEEHEAD
};

// The 3 gym locations (jobs §2.2 - Ganton/Garcia/Redsands East). Only the LS gym
// (Ganton) is reachable while CITY_ == LS; the SF/LV markers are kept for when those
// cities go live (they simply never get walked into meanwhile).
static const Float:g_GymSpots[][3] =
{
	{ 2229.190,  -1721.502, 12.867 },	// Ganton gym (LS)
	{ -2266.070, 149.145,   35.312 },	// Garcia gym (SF)
	{ 1348.930,  1970.470,  12.867 }	// Redsands East gym (LV)
};

static STREAMER_TAG_CP:g_GymCP[sizeof(g_GymSpots)];
static STREAMER_TAG_MAP_ICON:g_GymIcon[sizeof(g_GymSpots)];

//==============================================================================
//	Setup / teardown
//==============================================================================

FUNCTION FightStyle_Init()
{
	for(new i = 0; i < sizeof(g_GymSpots); i++)
	{
		g_GymCP[i] = CreateDynamicCP(g_GymSpots[i][0], g_GymSpots[i][1], g_GymSpots[i][2], 2.0, -1, -1, -1, 100.0);
		// A dumbbell map icon (model 32) so players can find a gym (§9.8).
		g_GymIcon[i] = CreateDynamicMapIcon(g_GymSpots[i][0], g_GymSpots[i][1], g_GymSpots[i][2], 32, 0, -1, -1, -1, 200.0, MAPICON_LOCAL);
	}
	return 1;
}

FUNCTION FightStyle_Cleanup()
{
	for(new i = 0; i < sizeof(g_GymSpots); i++)
	{
		if(DestroyDynamicCP(g_GymCP[i])) {}
		if(DestroyDynamicMapIcon(g_GymIcon[i])) {}
	}
	return 1;
}

// Is the player standing at any gym marker (same world-0 exterior)? Returns 1/0.
static FightStyle_AtGym(playerid)
{
	if(GetPlayerInterior(playerid) != 0) return 0;
	for(new i = 0; i < sizeof(g_GymSpots); i++)
	{
		if(IsPlayerInRangeOfPoint(playerid, FIGHTSTYLE_GYM_RANGE, g_GymSpots[i][0], g_GymSpots[i][1], g_GymSpots[i][2]))
			return 1;
	}
	return 0;
}

//==============================================================================
//	Spawn re-apply (§9.8 - persist + re-apply on every spawn)
//==============================================================================

FUNCTION FightStyle_ApplyOnSpawn(playerid)
{
	if(IsPlayerNPC(playerid)) return 0;
	new style = PlayerInfo[playerid][pFightStyle];
	// Guard a garbage/legacy value -> fall back to NORMAL. Raw SA-MP ids: NORMAL 4,
	// BOXING 5, KUNGFU 6, KNEEHEAD 7, GRABKICK 15, ELBOW 16 (the FIGHT_STYLE_* macros).
	switch(style)
	{
		case 4, 5, 6, 7, 15, 16: {}
		default: style = FIGHTSTYLE_DEFAULT;
	}
	SetPlayerFightingStyle(playerid, FIGHT_STYLE:style);
	return 1;
}

//==============================================================================
//	Command - /fightstyle (/fs /gym) - set a fighting style at a gym (§9.8)
//==============================================================================

CMD:fightstyle(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(IsPlayerInAnyVehicle(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Get out of your vehicle to use the gym.");
	if(!FightStyle_AtGym(playerid))
		return SendClientMessage(playerid, COLOR_ERROR, "You must be at a gym (Ganton, Garcia or Redsands East) to change your fighting style.");

	new list[512], row[96];
	for(new i = 0; i < sizeof(g_FightStyles); i++)
	{
		new bool:isCurrent = (g_FightStyles[i][fsId] == PlayerInfo[playerid][pFightStyle]);
		if(g_FightStyles[i][fsFee] > 0)
			format(row, sizeof(row), "%s%s\t"COL_GREEN"$%d\n", isCurrent ? (""COL_GREEN) : (""COL_WHITE), g_FightStyles[i][fsName], g_FightStyles[i][fsFee]);
		else
			format(row, sizeof(row), "%s%s\t"COL_WHITE"free\n", isCurrent ? (""COL_GREEN) : (""COL_WHITE), g_FightStyles[i][fsName]);
		strcat(list, row, sizeof(list));
	}
	ShowPlayerDialog(playerid, FIGHTSTYLE_DIALOG, DIALOG_STYLE_TABLIST, ""COL_WHITE"Gym - Fighting Style\tFee", list, "Set", "Cancel");
	return 1;
}
alias:fightstyle("fs", "gym")

// /fightstyle resolution (routed from OnDialogResponse in CnR.pwn).
FUNCTION FightStyle_OnDialogResponse(playerid, dialogid, response, listitem)
{
	if(dialogid != FIGHTSTYLE_DIALOG) return 0;
	if(!response) return 1;	// Cancel
	if(listitem < 0 || listitem >= sizeof(g_FightStyles)) return 1;

	// Must still be at a gym on confirm (anti-teleport / re-check).
	if(!FightStyle_AtGym(playerid))
		return SendClientMessage(playerid, COLOR_ERROR, "You have left the gym.");

	new style = g_FightStyles[listitem][fsId], fee = g_FightStyles[listitem][fsFee];
	if(style == PlayerInfo[playerid][pFightStyle])
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "That is already your fighting style.");

	if(fee > 0)
	{
		if(PlayerInfo[playerid][pMoney] < fee)
		{
			new err[128];
			format(err, sizeof(err), "That fighting style costs "COL_GREEN"$%d"COL_WHITE" cash - you cannot afford it.", fee);
			return SendClientMessage(playerid, COLOR_ERROR, err);
		}
		if(!Bank_TakeCash(playerid, fee)) return SendClientMessage(playerid, COLOR_ERROR, "Payment failed.");
	}

	PlayerInfo[playerid][pFightStyle] = style;
	SetPlayerFightingStyle(playerid, FIGHT_STYLE:style);
	Jail_SavePlayer(playerid);	// persist FightStyle via the shared LSplayers UPDATE

	new msg[160];
	format(msg, sizeof(msg), ""COL_GREEN"* Fighting style set to "COL_WHITE"%s"COL_GREEN"."COL_WHITE, g_FightStyles[listitem][fsName]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}
