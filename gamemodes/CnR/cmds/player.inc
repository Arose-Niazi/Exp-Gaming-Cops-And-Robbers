/*

	Player commands (/ad adrenaline, /advert paid ad, /radio) for Cops And
	Robbers script.
	Scripted by Arose Niazi.
	Milestone 4 (stage 2) — design §7 (/ad, /advert, /radio) + §4.2/§4.3
	(adrenaline = full heal + STD cure-all + overdose clear) + jobs §4.3 (radio).

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
