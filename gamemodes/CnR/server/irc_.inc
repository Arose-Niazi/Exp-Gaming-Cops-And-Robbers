/*

	IRC Script
	Scripted by Sasuke_Uchiha.
	Scripting started on 24th October 2017
	Last Updated: ----
	
	Credits:
		DowDow for IRC bot's and channels
	Functions:
		InitilizeIRC();
		UnloadIRC();
		
	Functions Being Called:
*/


#define MAX_BOTS 			5

#define BOT_NICKNAME 		"CnR"
#define BOT_REALNAME 		"Cops And Robbers"
#define BOT_USERNAME 		"Exp Gaming"
#define BOT_PASSWORD 		"bot123@CNRecho_"


new 
	gBotID[MAX_BOTS], 
	gGroupID;

new bool:radio_requests;

InitilizeIRC()
{
	CallBotsToConnect();
	gGroupID = IRC_CreateGroup();
	radio_requests = false;
	return 1;
}

UnloadIRC()
{
	for(new i = 0; i < MAX_BOTS; i++) 
		IRC_Quit(gBotID[i], "7» ERMAHGERD, I'VE BEEN UNLOADED!@#!");

	IRC_DestroyGroup(gGroupID);
	return 1;
}

public IRC_OnConnect(botid, ip[], port)
{
	IRC_AddToGroup(gGroupID, botid);
	//IRC_SendRaw(botid, "nickserv register "BOT_PASSWORD" noreply@exp-gaming.net");
	IRC_SendRaw(botid, "nickserv identify "BOT_PASSWORD);
	CallRemoteFunction("BotAdded","d",botid);
	IRC_JoinChannel(botid, IRC_ECHANNEL);
	IRC_JoinChannel(botid, IRC_ACHANNEL);
	IRC_JoinChannel(botid, IRC_MCHANNEL);
	IRC_JoinChannel(botid, IRC_RADIO);
	IRC_JoinChannel(botid, IRC_RADIO_DJ);
	IRC_JoinChannel(botid, IRC_BAN_CHANNEL);

	//printf("IRC bot %i connected to %s:%i", botid, ip, port);
	return 1;
}

public IRC_OnDisconnect(botid, ip[], port, reason[])
{
	IRC_RemoveFromGroup(gGroupID, botid);
	return 1;
}

public IRC_OnConnectAttempt(botid, ip[], port)
{
	new string[124];
	//printf("[IRC] BOT ID %d attempting to connect to %s:%d...", botid, ip, port);
	format(string,sizeof(string),"[IRC] BOT ID %d attempting to connect to %s:%d...",botid,ip,port);
	CallRemoteFunction("SendMessageToOwners","ds",COLOR_ADMIN_INFO,string);
	return 1;
}
public IRC_OnConnectAttemptFail(botid, ip[], port, reason[])
{
	new string[124];
	//printf("[IRC] BOT ID %d failed to connect to %s:%d (%s)", botid, ip, port, reason);
	format(string,sizeof(string),"[IRC] BOT ID %d failed to connect to %s:%d (%s)", botid, ip, port, reason);
	CallRemoteFunction("SendMessageToOwners","ds",COLOR_ADMIN_INFO,string);
	return 1;
}
public IRC_OnJoinChannel(botid, channel[])
{
	new string[70];
	//printf("[IRC] BOT ID %d joined channel %s", botid, channel);
	format(string,sizeof(string),"[IRC] BOT ID %d joined channel %s", botid, channel);
	CallRemoteFunction("SendMessageToOwners","ds",COLOR_ADMIN_INFO,string);
	return 1;
}
public IRC_OnLeaveChannel(botid, channel[], message[])
{
	new string[124];
	//printf("[IRC] BOT ID %d left channel %s (%s)", botid, channel, message);
	format(string,sizeof(string),"[IRC] BOT ID %d left channel %s (%s)", botid, channel, message);
	CallRemoteFunction("SendMessageToOwners","ds",COLOR_ADMIN_INFO,string);
	return 1;
}
public IRC_OnInvitedToChannel(botid, channel[], invitinguser[], invitinghost[])
{
	new string[124];
	//printf("[IRC] BOT ID %d invited to channel %s by %s (%s)", botid, channel, invitinguser, invitinghost);
	format(string,sizeof(string),"[IRC] BOT ID %d invited to channel %s by %s (%s)", botid, channel, invitinguser, invitinghost);
	CallRemoteFunction("SendMessageToOwners","ds",COLOR_ADMIN_INFO,string);
	return 1;
}
public IRC_OnKickedFromChannel(botid, channel[], oppeduser[], oppedhost[], message[])
{
	new string[124];
	//printf("[IRC] BOT ID %d kicked by %s (%s) from channel %s (%s)", botid, oppeduser, oppedhost, channel, message);
	format(string,sizeof(string),"[IRC] BOT ID %d kicked by %s (%s) from channel %s (%s)", botid, oppeduser, oppedhost, channel, message);
	CallRemoteFunction("SendMessageToOwners","ds",COLOR_ADMIN_INFO,string);
	IRC_JoinChannel(botid, channel);
	return 1;
}

