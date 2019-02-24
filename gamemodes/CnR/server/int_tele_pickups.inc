/*

	Interiors/Teleports Script
	Scripted by Arose Niazi.
	Scripting started on 20th October 2017
	Last Updated: -----
	
	Credits:
		
	Functions:
		AddInterior(playerid,text[]);
		LoadInterior();
		LoadInteriorID(interiorid);
		DeleteInterior();
		SetupInterior(interior_id);
		OnEditInteriorList(playerid,Option,ID)
		OnInteriorSettingChange(playerid,Option)
		UpdateInteriorSettings(playerid, interiorid, option)
		InteriorUpdated(playerid,interiorid)
		OnAddInteriorMenu(playerid,Option);
		/addinterior
		/stopadding
		/editinteriors
		
	Functions Being Called:
		
*/

#define MAX_INTERIORS 200

enum Interior_enum
{
	InteriorID,
	InteriorType,
	InteriorName[MAX_ZONE_NAME],
	InteriorOutZone[MAX_ZONE_NAME],
	Float:InteriorSpawnX,
	Float:InteriorSpawnY,
	Float:InteriorSpawnZ,
	Float:InteriorSpawnA,
	Float:InteriorPickupX,
	Float:InteriorPickupY,
	Float:InteriorPickupZ,
	InteriorInt,
	InteriorVW,
	bool:InteriorCivWeapons,
	bool:InteriorCopsWeapons,
	bool:InteriorWeapons,
	bool:InteriorLabels,
	Text3D:InteriorLabel
};

enum Interiro_E_enum
{
	ExteriorID,
	Float:ExteriorSpawnX,
	Float:ExteriorSpawnY,
	Float:ExteriorSpawnZ,
	Float:ExteriorSpawnA,
	Float:ExteriorPickupX,
	Float:ExteriorPickupY,
	Float:ExteriorPickupZ,
	Text3D:ExteriorLabel
};

new InteriorInfo[MAX_INTERIORS][Interior_enum];
new ExteriorInfo[MAX_INTERIORS][Interiro_E_enum];

FUNCTION LoadInterior()
{
	new string[128];
	new rows,Cache:result;
	mysql_format(g_SQL, string, sizeof(string), "SELECT * FROM Interiors");
	result = mysql_query(g_SQL,string);
	cache_get_row_count(rows);
	if(rows)
	{
		if(rows > MAX_INTERIORS) rows=MAX_INTERIORS;
		for(new i=0; i<rows; i++)
		{
			cache_get_value_int(i, "Type",InteriorInfo[i][InteriorType]);
			cache_get_value_int(i, "Interior",InteriorInfo[i][InteriorInt]);
			cache_get_value_int(i, "VirtualWorld",InteriorInfo[i][InteriorVW]);
			cache_get_value_float(i, "InSpawnX",InteriorInfo[i][InteriorSpawnX]);
			cache_get_value_float(i, "InSpawnY",InteriorInfo[i][InteriorSpawnY]);
			cache_get_value_float(i, "InSpawnZ",InteriorInfo[i][InteriorSpawnZ]);
			cache_get_value_float(i, "InSpawnA",InteriorInfo[i][InteriorSpawnA]);
			cache_get_value_float(i, "OutPickupX",InteriorInfo[i][InteriorPickupX]);
			cache_get_value_float(i, "OutPickupY",InteriorInfo[i][InteriorPickupY]);
			cache_get_value_float(i, "OutPickupZ",InteriorInfo[i][InteriorPickupZ]);
			cache_get_value_name(i, "InteriorName",InteriorInfo[i][InteriorName],MAX_ZONE_NAME);
			cache_get_value_name(i, "OutsideZone",InteriorInfo[i][InteriorOutZone],MAX_ZONE_NAME);
			cache_get_value_bool(i, "CivWeapons",InteriorInfo[i][InteriorCivWeapons]);
			cache_get_value_bool(i, "CopsWeapons",InteriorInfo[i][InteriorCopsWeapons]);
			cache_get_value_bool(i, "Weapons",InteriorInfo[i][InteriorWeapons]);
			cache_get_value_bool(i, "3DLabel",InteriorInfo[i][InteriorLabels]);
			cache_get_value_float(i, "OutSpawnX",ExteriorInfo[i][ExteriorSpawnX]);
			cache_get_value_float(i, "OutSpawnY",ExteriorInfo[i][ExteriorSpawnY]);
			cache_get_value_float(i, "OutSpawnZ",ExteriorInfo[i][ExteriorSpawnZ]);
			cache_get_value_float(i, "OutSpawnA",ExteriorInfo[i][ExteriorSpawnA]);
			cache_get_value_float(i, "InPickupX",ExteriorInfo[i][ExteriorPickupX]);
			cache_get_value_float(i, "InPickupY",ExteriorInfo[i][ExteriorPickupY]);
			cache_get_value_float(i, "InPickupZ",ExteriorInfo[i][ExteriorPickupZ]);
			SetupInterior(i);
		}
	}
	cache_delete(result);
	return 1;
}

