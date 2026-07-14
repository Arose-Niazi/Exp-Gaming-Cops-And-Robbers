/*

	Cops and Robbers Script — open.mp edition (SA-MP 0.3.7 clients).
	Scripted by Arose Niazi.
	Scripting started on 11th October 2017
	Ported to open.mp: July 2026 (legacy SA-MP tree preserved in the
	v0.3.7-legacy tag/release). The IRC bridge was retired in the port.

	Credits:
		SA-MP team / open.mp team
		Zeex for crashdetect
		Incognito for streamer
		maddinat0r for mysql, sscanf2 and discord connector
		Whitetiger for geolocation
		Y_Less for whirlpool, sscanf2
		emmet_ for sscanf2
		YourShadow for Pawn.CMD
		Gammer_Z for RouteConnector

	Functions:
		
		
	Functions Being Called:
		player_vars script;
			ClearPlayerVariables(playerid);
			
		Zones script;
			SetupZones();
			DestroyZones();
			ZoneCreateTD(playerid);
			ZoneDestroyTD(playerid);
			ZoneShowTD(playerid);
			ZoneHideTD(playerid);
			OnPlayerEnterZone(playerid,areaid);
			OnPlayerLeaveZone(playerid,areaid);
			
		Objects;
			RemoveObjectsForPlayer(playerid);
			CreateMap();
		
		peds;
			SetupPeds();
			CreateClassTD(playerid);
			DestroyClassTD(playerid);
			ShowPlayerClassTD(playerid,classid);
			HidePlayerClassTD(playerid);
		
		messages;
			TimeUpdateMessage(color,text); 
			SendMessageToModerators(color, text);
			SendConnectMessage(playerid,text,show); 
			IRC_SendMessage(message,color);
	
*/


//==============================================================================
// Includes
//==============================================================================

// Toggle the MySQL credential set in CnR/server/db_config.inc
// (comment out to use the remote / production credentials instead).
#define LOCAL_DB

#include <open.mp>
#include <crashdetect>
#include <streamer>
#include <sscanf2>
#include <a_mysql>
#include <geolocation>
#include <Pawn.CMD>
#include <RouteConnector>

//==============================================================================
//	Internal definitions and natives
//==============================================================================

#if defined WINDOWS_COMPILER
	#include "CnR\server\defines"
	#include "CnR\server\db_config"
#else
	#include "CnR/server/defines"
	#include "CnR/server/db_config"
#endif

native WP_Hash(buffer[],len,const str[]);

#define VERSION 					"0.01"
#define STATS_VERSION 				0

main()
{	
	print("\n");
	print("  |---------------------------------------------------");
	print("  |--- Cops and Robbers");
    printf("  |--  Script v%s",VERSION);
    print("  |--  Arose Niazi");
	print("  |---------------------------------------------------");
}

//==============================================================================
//	Global Variable and includes
//==============================================================================

#define CITY_ LS

#if CITY_ == LS
	#define CITY_NAME "Los Santos"
	new CITY = LS;
	#define CITY_S "LS"
	#define TABLE_PLAYERS_C "LSplayers"
	#define TABLE_VEHICLES "LSvehicles"
#elseif CITY_ == SF
	#define CITY_NAME "San Fierro"
	new CITY = SF;
	#define CITY_S "SF"
	#define TABLE_PLAYERS_C "SFplayers"
	#define TABLE_VEHICLES "SFvehicles"
#else
	#define CITY_NAME "Los Venturas"
	new CITY = LV;
	#define CITY_S "LV"
	#define TABLE_PLAYERS_C "LVplayers"
	#define TABLE_VEHICLES "LVvehicles"
#endif
#pragma unused CITY

new Text:DaysOfWeek;
new GameDay = 0;
new GameHour = 0;
new GameMinute = 0;
new GameWeather = 0;
// M4 (stage 1) — monotonic game-day counter (never resets on week rollover) that
// drives the every-N-game-days bank-interest + tax schedule (design §10.3).
new g_GameDayCounter = 0;

new fine_weather_ids[] = {1,2,3,4,5,6,7,12,13,14,15,17,18,24,25,26,27,28,29,30,40};
new foggy_weather_ids[] = {9,19,20,31,32};
new wet_weather_ids[] = {8};


new MySQL:g_SQL;
new g_MysqlRaceCheck[MAX_PLAYERS];

new Text:ConnectTD[2];

#if defined WINDOWS_COMPILER
	// ---- M7 : stock-market CORE (config + E_STOCK_* enum + PRIME_RATE_* + the
	// StockMarket_* price/pool state/functions). Included FIRST so server_vars.inc
	// (prime-rate clamp) and every price site (housing/fishing/farming/shop) can
	// reference the enum + multiplier. The heavy machinery is systems/stocks.inc.
	#include "CnR\systems\economy_stub"
	#include "CnR\server\server_vars"
	#include "CnR\players\player_vars"
	#include "CnR\server\misc"
	#include "CnR\players\login_register"
	#include "CnR\players\bans"
	#include "CnR\server\zones"

	#include "CnR\players\peds"
	#include "CnR\players\objects"

	#include "CnR\players\messages"
	#include "CnR\players\loading_data"
	#include "CnR\players\spawns"
	#include "CnR\players\menu"
	#include "CnR\server\int_tele_pickups"
	#include "CnR\players\vehicles"
	#include "CnR\players\elevator_samp"
	#include "CnR\players\elevator_golden"

	#include "CnR\players\cmds"
	#include "CnR\server\actors"
	#include "CnR\server\gps"

	// ---- M2 : command / moderation infra ----
	#include "CnR\cmds\messages"
	#include "CnR\cmds\admin"
	#include "CnR\cmds\teleport"
	#include "CnR\cmds\owner"
	#include "CnR\cmds\scripter"
	#include "CnR\systems\bans"

	// ---- M3 : core CnR loop ----
	#include "CnR\systems\wanted"
	#include "CnR\systems\jail"
	#include "CnR\cmds\cop"
	// ---- M3 (stage 3) : player crime commands (std/drugs before crime.inc) ----
	#include "CnR\systems\std"
	#include "CnR\systems\drugs"
	#include "CnR\cmds\crime"
	// ---- M5 (stage 1) : vehicle ownership — CB-simple model (§9.12) ----
	#include "CnR\systems\vehicle_own"
	// ---- M4 (stage 1) : bank, taxes & insurance (the money core) ----
	#include "CnR\systems\bank"
	// ---- M4 (stage 2) : lotto, money events, /ad + /advert, holdup base ----
	#include "CnR\systems\lotto"
	#include "CnR\systems\moneybag"
	#include "CnR\systems\moneyrush"
	#include "CnR\systems\holdup"
	// ---- M5 (stage 3) : housing (before robbery — /houserob calls House_* publics) ----
	#include "CnR\systems\housing"
	// ---- M5 (stage 2) : full robbery system + crowbar (after holdup/bank/wanted/jail/housing) ----
	#include "CnR\systems\robbery"
	// ---- M4 (stage 3) : reusable mission framework + 4 core missions ----
	#include "CnR\systems\missions"
	// ---- M5 (stage 4) : kidnap crime + the 3 M5 robbery missions (§4.4/§6.1/§6.7/§6.19) ----
	#include "CnR\systems\kidnap"
	// ---- M6 (stage 1) : skills, fighting styles & clothes (§9.8/§9.7/§5.7) ----
	// (after robbery/drugs/kidnap/vehicle_own — clothes calls Robbery_BuyCrowbar,
	// skills calls Veh_OnConnect + Drug_SavePlayer.)
	#include "CnR\systems\skills"
	#include "CnR\systems\fightstyle"
	#include "CnR\systems\clothes"
	// ---- M6 (stage 2) : fishing & farming (§9.2/§9.3) ----
	// The StockMarket_* core (economy_stub) is now included EARLY (top of this branch)
	// so fishing/farming's StockMarket_UpdateEarnings/PriceMult call sites resolve.
	#include "CnR\systems\fishing"
	#include "CnR\systems\farming"
	// ---- M7 : stock market + full economy (the finale, §9.4/§10) ----
	// The core lives in economy_stub (included early); THIS is the machinery (init,
	// async CRUD, the daily tick, dividends, reports, trading commands). After
	// bank/jail (trading money via Bank_*, holdings persisted with the shared UPDATE).
	#include "CnR\systems\stocks"
	#include "CnR\cmds\player"
	// ---- M6 (stage 3) : DM/sniper/duel arenas, DJ radio (§9.6/§8.1) ----
	// (after teleport.inc — arenas reuse StripParachute; after bank/jail/missions/
	// wanted — arenas call Bank_*/Jail_IsJailed/Mission_IsOnMission/ClearPlayerWanted.)
	#include "CnR\systems\arenas"
	#include "CnR\cmds\dj"
