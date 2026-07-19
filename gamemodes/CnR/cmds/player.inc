/*

	Player commands (/ad adrenaline, /advert paid ad, /radio) for Cops And
	Robbers script.
	Scripted by Arose Niazi.
	Milestone 4 (stage 2) — design §7 (/ad, /advert, /radio) + §4.2/§4.3
	(adrenaline = full heal + STD cure-all + overdose clear) + jobs §4.3 (radio).
	Stage D (FEATURE-EXPANSION-PLAN §4) — social / info command layer:
		/pinfo (/i)      — player class/skill/level/zone/wanted (self or by id)
		/getid (/id)     — resolve a partial name to player id(s)
		/stats (/sts)    — money, bank, score, wanted, cop rank, robbery record
		/complain        — civilian cheater/rule report to on-duty staff
		/locate (/loc)   — show a player's current zone
		/time (/day)     — show current game day and clock

	§7 DECISION (recorded): the roster's /ad is CB "adrenaline", NOT the paid ad.
	We keep /ad = adrenaline and implement the paid advertisement as /advert.

	/ad — consume one carried adrenaline pill: full heal, cure ALL STDs
	(STD_CureAll), clear a drug overdose (Drug_ClearOverdose). Carry up to
	ADRENALINE_MAX. Acquisition (buy at a hospital/shop) lands with the shop
	economy — for now a small starter grant seeds the pill on first use so the
	cure path is testable (see the acquisition note on the command).

	/advert [text] — a paid, server-wide coloured advertisement line. Costs
	ADVERT_COST (via Bank_TakeCash), ADVERT_COOLDOWN cooldown, capped at
	ADVERT_MAX_CHARS chars, blocks URLs and other-server-name patterns.

	/radio — a dialog of preset stream URLs (PlayAudioStreamForPlayer); the last
	entry stops the stream (StopAudioStreamForPlayer).

	Functions Being Called:
		bank:  Bank_TakeCash
		std:   STD_CureAll, STD_HasAny
		drugs: Drug_ClearOverdose
		misc:  IsCop, Rob_SuccessChance (used indirectly via pSuccessfulRobberies)
		messages: SendMessageToModerators (/complain admin-chat sink)
*/

//==============================================================================
//	/ad — adrenaline pill (full heal + cure-all + OD clear) (design §7 / §4.2/4.3)
//==============================================================================

CMD:ad(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");

	// ACQUISITION DEFERRED: buying adrenaline at a hospital/shop lands with the
	// shop economy (M4 shop / M6). So the carry path can be tested now, the very
	// first /ad seeds one starter pill; thereafter you must have one carried.
	if(PlayerInfo[playerid][pAdrenaline] <= 0)
	{
		if(PlayerInfo[playerid][pAdrenaline] == 0 && ADRENALINE_START > 0)
		{
			PlayerInfo[playerid][pAdrenaline] = ADRENALINE_START;
			SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_YELLOW"* (Starter) "COL_WHITE"You were given an adrenaline pill. Buying more at a hospital/shop comes with the shop system.");
		}
		else return SendClientMessage(playerid, COLOR_ERROR, "You have no adrenaline pills. Buy some at a hospital/shop (coming with the shop system).");
	}

	// Spend one pill and apply the full effect.
	PlayerInfo[playerid][pAdrenaline]--;
	SetPlayerHealth(playerid, 100.0);
	new bool:hadStd = (STD_HasAny(playerid) != 0);
	new bool:hadOd = PlayerInfo[playerid][pDrugOverdose];
	STD_CureAll(playerid, "adrenaline pill");
	Drug_ClearOverdose(playerid, "adrenaline pill");

	new string[176];
	format(string, sizeof(string), ""COL_GREEN"* Adrenaline pill used: "COL_WHITE"full health%s%s. "COL_GREEN"Pills left: "COL_WHITE"%d"COL_GREEN"/%d.",
		hadStd ? (", all disease cured") : (""), hadOd ? (", overdose stopped") : (""), PlayerInfo[playerid][pAdrenaline], ADRENALINE_MAX);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	return 1;
}

//==============================================================================
//	/advert [text] — paid server-wide coloured advertisement (design §7)
//==============================================================================

