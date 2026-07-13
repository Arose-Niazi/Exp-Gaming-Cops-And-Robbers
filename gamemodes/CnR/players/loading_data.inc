/*

	Loading Script
	Scripted by Arose Niazi.
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
	cache_get_value_bool(0, "ClassMusic",PlayerInfo[playerid][pClassMusic]);
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
	cache_get_value_int(0, "AdminJailUntil",PlayerInfo[playerid][pAdminJailUntil]);
	cache_get_value_int(0, "Wanted",PlayerInfo[playerid][pWanted]);	// M3 — wanted level (§2.1)
	// M3 (stage 2) — cop rewards + jail persistence (design §3).
	cache_get_value_int(0, "Score",PlayerInfo[playerid][pScore]);
	cache_get_value_int(0, "CopRank",PlayerInfo[playerid][pCopRank]);
	cache_get_value_int(0, "RefillPoints",PlayerInfo[playerid][pRefillPoints]);	// M3 — cop /refill points (§3.1)
	cache_get_value_int(0, "SeriousCrimes",PlayerInfo[playerid][pSeriousCrimes]);
	cache_get_value_bool(0, "EscapeChained",PlayerInfo[playerid][pEscapeChained]);
	cache_get_value_int(0, "Bail",PlayerInfo[playerid][pJailBail]);
	cache_get_value_name(0, "JailReason", PlayerInfo[playerid][pJailReason],64);
	// JailUntil is stored as absolute unix time (mirrors AdminJailUntil); the
	// spawn re-jail check (CnR.pwn) re-applies it, or clears it if it lapsed.
	cache_get_value_int(0, "JailUntil",PlayerInfo[playerid][pJailUntil]);
	if(PlayerInfo[playerid][pJailUntil] > gettime()) PlayerInfo[playerid][pJailed]=true;
	// M3 (stage 3) — crime/drug/STD persistence (design §4).
	cache_get_value_int(0, "Skill",PlayerInfo[playerid][pSkill]);
	cache_get_value_int(0, "Drugs",PlayerInfo[playerid][pDrugs]);
	cache_get_value_int(0, "STDs",PlayerInfo[playerid][pSTD]);
	cache_get_value_int(0, "Condoms",PlayerInfo[playerid][pCondoms]);
	cache_get_value_bool(0, "ChastityBelt",PlayerInfo[playerid][pChastityBelt]);
	TogglePlayerSpectating(playerid,false);
	return 1;
}