/*

	Elevators Script
	Scripted by Arose Niazi.
	Scripting started on 30th October 2017
	Last Updated: -----
	
	Credits:
		Zamaroht for basic elevator working
		Kye sound effects
		
	Functions:
		OnGRINElevatorMoved(objectid)
	OnGRINElevatorMenu(playerid, Option)
	Check_GRIN_Elevator(playerid)
	GRIN_Elevator_Initialize()
	GRIN_Elevator_Destroy()
	GRINElevator_OpenDoors()
	GRINElevator_CloseDoors()
	GRINFloor_OpenDoors(floorid)
	GRINFloor_CloseDoors(floorid)
	ContinueToNextFloorGRIN()
	Elevator_Boost_GRIN(floorid)
	AddFloorToQueue_GRIN(playerid,floorid)
	ResetGRINElevatorQueueGRIN()
	IsFloorInQueue_GRIN(floorid)
	CheckForNextGRIN()
	ShowGRINElevaorMenu(playerid)
	IsPlayerInGRINElevator(playerid)
	IsPlayerOnGRINFloor(playerid)
	FloorChanged_GRIN(floorid)
	Float:GetE_Z_C_Floor_G(floorid);
	Float:GetDoorsZCoordForFloorG(floorid);

	Functions Being Called:
		
*/

#define INVALID_FLOOR           (-1)

#define ELEVATOR_SPEED_GRIN      (3.5)
#define DOORS_SPEED_GRIN         (5.0)   
#define ELEVATOR_WAIT_TIME_GRIN  (5000)  

#define Y_DOOR_CLOSED_GRIN       (-1180.535917)
#define Y_DOOR_R_OPENED_GRIN      Y_DOOR_CLOSED_GRIN - 1.6
#define Y_DOOR_L_OPENED_GRIN      Y_DOOR_CLOSED_GRIN + 1.6
#define GROUND_Z_COORD_GRIN      (20.879316)
#define ELEVATOR_OFFSET_GRIN     (0.059523)

#define X_ELEVATOR_POS_GRIN      (1181.622924)
#define Y_ELEVATOR_POS_GRIN      (-1180.554687)

#define MAX_GRIN_FLOORS 11


static FloorNamesGRIN[MAX_GRIN_FLOORS][] =
{
	"Car Park",
	"Ground Floor",
	"First Floor",
	"Second Floor",
	"Third Floor",
	"Fourth Floor",
	"Fifth Floor",
	"Sixth Floor",
	"Seventh Floor",
	"Eighth Floor",
	"Ninth Floor"
};	

static Float:FloorZOffsetsGRIN[MAX_GRIN_FLOORS] =
{
    0.0, 		// Car Park
    13.604544,	// Ground Floor
    18.808519,	// First Floor = 13.604544 + 5.203975
    24.012494,  // Second Floor = 18.808519 + 5.203975
    29.216469,  // Third Floor = 24.012494 + 5.203975
    34.420444,  // Fourth Floor = 29.216469 + 5.203975
    39.624419,  // Fifth Floor = 34.420444 + 5.203975
    44.828394,  // Sixth Floor = 39.624419 + 5.203975
    50.032369,  // Seventh Floor = 44.828394 + 5.203975
    55.236344,  // Eighth Floor = 50.032369 + 5.203975
    60.440319   // Ninth Floor = 55.236344 + 5.203975
};	


/*enum elevator_enum
{
	ElevatorObject,
	ElevatorDoors[2],
	FloorDoors1[MAX_GRIN_FLOORS],
	FloorDoors2[MAX_GRIN_FLOORS],
	Text3D:ElevatorLabel,
	Text3D:FloorLables[MAX_GRIN_FLOORS],
	ElevatorState, //Idel,Waiting,Moving
	ElevatorMoving, //Up or Down
	ElevatorCurrentFloor,
	FloorRequested[MAX_GRIN_FLOORS],
	ElevatorBoostTimer
};*/

