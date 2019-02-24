/*

	Actors Script
	Scripted by Arose Niazi.
	Scripting started on 30th October 2017
	Last Updated: -----
	
	Credits:
		
	Functions:
		LoadActors();
		LoadActorsID(actorid);
		OnEditActorsList(playerid,Option,ID);
		OnActorSettingChange(playerid,Option);
		UpdateActorSettings(playerid, actorid, option);
		ActorUpdated(playerid,actorid);
		OnAddActorMenu(playerid,Option);
		AddNewActor(playerid,text[]);
		LoadActorsOnAdd();
		DeleteActor(playerid,actor_id);
		DeleteAllActors();
		SetupActor(actor_id);
		/addactor
		/stopaddingactor
		/editactors
		
	Functions Being Called:
		
*/

#define MAX_ACTORS_USED 200

enum atn_Enum
{
	TName[20],
	TJobName[25],
	TMenuInfo[40]
};

new ActorTypeName[][atn_Enum] = 
{
	{"Bank","Accountant", "Bank sale menu"},
	{"Casino","Casino Manager", "Casino menu"},
	{"City Hall","City Manager", "City Hall menu"},
	{"Hospital","Medical Worker", "Hospital menu"},
	{"24-7","Store Clerk", "24-7 menu"},
	{"Sex Shop","Sex Clerk", "sex shop menu"},
	{"Bar","Bartender", "bar menu"},
	{"Pizza Store","Pizza Vendor", "Well Stacked Pizza Co. menu"},
	{"Burger Store","Burger Vendor", "Burger Shot menu"},
	{"Chicken Store","Chicken Consieurr", "Clukin' Bell menu"},
	{"Vehicles","Car/Bike Dealers", "car/bike sale menu"},
	{"Boats","Boat Dealers", "boat sale menu"},
	{"Aircrafts","Aircraft Dealers", "plane/helicopter sale menu"},
	{"Aiport","Flight Scheduler", "Airport menu"},
	{"RPC Bar","RPC Bartender", "regular player club menu"},
	{"Curch","Priest","Curch Menu"},
	{"Fish","Fisherman","Fisherman Menu"},
	{"Ammunation","Weapon Dealer","Ammunation Menu"},
	{"PD","Police Clerk","Ploice Department menu"},
	{"Sheriff","Sheriff Helper","Sheriff Office menu"},
	{"FBI","FBI Clerk","FBI Office Menu"},
	{"Pet","Pet Store Clerk","Pet shop Menu"},
	{"Xoomer","Mechanic","Xoomer Menu"},
	{"Hippie","Hippie Store Clerk","hippie shop menu"},
	{"Drugs","Drug Dealer","drug dealer menu"},
	{"Barbar","Barbar","hair/wigs menu"}
};

enum Actor_enum
{
	ActorAID,
	ActorID,
	ActorType,
	ActorLocation[MAX_ZONE_NAME],
	Float:ActorSpawnX,
	Float:ActorSpawnY,
	Float:ActorSpawnZ,
	Float:ActorSpawnA,
	ActorSkin,
	ActorInt,
	ActorVW,
	ActorAnimationType,
	bool:ActorLabelEnable,
	Text3D:ActorLabel
};

