/*

	Admin command suite for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 2 - design §11.2
	Last Updated: -----

	Credits:

	Functions:
		ApplyAdminJail(playerid, mins, const reason[], const byname[])
		ReleaseAdminJail(playerid, bool:announce)
		ShowCommandList(playerid)

	Commands (gate in brackets):
		/a <text>                        (MOD)   admin chat
		/showcommands                    (player) toggle staff echo + list dialog
		/ajail <id> [mins] [reason]      (MOD)   admin-jail w/ auto-release timer
		/ajailed                         (MOD)   list admin-jailed players
		/aduty                           (MOD)   toggle on-duty admin
		/aka <id> /ips <id>              (MOD)   also-known-as / ip history (online)
		/offlineaka <name> /offlineips   (MOD)   aka / ips (offline account)
		/setarmour /sethealth            (MOD)
		/setvirtualworld /setinterior    (ADMIN)
		/aflip /move /goto               (MOD)
		/disarm                          (MOD)
		/arm /rangearm /rangedisarm      (ADMIN)
		/count /areacount /aslap         (MOD)
		/osearch                         (MOD)
		/healthhack /aimbotters          (MOD)   anti-cheat flag lists (M2 shell)
		/clearaimbotters                 (ADMIN)

	Functions Being Called:
		messages: SendMessageToModerators, SendMessageToAdmins, IRC_SendMessage
		misc:     GetXYInFrontOfPlayer
*/

// ---- Admin-jail apply / release -------------------------------------------

// Auto-release timer entry - fires when an admin-jail sentence expires.
FUNCTION AdminJailRelease(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	ReleaseAdminJail(playerid, true);
	return 1;
}

// Teleport + freeze a player into the admin jail. mins<=0 means "reload only"
// (used on spawn re-jail); a positive mins (re)arms the release timer.
ApplyAdminJail(playerid, mins, const reason[], const byname[])
{
	if(!IsPlayerConnected(playerid) || !PlayerInfo[playerid][pLoggedIn]) return 0;

	if(mins > 0)
	{
		if(mins > MAX_ADMIN_JAIL_MINS) mins = MAX_ADMIN_JAIL_MINS;
		PlayerInfo[playerid][pAdminJailUntil] = gettime() + (mins * 60);
	}

	PlayerInfo[playerid][pAdminJailed] = true;

	SetPlayerInterior(playerid, ADMIN_JAIL_INT);
	SetPlayerVirtualWorld(playerid, ADMIN_JAIL_VW);
	SetPlayerPos(playerid, ADMIN_JAIL_X, ADMIN_JAIL_Y, ADMIN_JAIL_Z);
	SetPlayerFacingAngle(playerid, ADMIN_JAIL_A);
	SetCameraBehindPlayer(playerid);
	ResetPlayerWeapons(playerid);
	TogglePlayerControllable(playerid, false);

	// M6 - force the player out of any arena so a death-respawn cannot re-place
	// them in the arena VW and skip the admin-jail (jail-bypass fix).
	PlayerInfo[playerid][pArena] = ARENA_NONE;
	PlayerInfo[playerid][pInDMZone] = false;
	PlayerInfo[playerid][pArenaSaved] = false;
	PlayerInfo[playerid][pInDuel] = false;

	// (Re)arm the per-player release timer for the remaining time.
	if(PlayerInfo[playerid][pAdminJailTimer] != -1)
	{
		KillTimer(PlayerInfo[playerid][pAdminJailTimer]);
		PlayerInfo[playerid][pAdminJailTimer] = -1;
	}
	new remain = PlayerInfo[playerid][pAdminJailUntil] - gettime();
	if(remain < 1) remain = 1;
	PlayerInfo[playerid][pAdminJailTimer] = SetTimerEx("AdminJailRelease", remain * 1000, false, "d", playerid);

	new string[144], mleft = (PlayerInfo[playerid][pAdminJailUntil] - gettime() + 59) / 60;
	if(!isNull(reason)) format(string, sizeof(string), "You have been admin-jailed for %d minute(s) by %s. Reason: %s", mleft, byname, reason);
	else format(string, sizeof(string), "You have been admin-jailed for %d minute(s) by %s.", mleft, byname);
	SendClientMessage(playerid, COLOR_ERROR, string);
	return 1;
}