new GRIN_INFO[elevator_enum];
OnGRINElevatorMoved(objectid)
{
    new Float:x, Float:y, Float:z;
	for(new i; i < MAX_GRIN_FLOORS; i ++)
	{
		if(objectid == GRIN_INFO[FloorDoors1][i])
		{
		    GetDynamicObjectPos(GRIN_INFO[FloorDoors1][i], x, y, z);
		    if(y < Y_DOOR_L_OPENED_GRIN - 0.5) 
		    {
				ContinueToNextFloorGRIN();
			}
		}
	}
	if(objectid == GRIN_INFO[ElevatorObject])   // The elevator reached the specified floor.
	{
		FloorChanged_GRIN(GRIN_INFO[ElevatorCurrentFloor]);
		if(IsFloorInQueue_GRIN(GRIN_INFO[ElevatorCurrentFloor]))
		{
			KillTimer(GRIN_INFO[ElevatorBoostTimer]);  // Kills the timer, in case the elevator reached the floor before boost.
			GRINElevator_OpenDoors();
			GRINFloor_OpenDoors(GRIN_INFO[ElevatorCurrentFloor]);
			
			GetDynamicObjectPos(GRIN_INFO[ElevatorObject], x, y, z);
			DestroyDynamic3DTextLabel(GRIN_INFO[ElevatorLabel]);
			GRIN_INFO[ElevatorLabel] = CreateDynamic3DTextLabel(""COL_WHITE"Press '"COL_SERVER_INFO"~k~~CONVERSATION_YES~"COL_WHITE"' to use elevator", COLOR_SERVER,  X_ELEVATOR_POS_GRIN - 1.7, Y_ELEVATOR_POS_GRIN - 1.75, z - 0.4, 4.0);

			GRIN_INFO[ElevatorState] 	= ELEVATOR_STATE_WAITING;
			SetTimer("CheckForNextGRIN", ELEVATOR_WAIT_TIME_GRIN, false);
		}
		else ContinueToNextFloorGRIN();
	}

	return 1;
}

OnGRINElevatorMenu(playerid, Option)
{
	if(Option == 1) Option = 0;
    if(GRIN_INFO[FloorRequested][Option] != INVALID_PLAYER_ID)
	{
		ShowGRINElevaorMenu(playerid);
		SendClientMessage(playerid, COLOR_ERROR,"Elevator already called on that floor.");
	}
	else
	{
		if(IsPlayerInGRINElevator(playerid)) 
		{
			AddFloorToQueue_GRIN(playerid, Option);
			ShowGRINElevaorMenu(playerid);
		}
		else
			SendClientMessage(playerid, COLOR_ERROR,"You are not inside the elevator.");
	}
	
	return 1;
}

Check_GRIN_Elevator(playerid)
{
	if(IsPlayerInGRINElevator(playerid))  
	    ShowGRINElevaorMenu(playerid);
	else  
	{
		new floor=IsPlayerOnGRINFloor(playerid);
	    if(floor <= 9 && floor > -1)
		{
			if(!IsFloorInQueue_GRIN(floor))
			{
				AddFloorToQueue_GRIN(playerid, floor);
				GameTextForPlayer(playerid, "~w~Elevator called", 2000, 3);
			}
			else
			{
				GameTextForPlayer(playerid, "~r~Elevator Already Called", 2000, 3);
			}
		}
	}
	return 1;
}