FUNCTION LoadInteriorID(interiorid)
{
	new string[128];
	new rows,Cache:result;
	mysql_format(g_SQL, string, sizeof(string), "SELECT * FROM Interiors WHERE ID=%d",interiorid);
	result = mysql_query(g_SQL,string);
	cache_get_row_count(rows);
	if(rows)
	{
		new i=interiorid-1;
		cache_get_value_int(0, "Type",InteriorInfo[i][InteriorType]);
		cache_get_value_int(0, "Interior",InteriorInfo[i][InteriorInt]);
		cache_get_value_int(0, "VirtualWorld",InteriorInfo[i][InteriorVW]);
		cache_get_value_float(0, "InSpawnX",InteriorInfo[i][InteriorSpawnX]);
		cache_get_value_float(0, "InSpawnY",InteriorInfo[i][InteriorSpawnY]);
		cache_get_value_float(0, "InSpawnZ",InteriorInfo[i][InteriorSpawnZ]);
		cache_get_value_float(0, "InSpawnA",InteriorInfo[i][InteriorSpawnA]);
		cache_get_value_float(0, "OutPickupX",InteriorInfo[i][InteriorPickupX]);
		cache_get_value_float(0, "OutPickupY",InteriorInfo[i][InteriorPickupY]);
		cache_get_value_float(0, "OutPickupZ",InteriorInfo[i][InteriorPickupZ]);
		cache_get_value_name(0, "InteriorName",InteriorInfo[i][InteriorName],MAX_ZONE_NAME);
		cache_get_value_name(0, "OutsideZone",InteriorInfo[i][InteriorOutZone],MAX_ZONE_NAME);
		cache_get_value_bool(0, "CivWeapons",InteriorInfo[i][InteriorCivWeapons]);
		cache_get_value_bool(0, "CopsWeapons",InteriorInfo[i][InteriorCopsWeapons]);
		cache_get_value_bool(0, "Weapons",InteriorInfo[i][InteriorWeapons]);
		cache_get_value_bool(0, "3DLabel",InteriorInfo[i][InteriorLabels]);
		cache_get_value_float(0, "OutSpawnX",ExteriorInfo[i][ExteriorSpawnX]);
		cache_get_value_float(0, "OutSpawnY",ExteriorInfo[i][ExteriorSpawnY]);
		cache_get_value_float(0, "OutSpawnZ",ExteriorInfo[i][ExteriorSpawnZ]);
		cache_get_value_float(0, "OutSpawnA",ExteriorInfo[i][ExteriorSpawnA]);
		cache_get_value_float(0, "InPickupX",ExteriorInfo[i][ExteriorPickupX]);
		cache_get_value_float(0, "InPickupY",ExteriorInfo[i][ExteriorPickupY]);
		cache_get_value_float(0, "InPickupZ",ExteriorInfo[i][ExteriorPickupZ]);
		SetupInterior(i);
	}
	cache_delete(result);
	return 1;
}


FUNCTION OnEditInteriorList(playerid,Option,ID)
{
	new string[100];
	new rows,Cache:result;
	switch (Option)
	{
		case 21:
		{
			format(string,sizeof(string),"%d",ID);
			callcmd::editinteriors(playerid,string);
		}
		default:
		{
			HideTextDrawMenu(playerid);
			mysql_format(g_SQL, string, sizeof(string), "SELECT * FROM Interiors WHERE ID = %d",ID);
			result = mysql_query(g_SQL,string);
			cache_get_row_count(rows);
			if(rows)
			{
				cache_get_value_int(0, "Type",PlayerInfo[playerid][pAddInteriorType]);
				cache_get_value_int(0, "Interior",PlayerInfo[playerid][pAddInteriorInt]);
				cache_get_value_int(0, "VirtualWorld",PlayerInfo[playerid][pAddInteriorVW]);
				cache_get_value_float(0, "InSpawnX",PlayerInfo[playerid][pAddInteriorISX]);
				cache_get_value_float(0, "InSpawnY",PlayerInfo[playerid][pAddInteriorISY]);
				cache_get_value_float(0, "InSpawnZ",PlayerInfo[playerid][pAddInteriorISZ]);
				cache_get_value_float(0, "InSpawnA",PlayerInfo[playerid][pAddInteriorISA]);
				cache_get_value_float(0, "OutPickupX",PlayerInfo[playerid][pAddInteriorOPX]);
				cache_get_value_float(0, "OutPickupY",PlayerInfo[playerid][pAddInteriorOPY]);
				cache_get_value_float(0, "OutPickupZ",PlayerInfo[playerid][pAddInteriorOPZ]);
				cache_get_value_name(0, "InteriorName",PlayerInfo[playerid][pAddInteriorZone],MAX_ZONE_NAME);
				cache_get_value_name(0, "OutsideZone",PlayerInfo[playerid][pAddInteriorOutZone],MAX_ZONE_NAME);
				cache_get_value_bool(0, "CivWeapons",PlayerInfo[playerid][pAddInteriorCivWeapons]);
				cache_get_value_bool(0, "CopsWeapons",PlayerInfo[playerid][pAddInteriorCopWeapons]);
				cache_get_value_bool(0, "Weapons",PlayerInfo[playerid][pAddInteriorWeapons]);
				cache_get_value_float(0, "OutSpawnX",PlayerInfo[playerid][pAddInteriorOSX]);
				cache_get_value_float(0, "OutSpawnY",PlayerInfo[playerid][pAddInteriorOSY]);
				cache_get_value_float(0, "OutSpawnZ",PlayerInfo[playerid][pAddInteriorOSZ]);
				cache_get_value_float(0, "OutSpawnA",PlayerInfo[playerid][pAddInteriorOSA]);
				cache_get_value_float(0, "InPickupX",PlayerInfo[playerid][pAddInteriorIPX]);
				cache_get_value_float(0, "InPickupY",PlayerInfo[playerid][pAddInteriorIPY]);
				cache_get_value_float(0, "InPickupZ",PlayerInfo[playerid][pAddInteriorIPZ]);
				cache_get_value_bool(0, "3DLabel",PlayerInfo[playerid][pAddInteriorLabels]);
				PlayerInfo[playerid][pEditingInterior]=true;
				PlayerInfo[playerid][pEditingIntID]=ID;
				ShowTextDrawMenu(playerid, MENU_EDIT_INT_SETTING,"~y~Interior Settings", 13, 12);
				if(strlen(PlayerInfo[playerid][pAddInteriorOutZone]) > 2) format(string,sizeof(string),"Editing ~p~%s ~w~in~p~ %s",PlayerInfo[playerid][pAddInteriorZone],PlayerInfo[playerid][pAddInteriorOutZone]);
				else format(string,sizeof(string),"Editing ~p~%s",PlayerInfo[playerid][pAddInteriorZone]);
				ShowTextDrawMenuItems(playerid, 0, string, " ", " ",0);
				format(string,sizeof(string),"~b~~h~1 ~w~- %s",PlayerInfo[playerid][pAddInteriorZone]);
				ShowTextDrawMenuItems(playerid, 2, " ", string, " ",0);
				format(string,sizeof(string),"~b~~h~2 ~w~- %s",PlayerInfo[playerid][pAddInteriorOutZone]);
				ShowTextDrawMenuItems(playerid, 3, " ", string, " ",0);
				format(string,sizeof(string),"~b~%d",PlayerInfo[playerid][pAddInteriorInt]);
				ShowTextDrawMenuItems(playerid, 4, " ", "~b~~h~3 ~w~- Interior ID", string,0);
				format(string,sizeof(string),"~b~%d",PlayerInfo[playerid][pAddInteriorVW]);
				ShowTextDrawMenuItems(playerid, 5, " ", "~b~~h~4 ~w~- Virtual World ID:", string,0);
				ShowTextDrawMenuItems(playerid, 6, " ", "~b~~h~5 ~w~- Outside Spawn", " ",0);
				ShowTextDrawMenuItems(playerid, 7, " ", "~b~~h~6 ~w~- Inside Spawn", " ",0);
				ShowTextDrawMenuItems(playerid, 8, " ", "~b~~h~7 ~w~- Outside Pickup", " ",0);
				ShowTextDrawMenuItems(playerid, 9, " ", "~b~~h~8 ~w~- Inside Pickup", " ",0);
				if(PlayerInfo[playerid][pAddInteriorCivWeapons])ShowTextDrawMenuItems(playerid, 10, " ", "~b~~h~9 ~w~- Civilian Weapons", "~g~Allowed",0);
				else ShowTextDrawMenuItems(playerid, 10, " ", "~b~~h~9 ~w~- Civilian Weapons", "~r~Disallowed",0);
				if(PlayerInfo[playerid][pAddInteriorCopWeapons])ShowTextDrawMenuItems(playerid, 11, " ", "~b~~h~10 ~w~- Cops Weapons", "~g~Allowed",0);
				else ShowTextDrawMenuItems(playerid, 11, " ", "~b~~h~10 ~w~- Cops Weapons", "~r~Disallowed",0);
				if(PlayerInfo[playerid][pAddInteriorWeapons])ShowTextDrawMenuItems(playerid, 12, " ", "~b~~h~11 ~w~- Weapons", "~g~Allowed",0);
				else ShowTextDrawMenuItems(playerid, 12, " ", "~b~~h~11 ~w~- Weapons", "~r~Disallowed",0);
				if(PlayerInfo[playerid][pAddInteriorLabels])ShowTextDrawMenuItems(playerid, 13, " ", "~b~~h~12 ~w~- 3D Label", "~g~Enabled",0);
				else ShowTextDrawMenuItems(playerid, 13, " ", "~b~~h~12 ~w~- 3D Label", "~r~Dsiabled",0);
			}
			cache_delete(result);	
		}
	}
	return 1;
}

