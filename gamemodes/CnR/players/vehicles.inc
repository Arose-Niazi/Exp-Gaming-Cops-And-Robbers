/*

	Vehucles Script
	Scripted by Sasuke_Uchiha.
	Scripting started on 19th October 2017
	Last Updated: -----
	
	Credits:
		
	Functions:
		CreateVeh(type,modelid,Float:X,Float:Y,Float:Z,Float:A,ColorOne,ColorTwo,bool:pstatic=true);
		AddVehicles();
		GetLowRider();
		GetCovertable();
		GetIndusterial();
		GetJeep();
		GetSaloon();
		GetWagoon();
		
	Functions Being Called:
		
*/
enum cInfo {
	cID,
	cType,
	cModel, 
	Float:cLocationX,
	Float:cLocationY,
	Float:cLocationZ,
	Float:cLocationA,
	cLocationI,
	cLocationW,
	cColorOne,
	cColorTwo,
	cOwned,
	cLock,
	cStaticVehicle,
	cDriverKitInstalled,
	cCopKitInstalled
};
new CarInfo[MAX_VEHICLES][cInfo];

new CurrentVehicleId = 0;

new LowRiders[]= {412,534,535,536,566,567,575,576};
new Wagoons[] = {404,418,458,479,561};
new Saloons[] = {401,405,410,419,421,426,436,445,466,467,474,491,492,504,507,516,517,518,526,527,529,540,542,546,547,549,550,551,560,562,580,585};
new Jeeps[] = {579,505,500,489,424,400};
new Covertables[] = {439,480,533,555};
new Industrial[] = {600,554,552,543,482,478,440,442,413};
new Police[] = {427,432,433,490,523,528,596,597,598,599,601,472,430};
new PublicService[] = {406,417,420,431,437,438,544};

AddVehicles()
{
	for(new i=0; i<MAX_VEHICLES; i++) CarInfo[i][cID]=-1;
	
	new string[75],rows,Cache:result;
	mysql_format(g_SQL,string,sizeof(string),"SELECT * FROM "TABLE_VEHICLES"");
	result=mysql_query(g_SQL,string);
	cache_get_row_count(rows);
	if(rows)
	{
		for(new i=0; i<rows; i++)
		{
			new id,type,Model,Float:x,Float:y,Float:z,Float:a,col1,col2;
			cache_get_value_int(i,"ID",id);
			cache_get_value_int(i,"Type",type);
			cache_get_value_int(i,"Model",Model);
			cache_get_value_float(i,"X",x);
			cache_get_value_float(i,"Y",y);
			cache_get_value_float(i,"Z",z);
			cache_get_value_float(i,"A",a);
			cache_get_value_int(i,"COL1",col1);
			cache_get_value_int(i,"COL2",col2);
			CreateStaticVehicle(id,type,Model,x,y,z,a,col1,col2);
		}	
	}
	cache_delete(result);
	return 1;
}

GetLowRider()
{
	static DoneVeh[8];
	new rand[8],k;
	Back:
	for(new i=0; i<8; i++)
	{
		if(!DoneVeh[i])
		{
			rand[k]=LowRiders[i];
			k++;
		}
	}
	if(k == 0)
	{
		for(new i=0; i<8; i++) DoneVeh[i]=false;
		goto Back;
	}
	k=random(k);
	DoneVeh[k]=true;
	return LowRiders[k];
}

GetWagoon()
{
	static DoneVeh[5];
	new rand[5],k;
	Back:
	for(new i=0; i<5; i++)
	{
		if(!DoneVeh[i])
		{
			rand[k]=Wagoons[i];
			k++;
		}
	}
	if(k == 0)
	{
		for(new i=0; i<5; i++) DoneVeh[i]=false;
		goto Back;
	}
	k=random(k);
	DoneVeh[k]=true;
	return Wagoons[k];
}

GetSaloon()
{
	static DoneVeh[32];
	new rand[32],k;
	Back:
	for(new i=0; i<32; i++)
	{
		if(!DoneVeh[i])
		{
			rand[k]=Saloons[i];
			k++;
		}
	}
	if(k == 0)
	{
		for(new i=0; i<32; i++) DoneVeh[i]=false;
		goto Back;
	}
	k=random(k);
	DoneVeh[k]=true;
	return Saloons[k];
}
GetJeep()
{
	static DoneVeh[6];
	new rand[6],k;
	Back:
	for(new i=0; i<6; i++)
	{
		if(!DoneVeh[i])
		{
			rand[k]=Jeeps[i];
			k++;
		}
	}
	if(k == 0)
	{
		for(new i=0; i<6; i++) DoneVeh[i]=false;
		goto Back;
	}
	k=random(k);
	DoneVeh[k]=true;
	return Jeeps[k];
}