new ActorInfo[MAX_ACTORS_USED][Actor_enum];
FUNCTION LoadActors()
{
	for(new i=0; i<MAX_ACTORS_USED; i++)
	{
		ActorInfo[i][ActorAID]=INVALID_ACTOR_ID;
	}
	new string[128];
	new rows,Cache:result;
	mysql_format(g_SQL, string, sizeof(string), "SELECT * FROM Actors");
	result = mysql_query(g_SQL,string);
	cache_get_row_count(rows);
	if(rows)
	{
		if(rows > MAX_ACTORS_USED) rows=MAX_ACTORS_USED;
		for(new i=0; i<rows; i++)
		{
			cache_get_value_int(i, "ID",ActorInfo[i][ActorID]);
			cache_get_value_int(i, "Type",ActorInfo[i][ActorType]);
			cache_get_value_int(i, "Interior",ActorInfo[i][ActorInt]);
			cache_get_value_int(i, "VirtualWorld",ActorInfo[i][ActorVW]);
			cache_get_value_float(i, "SpawnX",ActorInfo[i][ActorSpawnX]);
			cache_get_value_float(i, "SpawnY",ActorInfo[i][ActorSpawnY]);
			cache_get_value_float(i, "SpawnZ",ActorInfo[i][ActorSpawnZ]);
			cache_get_value_float(i, "SpawnA",ActorInfo[i][ActorSpawnA]);
			cache_get_value_int(i, "Skin",ActorInfo[i][ActorSkin]);
			cache_get_value_name(i, "Loaction",ActorInfo[i][ActorLocation],MAX_ZONE_NAME);
			cache_get_value_int(i, "AnimationType",ActorInfo[i][ActorAnimationType]);
			cache_get_value_bool(i, "3DLabel",ActorInfo[i][ActorLabelEnable]);
			SetupActor(i);
		}
	}
	cache_delete(result);
	return 1;
}

FUNCTION LoadActorsID(actorid)
{
	new string[128];
	new rows,Cache:result;
	mysql_format(g_SQL, string, sizeof(string), "SELECT * FROM Actors WHERE ID=%d",actorid);
	result = mysql_query(g_SQL,string);
	cache_get_row_count(rows);
	if(rows)
	{
		new actor_id=INVALID_ACTOR_ID;
		for(new i=0; i<MAX_ACTORS_USED; i++)
		{
			if(ActorInfo[i][ActorID] == actorid && ActorInfo[i][ActorAID] != INVALID_ACTOR_ID)
			{
				actor_id=i;
				break;
			}
		}
		if(actor_id == INVALID_ACTOR_ID)
		{
			for(new i=0; i<MAX_ACTORS_USED; i++)
			{
				if(ActorInfo[i][ActorAID] == INVALID_ACTOR_ID)
				{
					actor_id=i;
					break;
				}
			}
		}	
		if(actor_id == INVALID_ACTOR_ID)
		{
			printf("Max actors reacted ignoring next actor.");
			return 1;
		}
		ActorInfo[actor_id][ActorID]=actorid;
		cache_get_value_int(0, "Type",ActorInfo[actor_id][ActorType]);
		cache_get_value_int(0, "Interior",ActorInfo[actor_id][ActorInt]);
		cache_get_value_int(0, "VirtualWorld",ActorInfo[actor_id][ActorVW]);
		cache_get_value_float(0, "SpawnX",ActorInfo[actor_id][ActorSpawnX]);
		cache_get_value_float(0, "SpawnY",ActorInfo[actor_id][ActorSpawnY]);
		cache_get_value_float(0, "SpawnZ",ActorInfo[actor_id][ActorSpawnZ]);
		cache_get_value_float(0, "SpawnA",ActorInfo[actor_id][ActorSpawnA]);
		cache_get_value_int(0, "Skin",ActorInfo[actor_id][ActorSkin]);
		cache_get_value_name(0, "Loaction",ActorInfo[actor_id][ActorLocation],MAX_ZONE_NAME);
		cache_get_value_int(0, "AnimationType",ActorInfo[actor_id][ActorAnimationType]);
		cache_get_value_bool(0, "3DLabel",ActorInfo[actor_id][ActorLabelEnable]);
		SetupActor(actor_id);
	}
	cache_delete(result);
	return 1;
}


