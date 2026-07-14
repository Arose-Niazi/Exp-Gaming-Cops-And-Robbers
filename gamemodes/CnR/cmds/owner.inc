/*

	Owner commands for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 2 — design §11.5 / §6.2
	Last Updated: -----

	Credits:

	Functions:
		GrantRefreshPlayer(targetid)  : re-apply a live grant to an online player

	Commands:
		/atime <h> [m]                — set game clock + broadcast (OWNER)
		/makeadmin <id> <0-5>         — set staff rank (MANAGER+, up to rank<own)
		/makeregular <id> [0/1]       — toggle Regular Player (OWNER)
		/makedj <id> [0/1]            — toggle DJ (OWNER)
		/makedonator <id> [0/1]       — toggle VIP/Donator (OWNER)
		/startmoneyrush               — start the money-rush event (shell, OWNER)

	Functions Being Called:
		messages: SendMessageToAdmins, IRC_SendMessage

	Note: all grants persist via UPDATE players + a live PlayerInfo refresh.
	      No name-based logic anywhere (recorded security decision).
*/

// Re-fetch a granted player's account row so PlayerInfo reflects the DB write.
GrantRefreshPlayer(targetid)
{
	if(!IsPlayerConnected(targetid) || !PlayerInfo[targetid][pLoggedIn]) return 0;
	new string[128];
	mysql_format(g_SQL, string, sizeof(string), "SELECT Rank,Dj,Vip,RegularPlayer FROM "TABLE_PLAYERS" WHERE aID=%d LIMIT 1", PlayerInfo[targetid][pID]);
	mysql_pquery(g_SQL, string, "OnGrantRefresh", "dd", targetid, g_MysqlRaceCheck[targetid]);
	return 1;
}

FUNCTION OnGrantRefresh(targetid, race_check)
{
	if(!IsPlayerConnected(targetid) || !PlayerInfo[targetid][pLoggedIn]) return 0;
	if(race_check != g_MysqlRaceCheck[targetid]) return 0;
	new rows;
	cache_get_row_count(rows);
	if(!rows) return 0;
	cache_get_value_int(0, "Rank", PlayerInfo[targetid][pRank]);
	cache_get_value_bool(0, "Dj", PlayerInfo[targetid][pDj]);
	cache_get_value_bool(0, "Vip", PlayerInfo[targetid][pVip]);
	cache_get_value_bool(0, "RegularPlayer", PlayerInfo[targetid][pRegularPlayer]);
	return 1;
}

CMD:atime(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_OWNER) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new hour, minute;
	if(sscanf(params, "dD(0)", hour, minute))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /atime [hour 0-23] [minute 0-59]");
	if(hour < 0 || hour > 23 || minute < 0 || minute > 59)
		return SendClientMessage(playerid, COLOR_ERROR, "Hour must be 0-23 and minute 0-59.");
	GameHour = hour;
	GameMinute = minute;
	for(new i = 0; i < MAX_PLAYERS; i++)
		if(IsPlayerConnected(i)) SetPlayerTime(i, GameHour, GameMinute);
	new string[128];
	format(string, sizeof(string), ""COL_WHITE"Game time has been set to "COL_SERVER_INFO"%02d:%02d"COL_WHITE" by an owner.", GameHour, GameMinute);
	SendClientMessageToAll(COLOR_SERVER_INFO, string);
	IRC_SendMessage(string, IRC_ADMIN_COLOR_INFO);
	return 1;
}

CMD:makeadmin(playerid, params[])
{
	// MANAGER+ may grant, but only to a rank strictly below their own.
	if(PlayerInfo[playerid][pRank] < SERVER_MANAGER) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new targetid, rank;
	if(sscanf(params, "dd", targetid, rank))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /makeadmin [playerid] [rank 0-5]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || !PlayerInfo[targetid][pLoggedIn])
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected/logged in.");
	if(rank < 0 || rank > SERVER_MANAGER)
		return SendClientMessage(playerid, COLOR_ERROR, "Rank must be 0 (player) to 5 (manager). Use /makeowner or /makescripter for higher.");
	if(rank >= PlayerInfo[playerid][pRank])
		return SendClientMessage(playerid, COLOR_ERROR, "You can only grant a rank below your own.");
	if(PlayerInfo[targetid][pRank] >= PlayerInfo[playerid][pRank] && targetid != playerid)
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot change the rank of someone at or above your rank.");
	new string[160];
	mysql_format(g_SQL, string, sizeof(string), "UPDATE "TABLE_PLAYERS" SET Rank=%d WHERE aID=%d LIMIT 1", rank, PlayerInfo[targetid][pID]);
	mysql_pquery(g_SQL, string);
	GrantRefreshPlayer(targetid);
	format(string, sizeof(string), "%s (%d) set %s (%d) to staff rank %d.", PlayerInfo[playerid][pUserName], playerid, PlayerInfo[targetid][pUserName], targetid, rank);
	SendMessageToAdmins(COLOR_ADMIN_INFO, string);
	IRC_SendMessage(string, IRC_ADMIN_COLOR_INFO);
	SendClientMessage(targetid, COLOR_SERVER_INFO, "Your staff rank has been updated.");
	return 1;
}