ReleaseAdminJail(playerid, bool:announce)
{
	if(PlayerInfo[playerid][pAdminJailTimer] != -1)
	{
		KillTimer(PlayerInfo[playerid][pAdminJailTimer]);
		PlayerInfo[playerid][pAdminJailTimer] = -1;
	}
	PlayerInfo[playerid][pAdminJailed] = false;
	PlayerInfo[playerid][pAdminJailUntil] = 0;

	if(IsPlayerConnected(playerid))
	{
		TogglePlayerControllable(playerid, true);
		SetPlayerInterior(playerid, 0);
		SetPlayerVirtualWorld(playerid, 0);
		SetPlayerHealth(playerid, 100.0);
		SpawnPlayer(playerid);
		if(announce) SendClientMessage(playerid, COLOR_SERVER_INFO, "You have been released from admin-jail.");
	}

	// Persist the cleared state.
	new string[128];
	mysql_format(g_SQL, string, sizeof(string), "UPDATE "TABLE_PLAYERS_C" SET AdminJailUntil=0 WHERE aID=%d LIMIT 1", PlayerInfo[playerid][pID]);
	mysql_pquery(g_SQL, string);
	return 1;
}

// ---- Admin chat ------------------------------------------------------------

CMD:a(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	if(isNull(params)) return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /a [message]");
	new string[160];
	format(string, sizeof(string), "[ADMIN CHAT] %s (%d): %s", PlayerInfo[playerid][pUserName], playerid, params);
	SendMessageToModerators(COLOR_ADMIN_INFO, string);
	IRC_SendAdminMessage(string, IRC_ADMIN_COLOR_INFO);
	return 1;
}

// ---- /showcommands ---------------------------------------------------------

ShowCommandList(playerid)
{
	// A representative, paginated command reference (dialog-listed).
	new list[1400];
	strcat(list, "{FFFF00}Messaging:{FFFFFF}\n/pm  /reply (/r)  /nopm  /ignore (/mute)\n");
	strcat(list, "{FFFF00}Teleports:{FFFFFF}\n/drylake (/dl)  /sfairport (/sfa)  /bayside (/bs)  /lossantosdm (/lsadm)  /palominocreek (/pc)\n");
	strcat(list, "{FFFF00}GPS/Info:{FFFFFF}\n/gps  /commands  /showcommands\n");
	if(PlayerInfo[playerid][pRank] >= SERVER_MODERATOR)
	{
		strcat(list, "{FFFF00}Moderator:{FFFFFF}\n/a  /goto  /move  /ajail  /ajailed  /aduty  /aka  /ips  /offlineaka  /offlineips\n");
		strcat(list, "/setarmour  /sethealth  /aflip  /disarm  /count  /areacount  /aslap  /osearch  /ban  /offlineban  /kick\n");
		strcat(list, "/healthhack  /aimbotters\n");
	}
	if(PlayerInfo[playerid][pRank] >= SERVER_ADMIN)
	{
		strcat(list, "{FFFF00}Admin:{FFFFFF}\n/setvirtualworld  /setinterior  /arm  /rangearm  /rangedisarm  /clearaimbotters  /unban  /unbannick  /unbanid\n");
		strcat(list, "/saveloc  /teleback  /skin\n");
	}
	if(PlayerInfo[playerid][pRank] >= SERVER_OWNER)
		strcat(list, "{FFFF00}Owner:{FFFFFF}\n/atime  /makeadmin  /makeregular  /makedj  /makedonator  /startmoneyrush\n");
	if(PlayerInfo[playerid][pRank] >= SERVER_SCRIPTER)
		strcat(list, "{FFFF00}Scripter:{FFFFFF}\n/makeowner  /makescripter  /lgoto  /debug\n");
	ShowPlayerDialog(playerid, COMMANDS_DIALOG, DIALOG_STYLE_MSGBOX, "Command List", list, "Okay", "");
	return 1;
}

