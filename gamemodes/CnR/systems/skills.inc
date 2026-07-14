/*

	Skills system for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 6 (stage 1) - design §9.8 (skills & fighting styles) + jobs-progression
	§1.2/§1.3 (the CB skin->skill model - any civilian skin can pick any civilian
	skill at City Hall) + crazybobs-site-full §Skills.

	The CIVILIAN SKILL is a separate field from the skin (pSkill / LSplayers.Skill).
	Chosen at CITY HALL via /skill (/skills). The skill's EFFECT is already coded in
	the M3-M5 crime systems - this module is the data change that lets a player set
	pSkill so those effects light up:

	  - SKILL_PICKPOCKET / SKILL_CONARTIST  -> /rob cash cap + success  (cmds/crime.inc Crime_RobCapFor/Crime_RobSuccessFor)
	  - SKILL_RAPIST                        -> /rape success + infect   (cmds/crime.inc)
	  - SKILL_DRUGDEALER                    -> drug carry cap (2500g, 5000g w/ bag)  (systems/drugs.inc DrugCarryCap)
	  - SKILL_KIDNAPPER                     -> ransom scaling + grab bonus  (systems/kidnap.inc Kidnap_RansomMaxFor)
	  - SKILL_CARJACKER                     -> defeats vehicle locks  (systems/vehicle_own.inc Veh_RefreshLockForPlayer/Veh_OnPlayerStateChange)
	  - SKILL_HITMAN/ARMSDEALER/STREETVENDOR/PROSTITUTE/FOODDELIVERY - choosable now;
	    their vendor/contract systems land in M7 (choice persists so it is ready).

	Weapon skills (§9.8 resolved decision - CLASSIC FEEL): every weapon skill is set
	to WEAPON_SKILL_LEVEL (999, Hitman tier) on spawn (Skills_ApplyOnSpawn), so gunplay
	feels like classic CB regardless of the chosen civilian skill (jobs §2.1 [DESIGN]).

	Re-pick (jobs §1.2): the FIRST skill pick is free; CHANGING an existing skill costs
	SKILL_CHANGE_COST (via Bank_TakeCash) and is rate-limited by SKILL_CHANGE_CD.

	Public API:
		Skills_ApplyOnSpawn(playerid)                    : set all weapon skills to max on spawn (§9.8/§2.1)
		Skills_Name(skill, out[], len)                   : human name for a pSkill id (used by /stats etc.)
		Skills_OnDialogResponse(playerid, dialogid, response, listitem) : /skill picker resolution

	Commands:
		/skill (/skills /skl)  (civ, on-foot at City Hall)  choose / change the civilian skill

	Functions Being Called:
		misc:     IsCop, IsCivilian
		bank:     Bank_TakeCash
		drugs:    Drug_SavePlayer (persists Skill + HasDrugBag)
		vehicle:  Veh_OnConnect (re-push vehicle door locks after a carjacker skill change)
		actors:   ActorInfo / SetupActor proximity (City Hall actor type)
*/

//==============================================================================
//	Skill table (the choosable CIVILIAN skills - City Hall menu, jobs §1.2)
//==============================================================================

// A choosable civilian skill: its pSkill id + display name + one-line perk blurb.
// Cop-family skills are NOT here - they follow the team/agency (jobs §1.3). Order
// matches the /skill dialog rows; the id is the value written to pSkill.
enum e_SkillDef
{
	skId,				// pSkill id (SKILL_*)
	skName[24],			// display name
	skDesc[64]			// short perk description (shown in the picker)
};

static const g_SkillDefs[][e_SkillDef] =
{
	{ SKILL_NONE,        "None",         "No specialisation (base rob/rape/drug behaviour)" },
	{ SKILL_PICKPOCKET,  "Pickpocket",   "Very high /rob success, cap $100,000" },
	{ SKILL_CONARTIST,   "Con Artist",   "/rob up to $500,000, moderate success" },
	{ SKILL_RAPIST,      "Rapist",       "Higher /rape success and infection chance" },
	{ SKILL_DRUGDEALER,  "Drug Dealer",  "Carry 2500g drugs (5000g with a drug bag)" },
	{ SKILL_KIDNAPPER,   "Kidnapper",    "/kidnap ransom scaling + bigger grab bonus" },
	{ SKILL_CARJACKER,   "Car Jacker",   "Steal LOCKED cars (defeat vehicle locks)" },
	{ SKILL_HITMAN,      "Hitman",       "Fulfil hit contracts (system in M7)" },
	{ SKILL_ARMSDEALER,  "Arms Dealer",  "Sell weapons to civilians (system in M7)" },
	{ SKILL_STREETVENDOR,"Street Vendor","Sell items to anyone incl. cops (M7)" },
	{ SKILL_PROSTITUTE,  "Prostitute",   "/sex + strip for tips (system in M7)" },
	{ SKILL_FOODDELIVERY,"Food Delivery","Sell food/drinks (system in M7)" }
};