// Generic boolean grant helper for makeregular/makedj/makedonator.
GrantBoolFlag(playerid, const params[], const column[], const label[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_OWNER) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new targetid, value;
	new usage[80];
	if(sscanf(params, "dD(-1)", targetid, value))
	{
		format(usage, sizeof(usage), "USAGE: /make%s [playerid] [0/1] (blank = toggle)", label);
		return SendClientMessage(playerid, COLOR_SERVER_INFO, usage);
	}
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || !PlayerInfo[targetid][pLoggedIn])
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected/logged in.");
	new bool:cur;
	if(!strcmp(column, "Dj")) cur = PlayerInfo[targetid][pDj];
	else if(!strcmp(column, "Vip")) cur = PlayerInfo[targetid][pVip];
	else cur = PlayerInfo[targetid][pRegularPlayer];
	new set = (value == -1) ? (cur ? 0 : 1) : (value ? 1 : 0);
	new string[160];
	mysql_format(g_SQL, string, sizeof(string), "UPDATE "TABLE_PLAYERS" SET %s=%d WHERE aID=%d LIMIT 1", column, set, PlayerInfo[targetid][pID]);
	mysql_pquery(g_SQL, string);
	GrantRefreshPlayer(targetid);
	format(string, sizeof(string), "%s (%d) %s %s status for %s (%d).", PlayerInfo[playerid][pUserName], playerid, set ? "enabled" : "disabled", label, PlayerInfo[targetid][pUserName], targetid);
	SendMessageToAdmins(COLOR_ADMIN_INFO, string);
	IRC_SendMessage(string, IRC_ADMIN_COLOR_INFO);
	format(string, sizeof(string), "Your %s status has been %s.", label, set ? "enabled" : "disabled");
	SendClientMessage(targetid, COLOR_SERVER_INFO, string);
	return 1;
}

CMD:makeregular(playerid, params[]) return GrantBoolFlag(playerid, params, "RegularPlayer", "regular");
CMD:makedj(playerid, params[])      return GrantBoolFlag(playerid, params, "Dj", "dj");
CMD:makedonator(playerid, params[]) return GrantBoolFlag(playerid, params, "Vip", "donator");

// M4 (stage 2) — start (or force-end) the money-rush cash-rain event (§8.2).
// The event mechanics live in systems/moneyrush.inc; this is the owner gate.
CMD:startmoneyrush(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_OWNER) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);

	// /startmoneyrush stop → force-end a running rush early.
	if(!isNull(params) && !strcmp(params, "stop", true))
	{
		if(!Moneyrush_IsActive()) return SendClientMessage(playerid, COLOR_ERROR, "No money-rush event is running.");
		Moneyrush_ForceEnd(playerid);
		new s[128];
		format(s, sizeof(s), "%s (%d) force-ended the money-rush event.", PlayerInfo[playerid][pUserName], playerid);
		SendMessageToAdmins(COLOR_ADMIN_INFO, s);
		IRC_SendMessage(s, IRC_ADMIN_COLOR_INFO);
		return 1;
	}

	if(Moneyrush_IsActive()) return SendClientMessage(playerid, COLOR_ERROR, "A money-rush event is already running. Use /startmoneyrush stop to end it.");
	if(!Moneyrush_Start(playerid))
		return SendClientMessage(playerid, COLOR_ERROR, "The money-rush is on cooldown — try again shortly.");

	new string[128];
	format(string, sizeof(string), "%s (%d) started a money-rush event.", PlayerInfo[playerid][pUserName], playerid);
	SendMessageToAdmins(COLOR_ADMIN_INFO, string);
	IRC_SendMessage(string, IRC_ADMIN_COLOR_INFO);
	return 1;
}
