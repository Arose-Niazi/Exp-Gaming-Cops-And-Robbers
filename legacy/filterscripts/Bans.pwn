/*

	Ban System for Mini Missions/Mini Games/Cops And Robbers
	Scripted by Sasuke_Uchiha.
	Scripting started on 4th May 2017
	Last Updated: 19TH Ocotober 2017
	
	Functions:
		Check_StageOne(playerid) - To be called before player logins/ is shown dialog to login.
		BanPlayer(playerid,admin[],days,reason[])
		BanPlayerEx(user[], serial[], ip[], admin[], reason[], days)
		BanPlayerSerial(playerid,admin[],days,reason[])
		BanPlayerSerialEx(user[], serial[], ip[], admin[], reason[], days)
		BanPlayerRange(playerid,admin[],days,reason[],big)
		BanPlayerRangeEx(user[], serial[], ip[], admin[], reason[], days, big)
		IsBanned(value[]) - The value can be name/ip/range/serial or id
		UnBanPlayerEx(value[],reason[],Unbanby[],range,type) - Range = 0 - None, 1 - Small, 2 - Big | Type = 1 - Full, 2 - IP, 3 - Serial, 4 - Range, 5 - Nick
		EnableBan(value[],type,admin[]) Type = 1 - Nick, 2 - IP, 3- Serial, 4 - Small Range, 5 - Big Range
		
	Functions Being Called:
		BanCheckDone(playerid,true/false) True if player is banned, show dialog and kick. False, the player should be shown login dialog.
	
*/	

#include <a_samp>
#include <crashdetect>
#include <a_mysql>
#include <irc>
#include <sscanf2>

#define CNR 2
#define MM 1
#define MG 0
#define SERVER CNR
#define IRC_BAN_CHANNEL "#bans"

#if SERVER != MG
#define SERVER_NAME			"MM"
#define SERVER_NAME_IRC		"8,5[MM"
#elseif SERVER == MG
#define SERVER_NAME  		"MG"
#define SERVER_NAME_IRC 	"9,5[MG"
#else
#define SERVER_NAME  		"CNR"
#define SERVER_NAME_IRC 	"0,5[CnR"
#endif

native gpci(playerid, serial[], len);

#if SERVER != MG
new MySQL:SQL_Connection;
#else
new SQL_Connection = -1;
#endif 

enum pdata
{
	bName[MAX_PLAYER_NAME],
	bIP[16],
	bSerial[64]
};

new gData[MAX_PLAYERS][pdata];

#define BAN_TABLE		"player_bans"

#define FUNCTION%1(%2) 	forward %1(%2); 	public %1(%2)

public OnFilterScriptInit()
{
	print("--------------------------------------");
	print(" MM/MG Ban system loaded. ");
	print("--------------------------------------\n");
	
	#if SERVER == MM
	SQL_Connection = mysql_connect("127.0.0.1", "mini_missions", "AbGQEpSRaMLZ47nk","mini_missions");
	#elseif SERVER == MG
	SQL_Connection = mysql_connect("127.0.0.1", "mini_missions", "mini_missions", "AbGQEpSRaMLZ47nk");
	#else
	SQL_Connection = mysql_connect("149.202.65.49", "Sasuke", "w2yAYebiN3zwMAPb","mini_missions");
	#endif
	
	if (mysql_errno() != 0)	print("[BAN SCRIPT SQL]: Failed to connect to server..!!");

	for(new i = 0, j = GetPlayerPoolSize(); i <= j; i++)
	{
		if(IsPlayerConnected(i))
		{
			GetPlayerName(i, gData[i][bName], MAX_PLAYER_NAME);
			GetPlayerIp(i, gData[i][bIP], 16);
			gpci(i,gData[i][bSerial], 64);
		}
		else
		{
			strdel(gData[i][bName],0,MAX_PLAYER_NAME);
			strdel(gData[i][bIP],0,16);
			strdel(gData[i][bSerial],0,64);
		}
	}
	
	return 1;
}

public OnFilterScriptExit()
{
	print("--------------------------------------");
	print(" MM/MG Ban system unloaded. ");
	print("--------------------------------------\n");
	return 1;
}

public OnPlayerConnect(playerid)
{
	GetPlayerName(playerid, gData[playerid][bName], MAX_PLAYER_NAME);
	GetPlayerIp(playerid, gData[playerid][bIP], 16);
	gpci(playerid,gData[playerid][bSerial], 64);
	return 1;
}

FUNCTION ShowBanDialog(playerid,admin[],reason[],banID,banIP[],banName[], banDate[])
{
	new dialog[256*4],string[256],datetime[6];

	getdate(datetime[0], datetime[1], datetime[2]);
	gettime(datetime[3], datetime[4], datetime[5]);
	GetPlayerName(playerid, gData[playerid][bName], MAX_PLAYER_NAME);
	GetPlayerIp(playerid, gData[playerid][bIP], 16);
	strcat(dialog, "{FFFFFF}Oops! It seems that you've been {FF0000}banned{FFFFFF}.\n\nHere's some information regarding your ban:\n\n");
	format(string, sizeof(string), "{FF0000}Username on Ban:{FFFFFF} %s\n", banName); strcat(dialog, string);
	format(string, sizeof(string), "{FF0000}Banned By:{FFFFFF} %s\n", admin); strcat(dialog, string);
	format(string, sizeof(string), "{FF0000}IP on Ban:{FFFFFF} %s\n", banIP); strcat(dialog, string);
	format(string, sizeof(string), "{FF0000}Current username:{FFFFFF} %s\n", gData[playerid][bName]); strcat(dialog, string);
	format(string, sizeof(string), "{FF0000}Current IP:{FFFFFF} %s\n", gData[playerid][bIP]); strcat(dialog, string);
	format(string, sizeof(string), "{FF0000}Reason of Ban:{FFFFFF} %s\n", reason); strcat(dialog, string);
	format(string, sizeof(string), "{FF0000}Date of Ban:{FFFFFF} %s %s\n",banDate); strcat(dialog, string);
	format(string, sizeof(string), "{FF0000}Ban Identification Number:{FFFFFF} %i\n\n", banID); strcat(dialog, string);
	strcat(dialog, "You can appeal this ban at our forum found below;\n\n{FF0000}Forum:{FFFFFF} http://mg-s.us/mybb\n\n");
	strcat(dialog, "You can take a screenshot by using {FF0000}F8{FFFFFF} or {FF0000}PrintScreen{FFFFFF}, include this in your appeal.");

	ShowPlayerDialog(playerid, 0, DIALOG_STYLE_MSGBOX, "{FF0000}Information:{FFFFFF} Banned User", dialog, "OK", "");
	format(string,sizeof(string),"%s][BAN SYSTEM]: %s (%d) has been shown ban dialog, Ban ID - %d - .",SERVER_NAME_IRC,gData[playerid][bName],playerid,banID);
	IRC_GroupSay(1,IRC_BAN_CHANNEL,string);
	SetTimerEx("DelayKick",1000,false,"d",playerid);
}