#else
	// ---- M7 : stock-market CORE — see the WINDOWS_COMPILER branch note above.
	#include "CnR/systems/economy_stub"
	#include "CnR/server/server_vars"
	#include "CnR/players/player_vars"
	#include "CnR/server/misc"
	#include "CnR/players/login_register"
	#include "CnR/players/bans"
	#include "CnR/server/zones"

	#include "CnR/players/peds"
	#include "CnR/players/objects"

	#include "CnR/players/messages"
	#include "CnR/players/loading_data"
	#include "CnR/players/spawns"
	#include "CnR/players/menu"
	#include "CnR/server/int_tele_pickups"
	#include "CnR/players/vehicles"
	#include "CnR/players/elevator_samp"
	#include "CnR/players/elevator_golden"

	#include "CnR/players/cmds"
	#include "CnR/server/actors"
	#include "CnR/server/gps"

	// ---- M2 : command / moderation infra ----
	#include "CnR/cmds/messages"
	#include "CnR/cmds/admin"
	#include "CnR/cmds/teleport"
	#include "CnR/cmds/owner"
	#include "CnR/cmds/scripter"
	#include "CnR/systems/bans"

	// ---- M3 : core CnR loop ----
	#include "CnR/systems/wanted"
	#include "CnR/systems/jail"
	#include "CnR/cmds/cop"
	// ---- M3 (stage 3) : player crime commands (std/drugs before crime.inc) ----
	#include "CnR/systems/std"
	#include "CnR/systems/drugs"
	#include "CnR/cmds/crime"
	// ---- M5 (stage 1) : vehicle ownership — CB-simple model (§9.12) ----
	#include "CnR/systems/vehicle_own"
	// ---- M4 (stage 1) : bank, taxes & insurance (the money core) ----
	#include "CnR/systems/bank"
	// ---- M4 (stage 2) : lotto, money events, /ad + /advert, holdup base ----
	#include "CnR/systems/lotto"
	#include "CnR/systems/moneybag"
	#include "CnR/systems/moneyrush"
	#include "CnR/systems/holdup"
	// ---- M5 (stage 3) : housing (before robbery — /houserob calls House_* publics) ----
	#include "CnR/systems/housing"
	// ---- M5 (stage 2) : full robbery system + crowbar (after holdup/bank/wanted/jail/housing) ----
	#include "CnR/systems/robbery"
	// ---- M4 (stage 3) : reusable mission framework + 4 core missions ----
	#include "CnR/systems/missions"
	// ---- M5 (stage 4) : kidnap crime + the 3 M5 robbery missions (§4.4/§6.1/§6.7/§6.19) ----
	#include "CnR/systems/kidnap"
	// ---- M6 (stage 1) : skills, fighting styles & clothes (§9.8/§9.7/§5.7) ----
	// (after robbery/drugs/kidnap/vehicle_own — clothes calls Robbery_BuyCrowbar,
	// skills calls Veh_OnConnect + Drug_SavePlayer.)
	#include "CnR/systems/skills"
	#include "CnR/systems/fightstyle"
	#include "CnR/systems/clothes"
	// ---- M6 (stage 2) : fishing & farming (§9.2/§9.3) ----
	// The StockMarket_* core (economy_stub) is now included EARLY (top of this branch)
	// so fishing/farming's StockMarket_UpdateEarnings/PriceMult call sites resolve.
	#include "CnR/systems/fishing"
	#include "CnR/systems/farming"
	// ---- M7 : stock market + full economy (the finale, §9.4/§10) ----
	// The core lives in economy_stub (included early); THIS is the machinery (init,
	// async CRUD, the daily tick, dividends, reports, trading commands). After
	// bank/jail (trading money via Bank_*, holdings persisted with the shared UPDATE).
	#include "CnR/systems/stocks"
	#include "CnR/cmds/player"
	// ---- M6 (stage 3) : DM/sniper/duel arenas, DJ radio (§9.6/§8.1) ----
	// (after teleport.inc — arenas reuse StripParachute; after bank/jail/missions/
	// wanted — arenas call Bank_*/Jail_IsJailed/Mission_IsOnMission/ClearPlayerWanted.)
	#include "CnR/systems/arenas"
	#include "CnR/cmds/dj"
#endif

new WeekDays[7][] = {
	{"Monday"},
	{"Tuesday"},
	{"Wednesday"},
	{"Thursday"},
	{"Friday"},
	{"Saturday"},
	{"Sunday"}
};

//==============================================================================
//	Global callbacks 
//==============================================================================

