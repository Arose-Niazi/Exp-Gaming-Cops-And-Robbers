/*

	GPS Script
	Scripted by Arose Niazi.
	Scripting started on 20th October 2017
	Last Updated: ----
	
	Credits:
		
	Functions:
		GPS_OnPlayerDisconnect(playerid);
		
	Functions Being Called:
*/


new PlayerText:GPS_Textdraw[3];

enum {
	GPS_ROBBERY,
	GPS_MISSION,
	GPS_C_CITY_HALL,
	GPS_C_PD,
	GPS_C_HOSPITAL,
	GPS_C_BANK,
	GPS_C_24_7,
	GPS_C_BAIT,
	GPS_C_CLOTHES,
	GPS_C_PET,
	GPS_C_CRANE,
	GPS_POLICE,
	GPS_CITY_HALLS,
	GPS_BANKS,
	GPS_24_7,
	GPS_CAR_SALE,
	GPS_TOWNS,
	GPS_ROBBERY_LOC,
	GPS_CLOTHES,
	GPS_CUSTOM,
	GPS_MAP_MARKER
};


GPS_OnPlayerDisconnect(playerid)
{
	if(PlayerInfo[playerid][GPS_Destination] != -1) 
	{
		DisableGPS(playerid);
	}
	DestroyGPStd(playerid);
	return 1;
}

CMD:gps(playerid, params[]) 
{
	HideTextDrawMenu(playerid);
	new id,id2,id3;
	if(sscanf(params,"dD(-1)D(-1)",id,id2,id3))
	{
		ShowTextDrawMenu(playerid, MENU_GPS_OPTIONS,"~y~GPS Options", 3, 2);
		ShowTextDrawMenuItems(playerid, 0, "Select ~p~GPS~w~ option", " ", " ",0);
		ShowTextDrawMenuItems(playerid, 2, " ", "~b~~h~1 ~w~- Select Destination", "",0);
		if(PlayerInfo[playerid][GPS_Destination] != -1) ShowTextDrawMenuItems(playerid, 2, " ", "~b~~h~2 ~w~- Clear GPS", "",0);
		else ShowTextDrawMenuItems(playerid, 3, " ", "~b~~h~2 ~r~- Clear GPS", "",0);
	}
	else
	{
		OnGPSMenu(playerid,id,id2,id3);
	}
	return 1;
}


CMD:gpsoff(playerid) {
	if(PlayerInfo[playerid][GPS_Destination] == -1) return SendClientMessage(playerid,COLOR_ERROR,"Your GPS is not switched on.");
	SendClientMessage(playerid,COLOR_ERROR,"GPS switched off!");
	DisableGPS(playerid);
	return 1;
}

alias:gpsoff("cleargps");