FUNCTION Check_StageOne(playerid)
{
	new Cache:result, string[256], rows = 0, BanID;
	GetPlayerName(playerid, gData[playerid][bName], MAX_PLAYER_NAME);
	GetPlayerIp(playerid, gData[playerid][bIP], 16);
	mysql_format(SQL_Connection, string, sizeof(string), "SELECT * FROM %s WHERE Active = 1 AND Nick='%e' LIMIT 1", BAN_TABLE, gData[playerid][bName]);
	result = mysql_query(SQL_Connection, string);
	#if SERVER == 0
		rows = cache_get_row_count();
	#else
		cache_get_row_count(rows);
	#endif
	
	if(rows)
	{
		#if SERVER != MG
			cache_get_value_name_int(0, "BanID", BanID);
			cache_delete(result);
		#else
			BanID= cache_get_field_content_int(0, "BanID", SQL_Connection);
			cache_delete(result,SQL_Connection);
		#endif
		PlayerBanned(playerid,1,BanID);
	}
	else
	{
		Check_StageTwo(playerid);
	}
	return 1;
}

Check_StageTwo(playerid)
{
	new Cache:result, string[256], rows = 0,BanID;
	GetPlayerIp(playerid, gData[playerid][bIP], 16);
	mysql_format(SQL_Connection, string, sizeof(string), "SELECT * FROM %s WHERE IP_Banned = 1 AND IP='%e' LIMIT 1", BAN_TABLE, gData[playerid][bIP]);
	result = mysql_query(SQL_Connection, string);
	#if SERVER == 0
		rows = cache_get_row_count();
	#else
		cache_get_row_count(rows);
	#endif
	
	if(rows)
	{
		#if SERVER != MG
			cache_get_value_name_int(0, "BanID", BanID);
			cache_delete(result);
		#else
			BanID= cache_get_field_content_int(0, "BanID", SQL_Connection);
			cache_delete(result,SQL_Connection);
		#endif
		PlayerBanned(playerid,2,BanID);
	}
	else
	{
		Check_StageThree(playerid);
	}
	return 1;
}

Check_StageThree(playerid)
{
	new Cache:result, string[256], rows = 0,BanID;
	gpci(playerid,gData[playerid][bSerial], 64);
	mysql_format(SQL_Connection, string, sizeof(string), "SELECT * FROM %s WHERE Serial_Banned = 1 AND Serial='%e' LIMIT 1", BAN_TABLE, gData[playerid][bSerial]);
	result = mysql_query(SQL_Connection, string);
	#if SERVER == 0
		rows = cache_get_row_count();
	#else
		cache_get_row_count(rows);
	#endif
	if(rows)
	{
		#if SERVER != MG
			cache_get_value_name_int(0, "BanID", BanID);
			cache_delete(result);
		#else
			BanID= cache_get_field_content_int(0, "BanID", SQL_Connection);
			cache_delete(result,SQL_Connection);
		#endif
		PlayerBanned(playerid,3,BanID);
	}
	else
	{
		Check_StageFour(playerid);
	}
	return 1;
}

Check_StageFour(playerid)
{
	new Cache:result, string[256], rows = 0,BanID,ip[4][4];
	sscanf(gData[playerid][bIP],"p<.>s[4]s[4]s[4]s[4]",ip[0],ip[1],ip[2],ip[3]);
	mysql_format(SQL_Connection, string, sizeof(string), "SELECT * FROM %s WHERE ((Range_Banned_s = 1 AND IP LIKE '%e.%e.%e%%') OR (Range_Banned_b = 1 AND IP LIKE '%e.%e%%')) AND IP_Banned=1 LIMIT 1", BAN_TABLE, ip[0],ip[1],ip[2],ip[0],ip[1]);
	result = mysql_query(SQL_Connection, string);
	#if SERVER == 0
		rows = cache_get_row_count();
	#else
		cache_get_row_count(rows);
	#endif
	
	if(rows)
	{
		#if SERVER != MG
			cache_get_value_name_int(0, "BanID", BanID);
			cache_delete(result);
		#else
			BanID= cache_get_field_content_int(0, "BanID", SQL_Connection);
			cache_delete(result,SQL_Connection);
		#endif
		PlayerBanned(playerid,4,BanID);
	}
	else
	{
		printf("Cleared");
		CheckSerial(playerid);
		CallRemoteFunction("BanCheckDone","dd",playerid,0);
	}
	return 1;
}