FUNCTION OnEditActorsList(playerid,Option,ID)
{
	new string[100];
	new rows,Cache:result;
	switch (Option)
	{
		case 21:
		{
			format(string,sizeof(string),"%d",ID);
			callcmd::editactors(playerid,string);
		}
		default:
		{
			HideTextDrawMenu(playerid);
			mysql_format(g_SQL, string, sizeof(string), "SELECT * FROM Actors WHERE ID = %d",ID);
			result = mysql_query(g_SQL,string);
			cache_get_row_count(rows);
			if(rows)
			{
				cache_get_value_int(0, "Type",PlayerInfo[playerid][pAddActorType]);
				cache_get_value_int(0, "Interior",PlayerInfo[playerid][pAddActorInt]);
				cache_get_value_int(0, "VirtualWorld",PlayerInfo[playerid][pAddActorVW]);
				cache_get_value_float(0, "SpawnX",PlayerInfo[playerid][pAddActorSpawnX]);
				cache_get_value_float(0, "SpawnY",PlayerInfo[playerid][pAddActorSpawnY]);
				cache_get_value_float(0, "SpawnZ",PlayerInfo[playerid][pAddActorSpawnZ]);
				cache_get_value_float(0, "SpawnA",PlayerInfo[playerid][pAddActorSpawnA]);
				cache_get_value_int(0, "Skin",PlayerInfo[playerid][pAddActorSkin]);
				cache_get_value_name(0, "Loaction",PlayerInfo[playerid][pAddActorLocation],MAX_ZONE_NAME);
				cache_get_value_int(0, "AnimationType",PlayerInfo[playerid][pAddActorAnimationType]);
				cache_get_value_bool(0, "3DLabel",PlayerInfo[playerid][pAddActorLabelEnable]);
				PlayerInfo[playerid][pEditingActor]=true;
				PlayerInfo[playerid][pEditingActorID]=ID;
				ShowTextDrawMenu(playerid, MENU_EDIT_ACTOR_SETTING,"~y~Actor Settings", 8, 7);
		
				format(string,sizeof(string),"Editing ~p~%s ~w~in~p~ %s",ActorTypeName[PlayerInfo[playerid][pAddActorType]][TJobName],PlayerInfo[playerid][pAddActorLocation]);
				ShowTextDrawMenuItems(playerid, 0, string, " ", " ",0);
				format(string,sizeof(string),"~b~~h~1 ~w~- %s",PlayerInfo[playerid][pAddActorLocation]);
				ShowTextDrawMenuItems(playerid, 2, " ", string, " ",0);
				format(string,sizeof(string),"~b~%d",PlayerInfo[playerid][pAddActorInt]);
				ShowTextDrawMenuItems(playerid, 3, " ", "~b~~h~2 ~w~- Interior ID", string,0);
				format(string,sizeof(string),"~b~%d",PlayerInfo[playerid][pAddActorVW]);
				ShowTextDrawMenuItems(playerid, 4, " ", "~b~~h~3 ~w~- Virtual World ID:", string,0);
				format(string,sizeof(string),"~b~%d",PlayerInfo[playerid][pAddActorSkin]);
				ShowTextDrawMenuItems(playerid, 5, " ", "~b~~h~4 ~w~- Skin", string,0);
				format(string,sizeof(string),"~b~%s",ActorTypeName[PlayerInfo[playerid][pAddActorType]][TName]);
				ShowTextDrawMenuItems(playerid, 6, " ", "~b~~h~5 ~w~- Actor Type", " ",0);
				format(string,sizeof(string),"~b~%s",(PlayerInfo[playerid][pAddActorLabelEnable])? ("~b~Enabled"):("~b~Disabled"));
				ShowTextDrawMenuItems(playerid, 7, " ", "~b~~h~6 ~w~- Label", string,0);
				ShowTextDrawMenuItems(playerid, 8, " ", "~b~~h~7 ~w~- Delete Actor", " ",0);
			}
			cache_delete(result);	
		}
	}
	return 1;
}