OnGPSMenu(playerid,option1,option2,option3)
{
	switch(option1)
	{
		case 1:
		{
			switch(option2)
			{
				case -1:
				{
					new options,string[50];
					if(PlayerInfo[playerid][pHideOutLocation] || PlayerInfo[playerid][pMissionLocation]) options++;
					ShowTextDrawMenu(playerid, MENU_GPS_LIST,"~y~GPS Menu", options+2+19, options+19);
					ShowTextDrawMenuItems(playerid, 0, "Select ~p~GPS~w~ category or destination", " ", " ",0);
					if(PlayerInfo[playerid][pHideOutLocation]) ShowTextDrawMenuItems(playerid, 2, " ", "~b~~h~1 ~w~- Robbery Hideout", "",GPS_ROBBERY);
					else if(PlayerInfo[playerid][pMissionLocation]) ShowTextDrawMenuItems(playerid, 2, " ", "~b~~h~1 ~w~- Mission Destination", "",GPS_MISSION);
					format(string,sizeof(string),"~b~~h~%d ~w~- Closest City Hall",options+1);
					ShowTextDrawMenuItems(playerid, options+2, " ", string, "",GPS_C_CITY_HALL);
					options++;
					format(string,sizeof(string),"~b~~h~%d ~w~- Closest Police Department",options+1);
					ShowTextDrawMenuItems(playerid, options+2, " ", string, "",GPS_C_PD);
					options++;
					format(string,sizeof(string),"~b~~h~%d ~w~- Closest Hospital",options+1);
					ShowTextDrawMenuItems(playerid, options+2, " ", string, "",GPS_C_HOSPITAL);
					options++;
					format(string,sizeof(string),"~b~~h~%d ~w~- Closest Bank",options+1);
					ShowTextDrawMenuItems(playerid, options+2, " ", string, "",GPS_C_BANK);
					options++;
					format(string,sizeof(string),"~b~~h~%d ~w~- Closest 24-7",options+1);
					ShowTextDrawMenuItems(playerid, options+2, " ", string, "",GPS_C_24_7);
					options++;
					format(string,sizeof(string),"~b~~h~%d ~w~- Closest Bait Shop",options+1);
					ShowTextDrawMenuItems(playerid, options+2, " ", string, "",GPS_C_BAIT);
					options++;
					format(string,sizeof(string),"~b~~h~%d ~w~- Closest Clothes Store",options+1);
					ShowTextDrawMenuItems(playerid, options+2, " ", string, "",GPS_C_CLOTHES);
					options++;
					format(string,sizeof(string),"~b~~h~%d ~w~- Closest Pet Store",options+1);
					ShowTextDrawMenuItems(playerid, options+2, " ", string, "",GPS_C_PET);
					options++;
					format(string,sizeof(string),"~b~~h~%d ~w~- Closest Crane",options+1);
					ShowTextDrawMenuItems(playerid, options+2, " ", string, "",GPS_C_CRANE);
					options++;
					format(string,sizeof(string),"~b~~h~%d ~p~- Police Loactions",options+1);
					ShowTextDrawMenuItems(playerid, options+2, " ", string, "",GPS_POLICE);
					options++;
					format(string,sizeof(string),"~b~~h~%d ~p~- City Halls",options+1);
					ShowTextDrawMenuItems(playerid, options+2, " ", string, "",GPS_CITY_HALLS);
					options++;
					format(string,sizeof(string),"~b~~h~%d ~p~- Banks",options+1);
					ShowTextDrawMenuItems(playerid, options+2, " ", string, "",GPS_BANKS);
					options++;
					format(string,sizeof(string),"~b~~h~%d ~p~- 24-7 Stores",options+1);
					ShowTextDrawMenuItems(playerid, options+2, " ", string, "",GPS_24_7);
					options++;
					format(string,sizeof(string),"~b~~h~%d ~p~- Car Sale Points",options+1);
					ShowTextDrawMenuItems(playerid, options+2, " ", string, "",GPS_CAR_SALE);
					options++;
					format(string,sizeof(string),"~b~~h~%d ~p~- Towns",options+1);
					ShowTextDrawMenuItems(playerid, options+2, " ", string, "",GPS_TOWNS);
					options++;
					format(string,sizeof(string),"~b~~h~%d ~p~- Robbery Loactions",options+1);
					ShowTextDrawMenuItems(playerid, options+2, " ", string, "",GPS_ROBBERY_LOC);
					options++;
					format(string,sizeof(string),"~b~~h~%d ~p~- Clothes Stores",options+1);
					ShowTextDrawMenuItems(playerid, options+2, " ", string, "",GPS_CLOTHES);
					options++;
					format(string,sizeof(string),"~b~~h~%d ~p~- Custom Locations",options+1);
					ShowTextDrawMenuItems(playerid, options+2, " ", string, "",GPS_CUSTOM);
					options++;
					if(PlayerInfo[playerid][pCustomMapMarker][0] == 0.0 && PlayerInfo[playerid][pCustomMapMarker][1] == 0.0 && PlayerInfo[playerid][pCustomMapMarker][2] == 0.0) format(string,sizeof(string),"~b~~h~%d ~r~- Map Markers",options+1);
					else format(string,sizeof(string),"~b~~h~%d ~p~- Map Markers",options+1);
					ShowTextDrawMenuItems(playerid, options+2, " ", string, "",GPS_MAP_MARKER);
					options++;
				}
				/*case GPS_ROBBERY:
				case GPS_MISSION:
				case GPS_C_CITY_HALL:
				case GPS_C_PD:
				case GPS_C_HOSPITAL:
				case GPS_C_BANK:
				case GPS_C_24_7:
				case GPS_C_BAIT:
				case GPS_C_CLOTHES:
				case GPS_C_PET:
				case GPS_C_CRANE:
				case GPS_POLICE:
				case GPS_CITY_HALLS:
				case GPS_BANKS:
				case GPS_24_7:
				case GPS_CAR_SALE:
				case GPS_TOWNS:
				case GPS_ROBBERY_LOC:
				case GPS_CLOTHES:
				case GPS_CUSTOM:*/
				case GPS_MAP_MARKER:
				{
					if(PlayerInfo[playerid][pCustomMapMarker][0] == 0.0 && PlayerInfo[playerid][pCustomMapMarker][1] == 0.0 && PlayerInfo[playerid][pCustomMapMarker][2] == 0.0) return SendClientMessage(playerid,COLOR_ERROR,"No map marker found, please (re)add it.");
					SetGPS(playerid,PlayerInfo[playerid][pCustomMapMarker][0],PlayerInfo[playerid][pCustomMapMarker][1],PlayerInfo[playerid][pCustomMapMarker][2],"Custom Map Marker");
					
				}
				
			}
			
		}
		case 2:
		{
			callcmd::gpsoff(playerid);
		}
	}
	return 1;
}

