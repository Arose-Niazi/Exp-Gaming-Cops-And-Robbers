/*

	Textdraw menu Script
	Scripted by Arose Niazi.
	Scripting started on 20th October 2017
	Last Updated: ----
	
	Credits:
		Saadat
		
	Functions:
		CreateMenuBox(playerid);
		DestroyMenuBox(playerid);
		ShowTextDrawMenu(playerid, menuid, title, lines, options);
		ShowTextDrawMenuItems(playerid, row, stats[], column1[], column2[],item_price,item = 0);
		OnPlayerEnterTextDrawMenuOption(playerid,text[]);
		HideTextDrawMenu(playerid,show_msg = 0);
		
	Functions Being Called:
*/

#define MAX_TEXTDRAW_ROWS 25
new PlayerText:Menu_Title;
new PlayerText:Menu_Stats;
new PlayerText:Menu_StatsCont;
new PlayerText:Menu_End;
new PlayerText:Menu_Column1[MAX_TEXTDRAW_ROWS];
new PlayerText:Menu_Column2[MAX_TEXTDRAW_ROWS];
new Menu_Options[MAX_PLAYERS];
new Menu_CurrentOptionSlot[MAX_PLAYERS];
new Menu_Item_Price[MAX_PLAYERS][MAX_TEXTDRAW_ROWS];
new Menu_Item[MAX_PLAYERS][MAX_TEXTDRAW_ROWS];



CreateMenuBox(playerid)
{
	Menu_Title = CreatePlayerTextDraw(playerid, 116.000000, 120.000000, "_");
	PlayerTextDrawAlignment(playerid, Menu_Title, 2);
	PlayerTextDrawBackgroundColor(playerid, Menu_Title, 0x00000077);
	PlayerTextDrawFont(playerid, Menu_Title, 1);
	PlayerTextDrawLetterSize(playerid, Menu_Title, 0.509999, 1.400000);
	PlayerTextDrawColor(playerid, Menu_Title, -1);
	PlayerTextDrawSetOutline(playerid, Menu_Title, 1);
	PlayerTextDrawSetProportional(playerid, Menu_Title, 1);
	PlayerTextDrawSetShadow(playerid, Menu_Title, 1);
	PlayerTextDrawUseBox(playerid, Menu_Title, 1);
	PlayerTextDrawBoxColor(playerid, Menu_Title, 110);
	PlayerTextDrawTextSize(playerid, Menu_Title, 23.000000, 206.000000);

	Menu_Stats = CreatePlayerTextDraw(playerid, 115.000000, 162.000000, "_");
	PlayerTextDrawAlignment(playerid, Menu_Stats, 2);
	PlayerTextDrawBackgroundColor(playerid, Menu_Stats, 0x00000077);
	PlayerTextDrawFont(playerid, Menu_Stats, 1);
	PlayerTextDrawLetterSize(playerid, Menu_Stats, 0.240000, 1.200000);
	PlayerTextDrawColor(playerid, Menu_Stats, -1);
	PlayerTextDrawSetOutline(playerid, Menu_Stats, 1);
	PlayerTextDrawSetProportional(playerid, Menu_Stats, 1);
	PlayerTextDrawSetShadow(playerid, Menu_Stats, 1);

	Menu_StatsCont = CreatePlayerTextDraw(playerid, 115.000000, 162.000000, "_");
	PlayerTextDrawAlignment(playerid, Menu_StatsCont, 2);
	PlayerTextDrawBackgroundColor(playerid, Menu_StatsCont, 0x00000077);
	PlayerTextDrawFont(playerid, Menu_StatsCont, 1);
	PlayerTextDrawLetterSize(playerid, Menu_StatsCont, 0.240000, 1.200000);
	PlayerTextDrawColor(playerid, Menu_StatsCont, -1);
	PlayerTextDrawSetOutline(playerid, Menu_StatsCont, 1);
	PlayerTextDrawSetProportional(playerid, Menu_StatsCont, 1);
	PlayerTextDrawSetShadow(playerid, Menu_StatsCont, 1);

	Menu_End = CreatePlayerTextDraw(playerid, 115.000000, 200.800000, "_");
	PlayerTextDrawAlignment(playerid, Menu_End, 2);
	PlayerTextDrawBackgroundColor(playerid, Menu_End, 0x00000077);
	PlayerTextDrawFont(playerid, Menu_End, 1);
	PlayerTextDrawLetterSize(playerid, Menu_End, 0.240000, 1.200000);
	PlayerTextDrawColor(playerid, Menu_End, -1);
	PlayerTextDrawSetOutline(playerid, Menu_End, 1);
	PlayerTextDrawSetProportional(playerid, Menu_End, 1);
	PlayerTextDrawSetShadow(playerid, Menu_End, 1);



	new Float:ChatBoxY = 162.000000;

	for(new r = 0; r < MAX_TEXTDRAW_ROWS; r++)
	{

		Menu_Column1[r] = CreatePlayerTextDraw(playerid, 22.000000, ChatBoxY, "~y~1_~w~-_~g~Item_1");
		PlayerTextDrawBackgroundColor(playerid, Menu_Column1[r], 0x00000077);
		PlayerTextDrawFont(playerid, Menu_Column1[r], 1);
		PlayerTextDrawLetterSize(playerid, Menu_Column1[r], 0.240000, 1.200000);
		PlayerTextDrawColor(playerid, Menu_Column1[r], -1);
		PlayerTextDrawSetOutline(playerid, Menu_Column1[r], 1);
		PlayerTextDrawSetProportional(playerid, Menu_Column1[r], 1);
		PlayerTextDrawSetShadow(playerid, Menu_Column1[r], 1);

		Menu_Column2[r] = CreatePlayerTextDraw(playerid, 162.000000, ChatBoxY, "~b~$100000000");
		PlayerTextDrawBackgroundColor(playerid, Menu_Column2[r], 0x00000077);
		PlayerTextDrawFont(playerid, Menu_Column2[r], 1);
		PlayerTextDrawLetterSize(playerid, Menu_Column2[r], 0.240000, 1.200000);
		PlayerTextDrawColor(playerid, Menu_Column2[r], -1);
		PlayerTextDrawSetOutline(playerid, Menu_Column2[r], 1);
		PlayerTextDrawSetProportional(playerid, Menu_Column2[r], 1);
		PlayerTextDrawSetShadow(playerid, Menu_Column2[r], 1);

		ChatBoxY += 10.000000;
	}
	return 1;
}