FUNCTION OnActorSettingChange(playerid,Option)
{
	new string[125];
	switch (Option)
	{
		case 1:
		{
			format(PlayerInfo[playerid][pAddActorLocation],MAX_ZONE_NAME,"%s",PlayerInfo[playerid][pZoneName]);
			SendClientMessage(playerid,COLOR_ADMIN_INFO,"Actor Loaction updated.");
			UpdateActorSettings(playerid,PlayerInfo[playerid][pEditingActorID],Option);
			format(string,sizeof(string),"%s (%d) updated location name to %s for actor id '%d'",PlayerInfo[playerid][pUserName],playerid,PlayerInfo[playerid][pAddActorLocation],PlayerInfo[playerid][pEditingActorID]);
		}
		case 2:
		{
			PlayerInfo[playerid][pAddActorInt]=GetPlayerInterior(playerid);
			SendClientMessage(playerid,COLOR_ADMIN_INFO,"Interior Updated.");
			UpdateActorSettings(playerid,PlayerInfo[playerid][pEditingActorID],Option);
			format(string,sizeof(string),"%s (%d) updated interior id to %d for actor id '%d'",PlayerInfo[playerid][pUserName],playerid,PlayerInfo[playerid][pAddActorInt],PlayerInfo[playerid][pEditingActorID]);
		}
		case 3:
		{
			PlayerInfo[playerid][pAddActorVW]=GetPlayerVirtualWorld(playerid);
			SendClientMessage(playerid,COLOR_ADMIN_INFO,"Virtual World Updated.");
			UpdateActorSettings(playerid,PlayerInfo[playerid][pEditingActorID],Option);
			format(string,sizeof(string),"%s (%d) updated virtual world id to %d for actor id '%d'",PlayerInfo[playerid][pUserName],playerid,PlayerInfo[playerid][pAddActorVW],PlayerInfo[playerid][pEditingActorID]);
		}
		case 4:
		{
			PlayerInfo[playerid][pAddActorSkin]=GetPlayerSkin(playerid);
			SendClientMessage(playerid,COLOR_ADMIN_INFO,"skin updated.");
			UpdateActorSettings(playerid,PlayerInfo[playerid][pEditingActorID],Option);
			format(string,sizeof(string),"%s (%d) updated skin id to %d for actor id '%d'",PlayerInfo[playerid][pUserName],playerid,PlayerInfo[playerid][pAddActorSkin],PlayerInfo[playerid][pEditingActorID]);
		}
		case 5:
		{
			callcmd::addactor(playerid);
		}
		case 6:
		{
			PlayerInfo[playerid][pAddActorLabelEnable]=!PlayerInfo[playerid][pAddActorLabelEnable];
			SendClientMessage(playerid,COLOR_ADMIN_INFO,"Label updated.");
			UpdateActorSettings(playerid,PlayerInfo[playerid][pEditingActorID],Option);
			format(string,sizeof(string),"%s (%d) set label to %s for actor id '%d'",PlayerInfo[playerid][pUserName],playerid,(PlayerInfo[playerid][pAddActorLabelEnable])? ("Enabled"):("Disabled"),PlayerInfo[playerid][pEditingActorID]);
		}
		case 7:
		{
			DeleteActor(playerid,PlayerInfo[playerid][pEditingActorID]);
		}
	}
	if(Option != 5 && Option != 7)
	{
		SendMessageToManagers(COLOR_ADMIN_INFO,string);
		IRC_SendManagementMessage(string,IRC_ADMIN_COLOR_INFO);
	}
	return 1;
}

