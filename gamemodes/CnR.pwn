/*

	Cops and Robebrs Script.
	Scripted by Sasuke_Uchiha.
	Scripting started on 11th October 2017
	Last Updated: ---
	
	Credits:
		SA-MP team
		Zeex for crashdetect
		Incognito for streamer and irc
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

#include <a_samp>
#include <crashdetect>
#include <streamer>
#include <sscanf2>
#include <irc>
#include <a_mysql>
#include <geolocation>
#include <Pawn.CMD>
#include <RouteConnector>

//==============================================================================
//	Internal definitions and natives
//==============================================================================

#include "CnR\server\defines"

native WP_Hash(buffer[],len,const str[]);
native IsValidVehicle(vehicleid);
native gpci(playerid, serial[], len);


//MySQL configuration
#define SQL_HOST 					"127.0.0.1"
#define SQL_DB 						"cnr"
#define SQL_USER 					"admin"
#define SQL_PASS 					""

#define VERSION 					"0.01"
#define STATS_VERSION 				0

main()
{	
	print("\n");
	print("  |---------------------------------------------------");
	print("  |--- Cops and Robbers");
    printf("  |--  Script v%s",VERSION);
    print("  |--  Sasuke_Uchiha");
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

new Text:DaysOfWeek;
new GameDay = 0;
new GameHour = 0;
new GameMinute = 0;
new GameWeather = 0;

new fine_weather_ids[] = {1,2,3,4,5,6,7,12,13,14,15,17,18,24,25,26,27,28,29,30,40};
new foggy_weather_ids[] = {9,19,20,31,32};
new wet_weather_ids[] = {8};


new MySQL:g_SQL;
new g_MysqlRaceCheck[MAX_PLAYERS];

new Text:ConnectTD[2];

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
#include "CnR\server\irc_"

#include "CnR\players\cmds"
#include "CnR\server\actors"
#include "CnR\server\gps"

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
	mysql_log(ERROR | WARNING);
	EnableStuntBonusForAll(0); //Disabling stunt bonus.
	DisableInteriorEnterExits();  // will disable all interior enter/exits in the game.
	mysql_pquery(g_SQL, "UPDATE players SET Online=0 WHERE 1");
	
	GameDay = 0;
	GameHour = 0;
	GameMinute = 0;
	SetWorldTime(0);
	SetWeather(random(21));
	SendRconCommand("worldtime Sunday 00:00");
	ServerInfo[sTimer] = SetTimerEx("GameModeClock", 1000, true, "d", "d");
	new string[75];
	mysql_format(g_SQL,string,sizeof(string),"SELECT * FROM `server_data` WHERE Version=%d",STATS_VERSION);
	mysql_pquery(g_SQL, string, "OnServerDataLoad","");
	
	DaysOfWeek= TextDrawCreate(577.203552, 5.083323, WeekDays[GameDay]);
	TextDrawLetterSize(DaysOfWeek, 0.418272, 1.705000);
	TextDrawAlignment(DaysOfWeek, 2);
	TextDrawColor(DaysOfWeek, -1);
	TextDrawSetShadow(DaysOfWeek, 1);
	TextDrawSetOutline(DaysOfWeek, 0);
	TextDrawBackgroundColor(DaysOfWeek, 255);
	TextDrawFont(DaysOfWeek, 3);
	TextDrawSetProportional(DaysOfWeek, 1);
	TextDrawSetShadow(DaysOfWeek, 1);
	
	ConnectTD[0] = TextDrawCreate(327.481842, 129.333328, "_~n~_~n~_~n~_");
	TextDrawLetterSize(ConnectTD[0], 0.362518, 3.793333);
	TextDrawTextSize(ConnectTD[0], 0.000000, 300.000000);
	TextDrawAlignment(ConnectTD[0], 2);
	TextDrawColor(ConnectTD[0], -1);
	TextDrawUseBox(ConnectTD[0], 1);
	TextDrawBoxColor(ConnectTD[0], 0x00000055);
	TextDrawSetShadow(ConnectTD[0], 0);
	TextDrawSetOutline(ConnectTD[0], 0);
	TextDrawBackgroundColor(ConnectTD[0], 255);
	TextDrawFont(ConnectTD[0], 1);
	TextDrawSetProportional(ConnectTD[0], 1);
	TextDrawSetShadow(ConnectTD[0], 0);

	ConnectTD[1] = TextDrawCreate(330.292572, 136.333343, "~w~Welcome_To~n~~p~"COMMUNITY_NAME"~n~~b~Cops_~w~And_~r~~h~Robbers~n~~b~~h~Version__~w~"VERSION"~n~~r~~h~THIS_IS_NOT_A_DM_SERVER");
	TextDrawLetterSize(ConnectTD[1], 0.496983, 2.603333);
	TextDrawAlignment(ConnectTD[1], 2);
	TextDrawColor(ConnectTD[1], -1);
	TextDrawSetShadow(ConnectTD[1], 0);
	TextDrawSetOutline(ConnectTD[1], 1);
	TextDrawBackgroundColor(ConnectTD[1], 255);
	TextDrawFont(ConnectTD[1], 2);
	TextDrawSetProportional(ConnectTD[1], 1);
	TextDrawSetShadow(ConnectTD[1], 0);
	
	SAMP_Elevator_Initialize();
	GRIN_Elevator_Initialize();
	InitilizeIRC(); //Local
	LoadActors();
	return 1;
}

public OnGameModeExit()
{
	SAMP_Elevator_Destroy();
	GRIN_Elevator_Destroy();
	UnloadIRC();
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
		if(PlayerInfo[playerid][pLoggedIn])
		{
			mysql_format(g_SQL, string, sizeof(string), "UPDATE players SET LastOnline=NOW(),Online=0 WHERE aID=%d LIMIT 1",PlayerInfo[playerid][pID]);
			mysql_pquery(g_SQL, string);
		}
		
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
	if(!IsPlayerNPC(playerid))
	{
		ZoneShowTD(playerid);
		CreateClassTD(playerid);
	}	
	return 1;
}

public OnPlayerDeath(playerid,killerid,reason)
{
	if(!IsPlayerNPC(playerid))
	{
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
	}
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
		Check_SAMP_Elevator(playerid);
		Check_GRIN_Elevator(playerid);
	}
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
	//new string[256];
	if(result == -1) return SendClientMessage(playerid,COLOR_ERROR,INVALID_COMMAND);
	if(strcmp(cmd, "pm", true) == 0) return 1;
	/*for(new i=0,j=GetPlayerPoolSize(); i <= j; i++)
	{
	  	if(IsPlayerConnected(playerid) && PlayerInfo[i][pShowCommands] && PlayerInfo[i][pRank] >= PlayerInfo[playerid][pRank] && i != playerid)
	  	{
	  	    format(string,sizeof(string),"%s (%d) Has Used Command \"%s\".",PlayerInfo[playerid][pUserName],playerid,cmd);
	  	    SendClientMessage(i,COLOR_ADMIN_INFO,string);
 		}
	  }*/
	return 1;
}