CMD:showcommands(playerid)
{
	PlayerInfo[playerid][pShowCommands] = !PlayerInfo[playerid][pShowCommands];
	if(PlayerInfo[playerid][pShowCommands])
		SendClientMessage(playerid, COLOR_SERVER_INFO, "Command echo is now "COL_GREEN"ON"COL_WHITE" (you will see commands used by lower/equal ranks).");
	else
		SendClientMessage(playerid, COLOR_SERVER_INFO, "Command echo is now "COL_RED"OFF"COL_WHITE".");
	ShowCommandList(playerid);
	return 1;
}
alias:showcommands("commands")

// ---- /ajail + /ajailed -----------------------------------------------------

CMD:ajail(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new targetid, mins, reason[64];
	if(sscanf(params, "dD(30)z[64]", targetid, mins, reason))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /ajail [playerid] [minutes] [reason]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || !PlayerInfo[targetid][pLoggedIn])
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected/logged in.");
	if(PlayerInfo[targetid][pRank] >= PlayerInfo[playerid][pRank] && targetid != playerid)
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot admin-jail someone at or above your rank.");
	if(mins < 1 || mins > MAX_ADMIN_JAIL_MINS)
	{
		new usage[96];
		format(usage, sizeof(usage), "Minutes must be between 1 and %d.", MAX_ADMIN_JAIL_MINS);
		return SendClientMessage(playerid, COLOR_ERROR, usage);
	}

	ApplyAdminJail(targetid, mins, reason, PlayerInfo[playerid][p_AdminNick]);

	// Persist the release time so it survives a relog.
	new string[160];
	mysql_format(g_SQL, string, sizeof(string), "UPDATE "TABLE_PLAYERS_C" SET AdminJailUntil=%d WHERE aID=%d LIMIT 1", PlayerInfo[targetid][pAdminJailUntil], PlayerInfo[targetid][pID]);
	mysql_pquery(g_SQL, string);

	if(!isNull(reason)) format(string, sizeof(string), "%s (%d) admin-jailed %s (%d) for %d minute(s). Reason: %s", PlayerInfo[playerid][pUserName], playerid, PlayerInfo[targetid][pUserName], targetid, mins, reason);
	else format(string, sizeof(string), "%s (%d) admin-jailed %s (%d) for %d minute(s).", PlayerInfo[playerid][pUserName], playerid, PlayerInfo[targetid][pUserName], targetid, mins);
	SendMessageToModerators(COLOR_ADMIN_INFO, string);
	IRC_SendAdminMessage(string, IRC_ADMIN_COLOR_INFO);
	return 1;
}

CMD:unajail(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new targetid;
	if(sscanf(params, "u", targetid))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /unajail [playerid]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid))
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected.");
	if(!PlayerInfo[targetid][pAdminJailed])
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not admin-jailed.");
	ReleaseAdminJail(targetid, true);
	new string[144];
	format(string, sizeof(string), "%s (%d) released %s (%d) from admin-jail.", PlayerInfo[playerid][pUserName], playerid, PlayerInfo[targetid][pUserName], targetid);
	SendMessageToModerators(COLOR_ADMIN_INFO, string);
	IRC_SendAdminMessage(string, IRC_ADMIN_COLOR_INFO);
	return 1;
}

CMD:ajailed(playerid)
{
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new list[1400], row[96], count = 0;
	strcat(list, "{FFFF00}Player\tRemaining\n");
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || !PlayerInfo[i][pAdminJailed]) continue;
		new secs = PlayerInfo[i][pAdminJailUntil] - gettime();
		if(secs < 0) secs = 0;
		format(row, sizeof(row), "{FFFFFF}%s (%d)\t%d:%02d\n", PlayerInfo[i][pUserName], i, secs / 60, secs % 60);
		strcat(list, row);
		count++;
	}
	if(!count) return SendClientMessage(playerid, COLOR_SERVER_INFO, "No players are currently admin-jailed.");
	ShowPlayerDialog(playerid, ADMIN_INFO_DIALOG, DIALOG_STYLE_TABLIST_HEADERS, "Admin-Jailed Players", list, "Okay", "");
	return 1;
}

// ---- /aduty ----------------------------------------------------------------

