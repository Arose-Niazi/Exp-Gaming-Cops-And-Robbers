/*

	Elevators Script
	Scripted by Arose Niazi.
	Scripting started on 22nd October 2017
	Last Updated: -----
	
	Credits:
		Zamaroht for basic elevator working
		Kye sound effects
		
	Functions:
		OnSAMPElevatorMoved(objectid)
		OnSampElevatorMenu(playerid, Option)
		Check_SAMP_Elevator(playerid)
		SAMP_Elevator_Initialize()
		SAMP_Elevator_Destroy()
		SAMPElevator_OpenDoors()
		SAMPElevator_CloseDoors()
		SAMPFloor_OpenDoors(floorid)
		SAMPFloor_CloseDoors(floorid)
		ContinueToNextFloorSAMP()
		Elevator_Boost_SAMP(floorid)
		AddFloorToQueue_SAMP(playerid,floorid)
		ResetSAMPElevatorQueueSAMP()
		IsFloorInQueue_SAMP(floorid)
		CheckForNext()
		ShowSAMPElevaorMenu(playerid)
		IsPlayerInSAMPElevator(playerid)
		IsPlayerOnSAMPFloor(playerid)
		FloorChanged_SAMP(floorid)
		forward Float:GetE_Z_C_Floor_S(floorid);
		forward Float:GetDoorsZCoordForFloor(floorid);
		
	Functions Being Called:
		
*/

#define INVALID_FLOOR           (-1)

#define ELEVATOR_SPEED_SAMP      (5.0)
#define DOORS_SPEED_SAMP         (5.0)   
#define ELEVATOR_WAIT_TIME_SAMP  (5000)  

#define X_DOOR_CLOSED_SAMP       (1786.627685)
#define X_DOOR_R_OPENED_SAMP     (1785.027685)
#define X_DOOR_L_OPENED_SAMP     (1788.227685)
#define GROUND_Z_COORD_SAMP      (14.511476)
#define ELEVATOR_OFFSET_SAMP     (0.059523)


static FloorNamesSAMP[21][] =
{
	"Ground Floor",
	"First Floor",
	"Second Floor",
	"Third Floor",
	"Fourth Floor",
	"Fifth Floor",
	"Sixth Floor",
	"Seventh Floor",
	"Eighth Floor",
	"Ninth Floor",
	"Tenth Floor",
	"Eleventh Floor",
	"Twelfth Floor",
	"Thirteenth Floor",
	"Fourteenth Floor",
	"Fifteenth Floor",
	"Sixteenth Floor",
	"Seventeenth Floor",
	"Eighteenth Floor",
	"Nineteenth Floor",
	"Penthouse"
};	

static Float:FloorZOffsetsSAMP[21] =
{
    0.0,		// 0.0,
    8.5479,		// 8.5479,
    13.99945,   // 8.5479 + (5.45155 * 1.0),
    19.45100,   // 8.5479 + (5.45155 * 2.0),
    24.90255,   // 8.5479 + (5.45155 * 3.0),
    30.35410,   // 8.5479 + (5.45155 * 4.0),
    35.80565,   // 8.5479 + (5.45155 * 5.0),
    41.25720,   // 8.5479 + (5.45155 * 6.0),
    46.70875,   // 8.5479 + (5.45155 * 7.0),
    52.16030,   // 8.5479 + (5.45155 * 8.0),
    57.61185,   // 8.5479 + (5.45155 * 9.0),
    63.06340,   // 8.5479 + (5.45155 * 10.0),
    68.51495,   // 8.5479 + (5.45155 * 11.0),
    73.96650,   // 8.5479 + (5.45155 * 12.0),
    79.41805,   // 8.5479 + (5.45155 * 13.0),
    84.86960,   // 8.5479 + (5.45155 * 14.0),
    90.32115,   // 8.5479 + (5.45155 * 15.0),
    95.77270,   // 8.5479 + (5.45155 * 16.0),
    101.22425,  // 8.5479 + (5.45155 * 17.0),
    106.67580,	// 8.5479 + (5.45155 * 18.0),
    112.12735	// 8.5479 + (5.45155 * 19.0)
};	

