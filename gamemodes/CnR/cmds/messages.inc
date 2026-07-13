/*

	Messaging commands (private messages) for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 2 — design §11.1
	Last Updated: -----

	Credits:

	Functions:
		IsIgnoring(a, b)             : session ignore matrix lookup
		SetIgnore(a, b, bool:on)     : session ignore matrix write

	Commands:
		/pm (/msg /m)  — private message a player
		/reply (/r)    — reply to last PM sender
		/nopm          — toggle receiving PMs
		/ignore (/mute)— toggle muting a player's chat/PM

	Functions Being Called:
		messages: SendMessageToModerators
*/

// -- session ignore matrix (bit-packed [MAX_PLAYERS][MAX_PLAYERS] in pIgnored) --
IsIgnoring(a, b)
{
	if(a < 0 || a >= MAX_PLAYERS || b < 0 || b >= MAX_PLAYERS) return 0;
	return (pIgnored[a][b >> 5] & (1 << (b & 31))) ? 1 : 0;
}

SetIgnore(a, b, bool:on)
{
	if(a < 0 || a >= MAX_PLAYERS || b < 0 || b >= MAX_PLAYERS) return 0;
	if(on) pIgnored[a][b >> 5] |= (1 << (b & 31));
	else pIgnored[a][b >> 5] &= ~(1 << (b & 31));
	return 1;
}

CMD:pm(playerid, params[])
{
	new targetid, text[128], string[160];
	if(sscanf(params, "us[128]", targetid, text))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /pm [playerid] [message]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || !PlayerInfo[targetid][pLoggedIn])
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected.");
	if(targetid == playerid)
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot PM yourself.");
	if(isNull(text))
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot send an empty message.");

	// Flood throttle (staff exempt).
	if(PlayerInfo[playerid][pRank] < SERVER_MODERATOR)
	{
		new tick = GetTickCount();
		if(PlayerInfo[playerid][pPMFloodTime] != 0 && (tick - PlayerInfo[playerid][pPMFloodTime]) < PM_FLOOD_MS)
			return SendClientMessage(playerid, COLOR_ERROR, "You are sending private messages too fast, wait a moment.");
		PlayerInfo[playerid][pPMFloodTime] = tick;
	}

	if(PlayerInfo[targetid][pNoPM] && PlayerInfo[playerid][pRank] < SERVER_MODERATOR)
		return SendClientMessage(playerid, COLOR_ERROR, "That player has private messages disabled (/nopm).");
	if(IsIgnoring(targetid, playerid))
		return SendClientMessage(playerid, COLOR_ERROR, "That player is ignoring you.");

	format(string, sizeof(string), "PM to %s (%d): %s", PlayerInfo[targetid][pUserName], targetid, text);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	format(string, sizeof(string), "PM from %s (%d): %s", PlayerInfo[playerid][pUserName], playerid, text);
	SendClientMessage(targetid, COLOR_SERVER_INFO, string);

	PlayerInfo[targetid][pLastPMFrom] = playerid;

	// Staff spy: mirror PMs to on-duty moderators+ (not sender/receiver).
	format(string, sizeof(string), "[PM SPY] %s (%d) -> %s (%d): %s", PlayerInfo[playerid][pUserName], playerid, PlayerInfo[targetid][pUserName], targetid, text);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || i == playerid || i == targetid) continue;
		if(PlayerInfo[i][pRank] >= SERVER_MODERATOR && PlayerInfo[i][pAdminDuty])
			SendClientMessage(i, COLOR_ADMIN_INFO, string);
	}
	return 1;
}
alias:pm("msg", "m")

CMD:reply(playerid, params[])
{
	new text[128];
	if(sscanf(params, "s[128]", text))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /reply [message]");
	new targetid = PlayerInfo[playerid][pLastPMFrom];
	if(targetid < 0 || targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || !PlayerInfo[targetid][pLoggedIn])
		return SendClientMessage(playerid, COLOR_ERROR, "There is nobody to reply to.");
	// Route through /pm so all the throttle/ignore/nopm rules apply.
	new redir[144];
	format(redir, sizeof(redir), "%d %s", targetid, text);
	return callcmd::pm(playerid, redir);
}
alias:reply("r")

CMD:nopm(playerid)
{
	PlayerInfo[playerid][pNoPM] = !PlayerInfo[playerid][pNoPM];
	if(PlayerInfo[playerid][pNoPM])
		SendClientMessage(playerid, COLOR_SERVER_INFO, "You will no longer receive private messages.");
	else
		SendClientMessage(playerid, COLOR_SERVER_INFO, "You will now receive private messages.");
	return 1;
}

CMD:ignore(playerid, params[])
{
	new targetid;
	if(sscanf(params, "u", targetid))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /ignore [playerid]");
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid))
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected.");
	if(targetid == playerid)
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot ignore yourself.");
	new string[128];
	if(IsIgnoring(playerid, targetid))
	{
		SetIgnore(playerid, targetid, false);
		format(string, sizeof(string), "You are no longer ignoring %s (%d).", PlayerInfo[targetid][pUserName], targetid);
	}
	else
	{
		SetIgnore(playerid, targetid, true);
		format(string, sizeof(string), "You are now ignoring %s (%d). You will not see their chat or PMs.", PlayerInfo[targetid][pUserName], targetid);
	}
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	return 1;
}
alias:ignore("mute")
