/*
	Messages for Cops And Robbers script
	Scripted by Sasuke_Uchiha.
	Scripting started on 18th October 2017
	Last Updated: ----
	
	Credits:
		color-hex.com
		
	Functions:
		TimeUpdateMessage(color,text); 
		SendMessageToRetired(color, text);
		SendMessageToModerators(color, text);
		SendMessageToAdmins(color, text);
		SendMessageToHeadAdmins(color, text);
		SendMessageToManagers(color, text);
		SendMessageToOwners(color, text);
		SendAdminAnonmityMessage(adminid,COLOR,text,Irccol);
		SendAdminAnonmityMessageIRC(admin,COLOR,text,Irccol);
		SendConnectMessage(playerid,text,show); 
		IRC_SayColord(botid,channel,message,color);
		IRC_Message(channel,color,message);
		IRC_MessageToAll(color,message);
		IRC_SendAdminMessage(message,color);
		IRC_SendManagementMessage(message,color);
		IRC_SendMessage(message,color);
		SendRadioIRCMessage(message, color);
		SendMessageToDJ(color, text);
		
	Functions Being Called:
		
*/

FUNCTION TimeUpdateMessage(color,text[]) 
{
	for(new playerid=0,j=GetPlayerPoolSize(); playerid <= j; playerid++)
		if(IsPlayerConnected(playerid))
			if(PlayerInfo[playerid][pHourlyTimeUpdate])
				SendClientMessage(playerid, color, text);
}

FUNCTION SendMessageToRetired(color, text[])
{
	for(new i=0,j=GetPlayerPoolSize(); i <= j; i++)
	{
	    if(IsPlayerConnected(i) && (PlayerInfo[i][pRank] == RETIRED_STAFF || PlayerInfo[i][pRank] >= SERVER_MODERATOR)  && PlayerInfo[i][pLoggedIn]) 
		{
			SendClientMessage(i, color, text);
		}	
	}
}

FUNCTION SendMessageToModerators(color, text[])
{
	for(new i=0,j=GetPlayerPoolSize(); i <= j; i++)
	{
	    if(IsPlayerConnected(i) && PlayerInfo[i][pRank] >= SERVER_MODERATOR && PlayerInfo[i][pLoggedIn]) 
		{
			SendClientMessage(i, color, text);
		}	
	}
}
FUNCTION SendMessageToAdmins(color, text[])
{
	for(new i=0,j=GetPlayerPoolSize(); i <= j; i++)
	{
	    if(IsPlayerConnected(i) && PlayerInfo[i][pRank] >= SERVER_ADMIN && PlayerInfo[i][pLoggedIn]) 
		{
			SendClientMessage(i, color, text);
		}	
	}
}
FUNCTION SendMessageToHeadAdmins(color, text[])
{
	for(new i=0,j=GetPlayerPoolSize(); i <= j; i++)
	{
	    if(IsPlayerConnected(i) && PlayerInfo[i][pRank] >= SERVER_HEAD_ADMIN && PlayerInfo[i][pLoggedIn]) 
		{
			SendClientMessage(i, color, text);
		}	
	}
}
FUNCTION SendMessageToManagers(color, text[])
{
	for(new i=0,j=GetPlayerPoolSize(); i <= j; i++)
	{
	    if(IsPlayerConnected(i) && PlayerInfo[i][pRank] >= SERVER_MANAGER && PlayerInfo[i][pLoggedIn]) SendClientMessage(i, color, text);
	}
}
FUNCTION SendMessageToOwners(color, text[])
{
	for(new i=0,j=GetPlayerPoolSize(); i <= j; i++)
	{
	    if(IsPlayerConnected(i) && PlayerInfo[i][pRank] >= SERVER_OWNER && PlayerInfo[i][pLoggedIn]) SendClientMessage(i, color, text);
	}
}

FUNCTION SendMessageToDJ(color, text[])
{
	for(new i=0,j=GetPlayerPoolSize(); i <= j; i++)
	{
	    if(IsPlayerConnected(i) && PlayerInfo[i][pDj] && PlayerInfo[i][pLoggedIn]) SendClientMessage(i, color, text);
	}
}


FUNCTION SendAdminAnonmityMessage(adminid,COLOR,text[],Irccol[])
{
	new string[128];
	for(new i = 0, j = GetPlayerPoolSize(); i <= j; i++)
   	{
   		if(IsPlayerConnected(i))
      	{
      		if(PlayerInfo[i][pRank] >= SERVER_MODERATOR)
        	{
				format(string,sizeof(string),"%s (%d) %s",PlayerInfo[adminid][p_AdminNick],adminid,text);
      			SendClientMessage(i, COLOR, string);
      		}
			else
			{
				format(string,sizeof(string),"%s (-1) %s",SERVER_BOT,text);
      			SendClientMessage(i, COLOR, string);
			}
      	}
   	}
	format(string,sizeof(string),"%s (%d) %s",PlayerInfo[adminid][p_AdminNick],adminid,text);
	IRC_SendManagementMessage(string,Irccol);
	IRC_SendAdminMessage(string,Irccol);
	format(string,sizeof(string),"%s (-1) %s",SERVER_BOT,text);
	IRC_SendMessage(string,Irccol);
}