public OnGameModeInit()
{
	SendRconCommand("gamemodetext Cops and Robbers");
	SendRconCommand("mapname "CITY_NAME"");
	g_SQL = mysql_connect(SQL_HOST, SQL_USER, SQL_PASS, SQL_DB);
	if (mysql_errno() != 0)	print("SQL: Failed to connect to server !");
	SetupZones();
	SetupPeds();
	CreateMap();
	LoadInterior();
	AddVehicles();
	Veh_Init();	// M5 (stage 1) — reset per-vehicle ownership/lock state for a fresh session (§9.12)
	Robbery_Init();	// M5 (stage 2) — create bank/casino/special robbery checkpoints + bank map icons (§5.3-5.6)
	House_Init();	// M5 (stage 3) — load all houses (async) + build entrance pickups/labels/map icons (§9.1)
	FightStyle_Init();	// M6 (stage 1) — create the 3 gym markers + map icons (Ganton/Garcia/Redsands East, §9.8)
	Clothes_Init();	// M6 (stage 1) — create clothes-shop + crowbar-vendor map icons (§9.7/§5.7)
	Fishing_Init();	// M6 (stage 2) — create Bait Shop / fishing-spot / fish-market map icons (§9.2)
	Farm_Init();	// M6 (stage 2) — load the plants table (async) + create refill-point/farm map icons (§9.3)
	Stocks_Init();	// M7 — load the stocks table (async); seed the 21-stock roster on first boot (§9.4/§11.3)
	mysql_log(ERROR | WARNING);
	EnableStuntBonusForAll(false); //Disabling stunt bonus.
	DisableInteriorEnterExits();  // will disable all interior enter/exits in the game.
	mysql_pquery(g_SQL, "UPDATE players SET Online=0 WHERE 1");
	
	GameDay = 0;
	GameHour = 0;
	GameMinute = 0;
	SetWorldTime(0);
	SetWeather(random(21));
	SendRconCommand("worldtime Sunday 00:00");
	ServerInfo[sTimer] = SetTimerEx("GameModeClock", 1000, true, "d", 0);
	new string[256];
	// M5 (stage 2) — pull the bank-robbery cooldown DATETIMEs as unix seconds so the
	// int cache-getter can read them (aliased BankRobUnix{LS,SF,LV}, §5.3/§5.8).
	mysql_format(g_SQL,string,sizeof(string),"SELECT *, UNIX_TIMESTAMP(BankRobLastLS) AS BankRobUnixLS, UNIX_TIMESTAMP(BankRobLastSF) AS BankRobUnixSF, UNIX_TIMESTAMP(BankRobLastLV) AS BankRobUnixLV FROM `server_data` WHERE Version=%d",STATS_VERSION);
	mysql_pquery(g_SQL, string, "OnServerDataLoad","");
	
	DaysOfWeek= TextDrawCreate(577.203552, 5.083323, WeekDays[GameDay]);
	TextDrawLetterSize(DaysOfWeek, 0.418272, 1.705000);
	TextDrawAlignment(DaysOfWeek, 2);
	TextDrawColor(DaysOfWeek, -1);
	TextDrawSetShadow(DaysOfWeek, 1);
	TextDrawSetOutline(DaysOfWeek, 0);
	TextDrawBackgroundColor(DaysOfWeek, 255);
	TextDrawFont(DaysOfWeek, 3);
	TextDrawSetProportional(DaysOfWeek, true);
	TextDrawSetShadow(DaysOfWeek, 1);
	
	ConnectTD[0] = TextDrawCreate(327.481842, 129.333328, "_~n~_~n~_~n~_");
	TextDrawLetterSize(ConnectTD[0], 0.362518, 3.793333);
	TextDrawTextSize(ConnectTD[0], 0.000000, 300.000000);
	TextDrawAlignment(ConnectTD[0], 2);
	TextDrawColor(ConnectTD[0], -1);
	TextDrawUseBox(ConnectTD[0], true);
	TextDrawBoxColor(ConnectTD[0], 0x00000055);
	TextDrawSetShadow(ConnectTD[0], 0);
	TextDrawSetOutline(ConnectTD[0], 0);
	TextDrawBackgroundColor(ConnectTD[0], 255);
	TextDrawFont(ConnectTD[0], 1);
	TextDrawSetProportional(ConnectTD[0], true);
	TextDrawSetShadow(ConnectTD[0], 0);

	ConnectTD[1] = TextDrawCreate(330.292572, 136.333343, "~w~Welcome_To~n~~p~"COMMUNITY_NAME"~n~~b~Cops_~w~And_~r~~h~Robbers~n~~b~~h~Version__~w~"VERSION"~n~~r~~h~THIS_IS_NOT_A_DM_SERVER");
	TextDrawLetterSize(ConnectTD[1], 0.496983, 2.603333);
	TextDrawAlignment(ConnectTD[1], 2);
	TextDrawColor(ConnectTD[1], -1);
	TextDrawSetShadow(ConnectTD[1], 0);
	TextDrawSetOutline(ConnectTD[1], 1);
	TextDrawBackgroundColor(ConnectTD[1], 255);
	TextDrawFont(ConnectTD[1], 2);
	TextDrawSetProportional(ConnectTD[1], true);
	TextDrawSetShadow(ConnectTD[1], 0);
	
	SAMP_Elevator_Initialize();
	GRIN_Elevator_Initialize();
	LoadActors();
	// M4 (stage 2) — mark the money-rush pickup slots empty (arrays default to 0).
	Moneyrush_Init();
	return 1;
}

public OnGameModeExit()
{
	SAMP_Elevator_Destroy();
	GRIN_Elevator_Destroy();
	// M4 (stage 2) — destroy any money-bag / money-rush dynamic pickups.
	Moneybag_Cleanup();
	Moneyrush_Cleanup();
	// M5 — destroy robbery-site CPs + bank map icons, house pickups/labels/map icons,
	// and sweep any active Airport Robbery box pickups.
	Robbery_Cleanup();
	House_Cleanup();
	// M6 (stage 1) — destroy the gym markers + clothes-shop map icons (§9.8/§9.7).
	FightStyle_Cleanup();
	Clothes_Cleanup();
	// M6 (stage 2) — destroy fishing/farming map icons + all plant checkpoints/objects (§9.2/§9.3).
	Fishing_Cleanup();
	Farm_Cleanup();
	for(new i = 0; i < MAX_PLAYERS; i++) Mission_AirportCleanup(i);
	DestroyZones();
	KillTimer(ServerInfo[sTimer]);
	DeleteInterior();
	DeleteAllActors();
	TextDrawDestroy(DaysOfWeek);
	TextDrawDestroy(ConnectTD[0]);
	TextDrawDestroy(ConnectTD[1]);
	mysql_close();
	return 1;
}

public OnPlayerConnect(playerid)
{
	new string[75];
	ClearPlayerVariables(playerid);
	TogglePlayerClock(playerid,true);
	if(!IsPlayerNPC(playerid))
	{
		TogglePlayerSpectating(playerid, true);
		ZoneCreateTD(playerid);
		RemoveObjectsForPlayer(playerid);
		CreateClassTD(playerid);
		TextDrawShowForPlayer(playerid,DaysOfWeek);
		TextDrawShowForPlayer(playerid,ConnectTD[0]);
		TextDrawShowForPlayer(playerid,ConnectTD[1]);
		CreateGPStd(playerid);
		g_MysqlRaceCheck[playerid]++;
		GetPlayerName(playerid,PlayerInfo[playerid][pUserName],MAX_PLAYER_NAME);
		GetPlayerIp(playerid,PlayerInfo[playerid][pIP],MAX_IP_SIZE);
	
		GetPlayerCountry(playerid,PlayerInfo[playerid][pCurrent_country],56);
		if(GetPlayerProxy(playerid)) 
		{
			format(string, sizeof(string), "[PROXY DETECTED] %s (%d) is using proxy/VPN.", PlayerInfo[playerid][pUserName], playerid);
			SendMessageToModerators(COLOR_CONNECT, string);
			IRC_SendAdminMessage(IRC_CONNECT_COLOR,string);
			IRC_SendManagementMessage(IRC_CONNECT_COLOR,string);
		}
		for(new i=0; i<10; i++) SendClientMessage(playerid,-1,"");
		SetTimerEx("CallForChecking",3000,false,"d",playerid);
		CreateMenuBox(playerid);
		// M5 (stage 1) — reflect any already-locked vehicles' doors for this new player (§9.12).
		Veh_OnConnect(playerid);
	}
	format(string,sizeof(string),"[CONNECT] %s (%d).",PlayerInfo[playerid][pUserName],playerid);
	SendConnectMessage(playerid,string,true);
	ServerInfo[sPlayersOnline] ++;
	return 1;
}