FUNCTION OnInteriorSettingChange(playerid,Option)
{
	new string[125];
	switch (Option)
	{
		case 1:
		{
			ShowTextDrawMenu(playerid, MENU_EDIT_INT_SETTING_NAME,"~y~Zone Name Edit", 4, 0);
			format(string,sizeof(string),"Zone name will be edited.~n~Current Name: ~p~%s~n~~w~enter a new to change",PlayerInfo[playerid][pAddInteriorZone]);
			ShowTextDrawMenuItems(playerid, 0, string, " ", " ",0);
		}
		case 2:
		{
			format(PlayerInfo[playerid][pAddInteriorOutZone],MAX_ZONE_NAME,"%s",PlayerInfo[playerid][pZoneName]);
			SendClientMessage(playerid,COLOR_ADMIN_INFO,"Out zone name Updated.");
			UpdateInteriorSettings(playerid,PlayerInfo[playerid][pEditingIntID],Option);
			format(string,sizeof(string),"%s (%d) updated zone name to %s for interior id '%d'",PlayerInfo[playerid][pUserName],playerid,PlayerInfo[playerid][pAddInteriorOutZone],PlayerInfo[playerid][pEditingIntID]);
		}
		case 3:
		{
			PlayerInfo[playerid][pAddInteriorInt]=GetPlayerInterior(playerid);
			SendClientMessage(playerid,COLOR_ADMIN_INFO,"Interior Updated.");
			UpdateInteriorSettings(playerid,PlayerInfo[playerid][pEditingIntID],Option);
			format(string,sizeof(string),"%s (%d) updated interior id to %d for interior id '%d'",PlayerInfo[playerid][pUserName],playerid,PlayerInfo[playerid][pAddInteriorInt],PlayerInfo[playerid][pEditingIntID]);
		}
		case 4:
		{
			PlayerInfo[playerid][pAddInteriorVW]=GetPlayerVirtualWorld(playerid);
			SendClientMessage(playerid,COLOR_ADMIN_INFO,"Virtual World Updated.");
			UpdateInteriorSettings(playerid,PlayerInfo[playerid][pEditingIntID],Option);
			format(string,sizeof(string),"%s (%d) updated virtual world id to %d for interior id '%d'",PlayerInfo[playerid][pUserName],playerid,PlayerInfo[playerid][pAddInteriorVW],PlayerInfo[playerid][pEditingIntID]);
		}
		case 5:
		{
			GetPlayerPos(playerid,PlayerInfo[playerid][pAddInteriorOSX],PlayerInfo[playerid][pAddInteriorOSY],PlayerInfo[playerid][pAddInteriorOSZ]);
			GetPlayerFacingAngle(playerid,PlayerInfo[playerid][pAddInteriorOSA]);
			SendClientMessage(playerid,COLOR_ADMIN_INFO,"Updated outside spawn.");
			UpdateInteriorSettings(playerid,PlayerInfo[playerid][pEditingIntID],Option);
			format(string,sizeof(string),"%s (%d) updated outside spawn to (%0.2f,%0.2f,%0.2f,%0.2f) for interior id '%d'",PlayerInfo[playerid][pUserName],playerid,PlayerInfo[playerid][pAddInteriorOSX],PlayerInfo[playerid][pAddInteriorOSY],PlayerInfo[playerid][pAddInteriorOSZ],PlayerInfo[playerid][pAddInteriorOSA],PlayerInfo[playerid][pEditingIntID]);
		}
		case 6:
		{
			GetPlayerPos(playerid,PlayerInfo[playerid][pAddInteriorISX],PlayerInfo[playerid][pAddInteriorISY],PlayerInfo[playerid][pAddInteriorISZ]);
			GetPlayerFacingAngle(playerid,PlayerInfo[playerid][pAddInteriorISA]);
			SendClientMessage(playerid,COLOR_ADMIN_INFO,"Updated inside spawn.");
			UpdateInteriorSettings(playerid,PlayerInfo[playerid][pEditingIntID],Option);
			format(string,sizeof(string),"%s (%d) updated inside spawn to (%0.2f,%0.2f,%0.2f,%0.2f) for interior id '%d'",PlayerInfo[playerid][pUserName],playerid,PlayerInfo[playerid][pAddInteriorISX],PlayerInfo[playerid][pAddInteriorISY],PlayerInfo[playerid][pAddInteriorISZ],PlayerInfo[playerid][pAddInteriorISA],PlayerInfo[playerid][pEditingIntID]);
		}
		case 7:
		{
			GetPlayerPos(playerid,PlayerInfo[playerid][pAddInteriorOPX],PlayerInfo[playerid][pAddInteriorOPY],PlayerInfo[playerid][pAddInteriorOPZ]);
			SendClientMessage(playerid,COLOR_ADMIN_INFO,"Updated outside pickup.");
			UpdateInteriorSettings(playerid,PlayerInfo[playerid][pEditingIntID],Option);
			format(string,sizeof(string),"%s (%d) updated outside pickup to (%0.2f,%0.2f,%0.2f) for interior id '%d'",PlayerInfo[playerid][pUserName],playerid,PlayerInfo[playerid][pAddInteriorOPX],PlayerInfo[playerid][pAddInteriorOPY],PlayerInfo[playerid][pAddInteriorOPZ],PlayerInfo[playerid][pEditingIntID]);
		}
		case 8:
		{
			GetPlayerPos(playerid,PlayerInfo[playerid][pAddInteriorIPX],PlayerInfo[playerid][pAddInteriorIPY],PlayerInfo[playerid][pAddInteriorIPZ]);
			SendClientMessage(playerid,COLOR_ADMIN_INFO,"Updated inside pickup.");
			UpdateInteriorSettings(playerid,PlayerInfo[playerid][pEditingIntID],Option);
			format(string,sizeof(string),"%s (%d) updated inside pickup to (%0.2f,%0.2f,%0.2f) for interior id '%d'",PlayerInfo[playerid][pUserName],playerid,PlayerInfo[playerid][pAddInteriorIPX],PlayerInfo[playerid][pAddInteriorIPY],PlayerInfo[playerid][pAddInteriorIPZ],PlayerInfo[playerid][pEditingIntID]);
		}
		case 9:
		{
			PlayerInfo[playerid][pAddInteriorCivWeapons]= !PlayerInfo[playerid][pAddInteriorCivWeapons];
			SendClientMessage(playerid,COLOR_ADMIN_INFO,"Updated civilian weapons.");
			UpdateInteriorSettings(playerid,PlayerInfo[playerid][pEditingIntID],Option);
			format(string,sizeof(string),"%s (%d) changed civilian weapons %s for interior id '%d'",PlayerInfo[playerid][pUserName],playerid,(PlayerInfo[playerid][pAddInteriorCivWeapons])? ("enabled"):("disabled"),PlayerInfo[playerid][pEditingIntID]);
		}
		case 10:
		{
			PlayerInfo[playerid][pAddInteriorCopWeapons]= !PlayerInfo[playerid][pAddInteriorCopWeapons];
			SendClientMessage(playerid,COLOR_ADMIN_INFO,"Updated cops weapons.");
			UpdateInteriorSettings(playerid,PlayerInfo[playerid][pEditingIntID],Option);
			format(string,sizeof(string),"%s (%d) changed cops weapons %s for interior id '%d'",PlayerInfo[playerid][pUserName],playerid,(PlayerInfo[playerid][pAddInteriorCopWeapons])? ("enabled"):("disabled"),PlayerInfo[playerid][pEditingIntID]);
		}
		case 11:
		{
			PlayerInfo[playerid][pAddInteriorWeapons]= !PlayerInfo[playerid][pAddInteriorWeapons];
			SendClientMessage(playerid,COLOR_ADMIN_INFO,"Updated weapons.");
			UpdateInteriorSettings(playerid,PlayerInfo[playerid][pEditingIntID],Option);
			format(string,sizeof(string),"%s (%d) changed weapons %s for interior id '%d'",PlayerInfo[playerid][pUserName],playerid,(PlayerInfo[playerid][pAddInteriorWeapons])? ("enabled"):("disabled"),PlayerInfo[playerid][pEditingIntID]);
		}
		case 12:
		{
			PlayerInfo[playerid][pAddInteriorLabels]= !PlayerInfo[playerid][pAddInteriorLabels];
			SendClientMessage(playerid,COLOR_ADMIN_INFO,"Updated labels.");
			UpdateInteriorSettings(playerid,PlayerInfo[playerid][pEditingIntID],Option);
			format(string,sizeof(string),"%s (%d) changed 3d labels %s for interior id '%d'",PlayerInfo[playerid][pUserName],playerid,(PlayerInfo[playerid][pAddInteriorWeapons])? ("enabled"):("disabled"),PlayerInfo[playerid][pEditingIntID]);
		}
	}
	if(Option != 1)
	{
		SendMessageToManagers(COLOR_ADMIN_INFO,string);
		IRC_SendManagementMessage(string,IRC_ADMIN_COLOR_INFO);
	}
	return 1;
}

