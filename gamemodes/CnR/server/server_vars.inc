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
	sTimes_GPSUsed
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
	}
	else
	{
		new string[75];
		mysql_format(g_SQL, string, sizeof(string), "INSERT INTO "SERVER_TABLE" (Version) VALUES (%d)",STATS_VERSION);
		mysql_pquery(g_SQL, string);
	}
	return 1;
}