public OnPlayerDisconnect(playerid,reason)
{
	new string[75];
	if(!IsPlayerNPC(playerid)) 
	{
		ZoneDestroyTD(playerid);
		DestroyClassTD(playerid);
		TextDrawHideForPlayer(playerid,DaysOfWeek);
		DestroyMenuBox(playerid);
		GPS_OnPlayerDisconnect(playerid);
		if(PlayerInfo[playerid][pAdminJailTimer] != -1)
		{
			KillTimer(PlayerInfo[playerid][pAdminJailTimer]);
			PlayerInfo[playerid][pAdminJailTimer] = -1;
		}
		if(PlayerInfo[playerid][pLoggedIn])
		{
			mysql_format(g_SQL, string, sizeof(string), "UPDATE players SET LastOnline=NOW(),Online=0 WHERE aID=%d LIMIT 1",PlayerInfo[playerid][pID]);
			mysql_pquery(g_SQL, string);
			// M3 — persist the per-city wanted level (LSplayers.Wanted).
			Wanted_SavePlayer(playerid);
			// M3 (stage 3) — persist STD bitmask/condoms and drug stash/skill (§4.2/§4.3).
			STD_SavePlayer(playerid);
			Drug_SavePlayer(playerid);
			// M4 (stage 3) — persist the mission cooldown stamps (§6).
			Mission_SavePlayer(playerid);
		}
		// M3 (stage 2) — kill the jail countdown timer + persist the jail row
		// (Jail_OnDisconnect also unwinds any pending appeal/jury seat).
		Jail_OnDisconnect(playerid);
		// M3 (stage 2) — reset the cop /report undo state (static per-slot arrays).
		Cop_OnDisconnect(playerid);
		// M4 (stage 2) — kill any running holdup timer + drop the money-rush seat.
		Holdup_OnDisconnect(playerid);
		Moneyrush_OnDisconnect(playerid);
		// M5 (stage 2) — kill any running safe-crack timer (§5.3-5.6).
		Robbery_OnDisconnect(playerid);
		// M4 (stage 3) — persist cooldowns (if logged in) + drop active-mission state.
		Mission_OnDisconnect(playerid);
		// M5 (stage 4) — release any kidnap link; a victim who quits still pays the
		// ransom to the kidnapper (§4.4). Runs before Veh_OnDisconnect (order-agnostic).
		Kidnap_OnDisconnect(playerid);
		// M5 (stage 1) — drop this player's vehicle ownership marks + unlock their
		// locked cars so nothing is left sealed against everyone (§9.12).
		Veh_OnDisconnect(playerid);
		// M5 (stage 3) — drop the inside-house session marker + buy-dialog target (§9.1).
		House_OnDisconnect(playerid);
		// M6 (stage 2) — kill the reel-in timer + end an active farm job; persist the
		// player's plant grow-progress (plants freeze while the owner is offline, §9.2/§9.3).
		Fishing_OnDisconnect(playerid);
		Farm_OnDisconnect(playerid);
		// M6 (stage 3) — a live duel is a forfeit (opponent takes the pot, §6.4) + drop
		// pending duel offers both ways; arena state is session-only so it just falls
		// away. Then end a DJ broadcast if this was the streaming DJ (§8.1).
		Arena_OnDisconnect(playerid);
		DJ_OnDisconnect(playerid);

	}
	new szDisconnectReason[3][] =
	{
		"Timeout/Crash",
		"Quit",
		"Kick/Ban"
	};
	format(string,sizeof(string),"[DISCONNECT] %s (%d) - (%s).",PlayerInfo[playerid][pUserName],playerid,szDisconnectReason[reason]);
	SendConnectMessage(playerid,string,true);
	ServerInfo[sPlayersOnline] --;
	return 1;
}

public OnPlayerRequestClass(playerid, classid)
{
	if(PlayerInfo[playerid][pClassMusic]) PlayerPlaySound(playerid,1062,0.0,0.0,0.0);
	PlayerInfo[playerid][pClassSelection]=true;
	if(PlayerInfo[playerid][pSkin])
	{
		classid=PlayerInfo[playerid][pClassID];
		SetPlayerSkin(playerid,PedsInfo[PlayerInfo[playerid][pClassID]][PedSkinID]);
	}
	else PlayerInfo[playerid][pClassID]=classid;
	ShowPlayerClassTD(playerid,classid);
	switch(PedsInfo[classid][PedTeam])
	{
		case CIVIL,UC_COP:
		{
			SetPlayerPos(playerid,CIVIL_POS);
			SetPlayerFacingAngle(playerid,CIVIL_FACING);
			SetPlayerCameraPos(playerid,CIVIL_CAMERA);
			SetPlayerCameraLookAt(playerid,CIVIL_POS);
		}
		case POLICE:
		{
			SetPlayerPos(playerid,POLICE_POS);
			SetPlayerFacingAngle(playerid,POLICE_FACING);
			SetPlayerCameraPos(playerid,POLICE_CAMERA);
			SetPlayerCameraLookAt(playerid,POLICE_POS);
		}
		case SHERIFF:
		{
			SetPlayerPos(playerid,SHERIFF_POS);
			SetPlayerFacingAngle(playerid,SHERIFF_FACING);
			SetPlayerCameraPos(playerid,SHERIFF_CAMERA);
			SetPlayerCameraLookAt(playerid,SHERIFF_POS);
		}
		case FBI:
		{
			SetPlayerPos(playerid,FBI_POS);
			SetPlayerFacingAngle(playerid,FBI_FACING);
			SetPlayerCameraPos(playerid,FBI_CAMERA);
			SetPlayerCameraLookAt(playerid,FBI_POS);
		}
	}
	return 1;
}

public OnPlayerRequestSpawn(playerid)
{
	if(!IsPlayerNPC(playerid))
	{
		if(!PlayerInfo[playerid][pLoggedIn]) KickPlayer(playerid,SERVER_BOT,"Failed to login!");
		else
		{
			new string[MAX_SERIAL_SIZE+70];
			gpci(playerid, PlayerInfo[playerid][pSerial], MAX_SERIAL_SIZE);
			mysql_format(g_SQL, string, sizeof(string), "UPDATE "TABLE_PLAYERS" SET Latest_IP='%e', Latest_Serial='%e' WHERE aID=%d LIMIT 1",PlayerInfo[playerid][pIP],PlayerInfo[playerid][pSerial],PlayerInfo[playerid][pID]);
			mysql_pquery(g_SQL, string);
			
			switch(PedsInfo[PlayerInfo[playerid][pClassID]][PedTeam])
			{
				case CIVIL,UC_COP:
				{
					new rspawn=random(sizeof(Spawns));
					SetSpawnInfo(playerid,NO_TEAM,PedsInfo[PlayerInfo[playerid][pClassID]][PedSkinID],Spawns[rspawn][SpawnLocX],Spawns[rspawn][SpawnLocY],Spawns[rspawn][SpawnLocZ],Spawns[rspawn][SpawnLocA],0,0,0,0,0,0);
				}
				case POLICE:
				{
					new rspawn=random(sizeof(SpawnsCOP));
					SetSpawnInfo(playerid,NO_TEAM,PedsInfo[PlayerInfo[playerid][pClassID]][PedSkinID],SpawnsCOP[rspawn][SpawnLocX],SpawnsCOP[rspawn][SpawnLocY],SpawnsCOP[rspawn][SpawnLocZ],SpawnsCOP[rspawn][SpawnLocA],0,0,0,0,0,0);
				}
				case SHERIFF:
				{
					new rspawn=random(sizeof(SpawnsSHERIFF));
					SetSpawnInfo(playerid,NO_TEAM,PedsInfo[PlayerInfo[playerid][pClassID]][PedSkinID],SpawnsSHERIFF[rspawn][SpawnLocX],SpawnsSHERIFF[rspawn][SpawnLocY],SpawnsSHERIFF[rspawn][SpawnLocZ],SpawnsSHERIFF[rspawn][SpawnLocA],0,0,0,0,0,0);
				}
				case FBI:
				{
					new rspawn=random(sizeof(SpawnsFBI));
					SetSpawnInfo(playerid,NO_TEAM,PedsInfo[PlayerInfo[playerid][pClassID]][PedSkinID],SpawnsFBI[rspawn][SpawnLocX],SpawnsFBI[rspawn][SpawnLocY],SpawnsFBI[rspawn][SpawnLocZ],SpawnsFBI[rspawn][SpawnLocA],0,0,0,0,0,0);
				}
			}
		}
	}	
	return 1;
}