PlayerBanned(playerid,stage,banID)
{
	new string[256],Cache:result;
	mysql_format(SQL_Connection, string, sizeof(string), "SELECT UNIX_TIMESTAMP(`Duration`),Reason,Admin,BanID,Date,IP,Nick FROM %e WHERE BanID='%d' LIMIT 1", BAN_TABLE,banID);
	result = mysql_query(SQL_Connection, string);
	new rows;
	#if SERVER != MG
		cache_get_row_count(rows);
	#else 
		rows = cache_get_row_count();
	#endif
	new BanID;
	new data[5][64],exp_date;
	#if SERVER != MG
	cache_get_value_name_int(0, "UNIX_TIMESTAMP(`Duration`)", exp_date);
	cache_get_value_name(0, "Reason", data[1]);
	cache_get_value_name(0, "Admin", data[2]);
	cache_get_value_name_int(0, "BanID", BanID);
	cache_get_value_name(0, "Date", data[3]);
	cache_get_value_name(0, "IP", data[4]);
	cache_get_value_name(0, "Nick", data[0]);
	#else
	exp_date = cache_get_field_content_int(0, "UNIX_TIMESTAMP(`Duration`)", SQL_Connection);
	BanID= cache_get_field_content_int(0, "BanID", SQL_Connection);
	cache_get_field_content(0, "Reason", data[1], SQL_Connection, 64);
	cache_get_field_content(0, "Admin", data[2], SQL_Connection, 64);
	cache_get_field_content(0, "Date", data[3], SQL_Connection, 64);
	cache_get_field_content(0, "IP", data[4], SQL_Connection, 16);
	cache_get_field_content(0, "Nick", data[0], SQL_Connection, MAX_PLAYER_NAME);
	#endif
    
	if(gettime() >= exp_date && exp_date != 0)
	{
		#if SERVER != MG
		cache_delete(result);
		#else
		cache_delete(result,SQL_Connection);
		#endif
		UnBanPlayer(BanID, "Temporary ban expired",1,"Ban Script");
		switch(stage)
		{
			case 1: Check_StageTwo(playerid);
			case 2: Check_StageThree(playerid);
			case 3: Check_StageFour(playerid);
			case 4: CallRemoteFunction("BanCheckDone","dd",playerid,0);
		}	
	}	
	else
	{
		
		ShowBanDialog(playerid, data[2], data[1], BanID, data[4],data[0], data[3]);
		CallRemoteFunction("BanCheckDone","dd",playerid,1);
	}
	return 1;
}

FUNCTION UnBanPlayerEx(value[],reason[],Unbanby[],range,type)
{
	new string[256],Cache:result,rows,ip[4][4];
	if(IsNumeric(value))
	{
		mysql_format(SQL_Connection, string, sizeof(string), "SELECT * FROM %e WHERE Active = 1 AND BanID= %d LIMIT 1", BAN_TABLE, strval(value));
	}
	else
	{
	
		if(sscanf("p<.>s[4]s[4]S[4]S[4]",value,ip[0],ip[1],ip[2],ip[3]))
		{
			if(strlen(value) > 30) mysql_format(SQL_Connection, string, sizeof(string), "SELECT *,DATE_FORMAT(Duration,GET_FORMAT(DATETIME,'USA')) FROM %e WHERE Serial_Banned = 1 AND Serial= '%e' LIMIT 1", BAN_TABLE, value);
			else mysql_format(SQL_Connection, string, sizeof(string), "SELECT *,DATE_FORMAT(Duration,GET_FORMAT(DATETIME,'USA')) FROM %e WHERE Active = 1 AND Nick LIKE '%e%%' LIMIT 1", BAN_TABLE, value);
		}
		else
		{
			switch(range)
			{
				case 1: mysql_format(SQL_Connection, string, sizeof(string), "SELECT * FROM %e WHERE (IP_Banned=1 AND IP='%e') OR (Range_Banned_s=1 AND IP LIKE '%e.%e.%e%%') LIMIT 1", BAN_TABLE, value, ip[0],ip[1],ip[2]);
				case 2: mysql_format(SQL_Connection, string, sizeof(string), "SELECT * FROM %e WHERE (IP_Banned=1 AND IP='%e') OR (Range_Banned_b=1 AND  IP LIKE '%e.%e%%') OR (Range_Banned_s=1 AND IP LIKE '%e.%e.%e%%') LIMIT 1", BAN_TABLE, value, ip[0],ip[1],ip[0],ip[1],ip[2]);
				default: mysql_format(SQL_Connection, string, sizeof(string), "SELECT * FROM %e WHERE IP_Banned=1 AND IP='%e' LIMIT 1", BAN_TABLE, value);
			}
		}
	}
		

	result = mysql_query(SQL_Connection, string);
	#if SERVER == 0
		rows = cache_get_row_count();
	#else
		cache_get_row_count(rows);
	#endif	
	if(rows)
	{
		new BanID;
		#if SERVER != MG
			cache_get_value_name_int(0, "BanID", BanID);
			cache_delete(result);
		#else
			BanID= cache_get_field_content_int(0, "BanID", SQL_Connection);
			cache_delete(result,SQL_Connection);
		#endif
		UnBanPlayer(BanID,reason,type,Unbanby);
		return true;
	}	
	return false;
}