DestroyMenuBox(playerid)
{
	PlayerTextDrawDestroy(playerid, Menu_Title);
	PlayerTextDrawDestroy(playerid, Menu_Stats);
	PlayerTextDrawDestroy(playerid, Menu_StatsCont);
	PlayerTextDrawDestroy(playerid, Menu_End);
	for(new r = 0; r < MAX_TEXTDRAW_ROWS; r++)
	{
		PlayerTextDrawDestroy(playerid, Menu_Column1[r]);
		PlayerTextDrawDestroy(playerid, Menu_Column2[r]);
	}
	return 1;
}

ShowTextDrawMenu(playerid, menuid, title[], lines, options)
{
	new string[256];

	switch (lines)
	{
		case 1: format(string, sizeof(string), " ~n~%s~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~", title);
		case 2: format(string, sizeof(string), " ~n~%s~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ", title);
		case 3: format(string, sizeof(string), " ~n~%s~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ", title);
		case 4: format(string, sizeof(string), " ~n~%s~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~", title);
		case 5: format(string, sizeof(string), " ~n~%s~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ", title);
		case 6: format(string, sizeof(string), " ~n~%s~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ", title);
		case 7: format(string, sizeof(string), " ~n~%s~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ", title);
		case 8: format(string, sizeof(string), " ~n~%s~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ", title);
		case 9: format(string, sizeof(string), " ~n~%s~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ", title);
		case 10: format(string, sizeof(string), " ~n~%s~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ", title);
		case 11: format(string, sizeof(string), " ~n~%s~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ", title);
		case 12: format(string, sizeof(string), " ~n~%s~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ", title);
		case 13: format(string, sizeof(string), " ~n~%s~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ", title);
		case 14: format(string, sizeof(string), " ~n~%s~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~", title);
		case 15: format(string, sizeof(string), " ~n~%s~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ", title);
		case 16: format(string, sizeof(string), " ~n~%s~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ", title);
		case 17: format(string, sizeof(string), " ~n~%s~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ", title);
		case 18: format(string, sizeof(string), " ~n~%s~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ", title);
		case 19: format(string, sizeof(string), " ~n~%s~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ", title);
		case 20: format(string, sizeof(string), " ~n~%s~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ", title);
		case 21: format(string, sizeof(string), " ~n~%s~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ", title);
		case 22: format(string, sizeof(string), " ~n~%s~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~", title);
		case 23: format(string, sizeof(string), " ~n~%s~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ", title);
		case 24: format(string, sizeof(string), " ~n~%s~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~", title);
	}
	printf("lINES %d",lines);

	PlayerTextDrawSetString(playerid, Menu_Title, string);
	PlayerTextDrawShow(playerid, Menu_Title);
	if (options > 0)
	{
	    if(!IsPlayerInAnyVehicle(playerid))
	    {
			format(string, sizeof(string), "Press (~p~~h~T~w~) Or (~p~~h~F6~w~) and enter a number~n~Press ~y~~k~~PED_FIREWEAPON~ ~w~to close this box");
		}
		else
		{
		    format(string, sizeof(string), "Press (~p~~h~T~w~) Or (~p~~h~F6~w~) And Enter A Number~n~Press ~y~~k~~VEHICLE_HORN~ ~w~to close this box");
		}
	}
	else
	{
	    if(!IsPlayerInAnyVehicle(playerid))
	    {
			format(string, sizeof(string), "Press ~y~~k~~PED_FIREWEAPON~ ~w~to close this box");
		}
		else
		{
		    format(string, sizeof(string), "Press ~y~~k~~VEHICLE_HORN~ ~w~to close this box");
		}
	}

	switch (lines)
	{
		case 1: format(string, sizeof(string), "%s", string);
		case 2: format(string, sizeof(string), "%s", string);
		case 3: format(string, sizeof(string), " ~n~%s", string);
		case 4: format(string, sizeof(string), " ~n~ ~n~%s", string);
		case 5: format(string, sizeof(string), " ~n~ ~n~ ~n~%s", string);
		case 6: format(string, sizeof(string), " ~n~ ~n~ ~n~ ~n~%s", string);
		case 7: format(string, sizeof(string), " ~n~ ~n~ ~n~ ~n~ ~n~%s", string);
		case 8: format(string, sizeof(string), " ~n~ ~n~ ~n~ ~n~ ~n~ ~n~%s", string);
		case 9: format(string, sizeof(string), " ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~%s", string);
		case 10: format(string, sizeof(string), " ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~%s", string);
		case 11: format(string, sizeof(string), " ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~%s", string);
		case 12: format(string, sizeof(string), " ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~%s", string);
		case 13: format(string, sizeof(string), " ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~%s", string);
		case 14: format(string, sizeof(string), " ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~%s", string);
		case 15: format(string, sizeof(string), " ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~%s", string);
		case 16: format(string, sizeof(string), " ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~%s", string);
		case 17: format(string, sizeof(string), " ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~%s", string);
		case 18: format(string, sizeof(string), " ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~%s", string);
		case 19: format(string, sizeof(string), " ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~%s", string);
		case 20: format(string, sizeof(string), " ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~%s", string);
		case 21: format(string, sizeof(string), " ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~%s", string);
		case 22: format(string, sizeof(string), " ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~%s", string);
		case 23: format(string, sizeof(string), " ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~%s", string);
		case 24: format(string, sizeof(string), " ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~ ~n~%s", string);
	}

	PlayerTextDrawSetString(playerid, Menu_End, string);
	PlayerTextDrawShow(playerid, Menu_End);

	Menu_Options[playerid] = options;
	PlayerInfo[playerid][pMenu] = menuid;
}