CMD:aduty(playerid)
{
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	PlayerInfo[playerid][pAdminDuty] = !PlayerInfo[playerid][pAdminDuty];
	new string[144];
	if(PlayerInfo[playerid][pAdminDuty])
	{
		// On-duty: reveal the real admin name (anonymity off).
		GetPlayerName(playerid, PlayerInfo[playerid][p_AdminNick], MAX_PLAYER_NAME);
		SendClientMessage(playerid, COLOR_SERVER_INFO, "You are now "COL_GREEN"on duty"COL_WHITE" (admin name shown, anonymity off).");
	}
	else
	{
		// Off-duty: mask actions behind the server bot name (anonymity on).
		format(PlayerInfo[playerid][p_AdminNick], MAX_PLAYER_NAME, "%s", SERVER_BOT);
		SendClientMessage(playerid, COLOR_SERVER_INFO, "You are now "COL_RED"off duty"COL_WHITE" (anonymity on).");
	}
	format(string, sizeof(string), "%s (%d) is now %s duty.", PlayerInfo[playerid][pUserName], playerid, PlayerInfo[playerid][pAdminDuty] ? "on" : "off");
	SendMessageToModerators(COLOR_ADMIN_INFO, string);
	return 1;
}

// ---- also-known-as / ip history -------------------------------------------

// Online /aka - matches accounts sharing the target's Latest_IP or Latest_Serial.
CMD:aka(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new targetid;
	if(sscanf(params, "u", targetid))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /aka [playerid]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || !PlayerInfo[targetid][pLoggedIn])
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected/logged in.");
	new string[300], serial[MAX_SERIAL_SIZE];
	gpci(targetid, serial, MAX_SERIAL_SIZE);
	mysql_format(g_SQL, string, sizeof(string), "SELECT UserName,Latest_IP FROM "TABLE_PLAYERS" WHERE Latest_IP='%e' OR Latest_Serial='%e' ORDER BY UserName LIMIT 50", PlayerInfo[targetid][pIP], serial);
	mysql_pquery(g_SQL, string, "OnAdminInfoQuery", "ddsd", playerid, 0, PlayerInfo[targetid][pUserName], g_MysqlRaceCheck[playerid]);
	return 1;
}

CMD:offlineaka(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new name[MAX_PLAYER_NAME];
	if(sscanf(params, "s[24]", name))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /offlineaka [name]");
	new string[220];
	// Two-hop: resolve the name's IP/serial, then list matching accounts.
	mysql_format(g_SQL, string, sizeof(string), "SELECT UserName,Latest_IP FROM "TABLE_PLAYERS" WHERE Latest_IP=(SELECT Latest_IP FROM "TABLE_PLAYERS" WHERE UserName='%e' LIMIT 1) OR Latest_Serial=(SELECT Latest_Serial FROM "TABLE_PLAYERS" WHERE UserName='%e' LIMIT 1) ORDER BY UserName LIMIT 50", name, name);
	mysql_pquery(g_SQL, string, "OnAdminInfoQuery", "ddsd", playerid, 0, name, g_MysqlRaceCheck[playerid]);
	return 1;
}

// Online /ips - IP history for an online account (by aID).
CMD:ips(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new targetid;
	if(sscanf(params, "u", targetid))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /ips [playerid]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || !PlayerInfo[targetid][pLoggedIn])
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected/logged in.");
	new string[220];
	mysql_format(g_SQL, string, sizeof(string), "SELECT UserName,Latest_IP FROM "TABLE_PLAYERS" WHERE aID=%d LIMIT 1", PlayerInfo[targetid][pID]);
	mysql_pquery(g_SQL, string, "OnAdminInfoQuery", "ddsd", playerid, 1, PlayerInfo[targetid][pUserName], g_MysqlRaceCheck[playerid]);
	return 1;
}

CMD:offlineips(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new name[MAX_PLAYER_NAME];
	if(sscanf(params, "s[24]", name))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /offlineips [name]");
	new string[200];
	mysql_format(g_SQL, string, sizeof(string), "SELECT UserName,Latest_IP FROM "TABLE_PLAYERS" WHERE UserName='%e' LIMIT 1", name);
	mysql_pquery(g_SQL, string, "OnAdminInfoQuery", "ddsd", playerid, 1, name, g_MysqlRaceCheck[playerid]);
	return 1;
}