UpdateInteriorSettings(playerid, interiorid, option)
{
	new string[256];
	switch(option)
	{
		case 1: 
		{
			format(string,sizeof(string),"%s (%d) updated interior name to %s for interior id '%d'",PlayerInfo[playerid][pUserName],playerid,PlayerInfo[playerid][pAddInteriorZone],PlayerInfo[playerid][pEditingIntID]);
			SendMessageToManagers(COLOR_ADMIN_INFO,string);
			IRC_SendManagementMessage(string,IRC_ADMIN_COLOR_INFO);
			mysql_format(g_SQL,string,sizeof(string),"UPDATE Interiors SET InteriorName='%e' WHERE ID=%d",PlayerInfo[playerid][pAddInteriorZone],interiorid);
		}	
		case 2: mysql_format(g_SQL,string,sizeof(string),"UPDATE Interiors SET OutsideZone='%e' WHERE ID=%d",PlayerInfo[playerid][pAddInteriorOutZone],interiorid);
		case 3: mysql_format(g_SQL,string,sizeof(string),"UPDATE Interiors SET Interior=%d WHERE ID=%d",PlayerInfo[playerid][pAddInteriorInt],interiorid); 
		case 4: mysql_format(g_SQL,string,sizeof(string),"UPDATE Interiors SET VirtualWorld=%d WHERE ID=%d",PlayerInfo[playerid][pAddInteriorVW],interiorid);
		case 5: mysql_format(g_SQL,string,sizeof(string),"UPDATE Interiors SET OutSpawnX=%f , OutSpawnY=%f , OutSpawnZ=%f , OutSpawnA=%f  WHERE ID=%d",PlayerInfo[playerid][pAddInteriorOSX],PlayerInfo[playerid][pAddInteriorOSY],PlayerInfo[playerid][pAddInteriorOSZ],PlayerInfo[playerid][pAddInteriorOSA],interiorid);
		case 6: mysql_format(g_SQL,string,sizeof(string),"UPDATE Interiors SET InSpawnX=%f , InSpawnY=%f , InSpawnZ=%f , InSpawnA=%f  WHERE ID=%d",PlayerInfo[playerid][pAddInteriorISX],PlayerInfo[playerid][pAddInteriorISY],PlayerInfo[playerid][pAddInteriorISZ],PlayerInfo[playerid][pAddInteriorISA],interiorid);
		case 7: mysql_format(g_SQL,string,sizeof(string),"UPDATE Interiors SET OutPickupX=%f , OutPickupY=%f , OutPickupZ=%f WHERE ID=%d",PlayerInfo[playerid][pAddInteriorOPX],PlayerInfo[playerid][pAddInteriorOPY],PlayerInfo[playerid][pAddInteriorOPZ],interiorid);
		case 8: mysql_format(g_SQL,string,sizeof(string),"UPDATE Interiors SET InPickupX=%f , InPickupY=%f , InPickupZ=%f WHERE ID=%d",PlayerInfo[playerid][pAddInteriorIPX],PlayerInfo[playerid][pAddInteriorIPY],PlayerInfo[playerid][pAddInteriorIPZ],interiorid);
		case 9: mysql_format(g_SQL,string,sizeof(string),"UPDATE Interiors SET CivWeapons=%d WHERE ID=%d",PlayerInfo[playerid][pAddInteriorCivWeapons],interiorid);
		case 10: mysql_format(g_SQL,string,sizeof(string),"UPDATE Interiors SET CopWeapons=%d WHERE ID=%d",PlayerInfo[playerid][pAddInteriorCopWeapons],interiorid);
		case 11: mysql_format(g_SQL,string,sizeof(string),"UPDATE Interiors SET Weapons=%d WHERE ID=%d",PlayerInfo[playerid][pAddInteriorWeapons],interiorid);
		case 12: mysql_format(g_SQL,string,sizeof(string),"UPDATE Interiors SET 3DLabel=%d WHERE ID=%d",PlayerInfo[playerid][pAddInteriorLabels],interiorid);
	}
	mysql_pquery(g_SQL,string,"InteriorUpdated","dd",playerid,interiorid);
	return 1;
}

