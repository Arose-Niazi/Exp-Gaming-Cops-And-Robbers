/*

	Peds Script
	Scripted by Sasuke_Uchiha.
	Scripting started on 13th October 2017
	Last Updated: 17/10/2017
	
	Credits:
		ThecnoPrisoner for making list of skins.
		
	Functions:
		SetupPeds();
		CreateClassTD(playerid);
		DestroyClassTD(playerid);
		ShowPlayerClassTD(playerid,classid);
		HidePlayerClassTD(playerid);
		
	Functions Being Called:
		
*/

#if CITY_ == LS
#define CIVIL_POS 1523.9183,-1359.0746,330.0483
#define CIVIL_CAMERA 1532.1737,-1357.6241,334.3370
#define CIVIL_FACING 274.2386

#define POLICE_POS 1552.7271,-1675.4519,16.1953
#define POLICE_CAMERA 1540.2898,-1675.4633,19.8122
#define POLICE_FACING 91.4140

#define SHERIFF_POS 626.4219,-552.3862,16.8333
#define SHERIFF_CAMERA 634.6553,-552.2580,17.1006
#define SHERIFF_FACING 277.6387

#define FBI_POS 1468.1600,-1454.0551,41.7790
#define FBI_CAMERA 1465.0499,-1450.6631,41.8452
#define FBI_FACING 44.2217
#elseif CITY_ == SF
#define CIVIL_POS
#define CIVIL_CAMERA
#define CIVIL_FACING

#define POLICE_POS
#define POLICE_CAMERA
#define POLICE_FACING

#define SHERIFF_POS
#define SHERIFF_CAMERA
#define SHERIFF_FACING

#define FBI_POS
#define FBI_CAMERA
#define FBI_FACING
#else
#define CIVIL_POS
#define CIVIL_CAMERA
#define CIVIL_FACING

#define POLICE_POS
#define POLICE_CAMERA
#define POLICE_FACING

#define SHERIFF_POS
#define SHERIFF_CAMERA
#define SHERIFF_FACING

#define FBI_POS
#define FBI_CAMERA
#define FBI_FACING
#endif

new PlayerText:PlayerClassSelection;

enum peds_data
{
	PedSkinID,
	PedName[MAX_PED_NAME],
	PedTeam,
	PedGender
};