public OnPlayerSpawn(playerid)
{
	new string[128];
	UpdateMoney(playerid);
	SetPlayerInterior(playerid,0);
	SetPlayerVirtualWorld(playerid,0);
	SetCameraBehindPlayer(playerid);
	if(PlayerInfo[playerid][pClassSelection])
	{
		HidePlayerClassTD(playerid);
		PlayerInfo[playerid][pClassSelection]=false;
		if(PlayerInfo[playerid][pClassMusic]) PlayerPlaySound(playerid,1098,0.0,0.0,0.0);
	}	
	if(!PlayerInfo[playerid][pSkin])
	{
		PlayerInfo[playerid][pSkin]=GetPlayerSkin(playerid);
		for(new i=0; i<MAX_PEDS; i++)
		{
			if(PlayerInfo[playerid][pSkin] == PedsInfo[i][PedSkinID]) 
			{
				PlayerInfo[playerid][pClassID]=i;
				PlayerInfo[playerid][pTeam] =PedsInfo[i][PedTeam];
				break;
			}
		}
		mysql_format(g_SQL, string, sizeof(string), "UPDATE "TABLE_PLAYERS_C" SET Skin=%d, ClassID=%d WHERE aID=%d LIMIT 1",PlayerInfo[playerid][pSkin],PlayerInfo[playerid][pClassID],PlayerInfo[playerid][pID]);
		mysql_pquery(g_SQL, string);
	}
	PlayerInfo[playerid][pTeam] =PedsInfo[PlayerInfo[playerid][pClassID]][PedTeam];

	PlayerInfo[playerid][pSpawned]=true;
	PlayerInfo[playerid][pSpawns]++;
	// Leaving any DM zone on (re)spawn (anti-parachute flag clears).
	PlayerInfo[playerid][pInDMZone]=false;
	if(!IsPlayerNPC(playerid))
	{
		ZoneShowTD(playerid);
		CreateClassTD(playerid);
	}
	// M6 (stage 3) — arena (re)spawn: a DMS/sniper death-respawn stays INSIDE the
	// arena with a fresh kit; a died-out/duel-loss applies the deferred loadout
	// restore. If handled, skip the city jail/wanted/house placement (an arena
	// player is never jailed) but still apply skills/fighting style + DJ sync (§9.6).
	if(!IsPlayerNPC(playerid) && Arena_OnPlayerSpawn(playerid))
	{
		Skills_ApplyOnSpawn(playerid);
		FightStyle_ApplyOnSpawn(playerid);
		DJ_SyncPlayer(playerid);
		return 1;
	}
	// Re-apply a still-running admin-jail across relog/respawn (M2 — §11.2).
	// Admin-jail ALWAYS takes precedence over cop-jail: if an admin-jail is
	// running we re-place there and skip the cop-jail re-placement below.
	if(PlayerInfo[playerid][pAdminJailUntil] > gettime())
		ApplyAdminJail(playerid, 0, "Admin-jail resumed", SERVER_BOT);
	else
	{
		if(PlayerInfo[playerid][pAdminJailUntil] != 0)
			PlayerInfo[playerid][pAdminJailUntil]=0;
		// M3 (stage 2) — re-apply a still-running cop-jail sentence (§3.2). Only
		// reached when NOT admin-jailed, preserving admin-jail precedence.
		if(!IsPlayerNPC(playerid)) Jail_OnPlayerSpawn(playerid);
	}
	// M3 — spawn protection + re-apply wanted name/blip colour on (re)spawn (§2.1, §2.5).
	if(!IsPlayerNPC(playerid)) Wanted_OnPlayerSpawn(playerid);
	// M5 (stage 3) — spawn-at-house: clears the stale inside-house marker and, if the
	// owner set a house spawn (and is not jailed/admin-jailed), places them inside
	// their house instead of the city spawn (§9.1).
	if(!IsPlayerNPC(playerid))
		House_OnPlayerSpawn(playerid);
	// M6 (stage 1) — max all weapon skills (classic feel, §9.8/§2.1) + re-apply the
	// saved fighting style (SetPlayerFightingStyle) on every spawn (§9.8).
	if(!IsPlayerNPC(playerid))
	{
		Skills_ApplyOnSpawn(playerid);
		FightStyle_ApplyOnSpawn(playerid);
	}
	// M6 (stage 3) — if a DJ broadcast is live, tune this (re)spawning player in (§8.1).
	if(!IsPlayerNPC(playerid)) DJ_SyncPlayer(playerid);
	return 1;
}