FUNCTION InteriorUpdated(playerid,interiorid)
{
	LoadInteriorID(interiorid);
	if(PlayerInfo[playerid][pEditingInterior])
	{
		OnEditInteriorList(playerid,5,PlayerInfo[playerid][pEditingIntID]);
	}
	return 1;
}

OnAddInteriorMenu(playerid,Option)
{
	new string[500];
	switch (Option-1)
	{					
		case SIMPLE_INTERIOR:
		{
			ShowTextDrawMenu(playerid, MENU_INFO,"~y~Simple Interior", 10, 0);
			format(string,sizeof(string),"Adding simple inteiror.~n~Interior would added~n~");
		}
		case REGULAR_ONLY:
		{
			ShowTextDrawMenu(playerid, MENU_INFO,"~y~Regular Player Only", 10, 0);
			format(string,sizeof(string),"Adding regular player only inteiror.~n~Interior for regular players would added~n~");
		}
		case STAFF_ONLY:
		{
			ShowTextDrawMenu(playerid, MENU_INFO,"~Staff Member Only", 10, 0);
			format(string,sizeof(string),"Adding staff only inteiror.~n~Interior for staff would added~n~");
		}	
		case COPS_ONLY:
		{
			ShowTextDrawMenu(playerid, MENU_INFO,"~y~Cops Only", 10, 0);
			format(string,sizeof(string),"Adding cops only inteiror.~n~Interior for cops would added~n~");
		}
		case TELEPORTS:
		{
			ShowTextDrawMenu(playerid, MENU_INFO,"~y~Teleports", 10, 0);
			format(string,sizeof(string),"Adding teleport.~n~Teleport pickup would be added~n~");
		}
		case ROBBERY:
		{
			ShowTextDrawMenu(playerid, MENU_INFO,"~y~Robbery Locations", 10, 0);
			format(string,sizeof(string),"Adding Robbery Interior.~n~Robbery interior would be added~n~");
		}
		case POLICE_INT:
		{
			ShowTextDrawMenu(playerid, MENU_INFO,"~y~Robbery Locations", 10, 0);
			format(string,sizeof(string),"Adding Police Interior.~n~Police interior would be added~n~");
		}
	}
	format(string,sizeof(string),"%sEnter ~p~any name~w~ to enter interior name.~n~Enter ~p~zone~w~ to get outside zone.~n~Enter ~p~out spawn~w~ to get outside spawn.~n~Enter ~p~in spawn~w~ to get inside spawn.~n~Enter ~p~in pickup~w~ to get inside pickup.~n~Enter ~p~out pickup~w~ to get outside pickup.~n~Enter ~p~interior~w~ to get interior.~n~Enter ~p~finish~w~ to save the interior.",string);
	ShowTextDrawMenuItems(playerid, 0, string, " ", " ",0);
	PlayerInfo[playerid][pAddingInterior]=true;
	PlayerInfo[playerid][pAddInteriorType]=Option-1;
	return 1;
}	
AddInterior(playerid,text[])
{
	if(!strcmp(text,"out spawn",true))
	{
		GetPlayerPos(playerid,PlayerInfo[playerid][pAddInteriorOSX],PlayerInfo[playerid][pAddInteriorOSY],PlayerInfo[playerid][pAddInteriorOSZ]);
		GetPlayerFacingAngle(playerid,PlayerInfo[playerid][pAddInteriorOSA]);
		SendClientMessage(playerid,COLOR_ADMIN_INFO,"Outside spawn taken!");
		return 1;
	}
	else if(!strcmp(text,"in spawn",true))
	{
		GetPlayerPos(playerid,PlayerInfo[playerid][pAddInteriorISX],PlayerInfo[playerid][pAddInteriorISY],PlayerInfo[playerid][pAddInteriorISZ]);
		GetPlayerFacingAngle(playerid,PlayerInfo[playerid][pAddInteriorISA]);
		SendClientMessage(playerid,COLOR_ADMIN_INFO,"Inside spawn taken!");
		return 1;
	}
	else if(!strcmp(text,"out pickup",true))
	{
		GetPlayerPos(playerid,PlayerInfo[playerid][pAddInteriorOPX],PlayerInfo[playerid][pAddInteriorOPY],PlayerInfo[playerid][pAddInteriorOPZ]);
		SendClientMessage(playerid,COLOR_ADMIN_INFO,"Outside pikcup and taken!");
		return 1;
	}
	else if(!strcmp(text,"in pickup",true))
	{
		GetPlayerPos(playerid,PlayerInfo[playerid][pAddInteriorIPX],PlayerInfo[playerid][pAddInteriorIPY],PlayerInfo[playerid][pAddInteriorIPZ]);
		SendClientMessage(playerid,COLOR_ADMIN_INFO,"inside pikcup location taken!");
		return 1;
	}
	else if(!strcmp(text,"interior",true))
	{
		PlayerInfo[playerid][pAddInteriorInt]=GetPlayerInterior(playerid);
		SendClientMessage(playerid,COLOR_ADMIN_INFO,"Interior location!");
		return 1;
	}
	else if(!strcmp(text,"zone",true))
	{
		StringReplace(PlayerInfo[playerid][pAddInteriorOutZone],PlayerInfo[playerid][pZoneName],MAX_ZONE_NAME);
		SendClientMessage(playerid,COLOR_ADMIN_INFO,"Outside zone recored!");
		return 1;
	}
	else if(!strcmp(text,"finish",true))
	{
		
		new string[500];
		if(strlen(PlayerInfo[playerid][pAddInteriorOutZone]) > 2) format(string,sizeof(string),"%s (%d) added interior name %s in %s",PlayerInfo[playerid][pUserName],playerid,PlayerInfo[playerid][pAddInteriorZone],PlayerInfo[playerid][pAddInteriorOutZone]);
		else format(string,sizeof(string),"%s (%d) added interior name %s",PlayerInfo[playerid][pUserName],playerid,PlayerInfo[playerid][pAddInteriorZone]);
		SendMessageToManagers(COLOR_ADMIN_INFO,string);
		IRC_SendManagementMessage(string,IRC_ADMIN_COLOR_INFO);
		new Cache:result,rows,vw=INTERIOR_VW;
		mysql_format(g_SQL, string, sizeof(string), "SELECT VirtualWorld FROM Interiors ORDER BY VirtualWorld DESC LIMIT 1");
		result = mysql_query(g_SQL,string);
		cache_get_row_count(rows);
		if(rows)
		{
			cache_get_value_int(0, "VirtualWorld",vw);
			vw++;
		}
		cache_delete(result);
		format(string,sizeof(string),"INSERT INTO Interiors (Type,Interior,VirtualWorld,InteriorName,OutsideZone,OutSpawnX,OutSpawnY,OutSpawnZ,OutSpawnA,InSpawnX,InSpawnY,InSpawnZ,InSpawnA,OutPickupX,OutPickupY,OutPickupZ,InPickupX,InPickupY,InPickupZ)");
		mysql_format(g_SQL, string, sizeof(string), "%s VALUES (%d,%d,%d,'%e','%e',%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f)",string,
		PlayerInfo[playerid][pAddInteriorType],PlayerInfo[playerid][pAddInteriorInt],vw,PlayerInfo[playerid][pAddInteriorZone],PlayerInfo[playerid][pAddInteriorOutZone],PlayerInfo[playerid][pAddInteriorOSX],PlayerInfo[playerid][pAddInteriorOSY],PlayerInfo[playerid][pAddInteriorOSZ],PlayerInfo[playerid][pAddInteriorOSA],PlayerInfo[playerid][pAddInteriorISX],
		PlayerInfo[playerid][pAddInteriorISY],PlayerInfo[playerid][pAddInteriorISZ],PlayerInfo[playerid][pAddInteriorISA],PlayerInfo[playerid][pAddInteriorOPX],PlayerInfo[playerid][pAddInteriorOPY],PlayerInfo[playerid][pAddInteriorOPZ],PlayerInfo[playerid][pAddInteriorIPX],PlayerInfo[playerid][pAddInteriorIPY],PlayerInfo[playerid][pAddInteriorIPZ]);
		mysql_pquery(g_SQL,string, "LoadInterior", "");
		SendClientMessage(playerid,COLOR_ADMIN_INFO,"Interior Added!");
		PlayerInfo[playerid][pAddingInterior]=false;
		strdel(PlayerInfo[playerid][pAddInteriorZone],0,MAX_ZONE_NAME);
		strdel(PlayerInfo[playerid][pAddInteriorOutZone],0,MAX_ZONE_NAME);
		PlayerInfo[playerid][pAddInteriorOSX]=PlayerInfo[playerid][pAddInteriorOSY]=PlayerInfo[playerid][pAddInteriorOSZ]= PlayerInfo[playerid][pAddInteriorOSA]=PlayerInfo[playerid][pAddInteriorISX]=
		PlayerInfo[playerid][pAddInteriorISY]=PlayerInfo[playerid][pAddInteriorISZ]=PlayerInfo[playerid][pAddInteriorISA]=PlayerInfo[playerid][pAddInteriorOPX]=PlayerInfo[playerid][pAddInteriorOPY]=PlayerInfo[playerid][pAddInteriorOPZ]=PlayerInfo[playerid][pAddInteriorIPX]=PlayerInfo[playerid][pAddInteriorIPY]=PlayerInfo[playerid][pAddInteriorIPZ]=0.0;
		PlayerInfo[playerid][pAddInteriorInt]=0;
		return 1;
	}
	else
	{
		StringReplace(PlayerInfo[playerid][pAddInteriorZone],text,MAX_ZONE_NAME);
		SendClientMessage(playerid,COLOR_ADMIN_INFO,"Interior name recorded!");
	}
	return 0;
}