UnBanPlayer(banID,reason[],type,Unbanby[])
{
	new string[256];
	
	switch(type)
	{
		case 1:
		{
			format(string,sizeof(string),"%s][BAN SYSTEM]: Ban ID - %d - has been unbanned [Reason: %s | By: %s].",SERVER_NAME_IRC,banID,reason,Unbanby);
			IRC_GroupSay(1,IRC_BAN_CHANNEL,string);
			mysql_format(SQL_Connection, string, sizeof(string), "UPDATE %e SET Active = 0,IP_Banned = 0,Serial_Banned = 0,Range_Banned_b = 0, Range_Banned_s= 0, UnbanReason = '%e', UnbanBy='%e' WHERE BanID = %i", BAN_TABLE, reason, Unbanby, banID);
		}	
		case 2:
		{
			format(string,sizeof(string),"%s][BAN SYSTEM]: IP Unban for Ban ID - %d - [Reason: %s | By: %s].",SERVER_NAME_IRC,banID,reason,Unbanby);
			IRC_GroupSay(1,IRC_BAN_CHANNEL,string);
			mysql_format(SQL_Connection, string, sizeof(string), "UPDATE %e SET IP_Banned = 0, IP_UnbanReason = '%e', IP_UnbanBy='%e' WHERE BanID = %i", BAN_TABLE, reason, Unbanby, banID);
		}
		case 3:
		{
			format(string,sizeof(string),"%s][BAN SYSTEM]: Serial Unban for Ban ID - %d - [Reason: %s | By: %s].",SERVER_NAME_IRC,banID,reason,Unbanby);
			IRC_GroupSay(1,IRC_BAN_CHANNEL,string);
			mysql_format(SQL_Connection, string, sizeof(string), "UPDATE %e SET Serial_Banned = 0, Serial_UnbanReason = '%e', Serial_UnbanBy='%e' WHERE BanID = %i", BAN_TABLE, reason, Unbanby, banID);
		}
		case 4:
		{
			format(string,sizeof(string),"%s][BAN SYSTEM]: Range Unban for Ban ID - %d - [Reason: %s | By: %s].",SERVER_NAME_IRC,banID,reason,Unbanby);
			IRC_GroupSay(1,IRC_BAN_CHANNEL,string);
			mysql_format(SQL_Connection, string, sizeof(string), "UPDATE %e SET Range_Banned_b = 0, Range_Banned_s= 0, Range_UnbanReason = '%e', Range_UnbanBy='%e' WHERE BanID = %i", BAN_TABLE, reason, Unbanby, banID);
		}
		case 5:
		{
			format(string,sizeof(string),"%s][BAN SYSTEM]: Nick Unban for Ban ID - %d - [Reason: %s | By: %s].",SERVER_NAME_IRC,banID,reason,Unbanby);
			IRC_GroupSay(1,IRC_BAN_CHANNEL,string);
			mysql_format(SQL_Connection, string, sizeof(string), "UPDATE %e SET Active = 0,  UnbanReason = '%e', UnbanBy='%e' WHERE BanID = %i", BAN_TABLE, reason, Unbanby, banID);
		}
		
	}
	mysql_tquery(SQL_Connection, string);
	return true;
}

FUNCTION EnableBan(value[],type,admin[])
{
	new Cache:result, string[256], rows = 0,ip[4][4];
	if(IsNumeric(value))
	{
		mysql_format(SQL_Connection, string, sizeof(string), "SELECT * FROM %e WHERE Active = 1 AND BanID= %d LIMIT 1", BAN_TABLE, strval(value));
	}
	else
	{
	
		if(sscanf(value,"p<.>s[4]s[4]S[4]S[4]",ip[0],ip[1],ip[2],ip[3]))
		{
			if(strlen(value) > 30) mysql_format(SQL_Connection, string, sizeof(string), "SELECT *,DATE_FORMAT(Duration,GET_FORMAT(DATETIME,'USA')) FROM %e WHERE Serial_Banned = 1 AND Serial= '%e' LIMIT 1", BAN_TABLE, value);
			else mysql_format(SQL_Connection, string, sizeof(string), "SELECT *,DATE_FORMAT(Duration,GET_FORMAT(DATETIME,'USA')) FROM %e WHERE Active = 1 AND Nick='%e' LIMIT 1", BAN_TABLE, value);
		}
		else
		{
			mysql_format(SQL_Connection, string, sizeof(string), "SELECT * FROM %e WHERE IP_Banned=1 AND IP='%e' LIMIT 1", BAN_TABLE, value);
		}
	}
		

	result = mysql_query(SQL_Connection, string);
	#if SERVER == 0
		rows = cache_get_row_count();
	#else
		cache_get_row_count(rows);
	#endif	
	if(rows)
	{
		new banID;
		#if SERVER != MG
			cache_get_value_name_int(0, "BanID", banID);
			cache_delete(result);
		#else
			banID= cache_get_field_content_int(0, "BanID", SQL_Connection);
			cache_delete(result,SQL_Connection);
		#endif
		switch(type)
		{	
			case 1:
			{
				format(string,sizeof(string),"%s][BAN SYSTEM]: Nick ban enabled for Ban ID - %d - [By: %s].",SERVER_NAME_IRC,banID,admin);
				IRC_GroupSay(1,IRC_BAN_CHANNEL,string);
				mysql_format(SQL_Connection, string, sizeof(string), "UPDATE %e SET Active = 1 WHERE BanID = %i", BAN_TABLE,banID);
			}
			case 2:
			{
				format(string,sizeof(string),"%s][BAN SYSTEM]: IP ban enabled for Ban ID - %d - [By: %s].",SERVER_NAME_IRC,banID,admin);
				IRC_GroupSay(1,IRC_BAN_CHANNEL,string);
				mysql_format(SQL_Connection, string, sizeof(string), "UPDATE %e SET IP_Banned = 1 WHERE BanID = %i", BAN_TABLE, banID);
			}
			case 3:
			{
				format(string,sizeof(string),"%s][BAN SYSTEM]: Serial ban enabled for Ban ID - %d - [By: %s].",SERVER_NAME_IRC,banID,admin);
				IRC_GroupSay(1,IRC_BAN_CHANNEL,string);
				mysql_format(SQL_Connection, string, sizeof(string), "UPDATE %e SET Serial_Banned = 1 WHERE BanID = %i", BAN_TABLE, banID);
			}
			case 4:
			{
				format(string,sizeof(string),"%s][BAN SYSTEM]: Range(short) ban enabled for Ban ID - %d - [By: %s].",SERVER_NAME_IRC,banID,admin);
				IRC_GroupSay(1,IRC_BAN_CHANNEL,string);
				mysql_format(SQL_Connection, string, sizeof(string), "UPDATE %e SET Range_Banned_s = 1 WHERE BanID = %i", BAN_TABLE, banID);
			}
			case 5:
			{
				format(string,sizeof(string),"%s][BAN SYSTEM]: Range(Big) ban enabled for Ban ID - %d - [By: %s].",SERVER_NAME_IRC,banID,admin);
				IRC_GroupSay(1,IRC_BAN_CHANNEL,string);
				mysql_format(SQL_Connection, string, sizeof(string), "UPDATE %e SET Range_Banned_b = 1 WHERE BanID = %i", BAN_TABLE, banID);
			}
		}
		mysql_tquery(SQL_Connection, string);
		return true;
	}	
	return false;
}