public OnPlayerDeath(playerid,killerid,reason)
{
	// M6 (stage 3) — arena death (DMS/sniper respawn-inside + $1k fee, or duel
	// first-blood/forfeit resolution). The death feed runs FIRST, while pInDMZone is
	// still set on both parties, so the killer gains no wanted for a legal-DM kill;
	// Arena_OnPlayerDeath then resolves it and short-circuits the city death path.
	if(!IsPlayerNPC(playerid) && Arena_IsInArena(playerid))
	{
		Wanted_OnPlayerDeath(playerid, killerid, reason);	// death feed only (wanted suppressed by pInDMZone)
		Arena_OnPlayerDeath(playerid, killerid);
		return 1;
	}
	PlayerInfo[playerid][pInDMZone]=false;
	if(!IsPlayerNPC(playerid))
	{
		// M3 — crime wanted (killer scaling) + death feed / fire "grilled" fix (§2.3, §7.11).
		Wanted_OnPlayerDeath(playerid, killerid, reason);
		// M3 (stage 3) — a fresh spawn is clean: clear STDs + drug buzz/overdose (§4.2/§4.3).
		STD_OnPlayerDeath(playerid);
		Drug_OnPlayerDeath(playerid);
		// M4 (stage 1) — medical fee on death unless insured (design §11.2, economy
		// §1.5/§1.6). Returns 1 if a life policy was consumed (weapons preserved);
		// SA-MP always drops weapons on death, so the weapon-save is best-effort and
		// left as a stage-1 note (return value reserved for the M6 weapon system).
		Bank_OnPlayerDeath(playerid);
		// M4 (stage 2) — a holdup in progress is aborted on death (§5.2).
		Holdup_OnPlayerDeath(playerid);
		// M5 (stage 2) — a safe-type robbery in progress is aborted on death. The
		// crowbar is NOT confiscated on death — only a lawful arrest confiscates it,
		// so being killed (by a cop or anyone) keeps the crowbar (§5.7 exception).
		Robbery_OnPlayerDeath(playerid, killerid);
		// M4 (stage 3) — an active mission is aborted on death (§6, clean up CP).
		Mission_OnPlayerDeath(playerid);
		// M5 (stage 4) — death on either side of a kidnap link releases the victim (§4.4).
		Kidnap_OnPlayerDeath(playerid, killerid);
		// M6 (stage 2) — a cast fishing line / active farm job ends on death (§9.2/§9.3).
		Fishing_OnPlayerDeath(playerid);
		Farm_OnPlayerDeath(playerid);
		// Dying wipes the victim's wanted level (arrest/takedown/escape all reset here).
		ClearPlayerWanted(playerid, "");
		ZoneHideTD(playerid);
		switch(PedsInfo[PlayerInfo[playerid][pClassID]][PedTeam])
		{
			case CIVIL,UC_COP:
			{
				new rspawn=random(sizeof(Spawns));
				SetSpawnInfo(playerid,NO_TEAM,PedsInfo[PlayerInfo[playerid][pClassID]][PedSkinID],Spawns[rspawn][SpawnLocX],Spawns[rspawn][SpawnLocY],Spawns[rspawn][SpawnLocZ],Spawns[rspawn][SpawnLocA],0,0,0,0,0,0);
			}
			case POLICE:
			{
				new rspawn=random(sizeof(SpawnsCOP));
				SetSpawnInfo(playerid,NO_TEAM,PedsInfo[PlayerInfo[playerid][pClassID]][PedSkinID],SpawnsCOP[rspawn][SpawnLocX],SpawnsCOP[rspawn][SpawnLocY],SpawnsCOP[rspawn][SpawnLocZ],SpawnsCOP[rspawn][SpawnLocA],0,0,0,0,0,0);
			}
			case SHERIFF:
			{
				new rspawn=random(sizeof(SpawnsSHERIFF));
				SetSpawnInfo(playerid,NO_TEAM,PedsInfo[PlayerInfo[playerid][pClassID]][PedSkinID],SpawnsSHERIFF[rspawn][SpawnLocX],SpawnsSHERIFF[rspawn][SpawnLocY],SpawnsSHERIFF[rspawn][SpawnLocZ],SpawnsSHERIFF[rspawn][SpawnLocA],0,0,0,0,0,0);
			}
			case FBI:
			{
				new rspawn=random(sizeof(SpawnsFBI));
				SetSpawnInfo(playerid,NO_TEAM,PedsInfo[PlayerInfo[playerid][pClassID]][PedSkinID],SpawnsFBI[rspawn][SpawnLocX],SpawnsFBI[rspawn][SpawnLocY],SpawnsFBI[rspawn][SpawnLocZ],SpawnsFBI[rspawn][SpawnLocA],0,0,0,0,0,0);
			}
		}
	}	
	return 1;
}

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[]) 
{
	switch (dialogid)
	{
		case REGISTER_DIALOG_PASSWORD, REGISTER_DIALOG_EMAIL,LOGIN_DIALOG:
		{
			REG_LOG_OnDialogResponse(playerid,dialogid,response,inputtext);
		}
		// M3 (stage 2) — cop /refill purchase menu.
		case REFILL_DIALOG:
		{
			Refill_OnDialogResponse(playerid, response, listitem);
		}
		// M4 (stage 1) — bank menu + deposit/withdraw amount input dialogs.
		case BANK_DIALOG, BANK_DEPOSIT_DIALOG, BANK_WITHDRAW_DIALOG:
		{
			Bank_OnDialogResponse(playerid, dialogid, response, listitem, inputtext);
		}
		// M4 (stage 2) — /radio preset stream URL picker.
		case RADIO_DIALOG:
		{
			Radio_OnDialogResponse(playerid, response, listitem);
		}
		// M4 (stage 3) — /missions (/work) job launcher list — start the chosen job.
		case MISSION_START_DIALOG:
		{
			Mission_OnDialogResponse(playerid, response, listitem);
		}
		// M6 (stage 4) — /challenge Race Challenge picker (§6.13) — start the chosen race.
		case MRACE_SELECT_DIALOG:
		{
			Mission_RaceOnDialog(playerid, response, listitem);
		}
		// M5 (stage 3) — house owner menu + buy confirm + storage input dialogs (§9.1).
		case HOUSE_MENU_DIALOG, HOUSE_BUY_DIALOG, HOUSE_STORE_DIALOG, HOUSE_WITHDRAW_DIALOG:
		{
			House_OnDialogResponse(playerid, dialogid, response, listitem, inputtext);
		}
	}
	// M6 (stage 1) — /skill picker, /fightstyle picker, /clotheswear menu + skins list.
	// These use standalone high dialog ids (5401-5404, unique — outside the enum), so
	// route them after the enum switch (they return 0 if the id isn't theirs).
	Skills_OnDialogResponse(playerid, dialogid, response, listitem);
	FightStyle_OnDialogResponse(playerid, dialogid, response, listitem);
	Clothes_OnDialogResponse(playerid, dialogid, response, listitem);
	// M6 (stage 2) — Bait Shop buy menu (rod/cooler/permits/seeds, §9.2/§9.3). Uses a
	// standalone high dialog id (5501) outside the enum; returns 0 if not its dialog.
	Fishing_OnDialogResponse(playerid, dialogid, response, listitem);
	return 0;
}

public OnPlayerEnterDynamicArea(playerid,areaid)
{
	if(!IsPlayerNPC(playerid)) 
	{
		if(PlayerInfo[playerid][GPS_Destination] != -1) GPS_OnPlayerEnterRoute(playerid,areaid);
		OnPlayerEnterZone(playerid,areaid);
	}	
	return 1;
}

public OnPlayerLeaveDynamicArea(playerid,areaid)
{
	if(!IsPlayerNPC(playerid))
	{
		if(PlayerInfo[playerid][GPS_Destination] != -1) GPS_OnPlayerLeaveRoute(playerid,areaid);
		OnPlayerLeaveZone(playerid,areaid);
	}
	return 1;
}

// M4 (stage 3) — mission delivery routes use per-player RACE checkpoints; each
// drop is scored + paid here via the mission framework's dispatcher.
public OnPlayerEnterRaceCheckpoint(playerid)
{
	if(!IsPlayerNPC(playerid) && Mission_IsOnMission(playerid)) Mission_OnCheckpoint(playerid);
	return 1;
}

// M5 (stage 2) — the safe-carry hideout is a standard SetPlayerCheckpoint; the
// robbery module resolves the delivery here (§5.3/§5.6).
public OnPlayerEnterCheckpoint(playerid)
{
	if(!IsPlayerNPC(playerid)) Robbery_OnPlayerEnterCheckpoint(playerid);
	return 1;
}

// M5 (stage 2) — bank/casino/special robbery site checkpoints (Streamer dynamic
// CPs). The module prints the "type /bankrob" hint; the command re-checks range.
public OnPlayerEnterDynamicCP(playerid, STREAMER_TAG_CP:checkpointid)
{
	if(!IsPlayerNPC(playerid))
	{
		Robbery_OnEnterCP(playerid, checkpointid);
		// M6 (stage 2) — legal farm-job field checkpoints (§9.3). Returns 0 unless the
		// player is on a farm job and this is their next field.
		Farm_OnPlayerEnterCP(playerid, checkpointid);
	}
	return 1;
}

public OnPlayerText(playerid,text[])
{
	if(!PlayerInfo[playerid][pLoggedIn] && PlayerInfo[playerid][pRegistered]) { SendClientMessage(playerid,COLOR_ERROR,"You need to login before using chatbox."); return 0; }
	if(PlayerInfo[playerid][pMenu] > 0) 
	{ 
		if(OnPlayerEnterTextDrawMenuOption(playerid ,text))
		{
			return 0;
		}
	}
	if(ServerInfo[sChat_Locked] && PlayerInfo[playerid][pRank] < SERVER_MODERATOR) { SendClientMessage(playerid, COLOR_ERROR, "• The chat has been locked by a staff member."); return 0;}
	if(PlayerInfo[playerid][pAddingInterior])
	{
		if(AddInterior(playerid,text)) return 0;
	}
	if(PlayerInfo[playerid][pAddingActor])
	{
		if(AddNewActor(playerid,text)) return 0;
	}
	new string[256];
	format(string,sizeof(string),"%s (%d): %s",PlayerInfo[playerid][pUserName],playerid,text);
	IRC_SendMessage(string,IRC_NORMAL_COLOR);
	return 1;	
}