DeleteInterior()
{
	for(new i=0; i<MAX_INTERIORS; i++)
	{
		DestroyDynamicPickup(InteriorInfo[i][InteriorID]);
		DestroyDynamicPickup(ExteriorInfo[i][ExteriorID]);
	}
	return 1;
}

SetupInterior(interior_id)
{
	DestroyDynamicPickup(InteriorInfo[interior_id][InteriorID]);
	DestroyDynamicPickup(ExteriorInfo[interior_id][ExteriorID]);
	DestroyDynamic3DTextLabel(InteriorInfo[interior_id][InteriorLabel]);
	DestroyDynamic3DTextLabel(ExteriorInfo[interior_id][ExteriorLabel]);
	new model,Float:add;
	switch(InteriorInfo[interior_id][InteriorType])
	{
		case SIMPLE_INTERIOR:
		{
			model=19198;
			add=1.0;
		}	
		case REGULAR_ONLY:
		{
			model=19606;
			add =-1.0;
		}	
		case STAFF_ONLY: 
		{
			model=19606;
			add =-1.0;
		}
		case COPS_ONLY:
		{
			model=19606;
			add =-1.0;
		}
		case TELEPORTS: 
		{
			model=19135;
			add =1.0;
		}
		case ROBBERY: 
		{
			model=19605;
			add =-1.0;
		}
		case POLICE_INT: 
		{
			model=19607;
			add =-1.0;
		}
	}
	InteriorInfo[interior_id][InteriorID] = CreateDynamicPickup(model, 1, InteriorInfo[interior_id][InteriorPickupX], InteriorInfo[interior_id][InteriorPickupY], InteriorInfo[interior_id][InteriorPickupZ]+add,0,0,-1);
	ExteriorInfo[interior_id][ExteriorID] = CreateDynamicPickup(19197, 1, ExteriorInfo[interior_id][ExteriorPickupX], ExteriorInfo[interior_id][ExteriorPickupY], ExteriorInfo[interior_id][ExteriorPickupZ]+1.0,InteriorInfo[interior_id][InteriorVW],InteriorInfo[interior_id][InteriorInt],-1);
	if(InteriorInfo[interior_id][InteriorLabels])
	{
		InteriorInfo[interior_id][InteriorLabel] = CreateDynamic3DTextLabel(InteriorInfo[interior_id][InteriorName], 0xFFFFFFFF, InteriorInfo[interior_id][InteriorPickupX], InteriorInfo[interior_id][InteriorPickupY], InteriorInfo[interior_id][InteriorPickupZ],50.0, .worldid=0, .interiorid=0);
		if(strlen(InteriorInfo[interior_id][InteriorOutZone]) > 3)
		{
			ExteriorInfo[interior_id][ExteriorLabel] = CreateDynamic3DTextLabel(InteriorInfo[interior_id][InteriorOutZone], 0xFFFFFFFF, ExteriorInfo[interior_id][ExteriorPickupX], ExteriorInfo[interior_id][ExteriorPickupY], ExteriorInfo[interior_id][ExteriorPickupZ],50.0, .worldid=InteriorInfo[interior_id][InteriorVW], .interiorid=InteriorInfo[interior_id][InteriorInt]);
		}
		else
		{
			ExteriorInfo[interior_id][ExteriorLabel] = CreateDynamic3DTextLabel(GetZoneName(InteriorInfo[interior_id][InteriorPickupX], InteriorInfo[interior_id][InteriorPickupY],ExteriorInfo[interior_id][ExteriorPickupZ]), 0xFFFFFFFF, ExteriorInfo[interior_id][ExteriorPickupX], ExteriorInfo[interior_id][ExteriorPickupY], ExteriorInfo[interior_id][ExteriorPickupZ],50.0, .worldid=InteriorInfo[interior_id][InteriorVW], .interiorid=InteriorInfo[interior_id][InteriorInt]);
		}
	}
	return 1;
}