UpdateActorSettings(playerid, actorid, option)
{
	new string[256];
	switch(option)
	{	
		case 1: mysql_format(g_SQL,string,sizeof(string),"UPDATE Actors SET Loaction='%e' WHERE ID=%d",PlayerInfo[playerid][pAddActorLocation],actorid);
		case 2: mysql_format(g_SQL,string,sizeof(string),"UPDATE Actors SET Interior=%d WHERE ID=%d",PlayerInfo[playerid][pAddActorInt],actorid); 
		case 3: mysql_format(g_SQL,string,sizeof(string),"UPDATE Actors SET VirtualWorld=%d WHERE ID=%d",PlayerInfo[playerid][pAddActorVW],actorid);
		case 4: mysql_format(g_SQL,string,sizeof(string),"UPDATE Actors SET Skin=%d WHERE ID=%d",PlayerInfo[playerid][pAddActorSkin],actorid);
		case 5: mysql_format(g_SQL,string,sizeof(string),"UPDATE Actors SET Type=%d,AnimationType=%d WHERE ID=%d",PlayerInfo[playerid][pAddActorType],PlayerInfo[playerid][pAddActorAnimationType],actorid);
		case 6: mysql_format(g_SQL,string,sizeof(string),"UPDATE Actors SET 3DLabel=%d WHERE ID=%d",PlayerInfo[playerid][pAddActorLabelEnable],actorid);
	}
	mysql_pquery(g_SQL,string,"ActorUpdated","dd",playerid,actorid);
	return 1;
}

FUNCTION ActorUpdated(playerid,actorid)
{
	LoadActorsID(actorid);
	if(PlayerInfo[playerid][pEditingActor])
	{
		OnEditActorsList(playerid,5,PlayerInfo[playerid][pEditingActorID]);
	}
	return 1;
}