SetGPS(playerid,Float:X,Float:Y,Float:Z,Dest[])
{
	if(PlayerInfo[playerid][GPS_Calculating]) return SendClientMessage(playerid,COLOR_ERROR,"Please wait, the gps route is already being loacted.");
	if(PlayerInfo[playerid][GPS_Destination] != -1) DisableGPS(playerid);
	new PlayerClosest = NearestPlayerNode(playerid);
	new TempDestination = NearestNodeFromPoint(X,Y,Z);
	if(PlayerClosest == TempDestination) return SendClientMessage(playerid,COLOR_ERROR,"You are already there.");
	if(PlayerClosest == -1) return SendClientMessage(playerid,COLOR_ERROR,"Satellite can not find you at this location, go to the nearest road.");
	if(TempDestination == -1) return SendClientMessage(playerid,COLOR_ERROR,"Satellite can not find a destination, please try again.");
	if(CalculatePath(PlayerClosest,TempDestination,playerid,true,12.0,true)) 
	{
		new string[75];
		PlayerInfo[playerid][GPS_Calculating] = true;
		SendClientMessage(playerid,COLOR_ERROR,"Creating a route, please wait.");
		format(string,sizeof(string),"~b~~h~Destination:~w~ %s",Dest);
		PlayerTextDrawSetString(playerid,GPS_Textdraw[0],string);
	}
	else SendClientMessage(playerid,COLOR_ERROR,"Unknown error, please try again.");
	return 1;
}

public OnPlayerClosestNodeIDChange(playerid,old_NodeID,new_NodeID) 
{
	if(PlayerInfo[playerid][GPS_RouteID] != -1) 
	{
		if(PlayerInfo[playerid][GPS_Destination] != -1)
		{
			if(old_NodeID == PlayerInfo[playerid][GPS_Destination] || new_NodeID == PlayerInfo[playerid][GPS_Destination]) 
			{
				DisableGPS(playerid);
				
				SendClientMessage(playerid,COLOR_SERVER,"GPS Destination Reached.");
			}
			else
	        {
			    if(!IsPlayerInDynamicArea(playerid,PlayerInfo[playerid][GPS_RouteID]))
			    {
			        if(!PlayerInfo[playerid][GPS_Calculating])
			        {
						PlayerInfo[playerid][GPS_LostCount]++;
			            if(PlayerInfo[playerid][GPS_LostCount] >= 5)
			            {
			                PlayerInfo[playerid][GPS_Calculating] = true;
			                CalculatePath(NearestPlayerNode(playerid),PlayerInfo[playerid][GPS_Destination],playerid,true,12.0,true);
			            }
			        }
			    }
				else
				{
					new cnode;
					for(new i = 0; i < PlayerInfo[playerid][GPS_Lenght]; ++i) 
					{
						if(PlayerInfo[playerid][GPS_Nods][i] ==  new_NodeID)
						{
							cnode=i;
							break;
						}
					}
					for(new i = cnode; i < PlayerInfo[playerid][GPS_Lenght]; i++) 
					{
						
						if(i == cnode) 
						{
							new Float:xa,Float:ya,Float:za,Float:xb,Float:yb,Float:zb;
							GetPlayerPos(playerid,xa,ya,za);
							GetNodePos(PlayerInfo[playerid][GPS_Nods][i],xb,yb,zb);
							PlayerInfo[playerid][GPS_Distance]=GetDistanceBetweenPoints3D(xa,ya,za,xb,yb,zb);
							
						}
						PlayerInfo[playerid][GPS_Distance]+=PlayerInfo[playerid][GPS_NodsDistance][i];
					}
					ShowCurrentGPSDistance(playerid);
				}
	        }
		}
		else
	    {
			DisableGPS(playerid);
	    }		
	}
	return 1;
}

