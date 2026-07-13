/*

	Scripter commands for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 2 — design §11.5 / §6.3
	Last Updated: -----

	Credits:

	Functions:

	Commands:
		/makeowner <id>      — set players.Rank = SERVER_OWNER (SCRIPTER)
		/makescripter <id>   — set players.Rank = SERVER_SCRIPTER (SCRIPTER)
		/lgoto <name>        — teleport to a named saved location (SCRIPTER)
		/debug               — toggle debug output (SCRIPTER)

	Functions Being Called:
		messages: SendMessageToAdmins, SendMessageToOwners, IRC_SendMessage
		owner:    GrantRefreshPlayer

	Note: /addvehicle and /addinterior already exist (players/vehicles.inc,
	      server/int_tele_pickups.inc) and are re-gated to SERVER_SCRIPTER there.
*/

// Named saved-location table for /lgoto.
enum e_LGoto
{
	Float:lgX,
	Float:lgY,
	Float:lgZ,
	lgInt,
	lgVW,
	lgName[24]
};

new const LGotoData[][e_LGoto] =
{
	// X            Y           Z         Int VW  Name
	{  1481.0912, -1771.1055,   18.7958, 0,  0, "pd" },
	{  1544.1024, -1675.5677,  16.1953, 0,  0, "cityhall" },
	{  2035.9155, -1417.4022,  17.1471, 0,  0, "hospital" },
	{  1421.4783, -1046.6710,  25.6717, 0,  0, "bank" },
	{  2447.1584, -1971.3993,  13.5469, 0,  0, "grove" }
};

CMD:makeowner(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_SCRIPTER) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new targetid;
	if(sscanf(params, "d", targetid))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /makeowner [playerid]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || !PlayerInfo[targetid][pLoggedIn])
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected/logged in.");
	new string[160];
	mysql_format(g_SQL, string, sizeof(string), "UPDATE "TABLE_PLAYERS" SET Rank=%d WHERE aID=%d LIMIT 1", SERVER_OWNER, PlayerInfo[targetid][pID]);
	mysql_pquery(g_SQL, string);
	GrantRefreshPlayer(targetid);
	format(string, sizeof(string), "%s (%d) promoted %s (%d) to Server Owner.", PlayerInfo[playerid][pUserName], playerid, PlayerInfo[targetid][pUserName], targetid);
	SendMessageToAdmins(COLOR_ADMIN_INFO, string);
	IRC_SendMessage(string, IRC_ADMIN_COLOR_INFO);
	SendClientMessage(targetid, COLOR_SERVER_INFO, "You have been promoted to "COL_RED"Server Owner"COL_WHITE".");
	return 1;
}

CMD:makescripter(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_SCRIPTER) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new targetid;
	if(sscanf(params, "d", targetid))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /makescripter [playerid]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || !PlayerInfo[targetid][pLoggedIn])
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected/logged in.");
	new string[160];
	mysql_format(g_SQL, string, sizeof(string), "UPDATE "TABLE_PLAYERS" SET Rank=%d WHERE aID=%d LIMIT 1", SERVER_SCRIPTER, PlayerInfo[targetid][pID]);
	mysql_pquery(g_SQL, string);
	GrantRefreshPlayer(targetid);
	format(string, sizeof(string), "%s (%d) promoted %s (%d) to Server Scripter.", PlayerInfo[playerid][pUserName], playerid, PlayerInfo[targetid][pUserName], targetid);
	SendMessageToAdmins(COLOR_ADMIN_INFO, string);
	IRC_SendMessage(string, IRC_ADMIN_COLOR_INFO);
	SendClientMessage(targetid, COLOR_SERVER_INFO, "You have been promoted to "COL_RED"Server Scripter"COL_WHITE".");
	return 1;
}

CMD:lgoto(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_SCRIPTER) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new name[24], string[160];
	if(sscanf(params, "s[24]", name))
	{
		SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /lgoto [name]");
		new list[160] = "Locations:";
		for(new i = 0; i < sizeof(LGotoData); i++)
		{
			format(string, sizeof(string), "%s %s", list, LGotoData[i][lgName]);
			format(list, sizeof(list), "%s", string);
		}
		return SendClientMessage(playerid, COLOR_SERVER_INFO, list);
	}
	for(new i = 0; i < sizeof(LGotoData); i++)
	{
		if(!strcmp(name, LGotoData[i][lgName], true))
		{
			SetPlayerPos(playerid, LGotoData[i][lgX], LGotoData[i][lgY], LGotoData[i][lgZ]);
			SetPlayerInterior(playerid, LGotoData[i][lgInt]);
			SetPlayerVirtualWorld(playerid, LGotoData[i][lgVW]);
			format(string, sizeof(string), "Teleported to %s.", LGotoData[i][lgName]);
			return SendClientMessage(playerid, COLOR_SERVER_INFO, string);
		}
	}
	return SendClientMessage(playerid, COLOR_ERROR, "Unknown location. Type /lgoto for the list.");
}

CMD:debug(playerid)
{
	if(PlayerInfo[playerid][pRank] < SERVER_SCRIPTER) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	PlayerInfo[playerid][pDebugMode] = !PlayerInfo[playerid][pDebugMode];
	new string[128];
	format(string, sizeof(string), "%s (%d) turned debug mode %s.", PlayerInfo[playerid][pUserName], playerid, PlayerInfo[playerid][pDebugMode] ? "ON" : "OFF");
	SendMessageToOwners(COLOR_ADMIN_INFO, string);
	IRC_SendMessage(string, IRC_ADMIN_COLOR_INFO);
	return 1;
}
