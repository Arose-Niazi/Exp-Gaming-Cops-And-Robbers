/*

	Native ban system for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 2 — design §11.3

	Replaces the legacy Bans filterscript (shared MG/MM ban DB whose credentials
	are burned — never reused). All ban state now lives in the local `bans` table
	(§9.3): Type (0 name, 1 IP, 2 serial), Value, Nick, AdminName, Reason, Date,
	Expiry (NULL = permanent), Active.

	Credits:

	Functions:
		OnBanCheck(playerid, race_check)  : async ban-lookup result → BanCheckDone
		InsertBan(type, value, nick, admin, reason, days)
		BanTarget(adminid, targetid, days, reason)  : ban name+IP+serial, kick

	Commands (MOD unless noted):
		/ban <id> [days] [reason]
		/offlineban <name> [days] [reason]
		/unban <name>       (ADMIN)
		/unbannick <name>   (ADMIN)
		/unbanid <banID>    (ADMIN)

	Functions Being Called:
		bans:     BanCheckDone (login_register calls CallForChecking → here)
		messages: SendMessageToAdmins, IRC_SendMessage
*/

// Ban types.
#define BAN_TYPE_NAME		0
#define BAN_TYPE_IP			1
#define BAN_TYPE_SERIAL		2

// Async result of the ban lookup fired from CallForChecking (login_register).
FUNCTION OnBanCheck(playerid, race_check)
{
	if(!IsPlayerConnected(playerid)) return 0;
	if(race_check != g_MysqlRaceCheck[playerid]) return Kick(playerid);

	new rows;
	cache_get_row_count(rows);
	if(rows > 0)
	{
		// Show the ban reason/expiry, then kick after a short delay.
		new reason[128], admin[MAX_PLAYER_NAME], expiry[32], string[400];
		cache_get_value_name(0, "Reason", reason, sizeof(reason));
		cache_get_value_name(0, "AdminName", admin, sizeof(admin));
		cache_get_value_name(0, "Expiry", expiry, sizeof(expiry));
		if(isNull(expiry) || !strcmp(expiry, "NULL", true)) format(expiry, sizeof(expiry), "Never (permanent)");
		format(string, sizeof(string), ""COL_RED"You are banned from "COMMUNITY_NAME".\n\n"COL_WHITE"Reason: "COL_YELLOW"%s\n"COL_WHITE"By: "COL_YELLOW"%s\n"COL_WHITE"Expires: "COL_YELLOW"%s\n\n"COL_WHITE"Appeal on the community forums if you believe this is a mistake.", isNull(reason) ? ("No reason given") : reason, isNull(admin) ? (SERVER_BOT) : admin, expiry);
		ShowPlayerDialog(playerid, BAN_DIALOG, DIALOG_STYLE_MSGBOX, ""COMMUNITY_NAME" — Banned", string, "Okay", "");
		CallLocalFunction("BanCheckDone", "dd", playerid, 1);
	}
	else
	{
		CallLocalFunction("BanCheckDone", "dd", playerid, 0);
	}
	return 1;
}

// Insert a ban row. days<=0 → permanent (Expiry NULL).
InsertBan(type, const value[], const nick[], const admin[], const reason[], days)
{
	new string[400];
	if(days > 0)
	{
		mysql_format(g_SQL, string, sizeof(string),
			"INSERT INTO bans (Type,Value,Nick,AdminName,Reason,Date,Expiry,Active) VALUES (%d,'%e','%e','%e','%e',NOW(),DATE_ADD(NOW(), INTERVAL %d DAY),1)",
			type, value, nick, admin, reason, days);
	}
	else
	{
		mysql_format(g_SQL, string, sizeof(string),
			"INSERT INTO bans (Type,Value,Nick,AdminName,Reason,Date,Expiry,Active) VALUES (%d,'%e','%e','%e','%e',NOW(),NULL,1)",
			type, value, nick, admin, reason);
	}
	mysql_pquery(g_SQL, string);
	return 1;
}