// Shared async result formatter for aka(mode 0)/ips(mode 1) -> info dialog.
FUNCTION OnAdminInfoQuery(playerid, mode, const subject[], race_check)
{
	if(!IsPlayerConnected(playerid)) return 0;
	if(race_check != g_MysqlRaceCheck[playerid]) return 0;
	new rows;
	cache_get_row_count(rows);
	if(!rows) return SendClientMessage(playerid, COLOR_ERROR, "No matching accounts found.");
	new list[1400], row[100], uname[MAX_PLAYER_NAME], ip[MAX_IP_SIZE], title[80];
	strcat(list, "{FFFF00}Account\tLast IP\n");
	for(new i = 0; i < rows && i < 50; i++)
	{
		cache_get_value_name(i, "UserName", uname, MAX_PLAYER_NAME);
		cache_get_value_name(i, "Latest_IP", ip, MAX_IP_SIZE);
		format(row, sizeof(row), "{FFFFFF}%s\t%s\n", uname, ip);
		strcat(list, row);
	}
	if(mode == 0) format(title, sizeof(title), "AKA - %s (%d match(es))", subject, rows);
	else format(title, sizeof(title), "IP History - %s", subject);
	ShowPlayerDialog(playerid, ADMIN_INFO_DIALOG, DIALOG_STYLE_TABLIST_HEADERS, title, list, "Okay", "");
	return 1;
}

// ---- health / armour / world / interior -----------------------------------

CMD:sethealth(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new targetid, Float:amount;
	if(sscanf(params, "uf", targetid, amount))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /sethealth [playerid] [0-100]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid))
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected.");
	if(amount < 0.0 || amount > 100.0) return SendClientMessage(playerid, COLOR_ERROR, "Health must be 0-100.");
	if(PlayerInfo[targetid][pRank] >= PlayerInfo[playerid][pRank] && targetid != playerid)
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot use this on someone at or above your rank.");
	SetPlayerHealth(targetid, amount);
	new string[128];
	format(string, sizeof(string), "%s (%d) set %s (%d)'s health to %.0f.", PlayerInfo[playerid][pUserName], playerid, PlayerInfo[targetid][pUserName], targetid, amount);
	SendMessageToModerators(COLOR_ADMIN_INFO, string);
	return 1;
}

CMD:setarmour(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new targetid, Float:amount;
	if(sscanf(params, "uf", targetid, amount))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /setarmour [playerid] [0-100]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid))
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected.");
	if(amount < 0.0 || amount > 100.0) return SendClientMessage(playerid, COLOR_ERROR, "Armour must be 0-100.");
	if(PlayerInfo[targetid][pRank] >= PlayerInfo[playerid][pRank] && targetid != playerid)
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot use this on someone at or above your rank.");
	SetPlayerArmour(targetid, amount);
	new string[128];
	format(string, sizeof(string), "%s (%d) set %s (%d)'s armour to %.0f.", PlayerInfo[playerid][pUserName], playerid, PlayerInfo[targetid][pUserName], targetid, amount);
	SendMessageToModerators(COLOR_ADMIN_INFO, string);
	return 1;
}

CMD:setvirtualworld(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_ADMIN) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new targetid, vw;
	if(sscanf(params, "ud", targetid, vw))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /setvirtualworld [playerid] [world]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid))
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected.");
	if(PlayerInfo[targetid][pRank] >= PlayerInfo[playerid][pRank] && targetid != playerid)
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot use this on someone at or above your rank.");
	SetPlayerVirtualWorld(targetid, vw);
	new string[128];
	format(string, sizeof(string), "%s (%d) set %s (%d)'s virtual world to %d.", PlayerInfo[playerid][pUserName], playerid, PlayerInfo[targetid][pUserName], targetid, vw);
	SendMessageToModerators(COLOR_ADMIN_INFO, string);
	return 1;
}