GetCovertable()
{
	static DoneVeh[4];
	new rand[4],k;
	Back:
	for(new i=0; i<4; i++)
	{
		if(!DoneVeh[i])
		{
			rand[k]=Covertables[i];
			k++;
		}
	}
	if(k == 0)
	{
		for(new i=0; i<4; i++) DoneVeh[i]=false;
		goto Back;
	}
	k=random(k);
	DoneVeh[k]=true;
	return Covertables[k];
}

GetIndusterial()
{
	static DoneVeh[9];
	new rand[9],k;
	Back:
	for(new i=0; i<9; i++)
	{
		if(!DoneVeh[i])
		{
			rand[k]=Industrial[i];
			k++;
		}
	}
	if(k == 0)
	{
		for(new i=0; i<9; i++) DoneVeh[i]=false;
		goto Back;
	}
	k=random(k);
	DoneVeh[k]=true;
	return Industrial[k];
}
CreateStaticVehicle(id,type,modelid,Float:X,Float:Y,Float:Z,Float:A,ColorOne,ColorTwo,bool:pstatic=true,int=0,vw=0)
{

	new bool:found;
	for(new i=1; i<MAX_VEHICLES; i++)
	{
		if(CarInfo[i][cID] == -1) 
		{
			CurrentVehicleId=i;
			found =true;
			break;
		}
	}
	if (!found)
	{
		printf("Too Many Vehicleid's Ignoring Vehicleid %d",CurrentVehicleId);
		return 1;
	}

	new siren;
	if(type == VEHICLE_TYPE_COP) siren=true;
	switch(modelid)
	{
		case LOW_RIDER: modelid = GetLowRider();
		case WAGOON: modelid=GetWagoon();
		case SALOON: modelid=GetSaloon(); 
		case JEEP: modelid=GetJeep();
		case COVERTABLE: modelid=GetCovertable();
		case INDUSTERIAL: modelid=GetIndusterial();
	}
	switch(modelid)
	{
		case 537,538,569,570,590,449: AddStaticVehicleEx(modelid,X,Y,Z,A,ColorOne,ColorTwo,7 * 60,siren);
		default: CreateVehicle(modelid,X,Y,Z,A,ColorOne,ColorTwo,7 * 60,siren);
	}
	CarInfo[CurrentVehicleId][cID] = id;
	CarInfo[CurrentVehicleId][cType] = type;
	CarInfo[CurrentVehicleId][cModel] = modelid;
	CarInfo[CurrentVehicleId][cLocationX] = X;
	CarInfo[CurrentVehicleId][cLocationY] = Y;
	CarInfo[CurrentVehicleId][cLocationZ] = Z;
	CarInfo[CurrentVehicleId][cLocationA] = A;
	CarInfo[CurrentVehicleId][cColorOne] = ColorOne;
	CarInfo[CurrentVehicleId][cColorTwo] = ColorTwo;

	CarInfo[CurrentVehicleId][cLocationI] = int;//Interior
	CarInfo[CurrentVehicleId][cLocationW] = vw;//World

	CarInfo[CurrentVehicleId][cOwned] = -1;
	CarInfo[CurrentVehicleId][cLock] = 0;

	CarInfo[CurrentVehicleId][cStaticVehicle]=pstatic;
	
	CarInfo[CurrentVehicleId][cDriverKitInstalled]=0;
	CarInfo[CurrentVehicleId][cCopKitInstalled]=0;
	

	return 1;
}