#define MAX_SAMP_FLOORS 21

enum elevator_enum
{
	ElevatorObject,
	ElevatorDoors[2],
	FloorDoors1[MAX_SAMP_FLOORS],
	FloorDoors2[MAX_SAMP_FLOORS],
	Text3D:ElevatorLabel,
	Text3D:FloorLables[MAX_SAMP_FLOORS],
	ElevatorState, //Idel,Waiting,Moving
	ElevatorMoving, //Up or Down
	ElevatorCurrentFloor,
	FloorRequested[MAX_SAMP_FLOORS],
	ElevatorBoostTimer
};

new SAMP_INFO[elevator_enum];

OnSAMPElevatorMoved(objectid)
{
    new Float:x, Float:y, Float:z;
	for(new i; i < MAX_SAMP_FLOORS; i ++)
	{
		if(objectid == SAMP_INFO[FloorDoors1][i])
		{
		    GetDynamicObjectPos(SAMP_INFO[FloorDoors1][i], x, y, z);

		    if(x < X_DOOR_L_OPENED_SAMP - 0.5) 
		    {
				ContinueToNextFloorSAMP();
			}
		}
	}
	if(objectid == SAMP_INFO[ElevatorObject])   // The elevator reached the specified floor.
	{
		FloorChanged_SAMP(SAMP_INFO[ElevatorCurrentFloor]);
		if(IsFloorInQueue_SAMP(SAMP_INFO[ElevatorCurrentFloor]))
		{
			KillTimer(SAMP_INFO[ElevatorBoostTimer]);  // Kills the timer, in case the elevator reached the floor before boost.
			SAMPElevator_OpenDoors();
			SAMPFloor_OpenDoors(SAMP_INFO[ElevatorCurrentFloor]);
			
			GetDynamicObjectPos(SAMP_INFO[ElevatorObject], x, y, z);
			DestroyDynamic3DTextLabel(SAMP_INFO[ElevatorLabel]);
			SAMP_INFO[ElevatorLabel] = CreateDynamic3DTextLabel(""COL_WHITE"Press '"COL_SERVER_INFO"~k~~CONVERSATION_YES~"COL_WHITE"' to use elevator", COLOR_SERVER, 1784.9822, -1302.0426, z - 0.9, 4.0);

			SAMP_INFO[ElevatorState] 	= ELEVATOR_STATE_WAITING;
			SetTimer("CheckForNext", ELEVATOR_WAIT_TIME_SAMP, false);
		}
		else ContinueToNextFloorSAMP();
	}

	return 1;
}

OnSampElevatorMenu(playerid, Option)
{
    if(SAMP_INFO[FloorRequested][Option] != INVALID_PLAYER_ID)
	{
		ShowSAMPElevaorMenu(playerid);
		SendClientMessage(playerid, COLOR_ERROR,"Elevator already called on that floor.");
	}
	else
	{
		if(IsPlayerInSAMPElevator(playerid)) 
		{
			AddFloorToQueue_SAMP(playerid, Option);
			ShowSAMPElevaorMenu(playerid);
		}
		else
			SendClientMessage(playerid, COLOR_ERROR,"You are not inside the elevator.");
	}
	
	return 1;
}

Check_SAMP_Elevator(playerid)
{
	if(IsPlayerInSAMPElevator(playerid))  
	    ShowSAMPElevaorMenu(playerid);
	else  
	{
		new floor=IsPlayerOnSAMPFloor(playerid);
	    if(floor)
	    {
			if(floor <= 20)
			{
				
				if(!IsFloorInQueue_SAMP(floor))
				{
					AddFloorToQueue_SAMP(playerid, floor);
					GameTextForPlayer(playerid, "~w~Elevator called", 2000, 3);
				}
				else
				{
					GameTextForPlayer(playerid, "~r~Elevator Already Called", 2000, 3);
				}
			}
	    }
	}
	return 1;
}