CMD:setinterior(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_ADMIN) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new targetid, interior;
	if(sscanf(params, "ud", targetid, interior))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /setinterior [playerid] [interior]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid))
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected.");
	if(PlayerInfo[targetid][pRank] >= PlayerInfo[playerid][pRank] && targetid != playerid)
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot use this on someone at or above your rank.");
	SetPlayerInterior(targetid, interior);
	new string[128];
	format(string, sizeof(string), "%s (%d) set %s (%d)'s interior to %d.", PlayerInfo[playerid][pUserName], playerid, PlayerInfo[targetid][pUserName], targetid, interior);
	SendMessageToModerators(COLOR_ADMIN_INFO, string);
	return 1;
}

// ---- movement / vehicle helpers -------------------------------------------

CMD:aflip(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new targetid;
	if(sscanf(params, "u", targetid))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /aflip [playerid]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid))
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected.");
	new vehid = GetPlayerVehicleID(targetid);
	if(!vehid) return SendClientMessage(playerid, COLOR_ERROR, "That player is not in a vehicle.");
	new Float:zangle;
	GetVehicleZAngle(vehid, zangle);
	SetVehicleZAngle(vehid, zangle);
	RepairVehicle(vehid);
	new string[128];
	format(string, sizeof(string), "%s (%d) flipped %s (%d)'s vehicle upright.", PlayerInfo[playerid][pUserName], playerid, PlayerInfo[targetid][pUserName], targetid);
	SendMessageToModerators(COLOR_ADMIN_INFO, string);
	return 1;
}

// /move - bring the target to the admin.
CMD:move(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new targetid;
	if(sscanf(params, "u", targetid))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /move [playerid]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid))
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected.");
	if(targetid == playerid) return SendClientMessage(playerid, COLOR_ERROR, "You cannot move yourself.");
	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	SetPlayerPos(targetid, x + 1.5, y + 1.5, z);
	SetPlayerInterior(targetid, GetPlayerInterior(playerid));
	SetPlayerVirtualWorld(targetid, GetPlayerVirtualWorld(playerid));
	new string[128];
	format(string, sizeof(string), "%s (%d) teleported you to them.", PlayerInfo[playerid][pUserName], playerid);
	SendClientMessage(targetid, COLOR_SERVER_INFO, string);
	format(string, sizeof(string), "You brought %s (%d) to your location.", PlayerInfo[targetid][pUserName], targetid);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	return 1;
}

// /goto - staff-only (design §5) teleport to a player.
CMD:goto(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new targetid;
	if(sscanf(params, "u", targetid))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /goto [playerid]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid))
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected.");
	if(targetid == playerid) return SendClientMessage(playerid, COLOR_ERROR, "You cannot go to yourself.");
	new Float:x, Float:y, Float:z;
	GetPlayerPos(targetid, x, y, z);
	SetPlayerInterior(playerid, GetPlayerInterior(targetid));
	SetPlayerVirtualWorld(playerid, GetPlayerVirtualWorld(targetid));
	SetPlayerPos(playerid, x + 1.5, y + 1.5, z);
	new string[128];
	format(string, sizeof(string), "You teleported to %s (%d).", PlayerInfo[targetid][pUserName], targetid);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	return 1;
}

// ---- arm / disarm ----------------------------------------------------------

CMD:disarm(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new targetid;
	if(sscanf(params, "u", targetid))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /disarm [playerid]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid))
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected.");
	if(PlayerInfo[targetid][pRank] >= PlayerInfo[playerid][pRank] && targetid != playerid)
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot use this on someone at or above your rank.");
	ResetPlayerWeapons(targetid);
	new string[128];
	format(string, sizeof(string), "%s (%d) disarmed %s (%d).", PlayerInfo[playerid][pUserName], playerid, PlayerInfo[targetid][pUserName], targetid);
	SendMessageToModerators(COLOR_ADMIN_INFO, string);
	SendClientMessage(targetid, COLOR_SERVER_INFO, "You have been disarmed by a staff member.");
	return 1;
}