OnAddActorMenu(playerid,Option)
{
	new string[300];
	if(Option == 21)
	{
		ShowTextDrawMenu(playerid, MENU_ADD_ACTOR_TYPE_2,"~y~Actor Types", 8, 7);
		ShowTextDrawMenuItems(playerid, 0, "Select ~p~Actor~w~ type", " ", " ",0);
		for(new i=1; i<=6; i++)
		{
			format(string,sizeof(string),"~b~~h~%d ~w~- %s Actor",i,ActorTypeName[i-1+20][TName]);
			ShowTextDrawMenuItems(playerid, i+1, " ", string, " ",0);
		}
		ShowTextDrawMenuItems(playerid, 8, " ", "~b~~h~6 ~g~~h~- Previous page", " ",0);
		return 1;
	}
	if(Option == 26)
	{
		callcmd::addactor(playerid);
	}
	PlayerInfo[playerid][pAddActorType]=Option-1;
	if(PlayerInfo[playerid][pEditingActor])
	{
		SendClientMessage(playerid,COLOR_ADMIN_INFO,"Type updated.");
		PlayerInfo[playerid][pAddActorType]=Option-1;
		UpdateActorSettings(playerid,PlayerInfo[playerid][pEditingActorID],5);
		format(string,sizeof(string),"%s (%d) updated actor type to %s for actor id '%d'",PlayerInfo[playerid][pUserName],playerid,ActorTypeName[PlayerInfo[playerid][pAddActorType]][TName],PlayerInfo[playerid][pEditingActorID]);
		SendMessageToManagers(COLOR_ADMIN_INFO,string);
		IRC_SendManagementMessage(string,IRC_ADMIN_COLOR_INFO);
	}
	else
	{
		format(string,sizeof(string),"~y~%s Actor",ActorTypeName[Option-1][TName]);
		ShowTextDrawMenu(playerid, MENU_INFO,string, 5, 0);
		format(string,sizeof(string),"Adding ~p~%s",ActorTypeName[Option-1][TJobName]);
		format(string,sizeof(string),"%s~n~~w~Enter ~p~zone~w~ to get location name.~n~Enter ~p~skin~w~ to get skin.~n~Enter ~p~spawn~w~ to get spawn.~n~Enter ~p~finish~w~ to save the actor.",string);
		ShowTextDrawMenuItems(playerid, 0, string, " ", " ",0);
		PlayerInfo[playerid][pAddingActor]=true;
	}	
	return 1;
}	
AddNewActor(playerid,text[])
{
	if(!strcmp(text,"spawn",true))
	{
		GetPlayerPos(playerid,PlayerInfo[playerid][pAddActorSpawnX],PlayerInfo[playerid][pAddActorSpawnY],PlayerInfo[playerid][pAddActorSpawnZ]);
		GetPlayerFacingAngle(playerid,PlayerInfo[playerid][pAddActorSpawnA]);
		PlayerInfo[playerid][pAddActorInt]=GetPlayerInterior(playerid);
		PlayerInfo[playerid][pAddActorVW]=GetPlayerVirtualWorld(playerid);
		SendClientMessage(playerid,COLOR_ADMIN_INFO,"spawn, interior and virtual world taken!");
		return 1;
	}
	if(!strcmp(text,"skin",true))
	{
		PlayerInfo[playerid][pAddActorSkin]=GetPlayerSkin(playerid);
		SendClientMessage(playerid,COLOR_ADMIN_INFO,"Skin taken!");
		return 1;
	}
	else if(!strcmp(text,"zone",true))
	{
		StringReplace(PlayerInfo[playerid][pAddActorLocation],PlayerInfo[playerid][pZoneName],MAX_ZONE_NAME);
		SendClientMessage(playerid,COLOR_ADMIN_INFO,"Location zone recored!");
		return 1;
	}
	else if(!strcmp(text,"finish",true))
	{
		
		new string[500];
		format(string,sizeof(string),"%s (%d) added actor %s in %s",PlayerInfo[playerid][pUserName],playerid,ActorTypeName[PlayerInfo[playerid][pAddActorType]][TJobName],PlayerInfo[playerid][pAddActorLocation]);
		SendMessageToManagers(COLOR_ADMIN_INFO,string);
		IRC_SendManagementMessage(string,IRC_ADMIN_COLOR_INFO);
		format(string,sizeof(string),"INSERT INTO Actors (Type,Loaction,VirtualWorld,Interior,SpawnX,SpawnY,SpawnZ,SpawnA,AnimationType,Skin)");
		mysql_format(g_SQL, string, sizeof(string), "%s VALUES (%d,'%e',%d,%d,%f,%f,%f,%f,%d,%d)",string,
		PlayerInfo[playerid][pAddActorType],PlayerInfo[playerid][pAddActorLocation],PlayerInfo[playerid][pAddActorVW],PlayerInfo[playerid][pAddActorInt],PlayerInfo[playerid][pAddActorSpawnX],PlayerInfo[playerid][pAddActorSpawnY],PlayerInfo[playerid][pAddActorSpawnZ],PlayerInfo[playerid][pAddActorSpawnA],PlayerInfo[playerid][pAddActorAnimationType],PlayerInfo[playerid][pAddActorSkin]);
		mysql_pquery(g_SQL,string, "LoadActorsOnAdd", "");
		SendClientMessage(playerid,COLOR_ADMIN_INFO,"Actor Added!");
		PlayerInfo[playerid][pAddingActor]=false;
		strdel(PlayerInfo[playerid][pAddActorLocation],0,MAX_ZONE_NAME);
		PlayerInfo[playerid][pAddActorSpawnX]=PlayerInfo[playerid][pAddActorSpawnY]=PlayerInfo[playerid][pAddActorSpawnZ]= PlayerInfo[playerid][pAddActorSpawnA]=0.0;
		PlayerInfo[playerid][pAddActorInt]=PlayerInfo[playerid][pAddActorVW]=PlayerInfo[playerid][pAddActorSkin]=PlayerInfo[playerid][pAddActorAnimationType]=0;
		return 1;
	}
	return 0;
}

FUNCTION LoadActorsOnAdd()
{
	LoadActorsID(cache_insert_id());
	return 1;
}