CreateVeh(model,Float:x,Float:y,Float:z,Float:a,col1,col2,playerid=INVALID_PLAYER_ID)
{
	new type;
	for(new i=0; i<sizeof(LowRiders); i++)
	{
		if(model == LowRiders[i])
		{
			type=1;
			goto VehFound;
		}
	}
	for(new i=0; i<sizeof(Wagoons); i++)
	{
		if(model == Wagoons[i])
		{
			type=2;
			goto VehFound;
		
		}
	}
	for(new i=0; i<sizeof(Saloons); i++)
	{
		if(model == Saloons[i])
		{
			type=3;
			goto VehFound;
		}
	}
	for(new i=0; i<sizeof(Jeeps); i++)
	{
		if(model == Jeeps[i])
		{
			type=4;
			goto VehFound;
		}
	}
	for(new i=0; i<sizeof(Covertables); i++)
	{
		if(model == Covertables[i])
		{
			type=5;
			goto VehFound;
		}
	}
	for(new i=0; i<sizeof(Industrial); i++)
	{
		if(model == Industrial[i])
		{
			type=6;
			goto VehFound;
		}
	}
	for(new i=0; i<sizeof(Police); i++)
	{
		if(model == Police[i])
		{
			type=7;
			goto VehFound;
		}
	}
	for(new i=0; i<sizeof(PublicService); i++)
	{
		if(model == PublicService[i])
		{
			type=8;
			goto VehFound;
		}
	}
	VehFound:
	new string[500];
	if(playerid != INVALID_PLAYER_ID)
	{
		switch(type)
		{
			case 0:
			{
				format(string,sizeof(string),"%s (%d) added new vehicle %s at %s [Type: Civilian | Model %d | Colors %d:%d] ",PlayerInfo[playerid][pUserName],playerid,VehicleNames[model-400],GetZoneName(x,y,z),model,col1,col2);
			}
			case 1:
			{
				format(string,sizeof(string),"%s (%d) added new vehicle type Low Rider at %s [Type: Civilian | Model Random[LR] | Colors %d:%d] ",PlayerInfo[playerid][pUserName],playerid,GetZoneName(x,y,z),col1,col2);
			}
			case 2:
			{
				format(string,sizeof(string),"%s (%d) added new vehicle type Wagon at %s [Type: Civilian | Model Random[W] | Colors %d:%d] ",PlayerInfo[playerid][pUserName],playerid,GetZoneName(x,y,z),col1,col2);
			}
			case 3:
			{
				format(string,sizeof(string),"%s (%d) added new vehicle type Saloon at %s [Type: Civilian | Model Random[S] | Colors %d:%d] ",PlayerInfo[playerid][pUserName],playerid,GetZoneName(x,y,z),col1,col2);
			}
			case 4:
			{
				format(string,sizeof(string),"%s (%d) added new vehicle type Jeep at %s [Type: Civilian | Model Random[J] | Colors %d:%d] ",PlayerInfo[playerid][pUserName],playerid,GetZoneName(x,y,z),col1,col2);
			}
			case 5:
			{
				format(string,sizeof(string),"%s (%d) added new vehicle type Covertable at %s [Type: Civilian | Model Random[C] | Colors %d:%d] ",PlayerInfo[playerid][pUserName],playerid,GetZoneName(x,y,z),col1,col2);
			}
			case 6:
			{
				format(string,sizeof(string),"%s (%d) added new vehicle type Industrial at %s [Type: Civilian | Model Random[I] | Colors %d:%d] ",PlayerInfo[playerid][pUserName],playerid,GetZoneName(x,y,z),col1,col2);
			}
			case 7:
			{
				format(string,sizeof(string),"%s (%d) added new vehicle %s at %s [Type: Cop | Model %d | Colors %d:%d] ",PlayerInfo[playerid][pUserName],playerid,VehicleNames[model-400],GetZoneName(x,y,z),model,col1,col2);
			}
			case 8:
			{
				format(string,sizeof(string),"%s (%d) added new vehicle %s at %s [Type: Public Service | Model %d | Colors %d:%d] ",PlayerInfo[playerid][pUserName],playerid,VehicleNames[model-400],GetZoneName(x,y,z),model,col1,col2);
			}
		}
		SendMessageToManagers(COLOR_ADMIN_INFO,string);
		IRC_SendManagementMessage(string,IRC_ADMIN_COLOR_INFO);
	}	
	switch(type)
	{
		case 0: mysql_format(g_SQL,string,sizeof(string),"INSERT INTO "TABLE_VEHICLES" (Name,Location,Type,Model,X,Y,Z,A,COL1,COL2) VALUES ('%e','%e',%d,%d,%f,%f,%f,%f,%d,%d)",VehicleNames[model-400],GetZoneName(x,y,z),VEHICLE_TYPE_CIVILIAN,model,x,y,z,a,col1,col2);
		case 1: mysql_format(g_SQL,string,sizeof(string),"INSERT INTO "TABLE_VEHICLES" (Name,Location,Type,Model,X,Y,Z,A,COL1,COL2) VALUES ('%e','%e',%d,%d,%f,%f,%f,%f,%d,%d)","Low Riders",GetZoneName(x,y,z),VEHICLE_TYPE_CIVILIAN,LOW_RIDER,x,y,z,a,col1,col2);
		case 2: mysql_format(g_SQL,string,sizeof(string),"INSERT INTO "TABLE_VEHICLES" (Name,Location,Type,Model,X,Y,Z,A,COL1,COL2) VALUES ('%e','%e',%d,%d,%f,%f,%f,%f,%d,%d)","Wagons",GetZoneName(x,y,z),VEHICLE_TYPE_CIVILIAN,WAGOON,x,y,z,a,col1,col2);
		case 3: mysql_format(g_SQL,string,sizeof(string),"INSERT INTO "TABLE_VEHICLES" (Name,Location,Type,Model,X,Y,Z,A,COL1,COL2) VALUES ('%e','%e',%d,%d,%f,%f,%f,%f,%d,%d)","Saloons",GetZoneName(x,y,z),VEHICLE_TYPE_CIVILIAN,SALOON,x,y,z,a,col1,col2);
		case 4: mysql_format(g_SQL,string,sizeof(string),"INSERT INTO "TABLE_VEHICLES" (Name,Location,Type,Model,X,Y,Z,A,COL1,COL2) VALUES ('%e','%e',%d,%d,%f,%f,%f,%f,%d,%d)","Jeeps",GetZoneName(x,y,z),VEHICLE_TYPE_CIVILIAN,JEEP,x,y,z,a,col1,col2);
		case 5: mysql_format(g_SQL,string,sizeof(string),"INSERT INTO "TABLE_VEHICLES" (Name,Location,Type,Model,X,Y,Z,A,COL1,COL2) VALUES ('%e','%e',%d,%d,%f,%f,%f,%f,%d,%d)","Covertables",GetZoneName(x,y,z),VEHICLE_TYPE_CIVILIAN,COVERTABLE,x,y,z,a,col1,col2);
		case 6: mysql_format(g_SQL,string,sizeof(string),"INSERT INTO "TABLE_VEHICLES" (Name,Location,Type,Model,X,Y,Z,A,COL1,COL2) VALUES ('%e','%e',%d,%d,%f,%f,%f,%f,%d,%d)","Industrials",GetZoneName(x,y,z),VEHICLE_TYPE_CIVILIAN,INDUSTERIAL,x,y,z,a,col1,col2);
		case 7: mysql_format(g_SQL,string,sizeof(string),"INSERT INTO "TABLE_VEHICLES" (Name,Location,Type,Model,X,Y,Z,A,COL1,COL2) VALUES ('%e','%e',%d,%d,%f,%f,%f,%f,%d,%d)",VehicleNames[model-400],GetZoneName(x,y,z),VEHICLE_TYPE_COP,model,x,y,z,a,col1,col2);
		case 8:  mysql_format(g_SQL,string,sizeof(string),"INSERT INTO "TABLE_VEHICLES" (Name,Location,Type,Model,X,Y,Z,A,COL1,COL2) VALUES ('%e','%e',%d,%d,%f,%f,%f,%f,%d,%d)",VehicleNames[model-400],GetZoneName(x,y,z),VEHICLE_TYPE_PUBLIC,model,x,y,z,a,col1,col2);
	}
	mysql_pquery(g_SQL,string,"OnVehicleAdded","");
	return 1;
}