public IRC_OnUserJoinChannel(botid, channel[], user[], host[])
{
	if(botid != 1 ) return 1;
	new string[124];
	if(!strcmp(channel, IRC_ECHANNEL, true))
	{
		format(string,sizeof(string),"[IRC] %s Has Joined The Channel.", user);
		SendClientMessageToAll(COLOR_CONNECT,string);
	}
	if(!strcmp(channel, IRC_ACHANNEL, true))
	{
		format(string,sizeof(string),"[IRC] %s Has Joined The Admin Channel.", user);
		SendMessageToAdmins(COLOR_CONNECT,string);
	}
	if(!strcmp(channel, IRC_MCHANNEL, true))
	{
		format(string,sizeof(string),"[IRC] %s Has Joined The Management Channel.", user);
		SendMessageToManagers(COLOR_CONNECT,string);
	}
	if(!strcmp(channel, IRC_RADIO, true))
	{
		format(string,sizeof(string),"[IRC] %s Has Joined The Radio Channel.", user);
		SendMessageToDJ(COLOR_CONNECT,string);
	}
	if(!strcmp(channel, IRC_RADIO_DJ, true))
	{
		format(string,sizeof(string),"[IRC] %s Has Joined The Radio DJ Channel.", user);
		SendMessageToDJ(COLOR_CONNECT,string);
	}
	return 1;
}

public IRC_OnUserLeaveChannel(botid, channel[], user[], host[], message[])
{
	if(botid != 1) return 1;
	new string[256];
	if(!strcmp(channel, IRC_ECHANNEL, true))
	{
		format(string,sizeof(string),"[IRC] %s Has Left The Channel (Message: %s).", user,message);
		SendClientMessageToAll(COLOR_CONNECT,string);
	}
	if(!strcmp(channel, IRC_ACHANNEL, true))
	{
		format(string,sizeof(string),"[IRC] %s Has Left The Admin Channel (Message: %s).", user,message);
		SendMessageToAdmins(COLOR_CONNECT,string);
	}
	if(!strcmp(channel, IRC_MCHANNEL, true))
	{
		format(string,sizeof(string),"[IRC] %s Has Left The Management Channel (Message: %s).", user,message);
		SendMessageToManagers(COLOR_CONNECT,string);
	}
	if(!strcmp(channel, IRC_RADIO, true))
	{
		format(string,sizeof(string),"[IRC] %s Has Left The Radio Channel (Message: %s).", user,message);
		SendMessageToDJ(COLOR_CONNECT,string);
	}
	if(!strcmp(channel, IRC_RADIO_DJ, true))
	{
		format(string,sizeof(string),"[IRC] %s Has Left The Radio DJ Channel (Message: %s).", user,message);
		SendMessageToDJ(COLOR_CONNECT,string);
	}
	return 1;
}

public IRC_OnUserKickedFromChannel(botid, channel[], kickeduser[], oppeduser[], oppedhost[], message[])
{
	if(botid != 1) return 1;
	new string[256];
	if(!strcmp(channel, IRC_ECHANNEL, true))
	{
		format(string,sizeof(string),"[IRC] %s Has been kicked from The Channel (Message: %s).", kickeduser,message);
		SendClientMessageToAll(COLOR_CONNECT,string);
	}
	if(!strcmp(channel, IRC_ACHANNEL, true))
	{
		format(string,sizeof(string),"[IRC] %s Has been kicked from The Admin Channel (Message: %s).", kickeduser,message);
		SendMessageToAdmins(COLOR_CONNECT,string);
	}
	if(!strcmp(channel, IRC_MCHANNEL, true))
	{
		format(string,sizeof(string),"[IRC] %s Has been kicked from The Management Channel (Message: %s).", kickeduser,message);
		SendMessageToManagers(COLOR_CONNECT,string);
	}
	if(!strcmp(channel, IRC_RADIO, true))
	{
		format(string,sizeof(string),"[IRC] %s Has been kicked from The Radio Channel (Message: %s).", kickeduser,message);
		SendMessageToDJ(COLOR_CONNECT,string);
	}
	if(!strcmp(channel, IRC_RADIO_DJ, true))
	{
		format(string,sizeof(string),"[IRC] %s Has been kicked from The Radio DJ Channel (Message: %s).", kickeduser,message);
		SendMessageToDJ(COLOR_CONNECT,string);
	}
	return 1;
}