// True if `text` looks like a URL or another-server address (blocked, §7). Case
// is folded by strfind's default (ignorecase = true).
static bool:Advert_IsBlocked(const text[])
{
	if(strfind(text, "http", true) != -1) return true;
	if(strfind(text, "www.", true) != -1) return true;
	if(strfind(text, ".com", true) != -1) return true;
	if(strfind(text, ".net", true) != -1) return true;
	if(strfind(text, ".org", true) != -1) return true;
	if(strfind(text, ".gg", true) != -1) return true;
	if(strfind(text, "discord", true) != -1) return true;
	// An IP:port style server address (a colon between digits) reads as another
	// server — block a raw port marker.
	if(strfind(text, ":7777", true) != -1) return true;
	if(strfind(text, ":7778", true) != -1) return true;
	return false;
}

CMD:advert(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(isNull(params))
	{
		new usage[128];
		format(usage, sizeof(usage), "USAGE: /advert [text]  ("COL_GREEN"$%d"COL_WHITE" per ad, %ds cooldown, max %d chars)", ADVERT_COST, ADVERT_COOLDOWN, ADVERT_MAX_CHARS);
		return SendClientMessage(playerid, COLOR_SERVER_INFO, usage);
	}
	if(gettime() < PlayerInfo[playerid][pAdvertCooldown])
	{
		new err[96];
		format(err, sizeof(err), "You can advertise again in %d seconds.", PlayerInfo[playerid][pAdvertCooldown] - gettime());
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	if(strlen(params) > ADVERT_MAX_CHARS)
	{
		new err[96];
		format(err, sizeof(err), "Keep your advert under %d characters.", ADVERT_MAX_CHARS);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	if(Advert_IsBlocked(params))
		return SendClientMessage(playerid, COLOR_ERROR, "Adverts may not contain links or other server addresses.");

	// Charge the fee first (nothing is broadcast if they cannot pay).
	if(!Bank_TakeCash(playerid, ADVERT_COST))
	{
		new err[96];
		format(err, sizeof(err), "You cannot afford to advertise ($%d cash).", ADVERT_COST);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}

	PlayerInfo[playerid][pAdvertCooldown] = gettime() + ADVERT_COOLDOWN;

	new string[192];
	format(string, sizeof(string), ""COL_BLUE"[ADVERT] "COL_WHITE"%s"COL_BLUE"  ~  %s (%d)", params, PlayerInfo[playerid][pUserName], playerid);
	SendClientMessageToAll(COLOR_SERVER_INFO, string);
	// Log to the future Discord bridge (retired IRC no-op today).
	format(string, sizeof(string), "[ADVERT] %s (%d): %s", PlayerInfo[playerid][pUserName], playerid, params);
	IRC_SendMessage(string, IRC_NORMAL_COLOR);
	return 1;
}

//==============================================================================
//	/radio — preset stream URL picker (design §7 / jobs §4.3)
//==============================================================================

// Preset stream URLs. The last option stops the stream. These are public
// SHOUTcast/streaming endpoints; swap them for the community's stations later.
static const gRadioNames[][] =
{
	"Bassdrive (Drum & Bass)",
	"SomaFM Groove Salad (Chill)",
	"Radio Paradise (Eclectic)",
	"SomaFM Secret Agent (Lounge)",
	"181.fm The Beat (Hip-Hop)",
	"Stop the radio"
};
static const gRadioUrls[][] =
{
	"http://ice.somafm.com/bassdrive",
	"http://ice.somafm.com/groovesalad",
	"http://stream.radioparadise.com/mp3-128",
	"http://ice.somafm.com/secretagent",
	"http://listen.181fm.com/181-beat_128k.mp3",
	""
};

CMD:radio(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new list[512];
	for(new i = 0; i < sizeof(gRadioNames); i++)
	{
		strcat(list, gRadioNames[i]);
		strcat(list, "\n");
	}
	ShowPlayerDialog(playerid, RADIO_DIALOG, DIALOG_STYLE_LIST, "Radio", list, "Play", "Close");
	return 1;
}
alias:radio("cnrradio")

// Routed from OnDialogResponse in CnR.pwn.
FUNCTION Radio_OnDialogResponse(playerid, response, listitem)
{
	if(!response) return 1;
	if(listitem < 0 || listitem >= sizeof(gRadioUrls)) return 1;
	// The last entry (empty URL) stops the stream.
	if(isNull(gRadioUrls[listitem]))
	{
		StopAudioStreamForPlayer(playerid);
		SendClientMessage(playerid, COLOR_SERVER_INFO, "Radio stopped.");
		return 1;
	}
	PlayAudioStreamForPlayer(playerid, gRadioUrls[listitem]);
	new string[128];
	format(string, sizeof(string), ""COL_GREEN"* Now playing: "COL_WHITE"%s"COL_GREEN". Use "COL_WHITE"/radio"COL_GREEN" and pick \"Stop\" to end it.", gRadioNames[listitem]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	return 1;
}

//==============================================================================
//	Stage D §4 — /pinfo (/i) : player class/skill/level/zone/wanted
//	Blank arg = self; optional id = that player.
//	Name: /pinfo not /info — /info is reserved for the Stage C help browser.
//==============================================================================

CMD:pinfo(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new targetid;
	if(isNull(params))
		targetid = playerid;
	else
	{
		if(sscanf(params, "u", targetid))
			return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /pinfo [playerid]  (blank = yourself)");
	}
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || !PlayerInfo[targetid][pLoggedIn])
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected/logged in.");

	// Skill label (mirrors skills.inc g_SkillReg[] without an extra include).
	new skillName[16];
	switch(PlayerInfo[targetid][pSkill])
	{
		case SKILL_PICKPOCKET: format(skillName, sizeof(skillName), "Pickpocket");
		case SKILL_CONARTIST:  format(skillName, sizeof(skillName), "Con Artist");
		case SKILL_RAPIST:     format(skillName, sizeof(skillName), "Rapist");
		case SKILL_DRUGDEALER: format(skillName, sizeof(skillName), "Drug Dealer");
		default:               format(skillName, sizeof(skillName), "None");
	}

	new string[192];
	format(string, sizeof(string),
		""COL_YELLOW"[Info] "COL_WHITE"%s (%d)  "COL_YELLOW"Class: "COL_WHITE"%s  "COL_YELLOW"Skill: "COL_WHITE"%s  "COL_YELLOW"Score: "COL_WHITE"%d",
		PlayerInfo[targetid][pUserName], targetid,
		IsCop(targetid) ? "Cop" : "Civilian",
		skillName, PlayerInfo[targetid][pScore]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);

	// Zone: use a temp string to avoid a ternary-on-array in format() (Pawn limitation).
	new zone[MAX_ZONE_NAME];
	if(PlayerInfo[targetid][pZoneName][0])
		format(zone, sizeof(zone), "%s", PlayerInfo[targetid][pZoneName]);
	else
		format(zone, sizeof(zone), "Unknown");

	format(string, sizeof(string),
		""COL_YELLOW"Zone: "COL_WHITE"%s  "COL_YELLOW"Wanted: "COL_WHITE"%d",
		zone, PlayerInfo[targetid][pWanted]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	return 1;
}
alias:pinfo("i")

//==============================================================================
//	Stage D §4 — /getid (/id) : resolve a partial name to player id(s)
//	If one match: print "Name (id)".  If multiple: show a dialog list.
//==============================================================================

CMD:getid(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new fragment[MAX_PLAYER_NAME];
	if(sscanf(params, "s[24]", fragment))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /getid [partial name]");
	if(isNull(fragment)) return SendClientMessage(playerid, COLOR_ERROR, "Enter a name fragment to search.");

	new matches[MAX_PLAYERS], count = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || !PlayerInfo[i][pLoggedIn]) continue;
		if(strfind(PlayerInfo[i][pUserName], fragment, true) != -1)
		{
			matches[count] = i;
			count++;
		}
	}

	if(!count)
	{
		new err[96];
		format(err, sizeof(err), "No connected players match \"%s\".", fragment);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}

	if(count == 1)
	{
		new string[96];
		format(string, sizeof(string), ""COL_YELLOW"Match: "COL_WHITE"%s (%d).", PlayerInfo[matches[0]][pUserName], matches[0]);
		return SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	}

	// Multiple matches — show a dialog list.
	new list[1024], row[64], title[48];
	for(new j = 0; j < count; j++)
	{
		format(row, sizeof(row), "%s (%d)\n", PlayerInfo[matches[j]][pUserName], matches[j]);
		strcat(list, row, sizeof(list));
	}
	format(title, sizeof(title), "Matches for \"%s\" (%d)", fragment, count);
	ShowPlayerDialog(playerid, ADMIN_INFO_DIALOG, DIALOG_STYLE_LIST, title, list, "OK", "");
	return 1;
}
alias:getid("id")

//==============================================================================
//	Stage D §4 — /stats (/sts) : current-life stats
//	Shows money, bank, score, wanted, cop rank, and robbery record.
//	Blank = self; optional id = that player.
//==============================================================================

CMD:stats(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new targetid;
	if(isNull(params))
		targetid = playerid;
	else
	{
		if(sscanf(params, "u", targetid))
			return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /stats [playerid]  (blank = yourself)");
	}
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || !PlayerInfo[targetid][pLoggedIn])
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected/logged in.");

	new string[192];
	format(string, sizeof(string),
		""COL_YELLOW"[Stats] "COL_WHITE"%s (%d)  "COL_YELLOW"Cash: "COL_WHITE"$%d  "COL_YELLOW"Bank: "COL_WHITE"$%d  "COL_YELLOW"Score: "COL_WHITE"%d",
		PlayerInfo[targetid][pUserName], targetid,
		PlayerInfo[targetid][pMoney], PlayerInfo[targetid][pBankMoney], PlayerInfo[targetid][pScore]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);

	new rankStr[20];
	if(IsCop(targetid))
		format(rankStr, sizeof(rankStr), "%d", PlayerInfo[targetid][pCopRank]);
	else
		format(rankStr, sizeof(rankStr), "N/A");

	format(string, sizeof(string),
		""COL_YELLOW"Wanted: "COL_WHITE"%d  "COL_YELLOW"Cop Rank: "COL_WHITE"%s  "COL_YELLOW"Robberies: "COL_WHITE"%d ok / %d fail",
		PlayerInfo[targetid][pWanted], rankStr,
		PlayerInfo[targetid][pSuccessfulRobberies], PlayerInfo[targetid][pFailedRobberies]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);

	// Success-rate hint (avoid division-by-zero).
	new total = PlayerInfo[targetid][pSuccessfulRobberies] + PlayerInfo[targetid][pFailedRobberies];
	if(total > 0)
	{
		new pct = (PlayerInfo[targetid][pSuccessfulRobberies] * 100) / total;
		format(string, sizeof(string), ""COL_YELLOW"Rob success rate: "COL_WHITE"%d%%  "COL_YELLOW"(of %d attempts)", pct, total);
		SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	}
	return 1;
}
alias:stats("sts")

