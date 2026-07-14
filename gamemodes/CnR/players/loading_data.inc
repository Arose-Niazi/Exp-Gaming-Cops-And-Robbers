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
	// M6 (stage 1) — the drug bag doubles the Drug Dealer carry cap (§4.3/§16 #2);
	// bought via /clotheswear, persisted so it survives relogs (drugs.inc reads it).
	cache_get_value_bool(0, "HasDrugBag",PlayerInfo[playerid][pHasDrugBag]);
	cache_get_value_int(0, "STDs",PlayerInfo[playerid][pSTD]);
	cache_get_value_int(0, "Condoms",PlayerInfo[playerid][pCondoms]);
	cache_get_value_bool(0, "ChastityBelt",PlayerInfo[playerid][pChastityBelt]);
	// M4 (stage 1) — bank / taxes / insurance persistence (design §10/§11.2).
	cache_get_value_int(0, "BankMoney",PlayerInfo[playerid][pBankMoney]);
	cache_get_value_int(0, "TaxOwed",PlayerInfo[playerid][pTaxOwed]);
	cache_get_value_int(0, "LifeInsurance",PlayerInfo[playerid][pLifeInsurance]);
	cache_get_value_int(0, "HealthInsExpiry",PlayerInfo[playerid][pHealthInsExpiry]);
	// M4 (stage 2) — lotto ticket for the current game-day (§9.5). Cleared for
	// all rows (online + offline) by the bulk UPDATE in Lotto_Draw(); loading a
	// non-zero value here means the draw has not yet run this game-day.
	cache_get_value_int(0, "LottoNumber",PlayerInfo[playerid][pLottoNumber]);
	// M5 (stage 2) — crowbar equip flag + robbery-history bitmask (§5.7). Persisted
	// so an equipped crowbar survives relogs and the confiscation flag is durable.
	cache_get_value_bool(0, "CrowbarEquipped",PlayerInfo[playerid][pCrowbarEquipped]);
	cache_get_value_int(0, "RobberyHistory",PlayerInfo[playerid][pRobberyHistory]);
	// M5 (stage 3) — the houses.ID this player spawns at (/sethousespawn, §9.1). -1 =
	// default city spawn. House_OnPlayerLogin resolves it to a house index + tallies
	// how many houses this account owns (own-cap enforcement). housing.inc is
	// included after this module, so it is reached via CallLocalFunction.
	cache_get_value_int(0, "HouseSpawnID",PlayerInfo[playerid][pHouseSpawn]);
	CallLocalFunction("House_OnPlayerLogin", "d", playerid);
	// M6 (stage 1) — chosen fighting style (LSplayers.FightStyle, default 4=NORMAL).
	// Re-applied with SetPlayerFightingStyle on every spawn (§9.8). Column DEFAULT is
	// 4, so a first-time / migrated row loads NORMAL; guard against a garbage read.
	cache_get_value_int(0, "FightStyle",PlayerInfo[playerid][pFightStyle]);
	if(PlayerInfo[playerid][pFightStyle] < 0) PlayerInfo[playerid][pFightStyle]=FIGHTSTYLE_DEFAULT;
	// M6 (stage 2) — fishing & farming persistence (design §9.2/§9.3/§11.2). Permits,
	// rod/cooler/sales-permit flags and the compact cooler aggregate (count + pounds +
	// accumulated base value) survive relogs; the plant-count is tallied from the
	// plants table by Farm_OnPlayerLogin (farming.inc is included after this module).
	cache_get_value_int(0, "FishPermits",PlayerInfo[playerid][pFishPermits]);
	cache_get_value_int(0, "HuntPermits",PlayerInfo[playerid][pHuntPermits]);
	cache_get_value_int(0, "DrugSeeds",PlayerInfo[playerid][pDrugSeeds]);
	cache_get_value_bool(0, "HasFishingRod",PlayerInfo[playerid][pHasFishingRod]);
	cache_get_value_bool(0, "HasFishCooler",PlayerInfo[playerid][pHasFishCooler]);
	cache_get_value_bool(0, "FishSalesPermit",PlayerInfo[playerid][pHasFishSalesPermit]);
	cache_get_value_int(0, "CoolerCount",PlayerInfo[playerid][pCoolerCount]);
	cache_get_value_int(0, "CoolerWeight",PlayerInfo[playerid][pCoolerWeight]);
	cache_get_value_int(0, "CoolerValue",PlayerInfo[playerid][pCoolerValue]);
	CallLocalFunction("Farm_OnPlayerLogin", "d", playerid);
	// M7 — load this player's stock holdings from stock_holdings (async, race-guarded).
	// stocks.inc is included after this module, so it is reached via CallLocalFunction.
	CallLocalFunction("Stocks_OnPlayerLogin", "d", playerid);
	// M4 (stage 3) — mission cooldowns: one packed '|'-delimited column of per-mission
	// game-hour completion stamps (§6). Parse into the per-mission stamp array.
	new missionCd[255];	// widened 128->255 in M6 stage 4 for the 19-mission roster
	cache_get_value_name(0, "MissionCooldowns", missionCd, sizeof(missionCd));
	Mission_LoadCooldowns(playerid, missionCd);
	TogglePlayerSpectating(playerid,false);
	return 1;
}