public GPS_WhenRouteIsCalculated(routeid,node_id_array[],amount_of_nodes,Float:distance,Float:Polygon[],Polygon_Size,Float:NodePosX[],Float:NodePosY[],Float:NodePosZ[])
{
	new Float:xa,Float:ya,Float:za;
	GetPlayerPos(routeid,xa,ya,za);
	if(amount_of_nodes > 1 && amount_of_nodes < MAX_NODS) 
	{
		for(new i = 0; i < PlayerInfo[routeid][GPS_Lenght] ; i++)
		{
			DestroyDynamicObject(PlayerInfo[routeid][GPS_CreatedObjects][i]);
		}
		for(new i=0; i<PlayerInfo[routeid][GPS_MakerLength]; i++)
		{
			 DestroyDynamicMapIcon(PlayerInfo[routeid][GPS_CreatedMarkers][i]);
		}
		DestroyDynamicArea(PlayerInfo[routeid][GPS_RouteID]);
		PlayerInfo[routeid][GPS_RouteID] = CreateDynamicPolygon(Polygon,.maxpoints=Polygon_Size,.playerid=routeid);
	    for(new i = 0; i < amount_of_nodes; ++i) 
		{
			PlayerInfo[routeid][GPS_CreatedObjects][i] = CreateDynamicObject(1318,NodePosX[i],NodePosY[i],NodePosZ[i]+1.0,0.0,0.0,0.0,-1,-1,routeid,150.0); 
			
			if(i != amount_of_nodes-1)
			{
				SetDynamicObjectMaterial(PlayerInfo[routeid][GPS_CreatedObjects][i] , 0, 19063, "xmasorbs", "orbcap", 0x9900FF00);
				SetDynamicObjectRot(PlayerInfo[routeid][GPS_CreatedObjects][i], 0.0, 
				arw_GetYRotation(NodePosX[i],NodePosY[i],NodePosZ[i]+1.0,NodePosX[i+1],NodePosY[i+1],NodePosZ[i+1]+1.0), 
				arw_PointToPoint(NodePosX[i],NodePosY[i], NodePosX[i+1],NodePosY[i+1]));
			}
			else
			{
				SetDynamicObjectMaterial(PlayerInfo[routeid][GPS_CreatedObjects][i] , 0, 19063, "xmasorbs", "orbcap", 0x99FF0000);
			}
			PlayerInfo[routeid][GPS_Nods][i]= node_id_array[i];	
			if(i == 0) PlayerInfo[routeid][GPS_Distance]=0.0;
			else
			{
				xa=NodePosX[i-1];
				ya=NodePosY[i-1];
				za=NodePosZ[i-1];
			}
			PlayerInfo[routeid][GPS_NodsDistance][i]=GetDistanceBetweenPoints3D(xa,ya,za,NodePosX[i],NodePosY[i],NodePosZ[i]);
			PlayerInfo[routeid][GPS_Distance]+=PlayerInfo[routeid][GPS_NodsDistance][i];
			
	    }
		PlayerInfo[routeid][GPS_MakerLength]=0;
		for(new i = 0,j=(Polygon_Size-2)/2; i < j; i+=8)
		{
			PlayerInfo[routeid][GPS_CreatedMarkers][PlayerInfo[routeid][GPS_MakerLength]]=CreateDynamicMapIcon(Polygon[i],Polygon[i+1], 0.0, 0, 0xff797985, .playerid = routeid, .streamdistance=300.0);
			PlayerInfo[routeid][GPS_MakerLength]++;
		}
		if(PlayerInfo[routeid][GPS_Destination] == -1) SendClientMessage(routeid,COLOR_SERVER,"GPS Set!");
		PlayerTextDrawShow(routeid,GPS_Textdraw[0]);
		ShowCurrentGPSDistance(routeid);
	    PlayerInfo[routeid][GPS_Lenght] = amount_of_nodes;
		PlayerInfo[routeid][GPS_Destination] = node_id_array[amount_of_nodes-1];
		PlayerInfo[routeid][GPS_LostCount] = 0;
		PlayerInfo[routeid][GPS_Calculating] = false; 
	    gps_AddPlayer(routeid);
	}
	else 
	{
		if(PlayerInfo[routeid][GPS_Destination] != -1) DisableGPS(routeid);
		SendClientMessage(routeid,COLOR_ERROR,"Satellite can not find you at this location, go to the nearest road.");
		PlayerInfo[routeid][GPS_Calculating] = false;
	}

	return 1;
}