SAMP_Elevator_Initialize()
{
	// Initializes the elevator.

	SAMP_INFO[ElevatorObject] = CreateDynamicObject(18755, 1786.678100, -1303.459472, GROUND_Z_COORD_SAMP + ELEVATOR_OFFSET_SAMP, 0.000000, 0.000000, 270.000000);
	SAMP_INFO[ElevatorDoors][0] = CreateDynamicObject(18757, X_DOOR_CLOSED_SAMP, -1303.459472, GROUND_Z_COORD_SAMP, 0.000000, 0.000000, 270.000000);
	SAMP_INFO[ElevatorDoors][1]	= CreateDynamicObject(18756, X_DOOR_CLOSED_SAMP, -1303.459472, GROUND_Z_COORD_SAMP, 0.000000, 0.000000, 270.000000);

	SAMP_INFO[ElevatorLabel] = CreateDynamic3DTextLabel(""COL_WHITE"Press '"COL_SERVER_INFO"~k~~CONVERSATION_YES~"COL_WHITE"' to use elevator", COLOR_SERVER, 1784.9822, -1302.0426, 13.6491, 4.0);

	new string[175],
		Float:z;

	for(new i; i < MAX_SAMP_FLOORS; i ++)
	{
		if(i != 0) SAMP_INFO[FloorRequested][i]=INVALID_PLAYER_ID;
	    SAMP_INFO[FloorDoors1][i] = CreateDynamicObject(18757, X_DOOR_CLOSED_SAMP, -1303.171142, GetDoorsZCoordForFloor(i), 0.000000, 0.000000, 270.000000);
		SAMP_INFO[FloorDoors2][i] = CreateDynamicObject(18756, X_DOOR_CLOSED_SAMP, -1303.171142, GetDoorsZCoordForFloor(i), 0.000000, 0.000000, 270.000000);
		
		if(i != 0) format(string, sizeof(string), ""COL_SERVER"[%s]\n"COL_WHITE"Press '"COL_SERVER_INFO"~k~~CONVERSATION_YES~"COL_WHITE"' to call\nCurrently: "COL_SERVER"%s", FloorNamesSAMP[i],FloorNamesSAMP[SAMP_INFO[ElevatorCurrentFloor]]);
		else format(string, sizeof(string), ""COL_SERVER"[%s]\n\nCurrently: "COL_SERVER"%s", FloorNamesSAMP[i],FloorNamesSAMP[SAMP_INFO[ElevatorCurrentFloor]]);
		
		if(i == 0)
		    z = 13.4713;
		else
		    z = 13.4713 + 8.7396 + ((i-1) * 5.45155);

		SAMP_INFO[FloorLables][i] = CreateDynamic3DTextLabel(string, COLOR_SERVER, 1783.9799, -1300.7660, z, 10.5);
	}

	// Open ground floor doors:
	SAMPFloor_OpenDoors(0);
	SAMPElevator_OpenDoors();
	return 1;
}

SAMP_Elevator_Destroy()
{
	DestroyObject(SAMP_INFO[ElevatorObject]);
	DestroyObject(SAMP_INFO[ElevatorDoors][0]);
	DestroyObject(SAMP_INFO[ElevatorDoors][1]);
	Delete3DTextLabel(SAMP_INFO[ElevatorLabel]);

	for(new i; i < MAX_SAMP_FLOORS; i ++)
	{
	    DestroyObject(SAMP_INFO[FloorDoors1][i]);
		DestroyObject(SAMP_INFO[FloorDoors2][i]);
		Delete3DTextLabel(SAMP_INFO[FloorLables][i]);
	}

	return 1;
}

SAMPElevator_OpenDoors()
{
	// Opens the elevator's doors.

	new Float:x, Float:y, Float:z;

	GetDynamicObjectPos(SAMP_INFO[ElevatorDoors][0], x, y, z);
	MoveDynamicObject(SAMP_INFO[ElevatorDoors][0], X_DOOR_L_OPENED_SAMP, y, z, DOORS_SPEED_SAMP);
	MoveDynamicObject(SAMP_INFO[ElevatorDoors][1], X_DOOR_R_OPENED_SAMP, y, z, DOORS_SPEED_SAMP);

	return 1;
}