//==============================================================================
//	Helpers
//==============================================================================

// Human name for a pSkill id (for /stats, messages, /fakeskill later). Falls back
// to "None" for an unknown id.
FUNCTION Skills_Name(skill, out[], len)
{
	for(new i = 0; i < sizeof(g_SkillDefs); i++)
	{
		if(g_SkillDefs[i][skId] == skill)
		{
			format(out, len, "%s", g_SkillDefs[i][skName]);
			return 1;
		}
	}
	format(out, len, "None");
	return 0;
}

// Is the player standing at a City Hall actor (ActorType 2, jobs §1.3)? Returns 1 if
// in range of one (same interior/VW), 0 otherwise. If NO City Hall actor exists in
// the DB yet, returns -1 so the caller can allow the command anywhere with a note.
static Skills_AtCityHall(playerid)
{
	new bool:anyCityHall = false;
	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	for(new i = 0; i < MAX_ACTORS_USED; i++)
	{
		if(ActorInfo[i][ActorAID] == INVALID_ACTOR_ID) continue;
		if(ActorInfo[i][ActorType] != 2) continue;	// 2 = City Hall (ActorTypeName index)
		anyCityHall = true;
		if(GetPlayerInterior(playerid) != ActorInfo[i][ActorInt]) continue;
		if(GetPlayerVirtualWorld(playerid) != ActorInfo[i][ActorVW]) continue;
		if(IsPlayerInRangeOfPoint(playerid, SKILL_ACTOR_RANGE, ActorInfo[i][ActorSpawnX], ActorInfo[i][ActorSpawnY], ActorInfo[i][ActorSpawnZ]))
			return 1;
	}
	return anyCityHall ? 0 : -1;	// -1 = no City Hall actor placed -> allow anywhere (note)
}

// Commit a skill choice: set pSkill, persist (via the crime/drug UPDATE which owns
// Skill), re-push vehicle door locks (a carjacker change flips lock-out eligibility),
// and confirm to the player. `charged` prints whether a re-pick fee was taken.
static Skills_Set(playerid, skill, bool:charged)
{
	PlayerInfo[playerid][pSkill] = skill;
	PlayerInfo[playerid][pSkillChangeCd] = gettime() + SKILL_CHANGE_CD;
	Drug_SavePlayer(playerid);	// persists Skill + HasDrugBag (no competing query)

	// A carjacker gaining/losing the skill changes whether locked cars lock them out,
	// so re-push every locked vehicle's client door state to them right away (§9.12).
	Veh_OnConnect(playerid);

	new name[24];
	Skills_Name(skill, name, sizeof(name));
	new msg[176];
	if(charged)
		format(msg, sizeof(msg), ""COL_GREEN"* Skill changed to "COL_WHITE"%s"COL_GREEN" ($%d)."COL_WHITE" Its perk is now active.", name, SKILL_CHANGE_COST);
	else
		format(msg, sizeof(msg), ""COL_GREEN"* Skill set to "COL_WHITE"%s"COL_GREEN"."COL_WHITE" Its perk is now active.", name);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}

//==============================================================================
//	Weapon skills - classic feel (§9.8 resolved / §2.1)
//==============================================================================

// Max out every weapon skill on spawn so gunplay feels classic (Hitman tier),
// independent of the chosen civilian skill. Called from OnPlayerSpawn (CnR.pwn).
FUNCTION Skills_ApplyOnSpawn(playerid)
{
	if(IsPlayerNPC(playerid)) return 0;
	// SA-MP weapon-skill ids 0..10 (Pistol..Sniper). 999 = Hitman tier (§2.1). The
	// native takes a WEAPONSKILL:-tagged id, so tag the loop counter at the call.
	for(new s = 0; s <= 10; s++) SetPlayerSkillLevel(playerid, WEAPONSKILL:s, WEAPON_SKILL_LEVEL);
	return 1;
}