CMD:arm(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_ADMIN) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new targetid, weap, ammo;
	if(sscanf(params, "udD(500)", targetid, weap, ammo))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /arm [playerid] [weaponid] [ammo]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid))
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected.");
	if(weap < 0 || weap > 46) return SendClientMessage(playerid, COLOR_ERROR, "Weapon id must be 0-46.");
	GivePlayerWeapon(targetid, weap, ammo);
	new string[128];
	format(string, sizeof(string), "%s (%d) armed %s (%d) with weapon %d (%d ammo).", PlayerInfo[playerid][pUserName], playerid, PlayerInfo[targetid][pUserName], targetid, weap, ammo);
	SendMessageToAdmins(COLOR_ADMIN_INFO, string);
	return 1;
}

CMD:rangearm(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_ADMIN) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new weap, ammo; new Float:range;
	if(sscanf(params, "dD(500)F(30.0)", weap, ammo, range))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /rangearm [weaponid] [ammo] [range]");
	if(weap < 0 || weap > 46) return SendClientMessage(playerid, COLOR_ERROR, "Weapon id must be 0-46.");
	new Float:x, Float:y, Float:z, count = 0;
	GetPlayerPos(playerid, x, y, z);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(!IsPlayerInRangeOfPoint(i, range, x, y, z)) continue;
		GivePlayerWeapon(i, weap, ammo);
		count++;
	}
	new string[128];
	format(string, sizeof(string), "%s (%d) armed %d player(s) in %.1f range with weapon %d.", PlayerInfo[playerid][pUserName], playerid, count, range, weap);
	SendMessageToAdmins(COLOR_ADMIN_INFO, string);
	return 1;
}

CMD:rangedisarm(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_ADMIN) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new Float:range;
	if(sscanf(params, "F(30.0)", range))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /rangedisarm [range]");
	new Float:x, Float:y, Float:z, count = 0;
	GetPlayerPos(playerid, x, y, z);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(!IsPlayerInRangeOfPoint(i, range, x, y, z)) continue;
		ResetPlayerWeapons(i);
		count++;
	}
	new string[128];
	format(string, sizeof(string), "%s (%d) disarmed %d player(s) in %.1f range.", PlayerInfo[playerid][pUserName], playerid, count, range);
	SendMessageToAdmins(COLOR_ADMIN_INFO, string);
	return 1;
}

// ---- count / areacount / aslap --------------------------------------------

CMD:count(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new secs;
	if(sscanf(params, "d", secs))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /count [seconds 1-10]");
	if(secs < 1 || secs > 10) return SendClientMessage(playerid, COLOR_ERROR, "Countdown must be 1-10 seconds.");
	new string[96];
	format(string, sizeof(string), ""COL_RED"Countdown started by %s: %d...", PlayerInfo[playerid][pUserName], secs);
	SendClientMessageToAll(COLOR_SERVER_INFO, string);
	for(new i = 1; i <= secs; i++)
		SetTimerEx("AdminCountTick", (secs - i + 1) * 1000, false, "d", i);
	return 1;
}

FUNCTION AdminCountTick(value)
{
	new string[32], GameTextStr[16];
	if(value > 1) { format(GameTextStr, sizeof(GameTextStr), "~r~%d", value - 1); }
	else { format(GameTextStr, sizeof(GameTextStr), "~g~GO!"); }
	format(string, sizeof(string), "%s", GameTextStr);
	for(new i = 0; i < MAX_PLAYERS; i++)
		if(IsPlayerConnected(i)) GameTextForPlayer(i, string, 1000, 3);
	return 1;
}

CMD:areacount(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new Float:range;
	if(sscanf(params, "F(30.0)", range)) range = 30.0;
	new Float:x, Float:y, Float:z, count = 0;
	GetPlayerPos(playerid, x, y, z);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(IsPlayerInRangeOfPoint(i, range, x, y, z)) count++;
	}
	new string[96];
	format(string, sizeof(string), "There are %d player(s) within %.1f units of you.", count, range);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	return 1;
}