CreateGPStd(playerid)
{
	GPS_Textdraw[0] = CreatePlayerTextDraw(playerid, 543.001586, 369.508789, "Destination:_City_Hall_in_temple");
	PlayerTextDrawLetterSize(playerid, GPS_Textdraw[0], 0.189633, 1.063333);
	PlayerTextDrawAlignment(playerid, GPS_Textdraw[0], 2);
	PlayerTextDrawColor(playerid, GPS_Textdraw[0], -1);
	PlayerTextDrawSetShadow(playerid, GPS_Textdraw[0], 0);
	PlayerTextDrawSetOutline(playerid, GPS_Textdraw[0], 1);
	PlayerTextDrawBackgroundColor(playerid, GPS_Textdraw[0], 255);
	PlayerTextDrawFont(playerid, GPS_Textdraw[0], 1);
	PlayerTextDrawSetProportional(playerid, GPS_Textdraw[0], 1);
	PlayerTextDrawSetShadow(playerid, GPS_Textdraw[0], 0);

	GPS_Textdraw[1] = CreatePlayerTextDraw(playerid, 543.001586, 378.609344, "Distance_5.3KM");
	PlayerTextDrawLetterSize(playerid, GPS_Textdraw[1], 0.189633, 1.063333);
	PlayerTextDrawAlignment(playerid, GPS_Textdraw[1], 2);
	PlayerTextDrawColor(playerid, GPS_Textdraw[1], -1);
	PlayerTextDrawSetShadow(playerid, GPS_Textdraw[1], 0);
	PlayerTextDrawSetOutline(playerid, GPS_Textdraw[1], 1);
	PlayerTextDrawBackgroundColor(playerid, GPS_Textdraw[1], 255);
	PlayerTextDrawFont(playerid, GPS_Textdraw[1], 1);
	PlayerTextDrawSetProportional(playerid, GPS_Textdraw[1], 1);
	PlayerTextDrawSetShadow(playerid, GPS_Textdraw[1], 0);
	return 1;
}

DestroyGPStd(playerid)
{
	PlayerTextDrawDestroy(playerid,GPS_Textdraw[0]);
	PlayerTextDrawDestroy(playerid,GPS_Textdraw[1]);
	return 1;
}

DisableGPS(playerid)
{
	PlayerInfo[playerid][GPS_Destination] = -1;
	DestroyDynamicArea(PlayerInfo[playerid][GPS_RouteID]);
	for(new i = 0; i < PlayerInfo[playerid][GPS_Lenght]; i++)
	{
		DestroyDynamicObject(PlayerInfo[playerid][GPS_CreatedObjects][i]);
	}
	for(new i=0; i<PlayerInfo[playerid][GPS_MakerLength]; i++)
	{
		 DestroyDynamicMapIcon(PlayerInfo[playerid][GPS_CreatedMarkers][i]);
	}
	PlayerInfo[playerid][GPS_Lenght] = 0;
	PlayerInfo[playerid][GPS_LostCount] = 0;
	gps_RemovePlayer(playerid);
	PlayerTextDrawHide(playerid,GPS_Textdraw[0]);
	PlayerTextDrawHide(playerid,GPS_Textdraw[1]);
	return 1;
}