//==============================================================================
//	Stage D §4 — /complain : civilian report to on-duty staff
//	Feeds the same admin-chat sink that /a uses (SendMessageToModerators).
//	Distinct from cop-only /report.
//==============================================================================

CMD:complain(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(isNull(params)) return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /complain [text]  (report a cheater or rule-breaker to staff)");

	new string[192];
	format(string, sizeof(string), "[COMPLAINT] %s (%d): %s", PlayerInfo[playerid][pUserName], playerid, params);
	SendMessageToModerators(COLOR_ADMIN_INFO, string);

	SendClientMessage(playerid, COLOR_SERVER_INFO, "Your complaint has been sent to on-duty staff. Thank you.");
	return 1;
}

//==============================================================================
//	Stage D §4 — /locate (/loc) : show a player's current zone
//	Blank = self; optional id = that player.
//==============================================================================

CMD:locate(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new targetid;
	if(isNull(params))
		targetid = playerid;
	else
	{
		if(sscanf(params, "u", targetid))
			return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /locate [playerid]  (blank = yourself)");
	}
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || !PlayerInfo[targetid][pLoggedIn])
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected/logged in.");

	new zone[MAX_ZONE_NAME], string[128];
	if(PlayerInfo[targetid][pZoneName][0])
		format(zone, sizeof(zone), "%s", PlayerInfo[targetid][pZoneName]);
	else
		format(zone, sizeof(zone), "Unknown");

	if(targetid == playerid)
		format(string, sizeof(string), ""COL_YELLOW"Your location: "COL_WHITE"%s.", zone);
	else
		format(string, sizeof(string), ""COL_YELLOW"Location of %s (%d): "COL_WHITE"%s.", PlayerInfo[targetid][pUserName], targetid, zone);

	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	return 1;
}
alias:locate("loc")

//==============================================================================
//	Stage D §4 — /time (/day) : show current game day and clock
//	Reads the game-clock globals (GameDay, GameHour, GameMinute, WeekDays[]).
//==============================================================================

CMD:time(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");

	// WeekDays[] is declared after the cmds/ includes in CnR.pwn, so we keep a
	// local copy here rather than referencing the global directly.
	static const s_DayNames[7][] = {"Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"};
	new day = GameDay;
	if(day < 0 || day > 6) day = 0;

	new string[96];
	format(string, sizeof(string), ""COL_YELLOW"Game time: "COL_WHITE"%s, %02d:%02d", s_DayNames[day], GameHour, GameMinute);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	return 1;
}
alias:time("day")