FUNCTION BanPlayer(playerid,admin[],days,reason[])
{
	new string[450],temp[2][64];

	if(days > 0)
		format(temp[0], 64, "TIMESTAMPADD(DAY,%i,UTC_TIMESTAMP())", days);
	else
		format(temp[0], 64, "%i", 0);

	format(temp[1], 64, "%s", InsertTimeStamp());
	GetPlayerName(playerid, gData[playerid][bName], MAX_PLAYER_NAME);
	GetPlayerIp(playerid, gData[playerid][bIP], 16);
	gpci(playerid,gData[playerid][bSerial], 64);
	mysql_format(SQL_Connection, string, sizeof(string), "INSERT INTO %e (Nick, Admin, IP, Serial, Reason, Duration, Server, IP_Banned, Date, Active) VALUES ('%e', '%e', '%e', '%e', '%e', %s, '%e', 1, '%e', 1)", BAN_TABLE, gData[playerid][bName], admin, gData[playerid][bIP], gData[playerid][bSerial], reason, temp[0], SERVER_NAME, temp[1]);
	new Cache:result = mysql_query(SQL_Connection, string);
	format(string,sizeof(string),"%s][BAN SYSTEM]: %s (%d) has been banned, Ban ID - %d - [Reason: %s | By: %s | Days: %d].",SERVER_NAME_IRC,gData[playerid][bName],playerid,cache_insert_id(),reason,admin,days);
	IRC_GroupSay(1,IRC_BAN_CHANNEL,string);
	ShowBanDialog(playerid, admin, reason, cache_insert_id(), gData[playerid][bIP],  gData[playerid][bName], temp[1]);
	#if SERVER != MG
	cache_delete(result);
	CallRemoteFunction("OnPlayerBannedCalled","d",playerid);
	#else
	cache_delete(result,SQL_Connection);
	#endif
	
	return true;
}

FUNCTION BanPlayerEx(user[], serial[], ip[], admin[], reason[], days)
{
	new string[500],temp[2][64];

	if(days > 0)
		format(temp[0], 64, "TIMESTAMPADD(DAY,%i,UTC_TIMESTAMP())", days);
	else
		format(temp[0], 64, "%i", 0);
	format(temp[1], 64, "%s", InsertTimeStamp());
	format(string,sizeof(string),"%s][BAN SYSTEM]: %s has been offline banned. [Reason: %s | By: %s | Days: %d].",SERVER_NAME_IRC,user,reason,admin,days);
	IRC_GroupSay(1,IRC_BAN_CHANNEL,string);
	mysql_format(SQL_Connection, string, sizeof(string), "INSERT INTO %e (Nick, Admin, IP, Serial, Reason, Duration, Server, IP_Banned, Date, Active) VALUES ('%e', '%e', '%e', '%e', '%e', '%e', '%e', 1, '%e', 1)", BAN_TABLE, user, admin, ip, serial, reason, temp[0], SERVER_NAME, temp[1]);
	return mysql_tquery(SQL_Connection, string);
}