public OnPlayerKeyStateChange(playerid, newkeys, oldkeys)
{
	// M5 (stage 4) — Airport Robbery box-collection: the MMB (sub-mission key,
	// KEY_ACTION) grabs the nearest box on foot during the collect phase (§6.1). The
	// /box command is the typed equivalent. No-op unless on the airport run.
	if(!IsPlayerNPC(playerid) && (newkeys & KEY_ACTION) && !IsPlayerInAnyVehicle(playerid))
	{
		if(PlayerInfo[playerid][pOnMission] == MISSION_AIRPORT_ROBBERY && PlayerInfo[playerid][pAirportPhase] == AIRPHASE_COLLECT)
			Mission_AirportCollectBox(playerid);
	}
	if(newkeys & KEY_FIRE)
	{
		if (PlayerInfo[playerid][pMenu] > 0 && !IsPlayerInAnyVehicle(playerid))
		{
			HideTextDrawMenu(playerid);
		}
	}
	if(newkeys & KEY_CROUCH)
	{
	    if (PlayerInfo[playerid][pMenu] > 0 && IsPlayerInAnyVehicle(playerid))
		{
			HideTextDrawMenu(playerid);
		}
	}
	if(!IsPlayerInAnyVehicle(playerid) && (newkeys & KEY_YES))
	{
		// M5 (stage 3) — house enter/exit on the YES key (§9.1). If a house door was
		// handled, skip the elevator checks so the same key press does not double-fire.
		if(House_OnKeyEnter(playerid)) return 1;
		Check_SAMP_Elevator(playerid);
		Check_GRIN_Elevator(playerid);
	}
	return 1;
}

public OnPlayerStateChange(playerid, newstate, oldstate)
{
	if(!IsPlayerNPC(playerid))
	{
		// M3 — vehicle-jack crime detection (driver-seat theft of an OCCUPIED vehicle,
		// §2.3, WANTED_CARJACK +3). Runs first so an occupied jack is scored on the
		// jack path; Veh_OnPlayerStateChange below then handles the UNOCCUPIED /
		// parked-vehicle GTA path (§9.12) — the two are mutually exclusive.
		Wanted_OnPlayerStateChange(playerid, newstate, oldstate);
		// M5 (stage 1) — vehicle ownership / lock enforcement / grand-theft-auto of an
		// unoccupied owned vehicle (§9.12). Ordered after the jack detector.
		Veh_OnPlayerStateChange(playerid, newstate, oldstate);
		// M4 (stage 3) — a vehicle mission auto-cancels when the driver leaves the
		// bound mission vehicle (§6.17/§6.18 "auto-cancels if the vehicle is lost").
		Mission_OnPlayerStateChange(playerid, newstate, oldstate);
	}
	return 1;
}

public OnVehicleSpawn(vehicleid)
{
	// M5 (stage 1) — a respawned vehicle is pristine again: clear its last driver +
	// lock so it can be taken freely and nobody keeps stale ownership (§9.12).
	Veh_OnVehicleReset(vehicleid);
	return 1;
}

public OnVehicleDeath(vehicleid, killerid)
{
	// M5 (stage 1) — a destroyed vehicle loses its ownership + lock (§9.12).
	Veh_OnVehicleReset(vehicleid);
	#pragma unused killerid
	return 1;
}

public OnPlayerWeaponShot(playerid, WEAPON:weaponid, BULLET_HIT_TYPE:hittype, hitid, Float:fX, Float:fY, Float:fZ)
{
	if(!IsPlayerNPC(playerid))
	{
		// M3 — drive-by handling: fold into attack + small wanted (§2.3, open Q9).
		Wanted_OnPlayerWeaponShot(playerid, _:hittype, hitid);
	}
	#pragma unused weaponid, fX, fY, fZ
	return 1;
}

public OnQueryError(errorid, const error[], const callback[], const query[], MySQL:handle)
{
	switch(errorid)
	{
		case CR_SERVER_GONE_ERROR: print("MYSQL: Lost connection to server, trying reconnect...");
		case ER_SYNTAX_ERROR: printf("MYSQL: Something is wrong in your syntax, query: %s",query);
	}
	return 1;
}


public OnPlayerCommandReceived(playerid, cmd[], params[], flags) 
{ 
	if(!PlayerInfo[playerid][pLoggedIn])
	{
	    SendClientMessage(playerid,COLOR_ERROR,"Please Login, Before Using Any Command.");
	    return 0;
	}
	return 1;
}
public OnPlayerCommandPerformed(playerid, cmd[], params[], result, flags)
{
	new string[256];
	if(result == -1) return SendClientMessage(playerid,COLOR_ERROR,INVALID_COMMAND);
	if(strcmp(cmd, "pm", true) == 0 || strcmp(cmd, "msg", true) == 0 || strcmp(cmd, "m", true) == 0
	   || strcmp(cmd, "reply", true) == 0 || strcmp(cmd, "r", true) == 0) return 1;
	// Staff command-echo (/showcommands): mirror a command to any equal-or-higher
	// ranked staff who have echo enabled (M2 — §11.2).
	for(new i=0; i < MAX_PLAYERS; i++)
	{
	  	if(IsPlayerConnected(i) && PlayerInfo[i][pShowCommands] && PlayerInfo[i][pRank] >= PlayerInfo[playerid][pRank] && i != playerid)
	  	{
	  	    format(string,sizeof(string),"%s (%d) Has Used Command \"/%s %s\".",PlayerInfo[playerid][pUserName],playerid,cmd,params);
	  	    SendClientMessage(i,COLOR_ADMIN_INFO,string);
 		}
	}
	return 1;
}

public OnPlayerPickUpDynamicPickup(playerid, pickupid)
{
	Interior_PlayerPickup(playerid,pickupid);
	// M4 (stage 2) — money-bag grab + money-rush cash pickups (design §7).
	Moneybag_OnPickup(playerid, pickupid);
	Moneyrush_OnPickup(playerid, pickupid);
	// M5 (stage 3) — house entrance pickup → for-sale / enter hint (§9.1).
	House_OnPickup(playerid, pickupid);
	return 1;
}

public OnPlayerClickPlayer(playerid, clickedplayerid, source)
{
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR) return 1;
	new Float:paa[3];
	GetPlayerPos(clickedplayerid,paa[0],paa[1],paa[2]);
	SetPlayerPos(playerid,paa[0]+2,paa[1]+2,paa[2]+2);
	SetPlayerInterior(playerid,GetPlayerInterior(clickedplayerid));
	SetPlayerVirtualWorld(playerid,GetPlayerVirtualWorld(clickedplayerid));
	return 1;
}

public OnDynamicObjectMoved(objectid)
{
	OnSAMPElevatorMoved(objectid);
	OnGRINElevatorMoved(objectid);
	return 1;
}

public OnPlayerClickMap(playerid,Float:fX,Float:fY,Float:fZ)
{
	PlayerInfo[playerid][pCustomMapMarker][0]=fX;
	PlayerInfo[playerid][pCustomMapMarker][1]=fY;
	PlayerInfo[playerid][pCustomMapMarker][2]=fZ;
	return 1;
}

// Admin/dev utility commands — rank-gated in the open.mp port.
new Float:p[MAX_PLAYERS][3];
new pa[MAX_PLAYERS][2];
CMD:saveloc(playerid)
{
	if(PlayerInfo[playerid][pRank] < SERVER_ADMIN) return SendClientMessage(playerid,COLOR_ERROR,ERROR_NOT_ADMIN);
	GetPlayerPos(playerid,p[playerid][0],p[playerid][1],p[playerid][2]);
	pa[playerid][0] = GetPlayerInterior(playerid);
	pa[playerid][1] = GetPlayerVirtualWorld(playerid);
	SendClientMessage(playerid,-1,"Use /teleback to teleport here.");
	return 1;
}

CMD:teleback(playerid)
{
	if(PlayerInfo[playerid][pRank] < SERVER_ADMIN) return SendClientMessage(playerid,COLOR_ERROR,ERROR_NOT_ADMIN);
	SetPlayerPos(playerid,p[playerid][0],p[playerid][1],p[playerid][2]);
	SetPlayerInterior(playerid,pa[playerid][0]);
	SetPlayerVirtualWorld(playerid,pa[playerid][1]);
	SendClientMessage(playerid,-1,"Teleported back.");
	return 1;
}

