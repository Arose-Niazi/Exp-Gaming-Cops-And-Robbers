/*

	Players variables for Cops And Robbers script
	Scripted by Arose Niazi.
	Scripting started on 11th October 2017
	Last Updated: -----
	
	Credits:
		
	
	Functions:
		ClearPlayerVariables(playerid);
		
	Functions Being Called:
		
*/


enum pInfo
{
	pID,
	pUserName[MAX_PLAYER_NAME],
	pPassword[MAX_HASH_SIZE],
	pRegisteredDate[50],
	pEmail[MAX_EMAIL_SIZE],
	pCurrent_country[56],
	bool:pRegistered,
	bool:pLoggedIn,
	bool:pAutoLogin,
	pLoginAttempts,
	
	pRank,
	bool:pDj,
	bool:pVip,
	bool:pRegularPlayer,
	
	pMoney,
	
	pZone,
	pZoneName[MAX_ZONE_NAME],
	pExteriorName[MAX_ZONE_NAME],
	bool:pInPoliceProperty,
	pInterior,
	
	bool:pClassSelection,
	
	pClassID,
	pSkin,
	pTeam,
	pSkinsSelected[MAX_PEDS],
	pGender,
	pGenderChanges[3],
	
	bool:pSpawned,
	pSpawns,
	
	bool:pHourlyTimeUpdate,
	bool:pShowJoinMessages,
	bool:pClassMusic,
	
	pSerial[MAX_SERIAL_SIZE],
	pIP[MAX_IP_SIZE],
	p_AdminNick[MAX_PLAYER_NAME],
	
	pMonthlyActivity,
	
	pMenu,
	
	bool:pAddingInterior,
	pAddInteriorType,
	Float:pAddInteriorOSX,
	Float:pAddInteriorOSY,
	Float:pAddInteriorOSZ,
	Float:pAddInteriorOSA,
	Float:pAddInteriorISX,
	Float:pAddInteriorISY,
	Float:pAddInteriorISZ,
	Float:pAddInteriorISA,
	Float:pAddInteriorOPX,
	Float:pAddInteriorOPY,
	Float:pAddInteriorOPZ,
	Float:pAddInteriorIPX,
	Float:pAddInteriorIPY,
	Float:pAddInteriorIPZ,
	pAddInteriorInt,
	pAddInteriorOutZone[MAX_ZONE_NAME],
	pAddInteriorZone[MAX_ZONE_NAME],
	pAddInteriorVW,
	bool:pAddInteriorCivWeapons,
	bool:pAddInteriorCopWeapons,
	bool:pAddInteriorWeapons,
	pEditingIntID,
	pEditingInterior,
	bool:pAddInteriorLabels,
	
	pMenuTD_BG,
	
	bool:pAddingActor,
	bool:pEditingActor,
	pEditingActorID,
	pAddActorType,
	pAddActorInt,
	pAddActorVW,
	Float:pAddActorSpawnX,
	Float:pAddActorSpawnY,
	Float:pAddActorSpawnZ,
	Float:pAddActorSpawnA,
	pAddActorSkin,
	pAddActorLocation[MAX_ZONE_NAME],
	pAddActorAnimationType,
	bool:pAddActorLabelEnable,
	
	GPS_RouteID,
	GPS_Destination,
	GPS_CreatedObjects[MAX_NODS],
	GPS_CreatedMarkers[100],
	GPS_MakerLength,
	bool:GPS_Calculating,
	GPS_Lenght,
	GPS_LostCount,
	GPS_Nods[MAX_NODS],
	Float:GPS_NodsDistance[MAX_NODS],
	Float:GPS_Distance,
	pTimes_GPSUsed,
	bool:pDistanceInMiles,
	bool:pShowGPSDistance,
	
	pHideOutLocation,
	pMissionLocation,
	
	Float:pCustomMapMarker[3]
}

new PlayerInfo[MAX_PLAYERS][pInfo];


