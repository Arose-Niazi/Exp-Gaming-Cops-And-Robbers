/*
	Messages for Cops And Robbers script
	Scripted by Arose Niazi.
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
	for(new playerid=0; playerid < MAX_PLAYERS; playerid++)
		if(IsPlayerConnected(playerid))
			if(PlayerInfo[playerid][pHourlyTimeUpdate])
				SendClientMessage(playerid, color, text);
}

FUNCTION SendMessageToRetired(color, text[])
{
	for(new i=0; i < MAX_PLAYERS; i++)
	{
	    if(IsPlayerConnected(i) && (PlayerInfo[i][pRank] == RETIRED_STAFF || PlayerInfo[i][pRank] >= SERVER_MODERATOR)  && PlayerInfo[i][pLoggedIn]) 
		{
			SendClientMessage(i, color, text);
		}	
	}
}

FUNCTION SendMessageToModerators(color, text[])
{
	for(new i=0; i < MAX_PLAYERS; i++)
	{
	    if(IsPlayerConnected(i) && PlayerInfo[i][pRank] >= SERVER_MODERATOR && PlayerInfo[i][pLoggedIn]) 
		{
			SendClientMessage(i, color, text);
		}	
	}
}
FUNCTION SendMessageToAdmins(color, text[])
{
	for(new i=0; i < MAX_PLAYERS; i++)
	{
	    if(IsPlayerConnected(i) && PlayerInfo[i][pRank] >= SERVER_ADMIN && PlayerInfo[i][pLoggedIn]) 
		{
			SendClientMessage(i, color, text);
		}	
	}
}
FUNCTION SendMessageToHeadAdmins(color, text[])
{
	for(new i=0; i < MAX_PLAYERS; i++)
	{
	    if(IsPlayerConnected(i) && PlayerInfo[i][pRank] >= SERVER_HEAD_ADMIN && PlayerInfo[i][pLoggedIn]) 
		{
			SendClientMessage(i, color, text);
		}	
	}
}
FUNCTION SendMessageToManagers(color, text[])
{
	for(new i=0; i < MAX_PLAYERS; i++)
	{
	    if(IsPlayerConnected(i) && PlayerInfo[i][pRank] >= SERVER_MANAGER && PlayerInfo[i][pLoggedIn]) SendClientMessage(i, color, text);
	}
}
FUNCTION SendMessageToOwners(color, text[])
{
	for(new i=0; i < MAX_PLAYERS; i++)
	{
	    if(IsPlayerConnected(i) && PlayerInfo[i][pRank] >= SERVER_OWNER && PlayerInfo[i][pLoggedIn]) SendClientMessage(i, color, text);
	}
}

FUNCTION SendMessageToDJ(color, text[])
{
	for(new i=0; i < MAX_PLAYERS; i++)
	{
	    if(IsPlayerConnected(i) && PlayerInfo[i][pDj] && PlayerInfo[i][pLoggedIn]) SendClientMessage(i, color, text);
	}
}


FUNCTION SendAdminAnonmityMessage(adminid,COLOR,const text[],const Irccol[])
{
	new string[128];
	for(new i = 0; i < MAX_PLAYERS; i++)
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

FUNCTION SendAdminAnonmityMessageIRC(const admin[],COLOR,const text[],const Irccol[])
{
	new string[128];
	for(new i = 0; i < MAX_PLAYERS; i++)
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
		for(new i = 0; i < MAX_PLAYERS; i++)
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
		for(new i = 0; i < MAX_PLAYERS; i++)
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

/*
	The legacy IRC bridge was retired in the open.mp port — the private IRC
	network it relied on is gone. These wrappers are kept as no-ops so every
	call site (connect echoes, admin actions, registrations, …) still works;
	wire a Discord (or other) bridge through them later.
*/
FUNCTION IRC_SayColord(botid,const channel[],const message[],const color[])
{
	#pragma unused botid, channel, message, color
	return 1;
}
FUNCTION IRC_Message(const channel[],const color[],const message[])
{
	#pragma unused channel, color, message
	return 1;
}

FUNCTION IRC_MessageToAll(const color[],const message[])
{
	#pragma unused color, message
	return 1;
}

FUNCTION IRC_SendAdminMessage(const message[],const color[])
{
	#pragma unused message, color
	return 1;
}


FUNCTION IRC_SendManagementMessage(const message[],const color[])
{
	#pragma unused message, color
	return 1;
}


FUNCTION IRC_SendMessage(const message[],const color[])
{
	#pragma unused message, color
	return 1;
}

FUNCTION SendRadioIRCMessage(const message[], const color[])
{
	#pragma unused message, color
	return 1;
}