GRIN_Elevator_Initialize()
{
	// Initializes the elevator.

	GRIN_INFO[ElevatorObject] = CreateDynamicObject(18755, X_ELEVATOR_POS_GRIN, Y_ELEVATOR_POS_GRIN, GROUND_Z_COORD_GRIN + ELEVATOR_OFFSET_GRIN, 0.000000, 0.000000, 0.000000);
	GRIN_INFO[ElevatorDoors][0] = CreateDynamicObject(18757, X_ELEVATOR_POS_GRIN, Y_ELEVATOR_POS_GRIN, GROUND_Z_COORD_GRIN + ELEVATOR_OFFSET_GRIN, 0.000000, 0.000000, 0.000000);
	GRIN_INFO[ElevatorDoors][1]	= CreateDynamicObject(18756, X_ELEVATOR_POS_GRIN, Y_ELEVATOR_POS_GRIN, GROUND_Z_COORD_GRIN + ELEVATOR_OFFSET_GRIN, 0.000000, 0.000000, 0.000000);

	GRIN_INFO[ElevatorLabel] = CreateDynamic3DTextLabel(""COL_WHITE"Press '"COL_SERVER_INFO"~k~~CONVERSATION_YES~"COL_WHITE"' to use elevator", COLOR_SERVER, 1784.9822, -1302.0426, 13.6491, 4.0);

	new string[175],
		Float:z;

	for(new i; i < MAX_GRIN_FLOORS; i ++)
	{
		if(i != 1) GRIN_INFO[FloorRequested][i]=INVALID_PLAYER_ID;
	    GRIN_INFO[FloorDoors1][i] = CreateDynamicObject(18757, X_ELEVATOR_POS_GRIN - 0.245, Y_ELEVATOR_POS_GRIN, GetDoorsZCoordForFloorG(i), 0.000000, 0.000000, 0.000000);
		GRIN_INFO[FloorDoors2][i] = CreateDynamicObject(18756, X_ELEVATOR_POS_GRIN - 0.245, Y_ELEVATOR_POS_GRIN, GetDoorsZCoordForFloorG(i), 0.000000, 0.000000, 0.000000);
		
		if(i != 1) format(string, sizeof(string), ""COL_SERVER"[%s]\n"COL_WHITE"Press '"COL_SERVER_INFO"~k~~CONVERSATION_YES~"COL_WHITE"' to call\nCurrently: "COL_SERVER"%s", FloorNamesGRIN[i],FloorNamesGRIN[GRIN_INFO[ElevatorCurrentFloor]]);
		else format(string, sizeof(string), ""COL_SERVER"[%s]\n\nCurrently: "COL_SERVER"%s", FloorNamesGRIN[i],FloorNamesGRIN[GRIN_INFO[ElevatorCurrentFloor]]);
		
		z=GetDoorsZCoordForFloorG(i);

		GRIN_INFO[FloorLables][i] = CreateDynamic3DTextLabel(string, COLOR_SERVER, X_ELEVATOR_POS_GRIN - 2.5, Y_ELEVATOR_POS_GRIN - 2.5, z - 0.2, 10.5);
	}

	ContinueToNextFloorGRIN();
	return 1;
}

GRIN_Elevator_Destroy()
{
	DestroyObject(GRIN_INFO[ElevatorObject]);
	DestroyObject(GRIN_INFO[ElevatorDoors][0]);
	DestroyObject(GRIN_INFO[ElevatorDoors][1]);
	Delete3DTextLabel(GRIN_INFO[ElevatorLabel]);

	for(new i; i < MAX_GRIN_FLOORS; i ++)
	{
	    DestroyObject(GRIN_INFO[FloorDoors1][i]);
		DestroyObject(GRIN_INFO[FloorDoors2][i]);
		Delete3DTextLabel(GRIN_INFO[FloorLables][i]);
	}

	return 1;
}

GRINElevator_OpenDoors()
{
	// Opens the elevator's doors.

	new Float:x, Float:y, Float:z;

	GetDynamicObjectPos(GRIN_INFO[ElevatorDoors][0], x, y, z);
	MoveDynamicObject(GRIN_INFO[ElevatorDoors][0], x, Y_DOOR_L_OPENED_GRIN, z, DOORS_SPEED_GRIN);
	MoveDynamicObject(GRIN_INFO[ElevatorDoors][1], x, Y_DOOR_R_OPENED_GRIN, z, DOORS_SPEED_GRIN);

	return 1;
}

GRINElevator_CloseDoors()
{
    // Closes the elevator's doors.
	if(GRIN_INFO[ElevatorState] == ELEVATOR_STATE_MOVING)
	    return 0;
	new Float:x, Float:y, Float:z;

	GetDynamicObjectPos(GRIN_INFO[ElevatorDoors][0], x, y, z);
	MoveDynamicObject(GRIN_INFO[ElevatorDoors][0], x, Y_DOOR_CLOSED_GRIN, z, DOORS_SPEED_GRIN);
	MoveDynamicObject(GRIN_INFO[ElevatorDoors][1], x, Y_DOOR_CLOSED_GRIN, z, DOORS_SPEED_GRIN);

	return 1;
}

