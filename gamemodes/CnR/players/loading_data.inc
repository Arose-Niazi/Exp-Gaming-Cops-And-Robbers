/*

	Loading Script
	Scripted by Sasuke_Uchiha.
	Scripting started on 18th October 2017
	Last Updated: -----
	
	Credits:
		
	Functions:
		LoadPlayerData(playerid) - Overall
		LoadUserData(playerid) - City Account
		
	Functions Being Called:
		Menu: 
			UpdateMenuBG(playerid)
		
*/
FUNCTION LoadPlayerData(playerid)
{
	cache_get_value_name(0, "Password", PlayerInfo[playerid][pPassword],MAX_HASH_SIZE);
	cache_get_value_int(0, "aID",PlayerInfo[playerid][pID]);
	cache_get_value_name(0, "RegisterDate", PlayerInfo[playerid][pRegisteredDate],50);
	cache_get_value_name(0, "Email", PlayerInfo[playerid][pEmail],MAX_EMAIL_SIZE);
	cache_get_value_int(0, "Rank",PlayerInfo[playerid][pRank]);
	cache_get_value_bool(0, "Dj",PlayerInfo[playerid][pDj]);
	cache_get_value_bool(0, "Vip",PlayerInfo[playerid][pVip]);
	cache_get_value_bool(0, "RegularPlayer",PlayerInfo[playerid][pRegularPlayer]);
	cache_get_value_bool(0, "AutoLogin",PlayerInfo[playerid][pAutoLogin]);
	cache_get_value_bool(0, "HourlyTimeUpdate",PlayerInfo[playerid][pHourlyTimeUpdate]);
	cache_get_value_bool(0, "ShowJoinMessages",PlayerInfo[playerid][pShowJoinMessages]);
	cache_get_value_bool(0, "ClassMusic",PlayerInfo[playerid][pAutoLogin]);
	cache_get_value_int(0, "MenuTD_BG",PlayerInfo[playerid][pMenuTD_BG]);
	CallLocalFunction("UpdateMenuBG","d",playerid);
	new string[75];
	mysql_format(g_SQL, string,sizeof(string),"SELECT * FROM "TABLE_PLAYERS_C" WHERE aID=%d LIMIT 1",PlayerInfo[playerid][pID]);
	mysql_pquery(g_SQL, string, "LoadUserData","d",playerid);
	return 1;
}


FUNCTION LoadUserData(playerid)
{
	new string[2000];
	cache_get_value_int(0, "Money",PlayerInfo[playerid][pMoney]);
	cache_get_value_int(0, "ClassID",PlayerInfo[playerid][pClassID]);
	cache_get_value_int(0, "Skin",PlayerInfo[playerid][pSkin]);
	cache_get_value_int(0, "Gender",PlayerInfo[playerid][pGender]);
	cache_get_value_name(0, "GenderChanges", string,30);
	if(strlen(string) > 1)
	{
		for(new i=0; i<3; i++)
		{
			if(strlen(string) < 1) break;
			sscanf(string, "p<|>i", PlayerInfo[playerid][pGenderChanges][i]);
			new divider=strfind(string, "|", true);
			if(divider != -1) strdel(string, 0, divider+1);
		}
	}
	cache_get_value_name(0, "SkinsSelected", string,1555);
	if(strlen(string) > 1)
	{
		for(new i=0; i<MAX_PEDS; i++)
		{
			if(strlen(string) < 1) break;
			sscanf(string, "p<|>i", PlayerInfo[playerid][pSkinsSelected][i]);
			new divider=strfind(string, "|", true);
			if(divider != -1) strdel(string, 0, divider+1);
		}
	}
	cache_get_value_int(0, "Spawns",PlayerInfo[playerid][pSpawns]);
	cache_get_value_int(0, "MonthlyActivity",PlayerInfo[playerid][pMonthlyActivity]);
	cache_get_value_int(0, "Times_GPSUsed",PlayerInfo[playerid][pTimes_GPSUsed]);
	TogglePlayerSpectating(playerid,false);
	return 1;
}