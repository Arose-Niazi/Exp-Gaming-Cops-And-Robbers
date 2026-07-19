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
		/w (/whisper)  — local proximity whisper (Stage D §4)
		/say (/s)      — explicit global chat line (Stage D §4)
		/tip (/givetip)— send a cash tip to a nearby player (Stage D §4)

	Functions Being Called:
		messages: SendMessageToModerators
		bank:     Bank_TakeCash (tip payment)
*/

// Stage D (§4) — whisper range (proximity chat, not a PM).
#define WHISPER_RANGE	15.0

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

//==============================================================================
//	Stage D §4 — /w (/whisper) : local proximity whisper (NOT a PM)
//	Sends to all players within WHISPER_RANGE in same interior + VW.
//	Respects the ignore list (sender-muted players do not receive it).
//==============================================================================

CMD:w(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new text[128];
	if(sscanf(params, "s[128]", text))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /w [text]  (whisper to players within 15m)");
	if(isNull(text)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot whisper an empty message.");

	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	new myInt = GetPlayerInterior(playerid), myVW = GetPlayerVirtualWorld(playerid);

	new string[176];
	format(string, sizeof(string), ""COL_WHITE"* %s whispers: %s", PlayerInfo[playerid][pUserName], text);

	// Sender always sees their own whisper.
	SendClientMessage(playerid, 0x999999FF, string);

	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(i == playerid) continue;
		if(!IsPlayerConnected(i) || !PlayerInfo[i][pLoggedIn]) continue;
		if(GetPlayerInterior(i) != myInt || GetPlayerVirtualWorld(i) != myVW) continue;
		if(!IsPlayerInRangeOfPoint(i, WHISPER_RANGE, x, y, z)) continue;
		if(IsIgnoring(i, playerid)) continue;		// ignore list respected
		SendClientMessage(i, 0x999999FF, string);
	}
	return 1;
}
alias:w("whisper")

//==============================================================================
//	Stage D §4 — /say (/s) : explicit global chat line
//	Mirrors default chat; useful when other channels are the norm.
//==============================================================================

CMD:say(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new text[128];
	if(sscanf(params, "s[128]", text))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /say [text]");
	if(isNull(text)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot say an empty message.");

	new string[160];
	format(string, sizeof(string), "%s: %s", PlayerInfo[playerid][pUserName], text);
	SendClientMessageToAll(COLOR_WHITE, string);
	return 1;
}
alias:say("s")

//==============================================================================
//	Stage D §4 — /tip (/givetip) : send a cash tip to a nearby player
//	Thin wrapper over the /givecash money-move logic; reuses same proximity,
//	validation, cooldown and Bank_TakeCash/UpdateMoney path.  No duplicate
//	mint: money moves via the same pMoney delta + UpdateMoney the givecash cmd
//	uses (direct cash-in-hand transfer, not a bank operation).
//==============================================================================

CMD:tip(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new targetid, amount;
	if(sscanf(params, "ud", targetid, amount))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /tip [playerid] [amount]");

	if(amount < GIVECASH_MIN) return SendClientMessage(playerid, COLOR_ERROR, "Enter a positive amount to tip.");
	if(amount > MAX_MONEY) return SendClientMessage(playerid, COLOR_ERROR, "That amount is too large.");

	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || !PlayerInfo[targetid][pLoggedIn])
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected/logged in.");
	if(targetid == playerid) return SendClientMessage(playerid, COLOR_ERROR, "You cannot tip yourself.");
	if(IsPlayerNPC(targetid)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot tip that player.");

	// Cooldown shared with givecash (same anti-abuse window).
	if(gettime() < PlayerInfo[playerid][pGiveCashCooldown])
	{
		new err[96];
		format(err, sizeof(err), "Slow down - you can hand cash again in %d seconds.", PlayerInfo[playerid][pGiveCashCooldown] - gettime());
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}

	if(PlayerInfo[playerid][pMoney] < amount)
		return SendClientMessage(playerid, COLOR_ERROR, "You do not have that much cash on you.");

	// Proximity + same interior/VW (matches givecash range).
	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	if(GetPlayerInterior(playerid) != GetPlayerInterior(targetid) ||
		GetPlayerVirtualWorld(playerid) != GetPlayerVirtualWorld(targetid) ||
		!IsPlayerInRangeOfPoint(targetid, GIVECASH_RANGE, x, y, z))
		return SendClientMessage(playerid, COLOR_ERROR, "You must be right next to that player to tip them.");

	if(PlayerInfo[targetid][pMoney] > MAX_MONEY - amount)
		return SendClientMessage(playerid, COLOR_ERROR, "That player is carrying too much cash to receive a tip.");

	// Move the cash (same path as /givecash — direct pMoney delta, no bank op).
	PlayerInfo[playerid][pMoney] -= amount;
	UpdateMoney(playerid);
	PlayerInfo[targetid][pMoney] += amount;
	UpdateMoney(targetid);
	PlayerInfo[playerid][pGiveCashCooldown] = gettime() + GIVECASH_CD;

	new string[144];
	format(string, sizeof(string), ""COL_GREEN"* You tipped "COL_WHITE"$%d"COL_GREEN" to %s (%d). Thanks!"COL_WHITE, amount, PlayerInfo[targetid][pUserName], targetid);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	format(string, sizeof(string), ""COL_GREEN"* %s (%d) tipped you "COL_WHITE"$%d"COL_GREEN"!"COL_WHITE, PlayerInfo[playerid][pUserName], playerid, amount);
	SendClientMessage(targetid, COLOR_SERVER_INFO, string);

	// Audit log (same trail as givecash).
	format(string, sizeof(string), "[TIP] %s (%d) tipped $%d to %s (%d).", PlayerInfo[playerid][pUserName], playerid, amount, PlayerInfo[targetid][pUserName], targetid);
	SendMessageToModerators(COLOR_ADMIN_INFO, string);
	return 1;
}
alias:tip("givetip")