FUNCTION BanPlayerSerial(playerid,admin[],days,reason[])
{
	new string[450],temp[2][64];

	if(days > 0)
		format(temp[0], 64, "TIMESTAMPADD(DAY,%i,UTC_TIMESTAMP())", days);
	else
		format(temp[0], 64, "%i", 0);

	format(temp[1], 64, "%s", InsertTimeStamp());
	GetPlayerName(playerid, gData[playerid][bName], MAX_PLAYER_NAME);
	GetPlayerIp(playerid, gData[playerid][bIP], 16);
	gpci(playerid,gData[playerid][bSerial], 64);
	mysql_format(SQL_Connection, string, sizeof(string), "INSERT INTO %e (Nick, Admin, IP, Serial, Reason, Duration, Server, IP_Banned, Date, Serial_Banned, Active) VALUES ('%e', '%e', '%e', '%e', '%e', %s, '%e', 1, '%e', 1, 1)", BAN_TABLE, gData[playerid][bName], admin, gData[playerid][bIP], gData[playerid][bSerial], reason, temp[0], SERVER_NAME, temp[1]);
	new Cache:result = mysql_query(SQL_Connection, string);
	format(string,sizeof(string),"%s][BAN SYSTEM]: %s (%d) has been Serial banned, Ban ID - %d - [Reason: %s | By: %s | Days: %d].",SERVER_NAME_IRC,gData[playerid][bName],playerid,cache_insert_id(),reason,admin,days);
	IRC_GroupSay(1,IRC_BAN_CHANNEL,string);
	ShowBanDialog(playerid, admin, reason, cache_insert_id(), gData[playerid][bIP],  gData[playerid][bName], temp[1]);
	#if SERVER != MG
	cache_delete(result);
	CallRemoteFunction("OnPlayerBannedCalled","d",playerid);
	#else
	cache_delete(result,SQL_Connection);
	#endif
	return true;
}
FUNCTION BanPlayerSerialEx(user[], serial[], ip[], admin[], reason[], days)
{
	new string[256],temp[2][64];

	if(days > 0)
		format(temp[0], 64, "TIMESTAMPADD(DAY,%i,UTC_TIMESTAMP())", days);
	else
		format(temp[0], 64, "%i", 0);
	format(temp[1], 64, "%s", InsertTimeStamp());
	
	format(string,sizeof(string),"%s][BAN SYSTEM]: %s has been offline serial banned. [Reason: %s | By: %s | Days: %d].",SERVER_NAME_IRC,user,reason,admin,days);
	IRC_GroupSay(1,IRC_BAN_CHANNEL,string);
	mysql_format(SQL_Connection, string, sizeof(string), "INSERT INTO %e (Nick, Admin, IP, Serial, Reason, Duration, Server, IP_Banned, Date, Serial_Banned, Active) VALUES ('%e', '%e', '%e', '%e', '%e', %s, '%e', 1, '%e', 1, 1)", BAN_TABLE, user, admin, ip, serial, reason, temp[0], SERVER_NAME, temp[1]);
	return mysql_tquery(SQL_Connection, string);
}
FUNCTION BanPlayerRange(playerid,admin[],days,reason[],big)
{
	new string[450],temp[2][64];

	if(days > 0)
		format(temp[0], 64, "TIMESTAMPADD(DAY,%i,UTC_TIMESTAMP())", days);
	else
		format(temp[0], 64, "%i", 0);

	format(temp[1], 64, "%s", InsertTimeStamp());
	GetPlayerName(playerid, gData[playerid][bName], MAX_PLAYER_NAME);
	GetPlayerIp(playerid, gData[playerid][bIP], 16);
	gpci(playerid,gData[playerid][bSerial], 64);
	if(big) mysql_format(SQL_Connection, string, sizeof(string), "INSERT INTO %e (Nick, Admin, IP, Serial, Reason, Duration, Server, IP_Banned, Date, Range_Banned_b, Active) VALUES ('%e', '%e', '%e', '%e', '%e', %s, '%e', 1, '%e', 1, 1)", BAN_TABLE, gData[playerid][bName], admin, gData[playerid][bIP], gData[playerid][bSerial], reason, temp[0], SERVER_NAME, temp[1]);
	else  mysql_format(SQL_Connection, string, sizeof(string), "INSERT INTO %e (Nick, Admin, IP, Serial, Reason, Duration, Server, IP_Banned, Date, Range_Banned_s, Active) VALUES ('%e', '%e', '%e', '%e', '%e', %s, '%e', 1, '%e', 1, 1)", BAN_TABLE, gData[playerid][bName], admin, gData[playerid][bIP], gData[playerid][bSerial], reason, temp[0], SERVER_NAME,temp[1]);
	new Cache:result = mysql_query(SQL_Connection, string);
	format(string,sizeof(string),"%s][BAN SYSTEM]: %s (%d) has been range(%s) banned, Ban ID - %d - [Reason: %s | By: %s | Days: %d].",SERVER_NAME_IRC,gData[playerid][bName],playerid,(big) ? ("big") : ("short"),cache_insert_id(),reason,admin,days);
	IRC_GroupSay(1,IRC_BAN_CHANNEL,string);
	ShowBanDialog(playerid, admin, reason, cache_insert_id(), gData[playerid][bIP],  gData[playerid][bName], temp[1]);
	#if SERVER != MG
	cache_delete(result);
	CallRemoteFunction("OnPlayerBannedCalled","d",playerid);
	#else
	cache_delete(result,SQL_Connection);
	#endif
	return true;
}
FUNCTION BanPlayerRangeEx(user[], serial[], ip[], admin[], reason[], days, big)
{
	new string[256],temp[2][64];

	if(days > 0)
		format(temp[0], 64, "TIMESTAMPADD(DAY,%i,UTC_TIMESTAMP())", days);
	else
		format(temp[0], 64, "%i", 0);
	format(temp[1], 64, "%s", InsertTimeStamp());
	format(string,sizeof(string),"%s][BAN SYSTEM]: %s has been offline range banned. [Reason: %s | By: %s | Days: %d].",SERVER_NAME_IRC,user,(big) ? ("big") : ("short"),reason,admin,days);
	IRC_GroupSay(1,IRC_BAN_CHANNEL,string);
	if(big) mysql_format(SQL_Connection, string, sizeof(string), "INSERT INTO %e (Nick, Admin, IP, Serial, Reason, Duration, Server, IP_Banned, Date, Range_Banned_b, Active) VALUES ('%e', '%e', '%e', '%e', '%e', %s, '%e', 1, '%e', 1, 1)", BAN_TABLE, user, admin, ip, serial, reason, temp[0], SERVER_NAME, temp[1]);
	else  mysql_format(SQL_Connection, string, sizeof(string), "INSERT INTO %e (Nick, Admin, IP, Serial, Reason, Duration, Server, IP_Banned, Date, Range_Banned_s, Active) VALUES ('%e', '%e', '%e', '%e', '%e', %s, '%e', 1, '%e', 1, 1)", BAN_TABLE, user, admin, ip, serial, reason, temp[0], SERVER_NAME, temp[1]);
	return mysql_tquery(SQL_Connection, string);
}
stock IsNumeric(const string[])
{
    for (new i = 0, j = strlen(string); i < j; i++)
    {
        if (string[i] > '9' || string[i] < '0') return 0;
    }
    return 1;
}