SAMPElevator_CloseDoors()
{
    // Closes the elevator's doors.
	if(SAMP_INFO[ElevatorState] == ELEVATOR_STATE_MOVING)
	    return 0;
	new Float:x, Float:y, Float:z;

	GetDynamicObjectPos(SAMP_INFO[ElevatorDoors][0], x, y, z);
	MoveDynamicObject(SAMP_INFO[ElevatorDoors][0], X_DOOR_CLOSED_SAMP, y, z, DOORS_SPEED_SAMP);
	MoveDynamicObject(SAMP_INFO[ElevatorDoors][1], X_DOOR_CLOSED_SAMP, y, z, DOORS_SPEED_SAMP);

	return 1;
}

SAMPFloor_OpenDoors(floorid)
{

    MoveDynamicObject(SAMP_INFO[FloorDoors1][floorid], X_DOOR_L_OPENED_SAMP, -1303.171142, GetDoorsZCoordForFloor(floorid), DOORS_SPEED_SAMP);
	MoveDynamicObject(SAMP_INFO[FloorDoors2][floorid], X_DOOR_R_OPENED_SAMP, -1303.171142, GetDoorsZCoordForFloor(floorid), DOORS_SPEED_SAMP);
	
	PlaySoundForPlayersInRange(6401, 50.0, X_DOOR_CLOSED_SAMP, -1303.171142, GetDoorsZCoordForFloor(floorid) + 5.0);

	return 1;
}

SAMPFloor_CloseDoors(floorid)
{
    // Closes the doors at the specified floor.

    MoveDynamicObject(SAMP_INFO[FloorDoors1][floorid], X_DOOR_CLOSED_SAMP, -1303.171142, GetDoorsZCoordForFloor(floorid), DOORS_SPEED_SAMP);
	MoveDynamicObject(SAMP_INFO[FloorDoors2][floorid], X_DOOR_CLOSED_SAMP, -1303.171142, GetDoorsZCoordForFloor(floorid), DOORS_SPEED_SAMP);
	
	PlaySoundForPlayersInRange(6401, 50.0, X_DOOR_CLOSED_SAMP, -1303.171142, GetDoorsZCoordForFloor(floorid) + 5.0);

	return 1;
}

ContinueToNextFloorSAMP()
{
	// Moves the elevator to specified floor (doors are meant to be already closed).
	new cfloor=SAMP_INFO[ElevatorCurrentFloor];
	new floorid;
	SAMP_INFO[ElevatorState] = ELEVATOR_STATE_MOVING;
	if(SAMP_INFO[ElevatorMoving] == UP_WARDS)
	{
		UP:
		new down_;
		for(new i=cfloor+1; i<MAX_SAMP_FLOORS; i++)
		{
			if(IsFloorInQueue_SAMP(i)) 
			{
				down_=true;
				break;
			}
		}
		if(!down_) goto DOWN;
		for(new i=cfloor+1; i<MAX_SAMP_FLOORS; i++)
		{
			floorid=i;
			break;
		}
		if(floorid == 0 && cfloor != 0) goto DOWN;
	}
	else
	{
		DOWN:
		SAMP_INFO[ElevatorMoving] = DOWN_WARDS;
		for(new i=cfloor-1; i>0; i--)
		{
			floorid=i;
			break;
		}
		if(floorid == 0 && cfloor == 0) 
		{
			SAMP_INFO[ElevatorMoving] = UP_WARDS;
			for(new i=1; i<MAX_SAMP_FLOORS; i++)
			{
				if(IsFloorInQueue_SAMP(i)) 
				{
					goto UP;
				}	
			}
			SAMP_INFO[ElevatorState] = ELEVATOR_STATE_IDLE;
		}
	}
	
	SAMP_INFO[ElevatorCurrentFloor] = floorid;
	if(SAMP_INFO[ElevatorState] != ELEVATOR_STATE_IDLE)
	{
		// Move the elevator slowly, to give time to clients to sync the object surfing. Then, boost it up:
		MoveDynamicObject(SAMP_INFO[ElevatorObject], 1786.678100, -1303.459472, GetE_Z_C_Floor_S(floorid), 0.25);
		MoveDynamicObject(SAMP_INFO[ElevatorDoors][0], X_DOOR_CLOSED_SAMP, -1303.459472, GetDoorsZCoordForFloor(floorid), 0.25);
		MoveDynamicObject(SAMP_INFO[ElevatorDoors][1], X_DOOR_CLOSED_SAMP, -1303.459472, GetDoorsZCoordForFloor(floorid), 0.25);
		Delete3DTextLabel(SAMP_INFO[ElevatorLabel]);

		SAMP_INFO[ElevatorBoostTimer] = SetTimerEx("Elevator_Boost_SAMP", 2000, 0, "i", floorid);
	}	

	return 1;
}