Interior_PlayerPickup(playerid,pickupid)
{
	for(new i=0; i<MAX_INTERIORS; i++)
	{
		if(pickupid == InteriorInfo[i][InteriorID])
		{
			switch(InteriorInfo[i][InteriorType])
			{
				case TELEPORTS: { PlayerInfo[playerid][pInterior] = 0;}
				default: PlayerInfo[playerid][pInterior] = InteriorInfo[i][InteriorID];
			}
			format(PlayerInfo[playerid][pZoneName],MAX_ZONE_NAME,"%s",InteriorInfo[i][InteriorName]);
			if(strlen(InteriorInfo[i][InteriorOutZone]) > 2)
			{
				format(PlayerInfo[playerid][pExteriorName],MAX_ZONE_NAME,"%s",InteriorInfo[i][InteriorOutZone]);
			}
			else 
			{
				strdel(PlayerInfo[playerid][pExteriorName],0,MAX_ZONE_NAME);
			}
			
			PlayerEnterInterior(playerid);
			SetPlayerVirtualWorld(playerid,InteriorInfo[i][InteriorVW]);
			SetPlayerInterior(playerid,InteriorInfo[i][InteriorInt]);
			SetPlayerPos(playerid,InteriorInfo[i][InteriorSpawnX],InteriorInfo[i][InteriorSpawnY],InteriorInfo[i][InteriorSpawnZ]);
			SetPlayerFacingAngle(playerid,InteriorInfo[i][InteriorSpawnA]);
			SetCameraBehindPlayer(playerid);
			break;
		}
		if(pickupid == ExteriorInfo[i][ExteriorID])
		{
			
			SetPlayerVirtualWorld(playerid,0);
			SetPlayerInterior(playerid,0);
			if(SetPlayerPos(playerid,ExteriorInfo[i][ExteriorSpawnX],ExteriorInfo[i][ExteriorSpawnY],ExteriorInfo[i][ExteriorSpawnZ]))
			{
				PlayerInfo[playerid][pInterior] = 0;
				PlayerEnterInterior(playerid);
			}
			SetPlayerFacingAngle(playerid,ExteriorInfo[i][ExteriorSpawnA]);
			SetCameraBehindPlayer(playerid);
			strdel(PlayerInfo[playerid][pExteriorName],0,MAX_ZONE_NAME);
			strdel(PlayerInfo[playerid][pZoneName],0,MAX_ZONE_NAME);
			format(PlayerInfo[playerid][pZoneName],MAX_ZONE_NAME,"%s",InteriorInfo[i][InteriorName]);
			break;
		}
	}
	return 1;
}