FUNCTION IsBanned(value[])
{
	new Cache:result, string[256], rows = 0, tmp[64],ip[4][4];
	if(IsNumeric(value))
	{
		mysql_format(SQL_Connection, string, sizeof(string), "SELECT *,DATE_FORMAT(Duration,GET_FORMAT(DATETIME,'USA')) FROM %e WHERE BanID= %d ORDER BY BanID DESC LIMIT 15", BAN_TABLE, strval(value));
	}
	else
	{
	
		if(sscanf(value,"p<.>s[4]s[4]S[4]S[4]",ip[0],ip[1],ip[2],ip[3]))
		{
			if(strlen(value) > 30) mysql_format(SQL_Connection, string, sizeof(string), "SELECT *,DATE_FORMAT(Duration,GET_FORMAT(DATETIME,'USA')) FROM %e WHERE Serial_Banned = 1 AND Serial= '%e' ORDER BY BanID DESC LIMIT 15", BAN_TABLE, value);
			else mysql_format(SQL_Connection, string, sizeof(string), "SELECT *,DATE_FORMAT(Duration,GET_FORMAT(DATETIME,'USA')) FROM %e WHERE Active = 1 AND Nick LIKE '%e%%' ORDER BY BanID DESC LIMIT 15", BAN_TABLE, value);
		}
		else
		{
			mysql_format(SQL_Connection, string, sizeof(string), "SELECT *,DATE_FORMAT(Duration,GET_FORMAT(DATETIME,'USA')) FROM %e WHERE (IP_Banned=1 AND IP='%e') OR (Range_Banned_b=1 AND  IP LIKE '%e.%e%%') OR (Range_Banned_s=1 AND IP LIKE '%e.%e.%e%%') ORDER BY BanID DESC LIMIT 15", BAN_TABLE, value, ip[0],ip[1],ip[0],ip[1],ip[2]);
		}
	}
		
	
	result = mysql_query(SQL_Connection, string);
	#if SERVER == 0
		rows = cache_get_row_count();
	#else
		cache_get_row_count(rows);
	#endif	
	if(rows)
	{
		for(new i = 0; i <rows; i++)
		{
			format(tmp, sizeof(tmp), "ib_BanID%i", i+1);
			new BanID;
			#if SERVER == MG
				BanID= cache_get_field_content_int(i, "BanID", SQL_Connection);
			#else
				cache_get_value_name_int(i, "BanID", BanID);
			#endif
			SetSVarInt(tmp, BanID);

			format(tmp, sizeof(tmp), "ib_Name%i", i+1);
			#if SERVER == MG
				cache_get_field_content(i, "Nick", string, SQL_Connection);
			#else
				cache_get_value_name(i, "Nick", string);
			#endif
			SetSVarString(tmp, string);

			format(tmp, sizeof(tmp), "ib_Admin%i", i+1);
			#if SERVER == MG
				cache_get_field_content(i, "Admin", string, SQL_Connection);
			#else
				cache_get_value_name(i, "Admin", string);
			#endif
			SetSVarString(tmp, string);

			format(tmp, sizeof(tmp), "ib_IP%i", i+1);
			#if SERVER == MG
				cache_get_field_content(i, "IP", string, SQL_Connection);
			#else
				cache_get_value_name(i, "IP", string);
			#endif
			SetSVarString(tmp, string);

			format(tmp, sizeof(tmp), "ib_Reason%i", i+1);
			#if SERVER == MG
				cache_get_field_content(i, "Reason", string, SQL_Connection);
			#else
				cache_get_value_name(i, "Reason", string);
			#endif
			SetSVarString(tmp, string);

			format(tmp, sizeof(tmp), "ib_Date%i", i+1);
			#if SERVER == MG
				cache_get_field_content(i, "Date", string, SQL_Connection);
			#else
				cache_get_value_name(i, "Date", string);
			#endif
			SetSVarString(tmp, string);

			format(tmp,sizeof(tmp), "ib_Serial%i", i+1);
			#if SERVER == MG
				cache_get_field_content(i, "Serial", string, SQL_Connection);
			#else
				cache_get_value_name(i, "Serial", string);
			#endif
			SetSVarString(tmp, string);
			
			new time;
			format(tmp,sizeof(tmp), "ib_Duration%i", i+1);
			#if SERVER == MG
				time= cache_get_field_content_int(i, "Duration", SQL_Connection);
			#else
				cache_get_value_name_int(i, "Duration", time);
			#endif
			if(time > 0)
			{
				#if SERVER == MG
					cache_get_field_content(i, "DATE_FORMAT(Duration,GET_FORMAT(DATETIME,'USA'))", string, SQL_Connection);
				#else
					cache_get_value_name(i, "DATE_FORMAT(Duration,GET_FORMAT(DATETIME,'USA'))", string);
				#endif
				SetSVarString(tmp, string);
			}
			else SetSVarString(tmp, "Permanently");
			
			format(tmp,sizeof(tmp), "ib_Server%i", i+1);
			#if SERVER == MG
				cache_get_field_content(i, "Server", string, SQL_Connection);
			#else
				cache_get_value_name(i, "Server", string);
			#endif
			SetSVarString(tmp, string);
			
			new var;
			format(tmp,sizeof(tmp), "ib_Active%i", i+1);
			#if SERVER == MG
				var= cache_get_field_content_int(i, "Active", SQL_Connection);
			#else
				cache_get_value_name_int(i, "Active", var);
			#endif
			SetSVarInt(tmp, var);
			
			format(tmp,sizeof(tmp), "ib_IP_Banned%i", i+1);
			#if SERVER == MG
				var= cache_get_field_content_int(i, "IP_Banned", SQL_Connection);
			#else
				cache_get_value_name_int(i, "IP_Banned", var);
			#endif
			SetSVarInt(tmp, var);
			
			format(tmp,sizeof(tmp), "ib_Serial_Banned%i", i+1);
			#if SERVER == MG
				var= cache_get_field_content_int(i, "Serial_Banned", SQL_Connection);
			#else
				cache_get_value_name_int(i, "Serial_Banned", var);
			#endif
			SetSVarInt(tmp, var);
			
			format(tmp,sizeof(tmp), "ib_Range_Banned_b%i", i+1);
			#if SERVER == MG
				var= cache_get_field_content_int(i, "Range_Banned_b", SQL_Connection);
			#else
				cache_get_value_name_int(i, "Range_Banned_b", var);
			#endif
			SetSVarInt(tmp, var);
			
			format(tmp,sizeof(tmp), "ib_Range_Banned_s%i", i+1);
			#if SERVER == MG
				var= cache_get_field_content_int(i, "Range_Banned_s", SQL_Connection);
			#else
				cache_get_value_name_int(i, "Range_Banned_s", var);
			#endif
			SetSVarInt(tmp, var);
			
			format(tmp,sizeof(tmp), "ib_UnbanReason%i", i+1);
			#if SERVER == MG
				cache_get_field_content(i, "UnbanReason", string, SQL_Connection);
			#else
				cache_get_value_name(i, "UnbanReason", string);
			#endif
			SetSVarString(tmp, string);
			
			format(tmp,sizeof(tmp), "ib_UnbanBy%i", i+1);
			#if SERVER == MG
				cache_get_field_content(i, "UnbanBy", string, SQL_Connection);
			#else
				cache_get_value_name(i, "UnbanBy", string);
			#endif
			SetSVarString(tmp, string);
			
			format(tmp,sizeof(tmp), "ib_IP_UnbanReason%i", i+1);
			#if SERVER == MG
				cache_get_field_content(i, "IP_UnbanReason", string, SQL_Connection);
			#else
				cache_get_value_name(i, "IP_UnbanReason", string);
			#endif
			SetSVarString(tmp, string);
			
			format(tmp,sizeof(tmp), "ib_Serial_UnbanReason%i", i+1);
			#if SERVER == MG
				cache_get_field_content(i, "Serial_UnbanReason", string, SQL_Connection);
			#else
				cache_get_value_name(i, "Serial_UnbanReason", string);
			#endif
			SetSVarString(tmp, string);
			
			format(tmp,sizeof(tmp), "ib_Range_UnbanReason%i", i+1);
			#if SERVER == MG
				cache_get_field_content(i, "Range_UnbanReason", string, SQL_Connection);
			#else
				cache_get_value_name(i, "Range_UnbanReason", string);
			#endif
			SetSVarString(tmp, string);
			
			format(tmp,sizeof(tmp), "ib_IP_UnbanBy%i", i+1);
			#if SERVER == MG
				cache_get_field_content(i, "IP_UnbanBy", string, SQL_Connection);
			#else
				cache_get_value_name(i, "IP_UnbanBy", string);
			#endif
			SetSVarString(tmp, string);
			
			format(tmp,sizeof(tmp), "ib_Serial_UnbanBy%i", i+1);
			#if SERVER == MG
				cache_get_field_content(i, "Serial_UnbanBy", string, SQL_Connection);
			#else
				cache_get_value_name(i, "Serial_UnbanBy", string);
			#endif
			SetSVarString(tmp, string);
			
			format(tmp,sizeof(tmp), "ib_Range_UnbanBy%i", i+1);
			#if SERVER == MG
				cache_get_field_content(i, "Range_UnbanBy", string, SQL_Connection);
			#else
				cache_get_value_name(i, "Range_UnbanBy", string);
			#endif
			SetSVarString(tmp, string);
			
		}
	}
	#if SERVER != MG
	cache_delete(result);
	#else
	cache_delete(result,SQL_Connection);
	#endif
	return rows;
}