FUNCTION Elevator_Boost_SAMP(floorid)
{
	// Increases the elevator's speed until it reaches 'floorid'
	StopDynamicObject(SAMP_INFO[ElevatorObject]);
	StopDynamicObject(SAMP_INFO[ElevatorDoors][0]);
	StopDynamicObject(SAMP_INFO[ElevatorDoors][1]);
	
	MoveDynamicObject(SAMP_INFO[ElevatorObject], 1786.678100, -1303.459472, GetE_Z_C_Floor_S(floorid), ELEVATOR_SPEED_SAMP);
    MoveDynamicObject(SAMP_INFO[ElevatorDoors][0], X_DOOR_CLOSED_SAMP, -1303.459472, GetDoorsZCoordForFloor(floorid), ELEVATOR_SPEED_SAMP);
    MoveDynamicObject(SAMP_INFO[ElevatorDoors][1], X_DOOR_CLOSED_SAMP, -1303.459472, GetDoorsZCoordForFloor(floorid), ELEVATOR_SPEED_SAMP);

	return 1;
}

AddFloorToQueue_SAMP(playerid,floorid)
{
	if(SAMP_INFO[FloorRequested][floorid] != INVALID_PLAYER_ID) return 0;
	else 
	{
		SAMP_INFO[FloorRequested][floorid]=playerid;
		if(SAMP_INFO[ElevatorState] == ELEVATOR_STATE_IDLE)
	        CheckForNext();
		for(new i=0,j=GetPlayerPoolSize(); i<=j; i++)
		{
			if(IsPlayerConnected(i))
			{
				if(IsPlayerInSAMPElevator(i)) ShowSAMPElevaorMenu(i);
			}
		}		
		return 1;
	}
}

ResetSAMPElevatorQueueSAMP()
{
	// Resets the queue.

	for(new i=1; i < MAX_SAMP_FLOORS; i ++)
	{
	    SAMP_INFO[FloorRequested][i] = INVALID_FLOOR;
	}
	return 1;
}

IsFloorInQueue_SAMP(floorid)
{
	if(SAMP_INFO[FloorRequested][floorid] == INVALID_PLAYER_ID) return 0;
	else return 1;
}

FUNCTION CheckForNext()
{
	new close;
	if(SAMP_INFO[ElevatorCurrentFloor] == 0)
	{
		for(new i=1; i<MAX_SAMP_FLOORS; i++)
		{
			if(IsFloorInQueue_SAMP(i)) 
			{
				close=true;
				break;
			}
		}
	}
	if(SAMP_INFO[ElevatorCurrentFloor] != 0)SAMP_INFO[FloorRequested][SAMP_INFO[ElevatorCurrentFloor]]=INVALID_PLAYER_ID;
	if(SAMP_INFO[ElevatorCurrentFloor] != 0 || close)
	{
		SAMPElevator_CloseDoors();
		SAMPFloor_CloseDoors(SAMP_INFO[ElevatorCurrentFloor]);
	}
	else
	{
		SAMP_INFO[ElevatorState] = ELEVATOR_STATE_IDLE;
	}
	return 1;
}