public OnPlayerPickUpDynamicPickup(playerid, pickupid)
{
	Interior_PlayerPickup(playerid,pickupid);
	return 1;
}

public OnPlayerClickPlayer(playerid, clickedplayerid, source)
{
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

new Float:p[MAX_PLAYERS][3];
new pa[MAX_PLAYERS][2];
CMD:saveloc(playerid)
{
	GetPlayerPos(playerid,p[playerid][0],p[playerid][1],p[playerid][2]);
	pa[playerid][0] = GetPlayerInterior(playerid);
	pa[playerid][1] = GetPlayerVirtualWorld(playerid);
	SendClientMessage(playerid,-1,"Use /teleback to teleport here.");
	return 1;
}

CMD:teleback(playerid)
{
	SetPlayerPos(playerid,p[playerid][0],p[playerid][1],p[playerid][2]);
	SetPlayerInterior(playerid,pa[playerid][0]);
	SetPlayerVirtualWorld(playerid,pa[playerid][1]);
	SendClientMessage(playerid,-1,"Teleported back.");
	return 1;
}

CMD:resetsamp(playerid)
{
	ResetSAMPElevatorQueueSAMP();
	SendClientMessage(playerid,-1,"Reset Pass!");
	return 1;
}

CMD:resetgrin(playerid)
{
	ResetGRINElevatorQueueGRIN();
	SendClientMessage(playerid,-1,"Reset Pass!");
	return 1;
}

CMD:skin(playerid,params[])
{
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
		if(GameHour == 24)
		{
			GameMinute = 0;
			GameHour = 0;
			GameDay ++;
			
			
			if(GameDay == 7)
			{
				GameDay = 0;
				ServerInfo[sWeeksCompleted]++;
				mysql_format(g_SQL,string,sizeof(string),"UPDATE "SERVER_TABLE" Set WeeksCompleted=%d WHERE Version=%d",ServerInfo[sWeeksCompleted],STATS_VERSION);
				mysql_pquery(g_SQL,string);
			}
			format(string, sizeof(string), "%s",WeekDays[GameDay]);
			TextDrawSetString(DaysOfWeek, string);
		}
	}

	for(new playerid=0,j=GetPlayerPoolSize(); playerid <= j; playerid++)
	{
		SetPlayerTime(playerid, GameHour, GameMinute);
	}

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