//==============================================================================
//	Command - /skill (/skills /skl) - choose / change the civilian skill (§9.8)
//==============================================================================

CMD:skill(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	// Skills are the CIVILIAN specialisation family (jobs §1.3) - cops follow their
	// agency and cannot pick a civilian crime skill.
	if(IsCop(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Law enforcement skills follow your agency - civilian skills are not selectable.");
	if(!IsCivilian(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Only civilians can choose a skill.");
	if(IsPlayerInAnyVehicle(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Get out of your vehicle to talk to the City Manager.");

	// City Hall proximity (jobs §1.2 "change skill at City Hall"). -1 = no City Hall
	// actor is placed in the DB yet, so we allow it anywhere with a note (§9.8).
	new atHall = Skills_AtCityHall(playerid);
	if(atHall == 0)
		return SendClientMessage(playerid, COLOR_ERROR, "You must be at City Hall (talk to the City Manager) to change your skill.");
	// atHall == 1 (at a hall) or == -1 (no hall placed -> allowed anywhere, noted) both proceed.

	// Re-pick cooldown (§9.8) - only bites once a skill is already chosen.
	if(PlayerInfo[playerid][pSkill] != SKILL_NONE && gettime() < PlayerInfo[playerid][pSkillChangeCd])
	{
		new err[128];
		format(err, sizeof(err), "You changed your skill recently - wait %d seconds before changing it again.", PlayerInfo[playerid][pSkillChangeCd] - gettime());
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}

	// Build the picker. The first pick is FREE; changing an existing skill costs
	// SKILL_CHANGE_COST (charged on selection). Current skill is flagged in the list.
	new list[1024], row[96], curName[24];
	Skills_Name(PlayerInfo[playerid][pSkill], curName, sizeof(curName));
	for(new i = 0; i < sizeof(g_SkillDefs); i++)
	{
		new bool:isCurrent = (g_SkillDefs[i][skId] == PlayerInfo[playerid][pSkill]);
		format(row, sizeof(row), "%s%s\t%s\n", isCurrent ? (""COL_GREEN) : (""COL_WHITE), g_SkillDefs[i][skName], g_SkillDefs[i][skDesc]);
		strcat(list, row, sizeof(list));
	}

	new caption[160];
	if(PlayerInfo[playerid][pSkill] == SKILL_NONE)
		format(caption, sizeof(caption), ""COL_WHITE"Pick your skill (first choice is "COL_GREEN"free"COL_WHITE"). Current: "COL_GREEN"%s"COL_WHITE, curName);
	else
		format(caption, sizeof(caption), ""COL_WHITE"Change your skill (changing costs "COL_GREEN"$%d"COL_WHITE"). Current: "COL_GREEN"%s"COL_WHITE, SKILL_CHANGE_COST, curName);
	ShowPlayerDialog(playerid, SKILL_CHANGE_DIALOG, DIALOG_STYLE_TABLIST, caption, list, "Choose", "Cancel");
	return 1;
}
alias:skill("skills", "skl")

// /skill picker resolution (routed from OnDialogResponse in CnR.pwn).
FUNCTION Skills_OnDialogResponse(playerid, dialogid, response, listitem)
{
	if(dialogid != SKILL_CHANGE_DIALOG) return 0;
	if(!response) return 1;	// Cancel
	if(listitem < 0 || listitem >= sizeof(g_SkillDefs)) return 1;

	new pick = g_SkillDefs[listitem][skId];

	// Re-selecting the current skill is a no-op (no fee).
	if(pick == PlayerInfo[playerid][pSkill])
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "That is already your skill.");

	// First pick (currently SKILL_NONE) is free; changing an existing one is charged.
	if(PlayerInfo[playerid][pSkill] == SKILL_NONE)
	{
		Skills_Set(playerid, pick, false);
		return 1;
	}

	if(PlayerInfo[playerid][pMoney] < SKILL_CHANGE_COST)
	{
		new err[128];
		format(err, sizeof(err), "Changing your skill costs "COL_GREEN"$%d"COL_WHITE" cash - you cannot afford it.", SKILL_CHANGE_COST);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	if(!Bank_TakeCash(playerid, SKILL_CHANGE_COST)) return SendClientMessage(playerid, COLOR_ERROR, "Payment failed.");
	Skills_Set(playerid, pick, true);
	return 1;
}