new PedsInfo[MAX_PEDS][peds_data] = {
	{ 1,    "Herb Dealer",    CIVIL,MALE},
	{ 6,    "Rapist",    CIVIL,MALE},
	{ 7,    "Train Driver",    CIVIL,MALE},
	{ 8,    "Janitor",    CIVIL,MALE},
	{ 10,    "Granny",    CIVIL,FEMALE},
	{ 11,    "Casino Worker",    CIVIL,FEMALE},
	{ 15,    "Truck Driver",    CIVIL,MALE},
	{ 17,    "Lawyer",    CIVIL,MALE},
	{ 18,    "Bather",    CIVIL,MALE},
	{ 19,    "Freestyler",    CIVIL,MALE},
	{ 20,    "Pedestrian",    CIVIL,MALE},
	{ 21,    "Pedestrian",    CIVIL,MALE},
	{ 23,    "BMXer",    CIVIL,MALE},
	{ 24,    "Bodyguard",    CIVIL,MALE},
	{ 25,    "Bodyguard",    CIVIL,MALE},
	{ 26,    "Camper",    CIVIL,MALE},
	{ 27,    "Carpenter",    CIVIL,MALE},
	{ 28,    "Weapon Dealer",    CIVIL,MALE},
	{ 29,    "Drug Dealer",    CIVIL,MALE},
	{ 30,    "Drug Trafficker",    CIVIL,MALE},
	{ 31,    "Farmer",    CIVIL,FEMALE},
	{ 32,    "Pirate",    CIVIL,MALE},
	{ 33,    "Vampire Hunter",    CIVIL,MALE},
	{ 34,    "Cowboy",    CIVIL,MALE},
	{ 35,    "Gardener",    CIVIL,MALE},
	{ 36,    "Golfer",    CIVIL,MALE},
	{ 37,    "Golfer",    CIVIL,MALE},
	{ 39,    "Granny",    CIVIL,FEMALE},
	{ 40,    "Pedestrian",    CIVIL,FEMALE},
	{ 41,    "Pedestrian",    CIVIL,FEMALE},
	{ 42,    "Mechanic",    CIVIL,MALE},
	{ 43,    "Pedophile",    CIVIL,MALE},
	{ 44,    "Pedophile",    CIVIL,MALE},
	{ 45,    "Bather",    CIVIL,MALE},
	{ 46,    "Mafia Member",    CIVIL,MALE},
	{ 47,    "Drug Trafficker",    CIVIL,MALE},
	{ 48,    "Blind Man",    CIVIL,MALE},
	{ 49,    "Karate Master",    CIVIL,MALE},
	{ 50,    "Mechanic",    CIVIL,MALE},
	{ 51,    "Mountain Biker",    CIVIL,MALE},
	{ 52,    "Mountain Biker",    CIVIL,MALE},
	{ 55,    "Blind Girlr",    CIVIL,FEMALE},
	{ 56,    "Pedestrian",    CIVIL,FEMALE},
	{ 57,    "Lawyer",    CIVIL,MALE},
	{ 58,    "Pedestrian",    CIVIL,MALE},
	{ 59,    "Pedestrian",    CIVIL,MALE},
	{ 60,    "Pedestrian",    CIVIL,MALE},
	{ 61,    "Pilot",    CIVIL,MALE},
	{ 62,    "Granddad",    CIVIL,MALE},
	{ 63,    "Prostitute",    CIVIL,FEMALE},
	{ 64,    "Prostitute",    CIVIL,FEMALE},
	{ 66,    "Pool Player",    CIVIL,MALE},
	{ 67,    "Pool Player",    CIVIL,MALE},
	{ 68,    "Preacher",    CIVIL,MALE},
	{ 69,    "Pedestrian",    CIVIL,FEMALE},
	{ 70,    "Scientist",    CIVIL,MALE},
	{ 71,    "Security Guard",    CIVIL,MALE},
	{ 72,    "Hippy",    CIVIL,MALE},
	{ 73,    "Hippy",    CIVIL,MALE},
	{ 75,    "Prostitute",    CIVIL,FEMALE},
	{ 76,    "Lawyer",    CIVIL,FEMALE},
	{ 77,    "Homeless",    CIVIL,FEMALE},
	{ 78,    "Homeless",    CIVIL,MALE},
	{ 79,    "Homeless",    CIVIL,MALE},
	{ 80,    "Boxer",    CIVIL,MALE},
	{ 81,    "Boxer",    CIVIL,MALE},
	{ 82,    "Elvis",    CIVIL,MALE},
	{ 83,    "Elvis",    CIVIL,MALE},
	{ 84,    "Elvis",    CIVIL,MALE},
	{ 85,    "Prostitute",    CIVIL,FEMALE},
	{ 87,    "Stripper",    CIVIL,FEMALE},
	{ 90,    "Hippy",    CIVIL,FEMALE},
	{ 91,    "Pedestrian",    CIVIL,FEMALE},
	{ 92,    "Rollskater",    CIVIL,FEMALE},
	{ 93,    "Pedestrian",    CIVIL,FEMALE},
	{ 94,    "Pedestrian",    CIVIL,MALE},
	{ 95,    "Gas Station Attendant",    CIVIL,MALE},
	{ 97,    "Lifeguard",    CIVIL,MALE},
	{ 98,    "Mafia Member",    CIVIL,MALE},
	{ 99,    "Rollskater",    CIVIL,MALE},
	{ 100,    "Biker",    CIVIL,MALE},
	{ 101,    "Pedestrian",    CIVIL,MALE},
	{ 102,    "Gangsta",    CIVIL,MALE},
	{ 103,    "Gangsta",    CIVIL,MALE},
	{ 104,    "Gangsta",    CIVIL,MALE},
	{ 105,    "Gangsta",    CIVIL,MALE},
	{ 106,    "Gangsta",    CIVIL,MALE},
	{ 107,    "Gangsta",    CIVIL,MALE},
	{ 108,    "Gangsta",    CIVIL,MALE},
	{ 109,    "Gangsta",    CIVIL,MALE},
	{ 110,    "Gangsta",    CIVIL,MALE},
	{ 111,    "Mafia Member",    CIVIL,MALE},
	{ 112,    "Mafia Member",    CIVIL,MALE},
	{ 113,    "Mafia Member",    CIVIL,MALE},
	{ 114,    "Gangsta",    CIVIL,MALE},
	{ 115,    "Gangsta",    CIVIL,MALE},
	{ 116,    "Gangsta",    CIVIL,MALE},
	{ 117,    "Mafia Member",    CIVIL,MALE},
	{ 118,    "Mafia Member",    CIVIL,MALE},
	{ 119,    "Mafia Member",    CIVIL,MALE},
	{ 120,    "Mafia Member",    CIVIL,MALE},
	{ 123,    "Gangsta",    CIVIL,MALE},
	{ 124,    "Mafia Member",    CIVIL,MALE},
	{ 125,    "Mafia Member",    CIVIL,MALE},
	{ 126,    "Mafia Member",    CIVIL,MALE},
	{ 127,    "Mafia Boss",    CIVIL,MALE},
	{ 128,    "Farmer",    CIVIL,MALE},
	{ 129,    "Farmer",    CIVIL,FEMALE},
	{ 130,    "Farmer",    CIVIL,FEMALE},
	{ 131,    "Farmer",    CIVIL,FEMALE},
	{ 132,    "Farmer",    CIVIL,MALE},
	{ 133,    "Farmer",    CIVIL,MALE},
	{ 134,    "Homeless",    CIVIL,MALE},
	{ 135,    "Homeless",    CIVIL,MALE},
	{ 136,    "Homeless",    CIVIL,MALE},
	{ 137,    "Homeless",    CIVIL,MALE},
	{ 138,    "Beach Girl",    CIVIL,FEMALE},
	{ 139,    "Beach Girl",    CIVIL,FEMALE},
	{ 140,    "Beach Girl",    CIVIL,FEMALE},
	{ 141,    "Business Girl",    CIVIL,FEMALE},
	{ 142,    "Hippy",    CIVIL,MALE},
	{ 143,    "Drug Maker",    CIVIL,MALE},
	{ 144,    "Drug Maker",    CIVIL,MALE},
	{ 145,    "Whore",    CIVIL,FEMALE},
	{ 146,    "Drug Maker",    CIVIL,MALE},
	{ 147,    "Lawyer",    CIVIL,MALE},
	{ 148,    "Lawyer",    CIVIL,FEMALE},
	{ 150,    "Business women",    CIVIL,FEMALE},
	{ 151,    "Farmer",    CIVIL,FEMALE},
	{ 152,    "Prostitute",    CIVIL,FEMALE},
	{ 153,    "Carpenter",    CIVIL,MALE},
	{ 154,    "Lifeguard",    CIVIL,MALE},
	{ 155,    "Pizza Worker",    CIVIL,MALE},
	{ 157,    "Farmer",    CIVIL,FEMALE},
	{ 158,    "Farmer",    CIVIL,MALE},
	{ 159,    "Farmer",    CIVIL,MALE},
	{ 160,    "Farmer",    CIVIL,MALE},
	{ 161,    "Farmer",    CIVIL,MALE},
	{ 162,    "Farmer",    CIVIL,MALE},
	{ 167,    "Mascot",    CIVIL,MALE},
	{ 169,    "Pedestrian",    CIVIL,FEMALE},
	{ 170,    "Pedestrian",    CIVIL,MALE},
	{ 171,    "Casino Worker",    CIVIL,MALE},
	{ 172,    "Casino Worker",    CIVIL,FEMALE},
	{ 173,    "Gangsta",    CIVIL,MALE},
	{ 174,    "Gangsta",    CIVIL,MALE},
	{ 175,    "Gangsta",    CIVIL,MALE},
	{ 176,    "Barber",    CIVIL,MALE},
	{ 177,    "Barber",    CIVIL,MALE},
	{ 178,    "Stripper",    CIVIL,FEMALE},
	{ 179,    "Ammo Salesman",    CIVIL,MALE},
	{ 180,    "Tattoo Artist",    CIVIL,MALE},
	{ 181,    "Rockstar",    CIVIL,MALE},
	{ 182,    "Taxi Driver",    CIVIL,MALE},
	{ 184,    "Mafia Member",    CIVIL,MALE},
	{ 185,    "Kidnapper",    CIVIL,MALE},
	{ 186,    "Mafia Member",    CIVIL,MALE},
	{ 187,    "Lawyer",    CIVIL,MALE},
	{ 188,    "Pedestrian",    CIVIL,MALE},
	{ 189,    "Pedestrian",    CIVIL,MALE},
	{ 190,    "Pedestrian",    CIVIL,FEMALE},
	{ 193,    "Pedestrian",    CIVIL,FEMALE},
	{ 194,    "Pedestrian",    CIVIL,FEMALE},
	{ 195,    "Pedestrian",    CIVIL,FEMALE},
	{ 196,    "Farmer",    CIVIL,FEMALE},
	{ 197,    "Farmer",    CIVIL,FEMALE},
	{ 198,    "Farmer",    CIVIL,FEMALE},
	{ 199,    "Farmer",    CIVIL,FEMALE},
	{ 200,    "Homeless",    CIVIL,MALE},
	{ 201,    "Truck Driver",    CIVIL,FEMALE},
	{ 202,    "Truck Driver",    CIVIL,MALE},
	{ 203,    "Karate Apprentices",    CIVIL,MALE},
	{ 204,    "Karate Apprentices",    CIVIL,MALE},
	{ 205,    "Pizza Worker",    CIVIL,FEMALE},
	{ 206,    "Carpenter",    CIVIL,MALE},
	{ 207,    "Prostitute",    CIVIL,FEMALE},
	{ 208,    "Mafia Member",    CIVIL,MALE},
	{ 212,    "Homeless",    CIVIL,MALE},
	{ 213,    "Homeless",    CIVIL,MALE},
	{ 214,    "Pedestrian",    CIVIL,FEMALE},
	{ 215,    "Pedestrian",    CIVIL,FEMALE},
	{ 216,    "Pedestrian",    CIVIL,FEMALE},
	{ 219,    "Pedestrian",    CIVIL,FEMALE},
	{ 220,    "Hippy",    CIVIL,MALE},
	{ 223,    "Pedestrian",    CIVIL,MALE},
	{ 224,    "Pedestrian",    CIVIL,FEMALE},
	{ 225,    "Pedestrian",    CIVIL,FEMALE},
	{ 226,    "Pedestrian",    CIVIL,FEMALE},
	{ 227,    "Lawyer",    CIVIL,MALE},
	{ 228,    "Lawyer",    CIVIL,MALE},
	{ 230,    "Homeless",    CIVIL,MALE},
	{ 233,    "Pedestrian",    CIVIL,FEMALE},
	{ 237,    "Prostitute",    CIVIL,FEMALE},
	{ 238,    "Prostitute",    CIVIL,FEMALE},
	{ 239,    "Homeless",    CIVIL,MALE},
	{ 241,    "Pedestrian",    CIVIL,MALE},
	{ 242,    "Pedestrian",    CIVIL,MALE},
	{ 243,    "Prostitute",    CIVIL,FEMALE},
	{ 244,    "Prostitute",    CIVIL,FEMALE},
	{ 245,    "Prostitute",    CIVIL,FEMALE},
	{ 247,    "Biker",    CIVIL,MALE},
	{ 248,    "Biker",    CIVIL,MALE},
	{ 249,    "Pimp",    CIVIL,MALE},
	{ 250,    "Pedestrian",    CIVIL,MALE},
	{ 251,    "Lifeguard",    CIVIL,FEMALE},
	{ 253,    "Taxi Driver",    CIVIL,MALE},
	{ 254,    "Biker",    CIVIL,MALE},
	{ 256,    "Stripper",    CIVIL,FEMALE},
	{ 257,    "Stripper",    CIVIL,FEMALE},
	{ 258,    "Pedestrian",    CIVIL,MALE},
	{ 259,    "Pedestrian",    CIVIL,MALE},
	{ 260,    "Carpenter",    CIVIL,MALE},
	{ 261,    "Train Driver",    CIVIL,MALE},
	{ 262,    "Taxi Driver",    CIVIL,MALE},
	{ 264,    "Clown",    CIVIL,MALE},
	{ 268,    "Mechanic",    CIVIL,MALE},
	{ 269,    "Gangsta",    CIVIL,MALE},
	{ 270,    "Gangsta",    CIVIL,MALE},
	{ 271,    "Gangsta",    CIVIL,MALE},
	{ 272,    "Mafia Boss",    CIVIL,MALE},
	{ 274,    "Paramedic",    CIVIL,MALE},
	{ 275,    "Paramedic",    CIVIL,MALE},
	{ 276,    "Paramedic",    CIVIL,MALE},
	{ 277,    "Firefighter",    CIVIL,MALE},
	{ 278,    "Firefighter",    CIVIL,MALE},
	{ 279,    "Firefighter",    CIVIL,MALE},
	{ 289,    "Pedestrian",    CIVIL,MALE},
	{ 290,    "Pedestrian",    CIVIL,MALE},
	{ 291,    "Pedestrian",    CIVIL,MALE},
	{ 292,    "Pedestrian",    CIVIL,MALE},
	{ 293,    "Rapper",    CIVIL,MALE},
	{ 294,    "Mafia Member",    CIVIL,MALE},
	{ 295,    "Mafia Boss",    CIVIL,MALE},
	{ 296,    "Pimp",    CIVIL,MALE},
	{ 297,    "Rapper",    CIVIL,MALE},
	{ 298,    "Pedestrian",    CIVIL,FEMALE},
	{ 299,    "Pedestrian",    CIVIL,MALE},
	{ 303,    "Pedestrian",    CIVIL,MALE},
	{ 308,    "Paramedic",    CIVIL,FEMALE},
	{ 300,    "Policeman",    POLICE,MALE},
	{ 301,    "Policeman",    POLICE,MALE},
	{ 302,    "Policeman",    POLICE,MALE},
	{ 306,    "Policewoman",    POLICE,FEMALE},
	{ 307,    "Policewoman",    POLICE,FEMALE},
	{ 265,    "Policeman",    POLICE,MALE},
	{ 266,    "Policeman",    POLICE,MALE},
	{ 267,    "Policeman",    POLICE,MALE},
	{ 284,    "Policeman",    POLICE,MALE},
	{ 285,    "SWAT",    POLICE,MALE},
	{ 280,    "Policeman",    POLICE,MALE},
	{ 309,    "Sheriff",    SHERIFF,FEMALE},
	{ 281,    "Sheriff",    SHERIFF,MALE},
	{ 282,    "Sheriff",    SHERIFF,MALE},
	{ 283,    "Sheriff",   SHERIFF,MALE},
	{ 288,    "Sheriff",    SHERIFF,MALE},
	{ 310,    "Sheriff",    SHERIFF,MALE},
	{ 311,    "Sheriff",    SHERIFF,MALE},
	{ 191,    "Sheriff",    SHERIFF,FEMALE},
	{ 286,    "FBI",    FBI,MALE},
	{ 163,    "FBI",    FBI,MALE},
	{ 164,    "FBI",    FBI,MALE},
	{ 165,    "FBI",    FBI,MALE},
	{ 166,    "FBI",    FBI,MALE},
	{ 304,    "UNDERCOVER",    UC_COP,MALE},
	{ 305,    "UNDERCOVER",    UC_COP,MALE},
	{ 192,    "UNDERCOVER",    UC_COP,FEMALE},
	{ 246,    "UNDERCOVER",    UC_COP,FEMALE},
	{ 121,    "UNDERCOVER",    UC_COP,MALE},
	{ 122,    "UNDERCOVER",    UC_COP,MALE},
	{ 287,    "UNDERCOVER",    UC_COP,MALE}
};