CMD:aslap(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new targetid;
	if(sscanf(params, "u", targetid))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /aslap [playerid]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid))
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected.");
	if(PlayerInfo[targetid][pRank] >= PlayerInfo[playerid][pRank] && targetid != playerid)
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot use this on someone at or above your rank.");
	new Float:x, Float:y, Float:z, Float:hp;
	GetPlayerPos(targetid, x, y, z);
	SetPlayerPos(targetid, x, y, z + 5.0);
	GetPlayerHealth(targetid, hp);
	SetPlayerHealth(targetid, (hp - 5.0 < 0.0) ? 0.0 : (hp - 5.0));
	new string[128];
	format(string, sizeof(string), "%s (%d) slapped %s (%d).", PlayerInfo[playerid][pUserName], playerid, PlayerInfo[targetid][pUserName], targetid);
	SendMessageToModerators(COLOR_ADMIN_INFO, string);
	return 1;
}

// ---- /osearch --------------------------------------------------------------

CMD:osearch(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new targetid;
	if(sscanf(params, "u", targetid))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /osearch [playerid]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || !PlayerInfo[targetid][pLoggedIn])
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected/logged in.");
	new list[1400], row[128];
	format(row, sizeof(row), "{FFFF00}Search - %s (%d)\n{FFFFFF}Cash: $%d\n", PlayerInfo[targetid][pUserName], targetid, PlayerInfo[targetid][pMoney]);
	strcat(list, row);
	new Float:hp, Float:ar;
	GetPlayerHealth(targetid, hp);
	GetPlayerArmour(targetid, ar);
	format(row, sizeof(row), "Health: %.0f  Armour: %.0f\n", hp, ar);
	strcat(list, row);
	strcat(list, "{FFFF00}Weapons:\n{FFFFFF}");
	new weap, ammo, any = 0;
	for(new slot = 0; slot < 13; slot++)
	{
		GetPlayerWeaponData(targetid, slot, weap, ammo);
		if(weap == 0 || ammo == 0) continue;
		format(row, sizeof(row), "Weapon %d (%d ammo)\n", weap, ammo);
		strcat(list, row);
		any = 1;
	}
	if(!any) strcat(list, "(none)\n");
	ShowPlayerDialog(playerid, ADMIN_INFO_DIALOG2, DIALOG_STYLE_MSGBOX, "Offline Search", list, "Okay", "");
	return 1;
}

// ---- anti-cheat flag lists (M2 shells; detection lands in M3) --------------

CMD:healthhack(playerid, params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	#pragma unused params
	new list[1400], row[100], count = 0;
	strcat(list, "{FFFF00}HP-hack suspicion list:\n{FFFFFF}");
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || !PlayerInfo[i][pHealthHackFlags]) continue;
		format(row, sizeof(row), "%s (%d)\n", PlayerInfo[i][pUserName], i);
		strcat(list, row);
		count++;
	}
	if(!count) strcat(list, "(no players flagged - detection heuristic lands in M3)\n");
	ShowPlayerDialog(playerid, ADMIN_INFO_DIALOG, DIALOG_STYLE_MSGBOX, "Health-Hack Flags", list, "Okay", "");
	return 1;
}

CMD:aimbotters(playerid)
{
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new list[1400], row[100], count = 0;
	strcat(list, "{FFFF00}Aimbot suspicion list:\n{FFFFFF}");
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || !PlayerInfo[i][pAimbotFlags]) continue;
		format(row, sizeof(row), "%s (%d)\n", PlayerInfo[i][pUserName], i);
		strcat(list, row);
		count++;
	}
	if(!count) strcat(list, "(no players flagged - detection heuristic lands in M3)\n");
	ShowPlayerDialog(playerid, ADMIN_INFO_DIALOG, DIALOG_STYLE_MSGBOX, "Aimbot Flags", list, "Okay", "");
	return 1;
}

CMD:clearaimbotters(playerid)
{
	if(PlayerInfo[playerid][pRank] < SERVER_ADMIN) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	for(new i = 0; i < MAX_PLAYERS; i++)
		if(IsPlayerConnected(i)) PlayerInfo[i][pAimbotFlags] = false;
	new string[128];
	format(string, sizeof(string), "%s (%d) cleared the aimbot suspicion list.", PlayerInfo[playerid][pUserName], playerid);
	SendMessageToAdmins(COLOR_ADMIN_INFO, string);
	return 1;
}