ShowTextDrawMenuItems(playerid, row, stats[], column1[], column2[],item_price,item = 0)
{
	if (row == 0)
	{
		PlayerTextDrawSetString(playerid,Menu_Stats, stats);
		PlayerTextDrawSetString(playerid,Menu_StatsCont," ");
		PlayerTextDrawShow(playerid, Menu_Stats);
		PlayerTextDrawShow(playerid, Menu_StatsCont);
	}
	else
	{
		PlayerTextDrawSetString(playerid,Menu_Column1[row], column1);
		PlayerTextDrawSetString(playerid,Menu_Column2[row], column2);
		PlayerTextDrawShow(playerid, Menu_Column1[row]);
		PlayerTextDrawShow(playerid, Menu_Column2[row]);
		new rowsprice = row-1;
		Menu_Item_Price[playerid][rowsprice] = item_price;
	}
	Menu_Item[playerid][row] = item;
	return 1;
}

HideTextDrawMenu(playerid,show_msg = 0)
{

	PlayerTextDrawHide(playerid, Menu_Title);
	PlayerTextDrawHide(playerid, Menu_Stats);
	PlayerTextDrawHide(playerid, Menu_StatsCont);
	PlayerTextDrawHide(playerid, Menu_End);

    for(new r = 0; r < MAX_TEXTDRAW_ROWS; r++)
	{
		PlayerTextDrawHide(playerid, Menu_Column1[r]);
		PlayerTextDrawHide(playerid, Menu_Column2[r]);
	}


	if (show_msg == 1)
	{
		SendClientMessage(playerid, COLOR_ERROR, "Invalid option.");
	}

	if(PlayerInfo[playerid][pMenu] == MENU_EDIT_INT_SETTING || PlayerInfo[playerid][pMenu] == MENU_EDIT_INT_SETTING_NAME)
	{
		PlayerInfo[playerid][pEditingInterior]=false;
	}
	if(PlayerInfo[playerid][pMenu] == MENU_EDIT_ACTOR_SETTING)
	{
		PlayerInfo[playerid][pEditingInterior]=false;
	}
	Menu_CurrentOptionSlot[playerid] = 0;
	PlayerInfo[playerid][pMenu] = 0;
	return 1;
}