ShowSAMPElevaorMenu(playerid)
{
	new string[50];
	HideTextDrawMenu(playerid);
	ShowTextDrawMenu(playerid, MENU_SAMP_TOWER,"~y~SA-MP Tower", MAX_SAMP_FLOORS+1, MAX_SAMP_FLOORS);
	ShowTextDrawMenuItems(playerid, 0, "Select ~p~Floor~w~ to go", " ", " ",0);
	for(new i=1; i < MAX_SAMP_FLOORS; i ++)
	{
		if(i != 10)format(string,sizeof(string),"~b~~h~%d ~w~- %s",i,FloorNamesSAMP[i]);
		else format(string,sizeof(string),"~b~~h~%d ~w~- %s ~p~(Robbery)",i,FloorNamesSAMP[i]);
	    if(SAMP_INFO[FloorRequested][i] != INVALID_PLAYER_ID)
			ShowTextDrawMenuItems(playerid, i+1, " ", string, "~r~Requested",0);
		else
			ShowTextDrawMenuItems(playerid, i+1, " ", string, " ",0);
	}
	return 1;
}

IsPlayerInSAMPElevator(playerid)
{
	new Float:pos[3];
	GetPlayerPos(playerid, pos[0], pos[1], pos[2]);
	if(pos[1] < -1301.4 && pos[1] > -1305.2445 && pos[0] < 1788.5630 && pos[0] > 1784.1555) return 1;
	else return 0;
}

IsPlayerOnSAMPFloor(playerid)
{
	new Float:pos[3];
	GetPlayerPos(playerid, pos[0], pos[1], pos[2]);
	if(pos[1] > -1301.4 && pos[1] < -1299.1447 && pos[0] < 1785.6147 && pos[0] > 1781.9902)
	{
		new i=20;
		while(pos[2] < GetDoorsZCoordForFloor(i) + 3.5 && i > 0)
		    i --;

		if(i == 0 && pos[2] < GetDoorsZCoordForFloor(0) + 2.0)
		    i = -1;
		return i+1;	
	}
	return 0;
}

FloorChanged_SAMP(floorid)
{
	new string[175];
	format(string,sizeof(string),"~w~%s",FloorNamesSAMP[floorid]);
	for(new i=0,j=GetPlayerPoolSize(); i<=j; i++)
	{
		if(IsPlayerConnected(i))
		{
			if(IsPlayerInSAMPElevator(i))
			{
				GameTextForPlayer(i,string,2000,3);
			}
		}
	}
	for(new i; i < MAX_SAMP_FLOORS; i ++)
	{
		if(i != 0) format(string, sizeof(string), ""COL_SERVER"[%s]\n"COL_WHITE"Press '"COL_SERVER_INFO"~k~~CONVERSATION_YES~"COL_WHITE"' to call\nCurrently: "COL_SERVER"%s", FloorNamesSAMP[i],FloorNamesSAMP[floorid]);
		else format(string, sizeof(string), ""COL_SERVER"[%s]\n\nCurrently: "COL_SERVER"%s", FloorNamesSAMP[i],FloorNamesSAMP[floorid]);
		UpdateDynamic3DTextLabelText(SAMP_INFO[FloorLables][i], COLOR_SERVER, string);
	}
}

forward Float:GetE_Z_C_Floor_S(floorid);
public Float:GetE_Z_C_Floor_S(floorid)
{
	new Float:addition=(GROUND_Z_COORD_SAMP + FloorZOffsetsSAMP[floorid] + ELEVATOR_OFFSET_SAMP);
	return addition;
}

forward Float:GetDoorsZCoordForFloor(floorid);
public Float:GetDoorsZCoordForFloor(floorid)
	return (GROUND_Z_COORD_SAMP + FloorZOffsetsSAMP[floorid]);			