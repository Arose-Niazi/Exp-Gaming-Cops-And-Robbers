/*
	bans script
	Scripted by Arose Niazi.
	Scripting started on 19th October 2017
	Last Updated: ----
	
	Credits:
		
	Functions:
		BanCheckDone(playerid,banned)
		
	Functions Being Called:
		
*/
FUNCTION BanCheckDone(playerid,banned)
{
	new string[128];
	if(banned)
	{
		TextDrawHideForPlayer(playerid,ConnectTD[0]);
		TextDrawHideForPlayer(playerid,ConnectTD[1]);
		KickPlayer(playerid,SERVER_BOT,"Banned");
	}	
	else
	{
		GetPlayerName(playerid,string,sizeof(string));
		mysql_format(g_SQL, string,sizeof(string),"SELECT * FROM "TABLE_PLAYERS" WHERE UserName='%e' LIMIT 1",string);
		mysql_pquery(g_SQL, string, "CheckPlayer","dd",playerid,g_MysqlRaceCheck[playerid]);
	}	
	return 1;
}