DeleteActor(playerid,actor_id)
{
	new string[128];
	mysql_format(g_SQL, string, sizeof(string), "DELETE FROM Actors WHERE ID=%d",actor_id);
	mysql_pquery(g_SQL,string);
	for(new i=0; i<MAX_ACTORS_USED; i++)
	{
		if(ActorInfo[i][ActorID] == actor_id && ActorInfo[i][ActorAID] != INVALID_ACTOR_ID)
		{
			DestroyActor(ActorInfo[i][ActorAID]);
			ActorInfo[i][ActorAID] = INVALID_ACTOR_ID;
			if(ActorInfo[i][ActorLabelEnable]) DestroyDynamic3DTextLabel(ActorInfo[i][ActorLabel]);
		}
	}
	format(string,sizeof(string),"%s (%d) removed actor %s at %s actor id '%d'",PlayerInfo[playerid][pUserName],playerid,ActorTypeName[PlayerInfo[playerid][pAddActorType]][TJobName],PlayerInfo[playerid][pAddActorLocation],actor_id);
	SendMessageToManagers(COLOR_ADMIN_INFO,string);
	IRC_SendManagementMessage(string,IRC_ADMIN_COLOR_INFO);
	PlayerInfo[playerid][pAddingActor]=false;
	SendClientMessage(playerid,COLOR_ADMIN_INFO,"Actor deleted.");
	return 1;
}

DeleteAllActors()
{
	for(new i=0,j=GetActorPoolSize(); i<=j; i++)
	{
		if(ActorInfo[i][ActorAID] == INVALID_ACTOR_ID) continue;
		DestroyActor(ActorInfo[i][ActorAID]);
		ActorInfo[i][ActorAID] = INVALID_ACTOR_ID;
		if(ActorInfo[i][ActorLabelEnable]) DestroyDynamic3DTextLabel(ActorInfo[i][ActorLabel]);
	}
	return 1;
}

SetupActor(actor_id)
{
	if(ActorInfo[actor_id][ActorAID] != INVALID_ACTOR_ID)
	{
		DestroyActor(ActorInfo[actor_id][ActorAID]);
		ActorInfo[actor_id][ActorAID]=INVALID_ACTOR_ID;
		DestroyDynamic3DTextLabel(ActorInfo[actor_id][ActorLabel]);
	}
	ActorInfo[actor_id][ActorAID] = CreateActor(ActorInfo[actor_id][ActorSkin], ActorInfo[actor_id][ActorSpawnX], ActorInfo[actor_id][ActorSpawnY],ActorInfo[actor_id][ActorSpawnZ],ActorInfo[actor_id][ActorSpawnA]);
	SetActorVirtualWorld(ActorInfo[actor_id][ActorAID],ActorInfo[actor_id][ActorVW]);
	if(ActorInfo[actor_id][ActorLabelEnable])
	{
		new string[100];
		format(string,sizeof(string),"Press '"COL_SERVER_INFO"~k~~CONVERSATION_YES~"COL_WHITE"' for %s",ActorTypeName[ActorInfo[actor_id][ActorType]][TMenuInfo]);
		ActorInfo[actor_id][ActorLabel] = CreateDynamic3DTextLabel(string, 0xFFFFFFFF, ActorInfo[actor_id][ActorSpawnX], ActorInfo[actor_id][ActorSpawnY],ActorInfo[actor_id][ActorSpawnZ],15.0, .worldid=ActorInfo[actor_id][ActorVW], .interiorid=ActorInfo[actor_id][ActorInt]);
	}
	return 1;
}