GRINFloor_OpenDoors(floorid)
{

    MoveDynamicObject(GRIN_INFO[FloorDoors1][floorid], X_ELEVATOR_POS_GRIN - 0.245, Y_DOOR_L_OPENED_GRIN, GetDoorsZCoordForFloorG(floorid), DOORS_SPEED_GRIN);
	MoveDynamicObject(GRIN_INFO[FloorDoors2][floorid], X_ELEVATOR_POS_GRIN - 0.245, Y_DOOR_R_OPENED_GRIN, GetDoorsZCoordForFloorG(floorid), DOORS_SPEED_GRIN);
	
	PlaySoundForPlayersInRange(6401, 50.0, X_ELEVATOR_POS_GRIN, Y_ELEVATOR_POS_GRIN, GetDoorsZCoordForFloorG(floorid) + 5.0);

	return 1;
}

GRINFloor_CloseDoors(floorid)
{
    // Closes the doors at the specified floor.

    MoveDynamicObject(GRIN_INFO[FloorDoors1][floorid], X_ELEVATOR_POS_GRIN - 0.245, Y_ELEVATOR_POS_GRIN, GetDoorsZCoordForFloorG(floorid), DOORS_SPEED_GRIN);
	MoveDynamicObject(GRIN_INFO[FloorDoors2][floorid], X_ELEVATOR_POS_GRIN - 0.245, Y_ELEVATOR_POS_GRIN, GetDoorsZCoordForFloorG(floorid), DOORS_SPEED_GRIN);
	
	PlaySoundForPlayersInRange(6401, 50.0, X_ELEVATOR_POS_GRIN, Y_ELEVATOR_POS_GRIN, GetDoorsZCoordForFloorG(floorid) + 5.0);

	return 1;
}

ContinueToNextFloorGRIN()
{
	// Moves the elevator to specified floor (doors are meant to be already closed).
	new cfloor=GRIN_INFO[ElevatorCurrentFloor];
	new floorid=1;
	GRIN_INFO[ElevatorState] = ELEVATOR_STATE_MOVING;
	if(GRIN_INFO[ElevatorMoving] == UP_WARDS)
	{
		UP:
		new down_;
		for(new i=cfloor+1; i<MAX_GRIN_FLOORS; i++)
		{
			if(IsFloorInQueue_GRIN(i)) 
			{
				down_=true;
				break;
			}
		}
		if(!down_) goto DOWN;
		for(new i=cfloor+1; i<MAX_GRIN_FLOORS; i++)
		{
			floorid=i;
			break;
		}
		if(floorid == 1 && cfloor > 1) goto DOWN;
	}
	else
	{
		DOWN:
		GRIN_INFO[ElevatorMoving] = DOWN_WARDS;
		for(new i=cfloor-1; i>-1; i--)
		{
			if(cfloor == 1)	
			{
				if(IsFloorInQueue_GRIN(i)) floorid=i;
				continue;
			}	
			else
				floorid=i;
			break;
		}
		if(floorid == 1 && cfloor == 1) 
		{
			GRIN_INFO[ElevatorMoving] = UP_WARDS;
			for(new i=2; i<MAX_GRIN_FLOORS; i++)
			{
				if(IsFloorInQueue_GRIN(i)) 
				{
					goto UP;
				}	
			}
			GRIN_INFO[ElevatorState] = ELEVATOR_STATE_IDLE;
		}
	}
	
	GRIN_INFO[ElevatorCurrentFloor] = floorid;
	if(GRIN_INFO[ElevatorState] != ELEVATOR_STATE_IDLE)
	{
		// Move the elevator slowly, to give time to clients to sync the object surfing. Then, boost it up:
		MoveDynamicObject(GRIN_INFO[ElevatorObject], X_ELEVATOR_POS_GRIN, Y_ELEVATOR_POS_GRIN, GetE_Z_C_Floor_G(floorid), 0.25);
		MoveDynamicObject(GRIN_INFO[ElevatorDoors][0], X_ELEVATOR_POS_GRIN, Y_ELEVATOR_POS_GRIN, GetDoorsZCoordForFloorG(floorid), 0.25);
		MoveDynamicObject(GRIN_INFO[ElevatorDoors][1], X_ELEVATOR_POS_GRIN, Y_ELEVATOR_POS_GRIN, GetDoorsZCoordForFloorG(floorid), 0.25);
		Delete3DTextLabel(GRIN_INFO[ElevatorLabel]);

		GRIN_INFO[ElevatorBoostTimer] = SetTimerEx("Elevator_Boost_GRIN", 2000, 0, "i", floorid);
	}	

	return 1;
}

