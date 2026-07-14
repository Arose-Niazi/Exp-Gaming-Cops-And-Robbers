/*

	DJ radio broadcast (/djradio, /stopdjradio) for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 6 (stage 3) — design §8.1 (DJ /djradio) + jobs §4.3 (radio).

	A server DJ (pDj) broadcasts a live audio stream to EVERYONE: PlayAudioStreamForPlayer
	is called for every connected player with the DJ's stream URL, and the now-playing
	title is announced globally + to the DJ channel (SendMessageToDJ). The broadcast state
	is server-wide (one active DJ stream at a time) so a player who connects mid-broadcast
	is auto-synced to the stream on spawn (DJ_SyncPlayer). /stopdjradio ends it for all.

	This is separate from the personal /radio picker in cmds/player.inc (M4) — that plays a
	preset stream for one player only. Stopping the DJ broadcast does NOT clobber a player's
	own /radio choice beyond the shared StopAudioStreamForPlayer (a client has one stream
	slot; the design accepts that the DJ broadcast owns it while live).

	Public API (called by CnR.pwn):
		DJ_SyncPlayer(playerid)   : if a DJ broadcast is live, start it for this player (on spawn)
		DJ_OnDisconnect(playerid) : if the streaming DJ disconnects, end the broadcast

	Functions Being Called:
		messages: SendMessageToDJ, IRC_SendMessage (retired no-op)
*/

//==============================================================================
//	Server-wide DJ broadcast state (one active stream at a time)
//==============================================================================

new bool:g_DjBroadcasting = false;		// is a DJ stream live right now?
new g_DjBroadcaster = INVALID_PLAYER_ID;	// which player (a pDj) is streaming
new g_DjStreamUrl[192];					// the live stream URL
new g_DjNowPlaying[96];					// the now-playing title text

//==============================================================================
//	/djradio <url> [now-playing text] — start / update the broadcast (pDj)
//==============================================================================

CMD:djradio(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(!PlayerInfo[playerid][pDj] && PlayerInfo[playerid][pRank] < SERVER_ADMIN)
		return SendClientMessage(playerid, COLOR_ERROR, "Only a server "COL_RED"DJ"COL_WHITE" may broadcast.");

	new url[192], nowPlaying[96];
	if(sscanf(params, "s[192]S()[96]", url, nowPlaying))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /djradio <stream url> [now-playing text]   ("COL_WHITE"/stopdjradio"COL_WHITE" to end)");
	// A live stream URL is required; block an obviously-not-a-url token.
	if(strlen(url) < 7 || (strfind(url, "http", true) == -1))
		return SendClientMessage(playerid, COLOR_ERROR, "Provide a valid http(s) stream URL.");

	// Another DJ is already live — only that DJ (or an admin) may take over.
	if(g_DjBroadcasting && g_DjBroadcaster != playerid && g_DjBroadcaster != INVALID_PLAYER_ID && PlayerInfo[playerid][pRank] < SERVER_ADMIN)
		return SendClientMessage(playerid, COLOR_ERROR, "Another DJ is already broadcasting. Ask them to /stopdjradio first.");

	g_DjBroadcasting = true;
	g_DjBroadcaster = playerid;
	format(g_DjStreamUrl, sizeof(g_DjStreamUrl), "%s", url);
	if(isNull(nowPlaying)) format(g_DjNowPlaying, sizeof(g_DjNowPlaying), "Live DJ set");
	else format(g_DjNowPlaying, sizeof(g_DjNowPlaying), "%s", nowPlaying);

	// Push the stream to every connected, logged-in player.
	new count = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || !PlayerInfo[i][pLoggedIn]) continue;
		PlayAudioStreamForPlayer(i, g_DjStreamUrl);
		count++;
	}

	new string[192];
	format(string, sizeof(string), ""COL_PURPLE"[DJ] "COL_WHITE"%s (%d)"COL_PURPLE" is now broadcasting: "COL_WHITE"%s"COL_PURPLE"  ~  type "COL_WHITE"/radio"COL_PURPLE" and pick Stop to mute.", PlayerInfo[playerid][pUserName], playerid, g_DjNowPlaying);
	SendClientMessageToAll(COLOR_DJ, string);

	format(string, sizeof(string), ""COL_PURPLE"[DJ] "COL_WHITE"You are live to %d listener(s): "COL_WHITE"%s", count, g_DjNowPlaying);
	SendClientMessage(playerid, COLOR_DJ, string);

	// Log to the DJ channel + the future Discord bridge (retired IRC no-op today).
	format(string, sizeof(string), "[DJ] %s (%d) broadcasting: %s (%s)", PlayerInfo[playerid][pUserName], playerid, g_DjNowPlaying, g_DjStreamUrl);
	SendMessageToDJ(COLOR_DJ, string);
	IRC_SendMessage(string, IRC_RADIO_COLOR);
	return 1;
}