CreateVehEx(model,Float:x,Float:y,Float:z,Float:a,col1,col2,playerid=INVALID_PLAYER_ID)
{
	new type;
	for(new i=0; i<sizeof(Police); i++)
	{
		if(model == Police[i])
		{
			type=7;
			goto VehFound;
		}
	}
	for(new i=0; i<sizeof(PublicService); i++)
	{
		if(model == PublicService[i])
		{
			type=8;
			goto VehFound;
		}
	}
	VehFound:
	new string[500];
	if(playerid != INVALID_PLAYER_ID)
	{
		switch(type)
		{
			case 0:
			{
				format(string,sizeof(string),"%s (%d) added new vehicle %s at %s [Type: Civilian | Model %d | Colors %d:%d] ",PlayerInfo[playerid][pUserName],playerid,VehicleNames[model-400],GetZoneName(x,y,z),model,col1,col2);
			}
			case 7:
			{
				format(string,sizeof(string),"%s (%d) added new vehicle %s at %s [Type: Cop | Model %d | Colors %d:%d] ",PlayerInfo[playerid][pUserName],playerid,VehicleNames[model-400],GetZoneName(x,y,z),model,col1,col2);
			}
			case 8:
			{
				format(string,sizeof(string),"%s (%d) added new vehicle %s at %s [Type: Public Service | Model %d | Colors %d:%d] ",PlayerInfo[playerid][pUserName],playerid,VehicleNames[model-400],GetZoneName(x,y,z),model,col1,col2);
			}
		}
		SendMessageToManagers(COLOR_ADMIN_INFO,string);
		IRC_SendManagementMessage(string,IRC_ADMIN_COLOR_INFO);
	}	
	switch(type)
	{
		case 0: mysql_format(g_SQL,string,sizeof(string),"INSERT INTO "TABLE_VEHICLES" (Name,Location,Type,Model,X,Y,Z,A,COL1,COL2) VALUES ('%e','%e',%d,%d,%f,%f,%f,%f,%d,%d)",VehicleNames[model-400],GetZoneName(x,y,z),VEHICLE_TYPE_CIVILIAN,model,x,y,z,a,col1,col2);
		case 7: mysql_format(g_SQL,string,sizeof(string),"INSERT INTO "TABLE_VEHICLES" (Name,Location,Type,Model,X,Y,Z,A,COL1,COL2) VALUES ('%e','%e',%d,%d,%f,%f,%f,%f,%d,%d)",VehicleNames[model-400],GetZoneName(x,y,z),VEHICLE_TYPE_COP,model,x,y,z,a,col1,col2);
		case 8:  mysql_format(g_SQL,string,sizeof(string),"INSERT INTO "TABLE_VEHICLES" (Name,Location,Type,Model,X,Y,Z,A,COL1,COL2) VALUES ('%e','%e',%d,%d,%f,%f,%f,%f,%d,%d)",VehicleNames[model-400],GetZoneName(x,y,z),VEHICLE_TYPE_PUBLIC,model,x,y,z,a,col1,col2);
	}
	mysql_pquery(g_SQL,string,"OnVehicleAdded","");
	return 1;
}