public IRC_OnUserNickChange(botid, oldnick[], newnick[], host[])
{
	new string[124];
	if(botid != 1) return 1;
	format(string,sizeof(string),"[IRC] %s is now know as %s.", oldnick,newnick);
	SendClientMessageToAll(COLOR_CONNECT,string);
	return 1;
}

public IRC_OnUserSetChannelTopic(botid, channel[], user[], host[], topic[])
{
	if(botid != 1) return 1;
	new string[256];
	if(!strcmp(channel, IRC_ECHANNEL, true))
	{
		format(string,sizeof(string),"[IRC] %s changed topic to %s.", user,topic);
		SendClientMessageToAll(COLOR_CONNECT,string);
	}
	if(!strcmp(channel, IRC_ACHANNEL, true))
	{
		format(string,sizeof(string),"[IRC] %s changed topic %s for Admin Channel.", user,topic);
		SendMessageToAdmins(COLOR_CONNECT,string);
	}
	if(!strcmp(channel, IRC_MCHANNEL, true))
	{
		format(string,sizeof(string),"[IRC] %s changed topic %s for Management Channel.", user,topic);
		SendMessageToManagers(COLOR_CONNECT,string);
	}
	if(!strcmp(channel, IRC_RADIO, true))
	{
		format(string,sizeof(string),"[IRC] %s changed topic %s for Radio Channel.", user,topic);
		SendMessageToDJ(COLOR_CONNECT,string);
	}
	if(!strcmp(channel, IRC_RADIO_DJ, true))
	{
		format(string,sizeof(string),"[IRC] %s changed topic %s for Radio DJ Channel.", user,topic);
		SendMessageToDJ(COLOR_CONNECT,string);
	}
	return 1;
}

/*public IRC_OnUserSay(botid, recipient[], user[], host[], message[])
{
	new string[124];
	if(!strcmp(message,"!",true,1)) return 1;
	if(botid == 1) 
	{
		if(!strcmp(user,ADMIN_BOT_1_MAIN_NICKNAME)) return 1;
		format(string,sizeof(string),"(ADMIN CHAT) %s (IRC): %s",user,message);
		CallRemoteFunction("SendMessageToModerators","ds",COLOR_ADMIN_INFO,string);
	}
	else
	{
		if(!strcmp(user,SERVER_BOT_1_MAIN_NICKNAME) || !strcmp(user, SERVER_BOT_2_MAIN_NICKNAME) || !strcmp(user, SERVER_BOT_3_MAIN_NICKNAME)) return 1;
		format(string,sizeof(string),"%s (IRC): %s",user,message);
		SendClientMessageToAll(-1,string);
		CallRemoteFunction("PlayerLog","ss",message,user);
	}
	return 1;
}*/

public IRC_OnReceiveNumeric(botid, numeric, message[])
{
	new string[124];
	// Check if the numeric is an error defined by RFC 1459/2812
	if (numeric >= 400 && numeric <= 599)
	{
		const ERR_NICKNAMEINUSE = 433;
		if (numeric == ERR_NICKNAMEINUSE)
		{
			format(string, sizeof(string),"[BOT]%s%i",BOT_NICKNAME,botid);
			gBotID[botid-1] =IRC_ChangeNick(botid, string);
		}
		format(string,sizeof(string),"[IRC] BOT ID %d, ERROR %d : Message %s",botid,numeric,message);
		SendMessageToOwners(COLOR_ADMIN_INFO,string);
		printf(string);
	}
	return 1;
}
public IRC_OnReceiveRaw(botid, message[])
{
	new File:file;
	if (!fexist("irc_log.txt")) 
		file = fopen("irc_log.txt", io_write);
	else
		file = fopen("irc_log.txt", io_append);
	if (file)
	{
		fwrite(file, message);
		fwrite(file, "\r\n");
		fclose(file);
	}
	return 1;
}

//==============================================================================
//	callbacks
//==============================================================================
FUNCTION getRequestState()
{
	return radio_requests;
}

FUNCTION ChangeRadioRequest()
{
	radio_requests = !radio_requests;
	return 1;
}

FUNCTION GetIRCBotsGroup() return gGroupID;

FUNCTION CallBotsToConnect()
{
	new string[125];
	for(new i = 0; i < MAX_BOTS; i++)
	{
		format(string, sizeof(string),"%s%i",BOT_NICKNAME,i+1);
		gBotID[i] = IRC_Connect(IRC_SERVER, IRC_PORT, string, BOT_REALNAME, BOT_USERNAME);
		IRC_SetIntData(gBotID[i], E_IRC_CONNECT_DELAY, (i+1)*5);
	}
	return 1;
}