FUNCTION SendAdminAnonmityMessageIRC(admin[],COLOR,text[],Irccol[])
{
	new string[128];
	for(new i = 0, j = GetPlayerPoolSize(); i <= j; i++)
   	{
   		if(IsPlayerConnected(i))
      	{
      		if(PlayerInfo[i][pRank] >= SERVER_MODERATOR)
        	{
				format(string,sizeof(string),"%s (IRC) %s",admin,text);
      			SendClientMessage(i, COLOR, string);
      		}
			else
			{
				format(string,sizeof(string),"%s (-1) %s",SERVER_BOT,text);
      			SendClientMessage(i, COLOR, string);
			}
      	}
   	}
	format(string,sizeof(string),"%s (IRC) %s",admin,text);
	IRC_SendManagementMessage(string,Irccol);
	IRC_SendAdminMessage(string,Irccol);
	format(string,sizeof(string),"%s (-1) %s",SERVER_BOT,text);
	IRC_SendMessage(string,Irccol);
}

FUNCTION SendConnectMessage(playerid,text[],show) 
{	
	IRC_SendMessage(text,IRC_CONNECT_COLOR);
	new string[150];
	new Cache:result,rows;
	mysql_format(g_SQL, string, sizeof(string), "SELECT aID FROM "TABLE_PLAYERS" WHERE UserName='%e' AND Rank >= %d LIMIT 1",PlayerInfo[playerid][pUserName],SERVER_MODERATOR);
	result = mysql_query(g_SQL,string);
	cache_get_row_count(rows);
	if(rows || !show)
	{
		format(string,sizeof(string),"%s IP: %s / Country: %s.",text,PlayerInfo[playerid][pIP],PlayerInfo[playerid][pCurrent_country]);
		for(new i = 0, j = GetPlayerPoolSize(); i <= j; i++)
		{
			if(IsPlayerConnected(i))
			{
				if(PlayerInfo[i][pShowJoinMessages] && i != playerid && PlayerInfo[i][pRank] >= SERVER_MODERATOR)
				{
					SendClientMessage(i, COLOR_CONNECT, string);
				}
			}
		}
	}
	else
	{
		format(string,sizeof(string),"%s IP: %s / Country: %s.",text,PlayerInfo[playerid][pIP],PlayerInfo[playerid][pCurrent_country]);
		for(new i = 0, j = GetPlayerPoolSize(); i <= j; i++)
		{
			if(IsPlayerConnected(i))
			{
				if(PlayerInfo[i][pShowJoinMessages] && i != playerid)
				{
					if(PlayerInfo[i][pRank] >= SERVER_MODERATOR) SendClientMessage(i, COLOR_CONNECT, string);
					else SendClientMessage(i, COLOR_CONNECT, text);
				}
			}
		}
	}
	IRC_SendAdminMessage(string,IRC_CONNECT_COLOR);
	IRC_SendManagementMessage(string,IRC_CONNECT_COLOR);
	cache_delete(result);
	return 1;
}

FUNCTION IRC_SayColord(botid,channel[],message[],color[])
{
	new string[256];
	format(string,sizeof(string),"%s%s",color,message);
	IRC_Say(botid, channel, string);
	return 1;
}
FUNCTION IRC_Message(channel[],color[],message[])
{
	new string[256];
	format(string,sizeof(string),"%s%s",color,message);
	IRC_GroupSay(1, channel, string);
	return 1;
}

FUNCTION IRC_MessageToAll(color[],message[])
{
	new string[256];
	format(string,sizeof(string),"%s%s",color,message);
	IRC_GroupSay(1, IRC_ECHANNEL, string);
	IRC_GroupSay(1, IRC_ACHANNEL, string);
	IRC_GroupSay(1, IRC_MCHANNEL, string);
	return 1;
}

FUNCTION IRC_SendAdminMessage(message[],color[])
{
	new string[256];
	format(string,sizeof(string),"%s%s",color,message);
	IRC_GroupSay(1, IRC_ACHANNEL, string);
	//IRC_GroupSay(1, ECHO_IRC_CHANNEL, string);
	return 1;
}


FUNCTION IRC_SendManagementMessage(message[],color[])
{
	new string[256];
	format(string,sizeof(string),"%s%s",color,message);
	IRC_GroupSay(1, IRC_MCHANNEL, string);
	//IRC_GroupSay(1, ECHO_MANAGER_ADMIN, string);
	//IRC_GroupSay(1, ECHO_MANAGER_NORMAL, string);
	return 1;
}


FUNCTION IRC_SendMessage(message[],color[])
{
	new string[256];
	format(string,sizeof(string),"%s%s",color,message);
	IRC_GroupSay(1, IRC_ECHANNEL, string);
	return 1;
}

FUNCTION SendRadioIRCMessage(message[], color[])
{
	new string[256];
	format(string,sizeof(string),"%s%s",color,message);
	IRC_GroupSay(1,IRC_RADIO,string);
	IRC_GroupSay(1,IRC_RADIO_DJ,string);
	return 1;
}