FUNCTION OnVehicleAdded()
{
	new id=cache_insert_id();
	new string[75],rows,Cache:result;
	mysql_format(g_SQL,string,sizeof(string),"SELECT * FROM "TABLE_VEHICLES" WHERE ID=%d",id);
	result=mysql_query(g_SQL,string);
	cache_get_row_count(rows);
	if(rows)
	{
		new type,Model,Float:x,Float:y,Float:z,Float:a,col1,col2;
		cache_get_value_int(0,"Type",type);
		cache_get_value_int(0,"Model",Model);
		cache_get_value_float(0,"X",x);
		cache_get_value_float(0,"Y",y);
		cache_get_value_float(0,"Z",z);
		cache_get_value_float(0,"A",a);
		cache_get_value_int(0,"COL1",col1);
		cache_get_value_int(0,"COL2",col2);
		CreateStaticVehicle(id,type,Model,x,y,z,a,col1,col2);
	}
	else printf("OnVehicleAdded Fail");
	cache_delete(result);
}

FUNCTION DeleteVehicle(vehicleid)
{
	new string[75];
	mysql_format(g_SQL,string,sizeof(string),"DELETE FROM `LSvehicles` WHERE ID=%d",CarInfo[vehicleid][cID]);
	printf(string);
	mysql_pquery(g_SQL,string);
	DestroyVehicleEx(vehicleid);
	return 1;
}

