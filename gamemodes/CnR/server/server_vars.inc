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
	sLottoJackpot		// M4 (stage 2) — accumulating lotto jackpot (server_data.LottoJackpot, §9.5/§11.4)
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