FUNCTION Elevator_Boost_GRIN(floorid)
{
	// Increases the elevator's speed until it reaches 'floorid'
	StopDynamicObject(GRIN_INFO[ElevatorObject]);
	StopDynamicObject(GRIN_INFO[ElevatorDoors][0]);
	StopDynamicObject(GRIN_INFO[ElevatorDoors][1]);
	
	MoveDynamicObject(GRIN_INFO[ElevatorObject], X_ELEVATOR_POS_GRIN, Y_ELEVATOR_POS_GRIN, GetE_Z_C_Floor_G(floorid), ELEVATOR_SPEED_GRIN);
    MoveDynamicObject(GRIN_INFO[ElevatorDoors][0], X_ELEVATOR_POS_GRIN, Y_ELEVATOR_POS_GRIN, GetDoorsZCoordForFloorG(floorid), ELEVATOR_SPEED_GRIN);
    MoveDynamicObject(GRIN_INFO[ElevatorDoors][1], X_ELEVATOR_POS_GRIN, Y_ELEVATOR_POS_GRIN, GetDoorsZCoordForFloorG(floorid), ELEVATOR_SPEED_GRIN);

	return 1;
}

AddFloorToQueue_GRIN(playerid,floorid)
{
	if(GRIN_INFO[FloorRequested][floorid] != INVALID_PLAYER_ID) return 0;
	else 
	{
		GRIN_INFO[FloorRequested][floorid]=playerid;
		if(GRIN_INFO[ElevatorState] == ELEVATOR_STATE_IDLE)
	        CheckForNextGRIN();
		for(new i=0,j=GetPlayerPoolSize(); i<=j; i++)
		{
			if(IsPlayerConnected(i))
			{
				if(IsPlayerInGRINElevator(i)) ShowGRINElevaorMenu(i);
			}
		}		
		return 1;
	}
}

ResetGRINElevatorQueueGRIN()
{
	// Resets the queue.

	for(new i=0; i < MAX_GRIN_FLOORS; i ++)
	{
		if(i == 1) continue;
	    GRIN_INFO[FloorRequested][i] = INVALID_FLOOR;
	}
	return 1;
}

IsFloorInQueue_GRIN(floorid)
{
	if(GRIN_INFO[FloorRequested][floorid] == INVALID_PLAYER_ID) return 0;
	else return 1;
}

FUNCTION CheckForNextGRIN()
{
	new close;
	if(GRIN_INFO[ElevatorCurrentFloor] == 1)
	{
		for(new i=0; i<MAX_GRIN_FLOORS; i++)
		{
			if(i == 1) continue;
			if(IsFloorInQueue_GRIN(i)) 
			{
				close=true;
				break;
			}
		}
	}
	if(GRIN_INFO[ElevatorCurrentFloor] != 1) GRIN_INFO[FloorRequested][GRIN_INFO[ElevatorCurrentFloor]]=INVALID_PLAYER_ID;
	if(GRIN_INFO[ElevatorCurrentFloor] != 1 || close)
	{
		GRINElevator_CloseDoors();
		GRINFloor_CloseDoors(GRIN_INFO[ElevatorCurrentFloor]);
	}
	else
	{
		GRIN_INFO[ElevatorState] = ELEVATOR_STATE_IDLE;
	}
	return 1;
}