DestroyVehicleEx(vehicleid)
{
	CarInfo[vehicleid][cID]=-1;
	DestroyVehicle(vehicleid);
	return 1;
}
CMD:addvehicle(playerid,params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_MANAGER) return SendClientMessage(playerid,COLOR_ERROR,ERROR_NOT_ADMIN);
	if(!IsPlayerInAnyVehicle(playerid)) return SendClientMessage(playerid,COLOR_ERROR,"You need to be in vehicle to use this command.");
	new col1, col2;
	if(sscanf(params,"dd",col1,col2)) col1=col2=-1;
	new Float:x,Float:y,Float:z,Float:a;
	GetVehiclePos(GetPlayerVehicleID(playerid),x,y,z);
	GetVehicleZAngle(GetPlayerVehicleID(playerid),a);
	new model=GetVehicleModel(GetPlayerVehicleID(playerid));
	DestroyVehicle(GetPlayerVehicleID(playerid));
	CreateVeh(model,x,y,z,a,col1,col2,playerid);
	SendClientMessage(playerid,COLOR_ADMIN_INFO,"This vehicle has been added in database, would be loaded shortly.");
	return 1;
}
CMD:addvehicle2(playerid,params[])
{
	if(PlayerInfo[playerid][pRank] < SERVER_MANAGER) return SendClientMessage(playerid,COLOR_ERROR,ERROR_NOT_ADMIN);
	if(!IsPlayerInAnyVehicle(playerid)) return SendClientMessage(playerid,COLOR_ERROR,"You need to be in vehicle to use this command.");
	new col1, col2;
	if(sscanf(params,"dd",col1,col2)) col1=col2=-1;
	new Float:x,Float:y,Float:z,Float:a;
	GetVehiclePos(GetPlayerVehicleID(playerid),x,y,z);
	GetVehicleZAngle(GetPlayerVehicleID(playerid),a);
	new model=GetVehicleModel(GetPlayerVehicleID(playerid));
	DestroyVehicle(GetPlayerVehicleID(playerid));
	CreateVehEx(model,x,y,z,a,col1,col2,playerid);
	SendClientMessage(playerid,COLOR_ADMIN_INFO,"This vehicle has been added in database, would be loaded shortly");
	return 1;
}

CMD:removevehicle(playerid)
{
	if(PlayerInfo[playerid][pRank] < SERVER_MANAGER) return SendClientMessage(playerid,COLOR_ERROR,ERROR_NOT_ADMIN);
	if(!IsPlayerInAnyVehicle(playerid)) return SendClientMessage(playerid,COLOR_ERROR,"You need to be in vehicle to use this command.");
	new vehid=GetPlayerVehicleID(playerid);
	if(CarInfo[vehid][cID] == -1)  return SendClientMessage(playerid,COLOR_ERROR,"This vehicle is either not in database.");
	DeleteVehicle(vehid);
	new string[256];
	switch(CarInfo[vehid][cType])
	{
		case VEHICLE_TYPE_CIVILIAN: format(string,sizeof(string),"%s (%d) removed vehicle %s at %s [Type: Civilian | Model %d | Colors %d:%d] ",PlayerInfo[playerid][pUserName],playerid,VehicleNames[CarInfo[vehid][cModel]-400],GetZoneName(CarInfo[vehid][cLocationX],CarInfo[vehid][cLocationY],CarInfo[vehid][cLocationZ]),CarInfo[vehid][cModel],CarInfo[vehid][cColorOne],CarInfo[vehid][cColorTwo]);
		case VEHICLE_TYPE_PUBLIC: format(string,sizeof(string),"%s (%d) removed vehicle %s at %s [Type: Public Service | Model %d | Colors %d:%d] ",PlayerInfo[playerid][pUserName],playerid,VehicleNames[CarInfo[vehid][cModel]-400],GetZoneName(CarInfo[vehid][cLocationX],CarInfo[vehid][cLocationY],CarInfo[vehid][cLocationZ]),CarInfo[vehid][cModel],CarInfo[vehid][cColorOne],CarInfo[vehid][cColorTwo]);
		case VEHICLE_TYPE_COP: format(string,sizeof(string),"%s (%d) removed vehicle %s at %s [Type: Cops | Model %d | Colors %d:%d] ",PlayerInfo[playerid][pUserName],playerid,VehicleNames[CarInfo[vehid][cModel]-400],GetZoneName(CarInfo[vehid][cLocationX],CarInfo[vehid][cLocationY],CarInfo[vehid][cLocationZ]),CarInfo[vehid][cModel],CarInfo[vehid][cColorOne],CarInfo[vehid][cColorTwo]);
	}
	SendMessageToManagers(COLOR_ADMIN_INFO,string);
	IRC_SendManagementMessage(string,IRC_ADMIN_COLOR_INFO);
	SendClientMessage(playerid,COLOR_ADMIN_INFO,"This vehicle has been removed from in database.");
	return 1;
}