CMD:addactor(playerid)
{
	if(PlayerInfo[playerid][pRank] < SERVER_MANAGER) return SendClientMessage(playerid,COLOR_ERROR,ERROR_NOT_ADMIN);
	if(PlayerInfo[playerid][pEditingInterior]) return SendClientMessage(playerid,COLOR_ERROR,"You can not add actor while editing interior.");
	if(PlayerInfo[playerid][pAddingInterior]) return SendClientMessage(playerid,COLOR_ERROR,"You can not add actor while adding interior.");
	//if(PlayerInfo[playerid][pEditingActor])  return SendClientMessage(playerid,COLOR_ERROR,"You can not add actor while editing one.");
	if(PlayerInfo[playerid][pAddingActor])
	{
		OnAddActorMenu(playerid,PlayerInfo[playerid][pAddActorType]+1);
	}
	else
	{
		new string[75];
		ShowTextDrawMenu(playerid, MENU_ADD_ACTOR_TYPE,"~y~Actor Types", 22, 21);
		ShowTextDrawMenuItems(playerid, 0, "Select ~p~Actor~w~ type", " ", " ",0);
		for(new i=1; i<=20; i++)
		{
			format(string,sizeof(string),"~b~~h~%d ~w~- %s Actor",i,ActorTypeName[i-1][TName]);
			ShowTextDrawMenuItems(playerid, i+1, " ", string, " ",0);
		}
		ShowTextDrawMenuItems(playerid, 22, " ", "~b~~h~21 ~g~~h~- Next page", " ",0);
	}
	return 1;
}

CMD:stopaddingactor(playerid)
{
	if(PlayerInfo[playerid][pRank] < SERVER_MANAGER) return SendClientMessage(playerid,COLOR_ERROR,ERROR_NOT_ADMIN);
	if(PlayerInfo[playerid][pAddingActor])
	{
		PlayerInfo[playerid][pAddingActor]=false;
		SendClientMessage(playerid,COLOR_ADMIN_INFO,"Actor adding stopped.");
	}
	else SendClientMessage(playerid,COLOR_ADMIN_INFO,"You are not adding actor.");
	return 1;
}

CMD:editactors(playerid,params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_MANAGER) return SendClientMessage(playerid,COLOR_ERROR,ERROR_NOT_ADMIN);
	if(PlayerInfo[playerid][pEditingInterior]) return SendClientMessage(playerid,COLOR_ERROR,"You can not edit actor while editing interior.");
	if(PlayerInfo[playerid][pAddingInterior]) return SendClientMessage(playerid,COLOR_ERROR,"You can not edit actor while adding interior.");
	if(PlayerInfo[playerid][pAddingActor])  return SendClientMessage(playerid,COLOR_ERROR,"You can not edit actor while adding one.");
	if(PlayerInfo[playerid][pEditingActor])
	{
		PlayerInfo[playerid][pEditingActor]=false;
		HideTextDrawMenu(playerid);
		SendClientMessage(playerid,COLOR_ADMIN_INFO,"Stoped editing actor");
	}
	else
	{
		HideTextDrawMenu(playerid);
		new var;
		if(sscanf(params,"d",var)) var=0;
		new string[75];
		new rows,Cache:result;
		mysql_format(g_SQL, string, sizeof(string), "SELECT ID,Loaction,Type FROM Actors LIMIT %d , 20",var);
		result = mysql_query(g_SQL,string);
		cache_get_row_count(rows);
		if(rows)
		{
			ShowTextDrawMenu(playerid, MENU_EDIT_ACTORS_LIST,"~y~Actors", rows+2, rows+1);
			ShowTextDrawMenuItems(playerid, 0, "Select ~p~actor~w~ to edit", " ", " ",0);
			new id;
			for(new i=0; i<rows; i++)
			{
				new type,in[MAX_ZONE_NAME];
				cache_get_value_int(i,"Type",type);
				cache_get_value_name(i,"Loaction",in);
				cache_get_value_int(i,"ID",id);
				format(string,sizeof(string),"~b~~h~%d ~w~- %s ~y~in ~p~%s",i+1,ActorTypeName[type][TJobName],in);
				ShowTextDrawMenuItems(playerid, i+2, " ", string, " ",id);
			}
			ShowTextDrawMenuItems(playerid, rows+2, " ", "~b~~h~21 ~g~~h~- Next page", " ",id);

		}
		else SendClientMessage(playerid,COLOR_ERROR,"No Actors found after that ID.");
		cache_delete(result);
	}
	return 1;
}