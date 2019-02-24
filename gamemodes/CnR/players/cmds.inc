/*
	Player Commands Script
	Scripted by Sasuke_Uchiha.
	Scripting started on 24th October 2017
	Last Updated: ----
	
	Credits:
		
	Functions:
		!say (text)
		!players
		
	Functions Being Called:
*/
IRCCMD:say(botid, channel[], user[], host[], params[]) return irccmd_s(botid, channel, user, host, params);
IRCCMD:s(botid, channel[], user[], host[], params[])
{
	new string[128];
	if(!strcmp(channel, IRC_ECHANNEL, true))
	{	
		if (!IRC_IsVoice(botid, channel, user)) return IRC_Message(channel,IRC_ERROR_COLOR,ERROR_NOT_ADMIN);
		if(ServerInfo[sChat_Locked] && !IRC_IsHalfop(botid, channel, user)) return IRC_Message(channel,IRC_ERROR_COLOR,"The chat has been locked by a staff member.");
		if(isNull(params)) return IRC_Message(channel,IRC_ERROR_COLOR,"USAGE: !say (Message)");
		format(string,sizeof(string),"%s [IRC]:1 %s",user,params);
		IRC_Message(channel,IRC_NORMAL_COLOR,string);
		format(string,sizeof(string),""COL_BLUE"%s (IRC):"COL_WHITE" %s",user,params);
		SendClientMessageToAll(-1,string);
	}
	else if(!strcmp(channel, IRC_RADIO, true) || !strcmp(channel, IRC_RADIO_DJ, true))
	{
		if(!isNull(params))
		{
			format(string,sizeof(string),"[DC] %s [IRC]: %s",user,params);
			//SendRadioIRCMessage(string,IRC_RADIO_COLOR);
			SendMessageToDJ(COLOR_DJ,string);
		}	
	}
	else IRC_Message(channel,IRC_ERROR_COLOR,ERROR_WRONG_CHANNEL);
	return 1;
}

IRCCMD:players(botid, channel[], user[], host[], params[])
{
	if(!IRC_IsVoice(botid, channel, user)) return IRC_Message(channel,IRC_ERROR_COLOR,ERROR_NOT_ADMIN);
	if(!ServerInfo[sPlayersOnline]) return IRC_Message(channel,IRC_SERVER_COLOR,"No players are online");
	new players, admins, string[500];
	new breaker;
	for(new x = 0, j = GetPlayerPoolSize(); x <= j; x++)
	{
		if(!IsPlayerConnected(x)) continue;
		breaker++;
	    players++;
	    format(string, sizeof(string), "%s%s (%d) | ",string,PlayerInfo[x][pUserName],x);
		if(breaker == 7)
		{	
			if(isNull(string)) continue;
			IRC_Message(channel,IRC_LIST_COLOR,string);
			format(string, sizeof(string), "");
			breaker=0;
		}
	    if(PlayerInfo[x][pRank] < SERVER_MODERATOR) continue;
	    admins++;
	}
	if(breaker != 0) 
	{
		if(!isNull(string))
		{
			IRC_Message(channel,IRC_LIST_COLOR,string);
		}	
	}	
	format(string, sizeof(string), "Amount of players playing: %i | (%i %s + %i %s) ", players, players - admins, (players == 1) ? ("Player") : ("Players"), admins, (admins == 1) ? ("Admin") : ("Admins"));
	IRC_Message(channel,IRC_LIST_COLOR,string);
	return 1;
}