// Ban an online target on name + IP + serial, echo, and kick.
BanTarget(adminid, targetid, days, const reason[])
{
	new serial[MAX_SERIAL_SIZE];
	gpci(targetid, serial, MAX_SERIAL_SIZE);
	new adminName[MAX_PLAYER_NAME];
	GetPlayerName(adminid, adminName, MAX_PLAYER_NAME);

	InsertBan(BAN_TYPE_NAME, PlayerInfo[targetid][pUserName], PlayerInfo[targetid][pUserName], adminName, reason, days);
	if(!isNull(PlayerInfo[targetid][pIP])) InsertBan(BAN_TYPE_IP, PlayerInfo[targetid][pIP], PlayerInfo[targetid][pUserName], adminName, reason, days);
	if(!isNull(serial)) InsertBan(BAN_TYPE_SERIAL, serial, PlayerInfo[targetid][pUserName], adminName, reason, days);

	new string[256];
	if(days > 0) format(string, sizeof(string), "has banned %s (%d) for %d day(s). Reason: %s", PlayerInfo[targetid][pUserName], targetid, days, reason);
	else format(string, sizeof(string), "has permanently banned %s (%d). Reason: %s", PlayerInfo[targetid][pUserName], targetid, reason);
	SendAdminAnonmityMessage(adminid, COLOR_ADMIN_INFO, string, IRC_BAN_CHANNEL);

	KickPlayerHidden(targetid, adminid, "Banned");
	return 1;
}

CMD:ban(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new targetid, days, reason[128];
	if(sscanf(params, "dD(0)z[128]", targetid, days, reason))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /ban [playerid] [days, 0=perm] [reason]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || !PlayerInfo[targetid][pLoggedIn])
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected/logged in.");
	if(PlayerInfo[targetid][pRank] >= PlayerInfo[playerid][pRank] && targetid != playerid)
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot ban someone at or above your rank.");
	if(days < 0) return SendClientMessage(playerid, COLOR_ERROR, "Days cannot be negative (use 0 for permanent).");
	if(isNull(reason)) format(reason, sizeof(reason), "No reason given");
	BanTarget(playerid, targetid, days, reason);
	return 1;
}

CMD:offlineban(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new name[MAX_PLAYER_NAME], days, reason[128];
	if(sscanf(params, "s[24]D(0)z[128]", name, days, reason))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /offlineban [name] [days, 0=perm] [reason]");
	if(days < 0) return SendClientMessage(playerid, COLOR_ERROR, "Days cannot be negative (use 0 for permanent).");
	if(isNull(reason)) format(reason, sizeof(reason), "No reason given");
	new adminName[MAX_PLAYER_NAME];
	GetPlayerName(playerid, adminName, MAX_PLAYER_NAME);

	// Ban the name directly; resolve the account's stored IP/serial async.
	InsertBan(BAN_TYPE_NAME, name, name, adminName, reason, days);
	new string[300];
	mysql_format(g_SQL, string, sizeof(string), "SELECT Latest_IP,Latest_Serial FROM "TABLE_PLAYERS" WHERE UserName='%e' LIMIT 1", name);
	// Pack days/reason/admin/name through to the async completion.
	new packed[400];
	format(packed, sizeof(packed), "%d|%s|%s|%s", days, name, adminName, reason);
	mysql_pquery(g_SQL, string, "OnOfflineBanResolve", "ds", playerid, packed);

	format(string, sizeof(string), "%s offline-banned account '%s' (%s). Reason: %s", adminName, name, (days > 0) ? ("temp") : ("perm"), reason);
	SendMessageToAdmins(COLOR_ADMIN_INFO, string);
	IRC_SendMessage(string, IRC_BAN_CHANNEL);
	return 1;
}