CMD:addinterior(playerid)
{
	if(PlayerInfo[playerid][pRank] < SERVER_MANAGER) return SendClientMessage(playerid,COLOR_ERROR,ERROR_NOT_ADMIN);
	if(PlayerInfo[playerid][pEditingInterior]) return SendClientMessage(playerid,COLOR_ERROR,"You can not add interior while editing one.");
	if(PlayerInfo[playerid][pAddingInterior])
	{
		OnAddInteriorMenu(playerid,PlayerInfo[playerid][pAddInteriorType]+1);
	}
	else
	{
		ShowTextDrawMenu(playerid, MENU_ADD_INT_TYPE,"~y~Interior Types", 8, 7);
		ShowTextDrawMenuItems(playerid, 0, "Select ~p~interior~w~ type", " ", " ",0);
		ShowTextDrawMenuItems(playerid, 2, " ", "~b~~h~1 ~w~- Simple Interior", " ",0);
		ShowTextDrawMenuItems(playerid, 3, " ", "~b~~h~2 ~w~- Regular Players", " ",0);
		ShowTextDrawMenuItems(playerid, 4, " ", "~b~~h~3 ~w~- Staff Only", " ",0);
		ShowTextDrawMenuItems(playerid, 5, " ", "~b~~h~4 ~w~- Cops Only", " ",0);
		ShowTextDrawMenuItems(playerid, 6, " ", "~b~~h~5 ~w~- Teleports", " ",0);
		ShowTextDrawMenuItems(playerid, 7, " ", "~b~~h~6 ~w~- Robbery", " ",0);
		ShowTextDrawMenuItems(playerid, 8, " ", "~b~~h~7 ~w~- Police", " ",0);
	}
	return 1;
}

CMD:stopadding(playerid)
{
	if(PlayerInfo[playerid][pRank] < SERVER_MANAGER) return SendClientMessage(playerid,COLOR_ERROR,ERROR_NOT_ADMIN);
	if(PlayerInfo[playerid][pAddingInterior])
	{
		PlayerInfo[playerid][pAddingInterior]=false;
		SendClientMessage(playerid,COLOR_ADMIN_INFO,"Interior adding exited.");
	}
	else SendClientMessage(playerid,COLOR_ADMIN_INFO,"You are not adding interior.");
	return 1;
}

CMD:editinteriors(playerid,params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_MANAGER) return SendClientMessage(playerid,COLOR_ERROR,ERROR_NOT_ADMIN);
	if(PlayerInfo[playerid][pAddingInterior]) return SendClientMessage(playerid,COLOR_ERROR,"You can not edit interior while adding one.");
	if(PlayerInfo[playerid][pEditingInterior])
	{
		PlayerInfo[playerid][pEditingInterior]=false;
		HideTextDrawMenu(playerid);
		SendClientMessage(playerid,COLOR_ADMIN_INFO,"Stoped editing interior");
	}
	else
	{
		HideTextDrawMenu(playerid);
		new var;
		if(sscanf(params,"d",var)) var=0;
		new string[75];
		new rows,Cache:result;
		mysql_format(g_SQL, string, sizeof(string), "SELECT ID,InteriorName,OutsideZone  FROM Interiors LIMIT %d , 20",var);
		result = mysql_query(g_SQL,string);
		cache_get_row_count(rows);
		if(rows)
		{
			ShowTextDrawMenu(playerid, MENU_EDIT_INTERIORS_LIST,"~y~Interiors", rows+2, rows+1);
			ShowTextDrawMenuItems(playerid, 0, "Select ~p~interior~w~ to edit", " ", " ",0);
			new id;
			for(new i=0; i<rows; i++)
			{
				new name[MAX_ZONE_NAME],in[MAX_ZONE_NAME];
				cache_get_value_name(i,"InteriorName",name);
				cache_get_value_name(i,"OutsideZone",in);
				cache_get_value_int(i,"ID",id);
				if(strlen(in) > 3)format(string,sizeof(string),"~b~~h~%d ~w~- %s ~y~in ~p~%s",i+1,name,in);
				else format(string,sizeof(string),"~b~~h~%d ~w~- %s",i+1,name);
				ShowTextDrawMenuItems(playerid, i+2, " ", string, " ",id);
			}
			ShowTextDrawMenuItems(playerid, rows+2, " ", "~b~~h~21 ~g~~h~- Next page", " ",id);

		}
		else SendClientMessage(playerid,COLOR_ERROR,"No interiors found after that ID.");
		cache_delete(result);
	}
	return 1;
}