ShowGRINElevaorMenu(playerid)
{
	new string[50];
	HideTextDrawMenu(playerid);
	ShowTextDrawMenu(playerid, MENU_GRIN_TOWER,"~y~Golden Grin Tower", MAX_GRIN_FLOORS, MAX_GRIN_FLOORS-1);
	ShowTextDrawMenuItems(playerid, 0, "Select ~p~Floor~w~ to go", " ", " ",0);
	new num;
	for(new i=0; i < MAX_GRIN_FLOORS; i ++)
	{
		if(i == 1)
		{
			continue;
		}
		else num++;
		if(i != 10)format(string,sizeof(string),"~b~~h~%d ~w~- %s",num,FloorNamesGRIN[i]);
		else format(string,sizeof(string),"~b~~h~%d ~w~- %s ~p~(Robbery)",num,FloorNamesGRIN[i]);
	    if(GRIN_INFO[FloorRequested][i] != INVALID_PLAYER_ID)
			ShowTextDrawMenuItems(playerid, num+1, " ", string, "~r~Requested",0);
		else
			ShowTextDrawMenuItems(playerid, num+1, " ", string, " ",0);
	}
	return 1;
}

IsPlayerInGRINElevator(playerid)
{
	new Float:pos[3];
	GetPlayerPos(playerid, pos[0], pos[1], pos[2]);
	if(pos[1] > (Y_ELEVATOR_POS_GRIN - 1.8) && pos[1] < (Y_ELEVATOR_POS_GRIN + 1.8) && pos[0] < (X_ELEVATOR_POS_GRIN + 1.8) && pos[0] > (X_ELEVATOR_POS_GRIN - 1.8)) return 1;
	else return 0;
}

IsPlayerOnGRINFloor(playerid)
{
	new Float:pos[3];
	GetPlayerPos(playerid, pos[0], pos[1], pos[2]);
	if(pos[1] < (Y_ELEVATOR_POS_GRIN - 1.81) && pos[1] > (Y_ELEVATOR_POS_GRIN - 3.8) && pos[0] > (X_ELEVATOR_POS_GRIN - 3.8) && pos[0] < (X_ELEVATOR_POS_GRIN - 1.81))
	{
		new i = 10;
		
		// Loop
		while(pos[2] < GetDoorsZCoordForFloorG(i) + 3.5 && i > 0)
		    i --;	
		
		if(i == 0 && pos[2] < GetDoorsZCoordForFloorG(0) + 2.0)
		{
			i = -1;
		}
		   
		
		if(i+1 == 1) 
			i=-2;
		return i+1;	
	}
	return -1;
}

FloorChanged_GRIN(floorid)
{
	new string[175];
	format(string,sizeof(string),"~w~%s",FloorNamesGRIN[floorid]);
	for(new i=0,j=GetPlayerPoolSize(); i<=j; i++)
	{
		if(IsPlayerConnected(i))
		{
			if(IsPlayerInGRINElevator(i))
			{
				GameTextForPlayer(i,string,2000,3);
			}
		}
	}
	for(new i; i < MAX_GRIN_FLOORS; i ++)
	{
		if(i != 1) format(string, sizeof(string), ""COL_SERVER"[%s]\n"COL_WHITE"Press '"COL_SERVER_INFO"~k~~CONVERSATION_YES~"COL_WHITE"' to call\nCurrently: "COL_SERVER"%s", FloorNamesGRIN[i],FloorNamesGRIN[floorid]);
		else format(string, sizeof(string), ""COL_SERVER"[%s]\n\nCurrently: "COL_SERVER"%s", FloorNamesGRIN[i],FloorNamesGRIN[floorid]);
		UpdateDynamic3DTextLabelText(GRIN_INFO[FloorLables][i], COLOR_SERVER, string);
	}
}

forward Float:GetE_Z_C_Floor_G(floorid);
public Float:GetE_Z_C_Floor_G(floorid)
{
	new Float:addition=(GROUND_Z_COORD_GRIN + FloorZOffsetsGRIN[floorid] + ELEVATOR_OFFSET_GRIN);
	return addition;
}

forward Float:GetDoorsZCoordForFloorG(floorid);
public Float:GetDoorsZCoordForFloorG(floorid)
	return (GROUND_Z_COORD_GRIN + FloorZOffsetsGRIN[floorid] + ELEVATOR_OFFSET_GRIN);			