FUNCTION SetupPeds()
{
	for(new i=0; i<MAX_PEDS; i++)
	{
		AddPlayerClass(PedsInfo[i][PedSkinID],0.0,0.0,0.0,0.0,0,0,0,0,0,0);
	}
	return 1;
}

FUNCTION CreateClassTD(playerid)
{
	PlayerClassSelection= CreatePlayerTextDraw(playerid, 322.500000, 122.799926, "_");
	PlayerTextDrawLetterSize(playerid, PlayerClassSelection, 0.485998, 2.321778);
	PlayerTextDrawAlignment(playerid, PlayerClassSelection, 2);
	PlayerTextDrawColor(playerid, PlayerClassSelection, -855703297);
	PlayerTextDrawSetShadow(playerid, PlayerClassSelection, 0);
	PlayerTextDrawSetOutline(playerid, PlayerClassSelection, 1);
	PlayerTextDrawBackgroundColor(playerid, PlayerClassSelection, 255);
	PlayerTextDrawFont(playerid, PlayerClassSelection, 3);
	PlayerTextDrawSetProportional(playerid, PlayerClassSelection, 1);
	PlayerTextDrawSetShadow(playerid, PlayerClassSelection, 0);
	return 1;
}

FUNCTION DestroyClassTD(playerid)
{
	PlayerTextDrawDestroy(playerid,PlayerClassSelection);
	return 1;
}

FUNCTION ShowPlayerClassTD(playerid,classid)
{
	new string[MAX_PED_NAME + 6];
	switch(PedsInfo[classid][PedTeam])
	{
		case CIVIL: format(string,sizeof(string),"~w~%s",PedsInfo[classid][PedName]);
		case POLICE: format(string,sizeof(string),"~b~~h~%s",PedsInfo[classid][PedName]);
		case SHERIFF: format(string,sizeof(string),"~y~%s",PedsInfo[classid][PedName]);
		case FBI: format(string,sizeof(string),"~b~%s",PedsInfo[classid][PedName]);
		case UC_COP: format(string,sizeof(string),"~p~%s",PedsInfo[classid][PedName]);
	}
	PlayerTextDrawSetString(playerid,PlayerClassSelection,string);
	PlayerTextDrawShow(playerid,PlayerClassSelection);
	return 1;
}

FUNCTION HidePlayerClassTD(playerid)
{
	PlayerTextDrawHide(playerid,PlayerClassSelection);
	return 1;
}