//==============================================================================
//	/stopdjradio — end the broadcast for everyone (pDj / admin)
//==============================================================================

CMD:stopdjradio(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(!PlayerInfo[playerid][pDj] && PlayerInfo[playerid][pRank] < SERVER_ADMIN)
		return SendClientMessage(playerid, COLOR_ERROR, "Only a server "COL_RED"DJ"COL_WHITE" may control the broadcast.");
	if(!g_DjBroadcasting) return SendClientMessage(playerid, COLOR_ERROR, "There is no DJ broadcast running.");
	if(g_DjBroadcaster != playerid && PlayerInfo[playerid][pRank] < SERVER_ADMIN)
		return SendClientMessage(playerid, COLOR_ERROR, "Only the broadcasting DJ (or an admin) can stop it.");

	DJ_EndBroadcast(playerid);
	return 1;
}

// End the live broadcast: stop the stream for every listener + announce. `byid`
// is who ended it (INVALID_PLAYER_ID if it ended because the DJ disconnected).
DJ_EndBroadcast(byid)
{
	if(!g_DjBroadcasting) return 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		StopAudioStreamForPlayer(i);
	}
	new string[160];
	if(byid != INVALID_PLAYER_ID && IsPlayerConnected(byid))
		format(string, sizeof(string), ""COL_PURPLE"[DJ] "COL_WHITE"%s"COL_PURPLE" ended the broadcast. The radio is off — use "COL_WHITE"/radio"COL_PURPLE" for your own stream.", PlayerInfo[byid][pUserName]);
	else
		format(string, sizeof(string), ""COL_PURPLE"[DJ] "COL_WHITE"The DJ broadcast ended. Use "COL_WHITE"/radio"COL_PURPLE" for your own stream.");
	SendClientMessageToAll(COLOR_DJ, string);

	g_DjBroadcasting = false;
	g_DjBroadcaster = INVALID_PLAYER_ID;
	g_DjStreamUrl[0] = EOS;
	g_DjNowPlaying[0] = EOS;
	return 1;
}

//==============================================================================
//	Hooks — sync a (re)spawning player, end on DJ disconnect
//==============================================================================

// If a DJ broadcast is live, start the stream for a player who just spawned/connected.
FUNCTION DJ_SyncPlayer(playerid)
{
	if(!g_DjBroadcasting || IsPlayerNPC(playerid)) return 0;
	if(!PlayerInfo[playerid][pLoggedIn]) return 0;
	PlayAudioStreamForPlayer(playerid, g_DjStreamUrl);
	new string[176];
	format(string, sizeof(string), ""COL_PURPLE"[DJ] "COL_WHITE"Now tuned in to the live DJ broadcast: "COL_WHITE"%s"COL_PURPLE". Use "COL_WHITE"/radio"COL_PURPLE" (Stop) to mute.", g_DjNowPlaying);
	SendClientMessage(playerid, COLOR_DJ, string);
	return 1;
}

// If the broadcasting DJ disconnects, end the broadcast for everyone.
FUNCTION DJ_OnDisconnect(playerid)
{
	if(g_DjBroadcasting && g_DjBroadcaster == playerid) DJ_EndBroadcast(INVALID_PLAYER_ID);
	return 1;
}