InsertTimeStamp()
{
	new StrSmall[50];
	new day,month,year,hour,minute,second;
	gettime(hour,minute,second);
	getdate(year,month,day);
	format(StrSmall,sizeof(StrSmall),"%d/%d/%d %d:%d:%d",year,month,day,hour,minute,second);
	return StrSmall;
}

FUNCTION DelayKick(playerid)
{
	Kick(playerid);
	return 1;
}

CheckSerial(playerid)
{
	gpci(playerid,gData[playerid][bSerial], 64);
	new Cache:result, string[256], rows = 0;
	mysql_format(SQL_Connection, string, sizeof(string), "SELECT * FROM %e WHERE Active = 1 AND Serial='%e' LIMIT 1", BAN_TABLE, gData[playerid][bSerial]);
	result = mysql_query(SQL_Connection, string);
	#if SERVER == 0
		rows = cache_get_row_count();
	#else
		cache_get_row_count(rows);
	#endif
	if(rows)
	{
		GetPlayerName(playerid, gData[playerid][bName], MAX_PLAYER_NAME);
		new banid;
		#if SERVER == 0
			banid=cache_get_field_content_int(0, "BanID", SQL_Connection);
		#else
			cache_get_value_name_int(0, "BanID", banid);
		#endif
		format(string,sizeof(string),"%s][BAN SYSTEM]: %s (%d) has same serial as Ban ID - %d - .",SERVER_NAME_IRC,gData[playerid][bName],playerid,banid);
		IRC_GroupSay(1,IRC_BAN_CHANNEL,string);
		CallRemoteFunction("OnPlayerSerialMatched","dd",playerid,banid);
	}
	#if SERVER != MG
	cache_delete(result);
	#else
	cache_delete(result,gCon);
	#endif
	return 1;
}