GPS_OnPlayerEnterRoute(playerid,areaid)
{
	if(PlayerInfo[playerid][GPS_RouteID] == areaid)
	{
		PlayerInfo[playerid][GPS_LostCount] = 0;
    }
    return 1;
}

GPS_OnPlayerLeaveRoute(playerid,areaid)
{
	if(PlayerInfo[playerid][GPS_RouteID] == areaid && PlayerInfo[playerid][GPS_Destination] != -1)
	{
	    if(!PlayerInfo[playerid][GPS_Calculating])
	    {
			CalculatePath(NearestPlayerNode(playerid),PlayerInfo[playerid][GPS_Destination],playerid,true,12.0,true);
			PlayerInfo[playerid][GPS_Calculating] = true;
		}
	}
	return 1;
}
forward Float:arw_GetYRotation(Float:x,Float:y,Float:z, Float:ax, Float:ay, Float:az);
public Float:arw_GetYRotation(Float:x,Float:y,Float:z, Float:ax, Float:ay, Float:az)
{
    new Float:xd = ax - x;
    new Float:yd = ay - y;
    new Float:dist = floatsqroot(xd*xd+yd*yd);
    new Float:Yoff = atan((z-az)/dist);
    return Yoff-90;
}

forward Float:arw_PointToPoint(Float:x2, Float:y2, Float:X, Float:Y);
public Float:arw_PointToPoint(Float:x2, Float:y2, Float:X, Float:Y)

{
	new Float:DX, Float:DY;
	new Float:angle;
	
	DX = floatabs(floatsub(x2,X));
	DY = floatabs(floatsub(y2,Y));
	
	if (DY == 0.0 || DX == 0.0)
	{
		if(DY == 0 && DX > 0) angle = 0.0;
		else if(DY == 0 && DX < 0) angle = 180.0;
		else if(DY > 0 && DX == 0) angle = 90.0;
		else if(DY < 0 && DX == 0) angle = 270.0;
		else if(DY == 0 && DX == 0) angle = 0.0;
	}
	else
	{
		angle = atan(DX/DY);
		
		if(X > x2 && Y <= y2) angle += 90.0;
		else if(X <= x2 && Y < y2) angle = floatsub(90.0, angle);
		else if(X < x2 && Y >= y2) angle -= 90.0;
		else if(X >= x2 && Y > y2) angle = floatsub(270.0, angle);
	}
	return floatadd(angle, 180.0);
}
forward Float:GetDistanceBetweenPoints3D(Float:x1,Float:y1,Float:z1,Float:x2,Float:y2,Float:z2);
public Float:GetDistanceBetweenPoints3D(Float:x1,Float:y1,Float:z1,Float:x2,Float:y2,Float:z2){
    return VectorSize(x1-x2,y1-y2,z1-z2);
}  

ShowCurrentGPSDistance(playerid)
{
	new string[40];
	if(PlayerInfo[playerid][pShowGPSDistance])
	{
		if(PlayerInfo[playerid][GPS_Distance] > 1000.0)
		{
			if(PlayerInfo[playerid][pDistanceInMiles])
			{
				format(string,sizeof(string),"~b~~h~Distance:~w~ %0.2f mi", floatmul(floatdiv(PlayerInfo[playerid][GPS_Distance],1000.0),0.621371));
			}
			else
			{
				format(string,sizeof(string),"~b~~h~Distance:~w~ %0.2f km", floatdiv(PlayerInfo[playerid][GPS_Distance],1000.0));
			}
		}
		else
		{
			format(string,sizeof(string),"~b~~h~Distance:~w~ %0.2f m", PlayerInfo[playerid][GPS_Distance],1000.0);
		}
		PlayerTextDrawSetString(playerid,GPS_Textdraw[1],string);
		PlayerTextDrawShow(playerid,GPS_Textdraw[1]);
	}	
}