FUNCTION ClearPlayerVariables(playerid)
{
	PlayerInfo[playerid][pID]=0;
	strdel(PlayerInfo[playerid][pUserName],0,MAX_PLAYER_NAME);
	strdel(PlayerInfo[playerid][pPassword],0,MAX_HASH_SIZE);
	PlayerInfo[playerid][pRegistered]=PlayerInfo[playerid][pLoggedIn]=PlayerInfo[playerid][pDj]=PlayerInfo[playerid][pVip]=PlayerInfo[playerid][pRegularPlayer]=false;
	PlayerInfo[playerid][pLoginAttempts]=PlayerInfo[playerid][pRank]=PlayerInfo[playerid][pMoney]=0;
	strdel(PlayerInfo[playerid][pRegisteredDate],0,50);
	strdel(PlayerInfo[playerid][pEmail],0,MAX_EMAIL_SIZE);
	strdel(PlayerInfo[playerid][pCurrent_country],0,56);
	
	PlayerInfo[playerid][pZone]=-1;
	strdel(PlayerInfo[playerid][pZoneName],0,MAX_ZONE_NAME);
	strdel(PlayerInfo[playerid][pExteriorName],0,MAX_ZONE_NAME);
	PlayerInfo[playerid][pInPoliceProperty]=false;
	PlayerInfo[playerid][pInterior]=0;
	
	PlayerInfo[playerid][pClassSelection]=PlayerInfo[playerid][pDistanceInMiles]=false;
	
	PlayerInfo[playerid][pClassID]=-1;
	PlayerInfo[playerid][pSkin]=0;
	for(new i=0; i<MAX_PEDS; i++) PlayerInfo[playerid][pSkinsSelected][i]=0;
	PlayerInfo[playerid][pGender]=MALE;
	for(new i=0; i<3; i++) PlayerInfo[playerid][pGenderChanges][i]=0;
	
	PlayerInfo[playerid][pSpawns]=0;
	PlayerInfo[playerid][pAutoLogin]=PlayerInfo[playerid][pHourlyTimeUpdate]=PlayerInfo[playerid][pShowJoinMessages]=PlayerInfo[playerid][pClassMusic]=PlayerInfo[playerid][pShowGPSDistance]=true;
	
	strdel(PlayerInfo[playerid][pSerial],0,MAX_SERIAL_SIZE);
	strdel(PlayerInfo[playerid][p_AdminNick],0,MAX_PLAYER_NAME);
	strdel(PlayerInfo[playerid][pIP],0,MAX_IP_SIZE);
	PlayerInfo[playerid][pMonthlyActivity]=0;
	PlayerInfo[playerid][pTeam]=-1;
	PlayerInfo[playerid][pMenu]=NO_MENU;
	PlayerInfo[playerid][pAddInteriorType]=-1;
	PlayerInfo[playerid][pEditingInterior]=PlayerInfo[playerid][pAddInteriorCivWeapons]=PlayerInfo[playerid][pAddInteriorCopWeapons]=PlayerInfo[playerid][pAddInteriorWeapons]=PlayerInfo[playerid][pAddingInterior]=PlayerInfo[playerid][pAddInteriorLabels]=false;
	strdel(PlayerInfo[playerid][pAddInteriorZone],0,MAX_ZONE_NAME);
	strdel(PlayerInfo[playerid][pAddInteriorOutZone],0,MAX_ZONE_NAME);
	PlayerInfo[playerid][pAddInteriorOSX]=PlayerInfo[playerid][pAddInteriorOSY]=PlayerInfo[playerid][pAddInteriorOSZ]= PlayerInfo[playerid][pAddInteriorOSA]=PlayerInfo[playerid][pAddInteriorISX]=PlayerInfo[playerid][pAddInteriorISY]=
	PlayerInfo[playerid][pAddInteriorISZ]=PlayerInfo[playerid][pAddInteriorISA]=PlayerInfo[playerid][pAddInteriorOPX]=PlayerInfo[playerid][pAddInteriorOPY]=PlayerInfo[playerid][pAddInteriorOPZ]=PlayerInfo[playerid][pAddInteriorIPX]=PlayerInfo[playerid][pAddInteriorIPY]=PlayerInfo[playerid][pAddInteriorIPZ]=
	PlayerInfo[playerid][pCustomMapMarker][0]=PlayerInfo[playerid][pCustomMapMarker][1]=PlayerInfo[playerid][pCustomMapMarker][2]=0.0;
	PlayerInfo[playerid][pAddInteriorInt]=PlayerInfo[playerid][pAddInteriorVW]=PlayerInfo[playerid][pEditingIntID]=0;
	PlayerInfo[playerid][pMenuTD_BG]=4294967125;
	
	PlayerInfo[playerid][pAddingActor]= PlayerInfo[playerid][pEditingActor]=PlayerInfo[playerid][pAddActorLabelEnable]=false;
	PlayerInfo[playerid][pEditingActorID]=PlayerInfo[playerid][pAddActorType]=PlayerInfo[playerid][pAddActorInt]=PlayerInfo[playerid][pAddActorVW]=PlayerInfo[playerid][pAddActorSkin]=PlayerInfo[playerid][pAddActorAnimationType]=0;
	PlayerInfo[playerid][pAddActorSpawnX]=PlayerInfo[playerid][pAddActorSpawnY]=PlayerInfo[playerid][pAddActorSpawnZ]=PlayerInfo[playerid][pAddActorSpawnA]=0.0;
	strdel(PlayerInfo[playerid][pAddActorLocation],0,MAX_ZONE_NAME);
	
	PlayerInfo[playerid][pHideOutLocation]=PlayerInfo[playerid][pMissionLocation]=-1;
	
	PlayerInfo[playerid][GPS_RouteID]=PlayerInfo[playerid][GPS_Destination]=-1;
	for(new i=0; i<MAX_NODS; i++)
	{
		PlayerInfo[playerid][GPS_CreatedObjects][i]=INVALID_OBJECT_ID;
		PlayerInfo[playerid][GPS_Nods][i]=-1;
		PlayerInfo[playerid][GPS_NodsDistance][i]=0.0;
	}
	for(new i=0; i<100; i++)
	{
		PlayerInfo[playerid][GPS_CreatedMarkers][i]=-1;
	}
	PlayerInfo[playerid][GPS_Calculating]=false;
	PlayerInfo[playerid][GPS_Lenght]=PlayerInfo[playerid][GPS_LostCount]=PlayerInfo[playerid][pTimes_GPSUsed]=PlayerInfo[playerid][GPS_MakerLength]=0;
	PlayerInfo[playerid][GPS_Distance]=0.0;
	
	
	
	return 1;
}	