OnPlayerEnterTextDrawMenuOption(playerid,text[])
{
	new /*string[500],*/Option,str[256];

	new Float:X, Float:Y, Float:Z, Float:A, Float:Health, Float:Armour;
	GetPlayerPos(playerid, X, Y, Z);
	GetPlayerFacingAngle(playerid, A);
	GetXYInFrontOfPlayer(playerid, X, Y, 5.0);
	GetPlayerHealth(playerid,Health);
	GetPlayerArmour(playerid,Armour);

	if (isNumeric(text))
	{
    	Option = strval(text);
	}
	else
	{
		StringReplace(str,text,256);
		Option = -1;
	}

	switch (PlayerInfo[playerid][pMenu])
	{
	 	case NO_MENU,MENU_INFO: {}
	 	case MENU_ADD_INT_TYPE:
 		{
		  	if (Option != -1)
			{
				if (Option >= 1 && Option <= Menu_Options[playerid] )
				{
					PlayerTextDrawHide(playerid, Menu_Title);
					PlayerTextDrawHide(playerid, Menu_Stats);
					PlayerTextDrawHide(playerid, Menu_StatsCont);
					PlayerTextDrawHide(playerid, Menu_End);

		    		for(new r = 0; r < MAX_TEXTDRAW_ROWS; r++)
					{
						PlayerTextDrawHide(playerid, Menu_Column1[r]);
						PlayerTextDrawHide(playerid, Menu_Column2[r]);
					}

					OnAddInteriorMenu(playerid,Option);
					return 1;
				}	
				else
				{
					SendClientMessage(playerid, COLOR_ERROR, "Invalid option.");
				}
			}
			else return 0;
		}
		case MENU_EDIT_INTERIORS_LIST:
 		{
		  	if (Option != -1)
			{
				if (Option >= 1 && Option <= Menu_Options[playerid] )
				{
					PlayerTextDrawHide(playerid, Menu_Title);
					PlayerTextDrawHide(playerid, Menu_Stats);
					PlayerTextDrawHide(playerid, Menu_StatsCont);
					PlayerTextDrawHide(playerid, Menu_End);

		    		for(new r = 0; r < MAX_TEXTDRAW_ROWS; r++)
					{
						PlayerTextDrawHide(playerid, Menu_Column1[r]);
						PlayerTextDrawHide(playerid, Menu_Column2[r]);
					}
					OnEditInteriorList(playerid,Option,Menu_Item_Price[playerid][Option]);
					return 1;
				}	
				else
				{
					SendClientMessage(playerid, COLOR_ERROR, "Invalid option.");
				}
			}
			else return 0;
		}
		case MENU_EDIT_INT_SETTING:
 		{
		  	if (Option != -1)
			{
				if (Option >= 1 && Option <= Menu_Options[playerid] )
				{
					PlayerTextDrawHide(playerid, Menu_Title);
					PlayerTextDrawHide(playerid, Menu_Stats);
					PlayerTextDrawHide(playerid, Menu_StatsCont);
					PlayerTextDrawHide(playerid, Menu_End);

		    		for(new r = 0; r < MAX_TEXTDRAW_ROWS; r++)
					{
						PlayerTextDrawHide(playerid, Menu_Column1[r]);
						PlayerTextDrawHide(playerid, Menu_Column2[r]);
					}
					OnInteriorSettingChange(playerid,Option);
					return 1;
				}	
				else
				{
					SendClientMessage(playerid, COLOR_ERROR, "Invalid option.");
				}
			}
			else return 0;
		}
		case MENU_EDIT_INT_SETTING_NAME:
 		{
		  	PlayerTextDrawHide(playerid, Menu_Title);
			PlayerTextDrawHide(playerid, Menu_Stats);
			PlayerTextDrawHide(playerid, Menu_StatsCont);
			PlayerTextDrawHide(playerid, Menu_End);

		    for(new r = 0; r < MAX_TEXTDRAW_ROWS; r++)
			{
				PlayerTextDrawHide(playerid, Menu_Column1[r]);
				PlayerTextDrawHide(playerid, Menu_Column2[r]);
			}
			format(PlayerInfo[playerid][pAddInteriorZone],MAX_ZONE_NAME,"%s",str);
			SendClientMessage(playerid,COLOR_ADMIN_INFO,"Updated interior name.");
			UpdateInteriorSettings(playerid,PlayerInfo[playerid][pEditingIntID],1);
		}
		case MENU_SAMP_TOWER,MENU_GRIN_TOWER:
 		{
		  	if (Option != -1)
			{
				if (Option >= 1 && Option <= Menu_Options[playerid] )
				{
					PlayerTextDrawHide(playerid, Menu_Title);
					PlayerTextDrawHide(playerid, Menu_Stats);
					PlayerTextDrawHide(playerid, Menu_StatsCont);
					PlayerTextDrawHide(playerid, Menu_End);

		    		for(new r = 0; r < MAX_TEXTDRAW_ROWS; r++)
					{
						PlayerTextDrawHide(playerid, Menu_Column1[r]);
						PlayerTextDrawHide(playerid, Menu_Column2[r]);
					}
					if(PlayerInfo[playerid][pMenu] == MENU_SAMP_TOWER) OnSampElevatorMenu(playerid,Option);
					else OnGRINElevatorMenu(playerid,Option);
					return 1;
				}	
				else
				{
					SendClientMessage(playerid, COLOR_ERROR, "Invalid Floor.");
					return 1;
				}
			}
			else return 0;
		}
		case MENU_ADD_ACTOR_TYPE, MENU_ADD_ACTOR_TYPE_2:
 		{
		  	if (Option != -1)
			{
				if (Option >= 1 && Option <= Menu_Options[playerid] )
				{
					PlayerTextDrawHide(playerid, Menu_Title);
					PlayerTextDrawHide(playerid, Menu_Stats);
					PlayerTextDrawHide(playerid, Menu_StatsCont);
					PlayerTextDrawHide(playerid, Menu_End);

		    		for(new r = 0; r < MAX_TEXTDRAW_ROWS; r++)
					{
						PlayerTextDrawHide(playerid, Menu_Column1[r]);
						PlayerTextDrawHide(playerid, Menu_Column2[r]);
					}
					if(PlayerInfo[playerid][pMenu] == MENU_ADD_ACTOR_TYPE_2) Option+=20;
					OnAddActorMenu(playerid,Option);
					return 1;
				}	
				else
				{
					SendClientMessage(playerid, COLOR_ERROR, "Invalid option.");
				}
			}
			else return 0;
		}
		case MENU_EDIT_ACTORS_LIST:
 		{
		  	if (Option != -1)
			{
				if (Option >= 1 && Option <= Menu_Options[playerid] )
				{
					PlayerTextDrawHide(playerid, Menu_Title);
					PlayerTextDrawHide(playerid, Menu_Stats);
					PlayerTextDrawHide(playerid, Menu_StatsCont);
					PlayerTextDrawHide(playerid, Menu_End);

		    		for(new r = 0; r < MAX_TEXTDRAW_ROWS; r++)
					{
						PlayerTextDrawHide(playerid, Menu_Column1[r]);
						PlayerTextDrawHide(playerid, Menu_Column2[r]);
					}
					OnEditActorsList(playerid,Option,Menu_Item_Price[playerid][Option]);
					return 1;
				}	
				else
				{
					SendClientMessage(playerid, COLOR_ERROR, "Invalid option.");
				}
			}
			else return 0;
		}
		case MENU_EDIT_ACTOR_SETTING:
 		{
		  	if (Option != -1)
			{
				if (Option >= 1 && Option <= Menu_Options[playerid] )
				{
					PlayerTextDrawHide(playerid, Menu_Title);
					PlayerTextDrawHide(playerid, Menu_Stats);
					PlayerTextDrawHide(playerid, Menu_StatsCont);
					PlayerTextDrawHide(playerid, Menu_End);

		    		for(new r = 0; r < MAX_TEXTDRAW_ROWS; r++)
					{
						PlayerTextDrawHide(playerid, Menu_Column1[r]);
						PlayerTextDrawHide(playerid, Menu_Column2[r]);
					}
					OnActorSettingChange(playerid,Option);
					return 1;
				}	
				else
				{
					SendClientMessage(playerid, COLOR_ERROR, "Invalid option.");
				}
			}
			else return 0;
		}
		case MENU_GPS_OPTIONS:
 		{
		  	if (Option != -1)
			{
				if (Option >= 1 && Option <= Menu_Options[playerid] )
				{
					PlayerTextDrawHide(playerid, Menu_Title);
					PlayerTextDrawHide(playerid, Menu_Stats);
					PlayerTextDrawHide(playerid, Menu_StatsCont);
					PlayerTextDrawHide(playerid, Menu_End);

		    		for(new r = 0; r < MAX_TEXTDRAW_ROWS; r++)
					{
						PlayerTextDrawHide(playerid, Menu_Column1[r]);
						PlayerTextDrawHide(playerid, Menu_Column2[r]);
					}

					OnGPSMenu(playerid,Option,-1,-1);
					return 1;
				}	
				else
				{
					SendClientMessage(playerid, COLOR_ERROR, "Invalid option.");
				}
			}
			else return 0;
		}
		case MENU_GPS_LIST:
 		{
		  	if (Option != -1)
			{
				if (Option >= 1 && Option <= Menu_Options[playerid] )
				{
					PlayerTextDrawHide(playerid, Menu_Title);
					PlayerTextDrawHide(playerid, Menu_Stats);
					PlayerTextDrawHide(playerid, Menu_StatsCont);
					PlayerTextDrawHide(playerid, Menu_End);

		    		for(new r = 0; r < MAX_TEXTDRAW_ROWS; r++)
					{
						PlayerTextDrawHide(playerid, Menu_Column1[r]);
						PlayerTextDrawHide(playerid, Menu_Column2[r]);
					}

					OnGPSMenu(playerid,1,Menu_Item_Price[playerid][Option],-1);
					return 1;
				}	
				else
				{
					SendClientMessage(playerid, COLOR_ERROR, "Invalid option.");
				}
			}
			else return 0;
		}
	}
	return 0;
}

FUNCTION UpdateMenuBG(playerid)
{
	PlayerTextDrawBoxColor(playerid, Menu_Title, PlayerInfo[playerid][pMenuTD_BG]);
	return 1;
}