CMD:resetsamp(playerid)
{
	if(PlayerInfo[playerid][pRank] < SERVER_ADMIN) return SendClientMessage(playerid,COLOR_ERROR,ERROR_NOT_ADMIN);
	ResetSAMPElevatorQueueSAMP();
	SendClientMessage(playerid,-1,"Reset Pass!");
	return 1;
}

CMD:resetgrin(playerid)
{
	if(PlayerInfo[playerid][pRank] < SERVER_ADMIN) return SendClientMessage(playerid,COLOR_ERROR,ERROR_NOT_ADMIN);
	ResetGRINElevatorQueueGRIN();
	SendClientMessage(playerid,-1,"Reset Pass!");
	return 1;
}

CMD:skin(playerid,params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_ADMIN) return SendClientMessage(playerid,COLOR_ERROR,ERROR_NOT_ADMIN);
	new skinid=strval(params);
	if (skinid < 0 || skinid > 311) return SendClientMessage(playerid, -1,"ERROR: Invalid skin");
	SetPlayerSkin(playerid, skinid);
	return 1;
}
//==============================================================================
//	Global Functions 
//==============================================================================

FUNCTION GameModeClock()
{
	new string[75];
	// M4 (stage 2) — guard for the once-per-game-day lotto draw (§10.3). Static so
	// it persists across ticks; reset to -1 on the game-week rollover below.
	static lastLottoDay = -1;
	// M6 (stage 2) — guard for the once-per-game-day bonus-fish announce (§9.2). Same
	// pattern as the lotto guard; reset on the game-week rollover below.
	static lastBonusFishDay = -1;
	// M7 — guard for the once-per-game-day STOCK MARKET tick (§10.3). Same pattern as
	// the lotto guard: the tick fires exactly once per game-day at the day rollover
	// (GameHour==0) and never double-fires. Reset on the game-week rollover below.
	static lastMarketDay = -1;
	GameMinute ++;
	if(GameMinute == 60)
	{
		GameMinute = 0;
		GameHour ++;
		GameWeather ++;
		if(GameWeather == 32)
		{
			new next_weather_prob = random(120);
			if(next_weather_prob < 70) 	SetWeather(fine_weather_ids[random(sizeof(fine_weather_ids))]);
			else if(next_weather_prob < 105) SetWeather(foggy_weather_ids[random(sizeof(foggy_weather_ids))]);
			else SetWeather(wet_weather_ids[random(sizeof(wet_weather_ids))]);
			GameWeather = 0;
		}
		// M4 (stage 2) — LOTTO DRAW bound to the IN-GAME clock hour transition,
		// NOT a real-time timer (design §10.3 — the legacy "lottery timer bug"
		// fix). The draw fires when GameHour reaches LOTTO_DRAW_HOUR (18), guarded
		// by the last-drawn game-day so it fires exactly once per game-day and
		// never double-fires on a skipped/relogged hour. The guard is reset on the
		// game-week rollover below.
		if(GameHour == LOTTO_DRAW_HOUR && lastLottoDay != GameDay)
		{
			lastLottoDay = GameDay;
			Lotto_Draw();
		}

		// M6 (stage 2) — daily BONUS FISH announced at game-hour 05:00 (§9.2), on the
		// same hour-transition mechanism as the lotto draw. Guarded once-per-game-day;
		// the first player to /fish after this wins the reward (resolved in fishing.inc).
		if(GameHour == FISH_BONUS_HOUR && lastBonusFishDay != GameDay)
		{
			lastBonusFishDay = GameDay;
			Fishing_OnBonusHour();
		}

		if(GameHour == 24)
		{
			GameMinute = 0;
			GameHour = 0;
			GameDay ++;


			// M4 (stage 1) — day-transition money hooks (design §10.3): bank
			// interest + tax tick fire on the game-clock day rollover (NO new
			// global timer). GameDay resets each game-week, so we drive the
			// every-N-days schedule off a monotonic counter that never resets.
			g_GameDayCounter++;
			Bank_OnGameDay(g_GameDayCounter);
			Tax_OnGameDay(g_GameDayCounter);
			// M5 (stage 3) — house rent charge + 2-week inactivity decay on the same
			// game-day rollover (NO new global timer, §3.2/§3.4).
			House_OnGameDay();
			// M7 — the STOCK MARKET tick fires on the game-day rollover (GameHour just
			// wrapped to 0, §10.3): recompute every price from pool+drift, pay dividends,
			// check bankruptcy, move the prime rate and write a report row. Guarded by
			// lastMarketDay so it fires exactly once per game-day (mirrors the lotto guard).
			if(lastMarketDay != GameDay)
			{
				lastMarketDay = GameDay;
				Stocks_Tick();
			}

			if(GameDay == 7)
			{
				GameDay = 0;
				ServerInfo[sWeeksCompleted]++;
				mysql_format(g_SQL,string,sizeof(string),"UPDATE "SERVER_TABLE" Set WeeksCompleted=%d WHERE Version=%d",ServerInfo[sWeeksCompleted],STATS_VERSION);
				mysql_pquery(g_SQL,string);
				// M4 (stage 2) — reset the lotto draw guard on the week rollover
				// (§10.3: "Guard vars reset per game-week rollover").
				lastLottoDay = -1;
				// M6 (stage 2) — reset the bonus-fish guard on the same rollover.
				lastBonusFishDay = -1;
				// M7 — reset the stock-market tick guard on the same rollover (§10.3).
				lastMarketDay = -1;
			}
			format(string, sizeof(string), "%s",WeekDays[GameDay]);
			TextDrawSetString(DaysOfWeek, string);
		}
	}

	for(new playerid=0; playerid < MAX_PLAYERS; playerid++)
	{
		if(!IsPlayerConnected(playerid)) continue;
		SetPlayerTime(playerid, GameHour, GameMinute);
	}

	// M3 — wanted-level decay runs on this 1-second tick (no new global timer, §2.4).
	Wanted_OnGameModeClockTick();
	// M3 (stage 2) — ticket→warrant escalation on the same tick (§2.4).
	Cop_OnGameModeClockTick();
	// M3 (stage 3) — STD HP drain + drug heal/overdose on the same tick (§4.2/§4.3).
	STD_OnGameModeClockTick();
	Drug_OnGameModeClockTick();
	// M4 (stage 2) — money-bag timeout/relocate + money-rush duration on the same
	// 1-second tick (no new global timers, design §7).
	Moneybag_OnTick();
	Moneyrush_OnTick();
	// M4 (stage 3) — mission off-route / jailed guard on the same tick (§6, no new
	// timer). Game-hour cooldowns need no tick — they read the clock on demand.
	Mission_OnTick();
	// M5 (stage 2) — robbery safe-carry 12-minute hideout deadline on the same tick
	// (gettime()-deadline check, no new global timer for the window, §5.6).
	Robbery_OnTick();
	// M5 (stage 4) — kidnap hideout delivery + escape guard on the same tick (§4.4).
	Kidnap_OnTick();
	// M6 (stage 2) — drug-plant growth on the same 1-second tick (§9.3): each plant
	// accrues grow-seconds only while its owner is online (no offline growth). No new
	// global timer — reuses this clock like every other system.
	Farm_OnGameModeClockTick();
	// M6 (stage 3) — expire stale pending duel challenges on the same tick (§6.4).
	Arena_OnGameModeClockTick();

	format(string, sizeof(string), "%s, %02d:%02d",WeekDays[GameDay],GameHour,GameMinute);

	if(GameMinute == 0)
	{
		format(string, sizeof(string), "worldtime %s",string);
		SendRconCommand(string);
		strdel(string,0,10);
		
		format(string, sizeof(string), ""COL_WHITE"Game Time: "COL_SERVER_INFO"%s",string);
		TimeUpdateMessage(COLOR_WHITE, string);
	}
	return 1;
}