// Completion for /offlineban: adds IP/serial ban rows if the account exists.
FUNCTION OnOfflineBanResolve(playerid, const packed[])
{
	new rows;
	cache_get_row_count(rows);
	if(!rows)
	{
		if(IsPlayerConnected(playerid)) SendClientMessage(playerid, COLOR_SERVER_INFO, "Name banned. (No account row found for IP/serial — name-ban only.)");
		return 0;
	}
	new ip[MAX_IP_SIZE], serial[MAX_SERIAL_SIZE];
	cache_get_value_name(0, "Latest_IP", ip, sizeof(ip));
	cache_get_value_name(0, "Latest_Serial", serial, sizeof(serial));

	// Unpack: days|name|admin|reason
	new days, name[MAX_PLAYER_NAME], admin[MAX_PLAYER_NAME], reason[128], tmp[400];
	format(tmp, sizeof(tmp), "%s", packed);
	sscanf(tmp, "p<|>ds[24]s[24]s[128]", days, name, admin, reason);

	if(!isNull(ip)) InsertBan(BAN_TYPE_IP, ip, name, admin, reason, days);
	if(!isNull(serial)) InsertBan(BAN_TYPE_SERIAL, serial, name, admin, reason, days);
	if(IsPlayerConnected(playerid)) SendClientMessage(playerid, COLOR_SERVER_INFO, "Offline ban applied (name + IP + serial).");
	return 1;
}

CMD:unban(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_ADMIN) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new name[MAX_PLAYER_NAME];
	if(sscanf(params, "s[24]", name))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /unban [name]  (lifts name + IP + serial bans tied to that account)");
	new string[400];
	// Lift the name ban plus any IP/serial ban whose Value matches this account's.
	mysql_format(g_SQL, string, sizeof(string),
		"UPDATE bans SET Active=0 WHERE (Type=%d AND Value='%e') OR (Nick='%e') OR (Type=%d AND Value=(SELECT Latest_IP FROM "TABLE_PLAYERS" WHERE UserName='%e' LIMIT 1)) OR (Type=%d AND Value=(SELECT Latest_Serial FROM "TABLE_PLAYERS" WHERE UserName='%e' LIMIT 1))",
		BAN_TYPE_NAME, name, name, BAN_TYPE_IP, name, BAN_TYPE_SERIAL, name);
	mysql_pquery(g_SQL, string);
	new adminName[MAX_PLAYER_NAME];
	GetPlayerName(playerid, adminName, MAX_PLAYER_NAME);
	format(string, sizeof(string), "%s unbanned account '%s' (name + IP + serial).", adminName, name);
	SendMessageToAdmins(COLOR_ADMIN_INFO, string);
	IRC_SendMessage(string, IRC_BAN_CHANNEL);
	return 1;
}

CMD:unbannick(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_ADMIN) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new nick[MAX_PLAYER_NAME];
	if(sscanf(params, "s[24]", nick))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /unbannick [nick]  (lifts every ban row logged under that nick)");
	new string[220];
	mysql_format(g_SQL, string, sizeof(string), "UPDATE bans SET Active=0 WHERE Nick='%e'", nick);
	mysql_pquery(g_SQL, string);
	new adminName[MAX_PLAYER_NAME];
	GetPlayerName(playerid, adminName, MAX_PLAYER_NAME);
	format(string, sizeof(string), "%s unbanned all ban rows under nick '%s'.", adminName, nick);
	SendMessageToAdmins(COLOR_ADMIN_INFO, string);
	IRC_SendMessage(string, IRC_BAN_CHANNEL);
	return 1;
}

CMD:unbanid(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_ADMIN) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new banID;
	if(sscanf(params, "d", banID))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /unbanid [banID]");
	new string[160];
	mysql_format(g_SQL, string, sizeof(string), "UPDATE bans SET Active=0 WHERE BanID=%d LIMIT 1", banID);
	mysql_pquery(g_SQL, string);
	new adminName[MAX_PLAYER_NAME];
	GetPlayerName(playerid, adminName, MAX_PLAYER_NAME);
	format(string, sizeof(string), "%s lifted ban ID #%d.", adminName, banID);
	SendMessageToAdmins(COLOR_ADMIN_INFO, string);
	IRC_SendMessage(string, IRC_BAN_CHANNEL);
	return 1;
}
