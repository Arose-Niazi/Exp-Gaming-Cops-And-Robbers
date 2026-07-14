/*

	server data Script
	Scripted by Arose Niazi.
	Scripting started on 24th October 2017
	Last Updated: ----
	
	Credits:
		
	Functions:
		OnServerDataLoad()
		
	Functions Being Called:
*/

enum sInfo {
	sPlayersOnline,
	bool:sChat_Locked,
	sWeeksCompleted,
	sTimer,
	sTimes_GPSUsed,
	sLottoJackpot,		// M4 (stage 2) — accumulating lotto jackpot (server_data.LottoJackpot, §9.5/§11.4)
	// M5 (stage 2) — per-branch bank-robbery cooldown: gettime() the branch may be
	// robbed again (server_data.BankRobLast{LS,SF,LV} DATETIME, §5.3/§5.8). Stored
	// in the DB as a DATETIME string but tracked in-code as a unix timestamp.
	sBankRobLastLS,
	sBankRobLastSF,
	sBankRobLastLV
};
new ServerInfo[sInfo];

FUNCTION OnServerDataLoad()
{
	new rows;
	cache_get_row_count(rows);
	if(rows)
	{
		cache_get_value_int(0, "WeeksCompleted",ServerInfo[sWeeksCompleted]);
		cache_get_value_int(0, "Times_GPSUsed",ServerInfo[sTimes_GPSUsed]);
		cache_get_value_int(0, "LottoJackpot",ServerInfo[sLottoJackpot]);
		if(ServerInfo[sLottoJackpot] < LOTTO_JACKPOT_BASE) ServerInfo[sLottoJackpot] = LOTTO_JACKPOT_BASE;
		// M5 (stage 2) — per-branch bank-robbery cooldowns. Stored as DATETIME; the
		// load query (OnGameModeInit) converts them to unix seconds via
		// UNIX_TIMESTAMP() and aliases them BankRobUnix{LS,SF,LV} (§5.3/§5.8). A NULL
		// DATETIME converts to 0 (never robbed). Saved back via FROM_UNIXTIME().
		cache_get_value_int(0, "BankRobUnixLS", ServerInfo[sBankRobLastLS]);
		cache_get_value_int(0, "BankRobUnixSF", ServerInfo[sBankRobLastSF]);
		cache_get_value_int(0, "BankRobUnixLV", ServerInfo[sBankRobLastLV]);
	}
	else
	{
		new string[75];
		mysql_format(g_SQL, string, sizeof(string), "INSERT INTO "SERVER_TABLE" (Version) VALUES (%d)",STATS_VERSION);
		mysql_pquery(g_SQL, string);
		ServerInfo[sLottoJackpot] = LOTTO_JACKPOT_BASE;
	}
	return 1;
}

// M4 (stage 2) — persist the accumulating lotto jackpot to server_data. Called
// after ticket sales feed it and after a draw pays out / rolls over (§9.5).
FUNCTION SaveLottoJackpot()
{
	new string[128];
	mysql_format(g_SQL, string, sizeof(string), "UPDATE "SERVER_TABLE" SET LottoJackpot=%d WHERE Version=%d", ServerInfo[sLottoJackpot], STATS_VERSION);
	mysql_pquery(g_SQL, string);
	return 1;
}

// M5 (stage 2) — persist the per-branch bank-robbery cooldowns (§5.3/§5.8). The
// in-code values are unix timestamps; FROM_UNIXTIME() writes them into the
// DATETIME columns (a 0 stamp → NULL = never robbed). Called after a bank rob
// stamps a branch so the 60-minute close survives a restart.
FUNCTION SaveBankRobCooldowns()
{
	new string[256];
	mysql_format(g_SQL, string, sizeof(string),
		"UPDATE "SERVER_TABLE" SET BankRobLastLS=IF(%d=0,NULL,FROM_UNIXTIME(%d)), BankRobLastSF=IF(%d=0,NULL,FROM_UNIXTIME(%d)), BankRobLastLV=IF(%d=0,NULL,FROM_UNIXTIME(%d)) WHERE Version=%d",
		ServerInfo[sBankRobLastLS], ServerInfo[sBankRobLastLS],
		ServerInfo[sBankRobLastSF], ServerInfo[sBankRobLastSF],
		ServerInfo[sBankRobLastLV], ServerInfo[sBankRobLastLV],
		STATS_VERSION);
	mysql_pquery(g_SQL, string);
	return 1;
}