/*

	Miscellaneous Script
	Scripted by Arose Niazi.
	Scripting started on 19th October 2017
	Last Updated: -----
	
	Credits:
		Beremix for timestamp function.
		
	Functions:
		InsertTimeStamp();
		DelayedKick(playerid);
		KickPlayer(playerid,KickerName,Reason);
		KickPlayerHidden(playerid,kickerid,Reason);
		UpdateMoney(playerid);
		GetXYInFrontOfPlayer(playerid, &Float:x, &Float:y, Float:distance);
		StringReplace(string[],replace[],const size);
		isNumeric(const string[]);
		VehicleNames[model-400];
		PlaySoundForPlayersInRange(soundid, Float:range, Float:x, Float:y, Float:z);
		
	Functions Being Called:
		
*/

InsertTimeStamp()
{
	new StrSmall[50];
	new day,month,year,hour,minute,second;
	gettime(hour,minute,second);
	getdate(year,month,day);
	format(StrSmall,sizeof(StrSmall),"%d/%d/%d %d:%d:%d",day,month,year,hour,minute,second);
	return StrSmall;
}

FUNCTION KickPlayer(playerid,KickerName[],Reason[])
{
	static bool:Kicked[MAX_PLAYERS];
	if(Kicked[playerid]) return 0;
	Kicked[playerid]=true;
	new string[130];
	format(string, sizeof(string), "has kicked %s (%d). Reason: %s.",PlayerInfo[playerid][pUserName],playerid,Reason);
	SendAdminAnonmityMessageIRC(KickerName,COLOR_ADMIN_INFO,string,IRC_ADMIN_COLOR_INFO);
	SetTimerEx("DelayedKick", 1000, false, "d", playerid);
	return 1;
}
FUNCTION KickPlayerHidden(playerid,kickerid,Reason[])
{
	static bool:Kicked[MAX_PLAYERS];
	if(Kicked[playerid]) return 0;
	Kicked[playerid]=true;
	new string[130];
	format(string, sizeof(string), "has kicked %s (%d). Reason: %s.",PlayerInfo[playerid][pUserName],playerid,Reason);
	SendAdminAnonmityMessage(kickerid,COLOR_ADMIN_INFO, string,IRC_ADMIN_COLOR_INFO);
	SetTimerEx("DelayedKick", 1000, false, "d", playerid);
	return 1;
}

FUNCTION DelayedKick(playerid)
{
	Kick(playerid);
	return 1;
}

UpdateMoney(playerid)
{
	if(PlayerInfo[playerid][pMoney] > GetPlayerMoney(playerid)) GivePlayerMoney(playerid,PlayerInfo[playerid][pMoney]-GetPlayerMoney(playerid));
	else if(PlayerInfo[playerid][pMoney] < GetPlayerMoney(playerid)) GivePlayerMoney(playerid,GetPlayerMoney(playerid)-PlayerInfo[playerid][pMoney]);
	return 1;
}

GetXYInFrontOfPlayer(playerid, &Float:x, &Float:y, Float:distance)
{
    new Float:a;
    GetPlayerPos(playerid, x, y, a);
    GetPlayerFacingAngle(playerid, a);
    if (GetPlayerVehicleID(playerid))
    {
        GetVehicleZAngle(GetPlayerVehicleID(playerid), a);
    }
    x += (distance * floatsin(-a, degrees));
    y += (distance * floatcos(-a, degrees));
}
StringReplace(string[],replace[],const size)
{
	strdel(string,0,size);
	strins(string,replace,0,size);
	return string;
}
isNumeric(const string[])
{
	new length=strlen(string);
  	if(length==0) return false;
  	for(new i = 0; i < length; i++)
    {
      	if(
       		(string[i] > '9' || string[i] < '0' && string[i]!='-' && string[i]!='+') // Not a number,'+' or '-'
         	|| (string[i]=='-' && i!=0)                                             // A '-' but not at first.
          	|| (string[i]=='+' && i!=0)                                             // A '+' but not at first.
  		   ) return false;
    }
  	if(length==1 && (string[0]=='-' || string[0]=='+')) return false;
  	return true;
}

stock const VehicleNames[212][] =
{
    "Landstalker",          "Bravura",              "Buffalo",              "Linerunner",           "Pereniel",             "Sentinel",
    "Dumper",               "Firetruck",            "Trashmaster",          "Stretch",              "Manana",               "Infernus",
    "Voodoo",               "Pony","Mule",          "Cheetah",              "Ambulance",            "Leviathan",            "Moonbeam",
    "Esperanto",            "Taxi",                 "Washington",           "Bobcat",               "Mr Whoopee",           "BF Injection",
    "Hunter",               "Premier",              "Enforcer",             "Securicar",            "Banshee",              "Predator",
    "Bus",                  "Rhino",                "Barracks",             "Hotknife",             "Trailer",              "Previon",
    "Coach",                "Cabbie",               "Stallion",             "Rumpo",                "RC Bandit",            "Romero",
    "Packer",               "Monster",              "Admiral",              "Squalo",               "Seasparrow",           "Pizzaboy",
    "Tram",                 "Trailer",              "Turismo",              "Speeder",              "Reefer",               "Tropic",
    "Flatbed",              "Yankee",               "Caddy",                "Solair",               "Berkley's RC Van",     "Skimmer",
    "PCJ-600",              "Faggio",               "Freeway",              "RC Baron",             "RC Raider",            "Glendale",
    "Oceanic",              "Sanchez",              "Sparrow",              "Patriot",              "Quad",                 "Coastguard",
    "Dinghy",               "Hermes",               "Sabre",                "Rustler",              "ZR3 50",               "Walton",
    "Regina",               "Comet",                "BMX",                  "Burrito",              "Camper",               "Marquis",
    "Baggage",              "Dozer",                "Maverick",             "News Chopper",         "Rancher",              "FBI Rancher",
    "Virgo",                "Greenwood",            "Jetmax",               "Hotring",              "Sandking",             "Blista Compact",
    "Police Maverick",      "Boxville",             "Benson",               "Mesa",                 "RC Goblin",            "Hotring Racer A",
    "Hotring Racer B",      "Bloodring Banger",     "Rancher",              "Super GT",             "Elegant",              "Journey",
    "Bike",                 "Mountain Bike",        "Beagle",               "Cropdust",             "Stunt",                "Tanker",
    "RoadTrain",            "Nebula",               "Majestic",             "Buccaneer",            "Shamal",               "Hydra",
    "FCR-900",              "NRG-500",              "HPV1000",              "Cement Truck",         "Tow Truck",            "Fortune",
    "Cadrona",              "FBI Truck",            "Willard",              "Forklift",             "Tractor",              "Combine",
    "Feltzer",              "Remington",            "Slamvan",              "Blade",                "Freight",              "Streak",
    "Vortex",               "Vincent",              "Bullet",               "Clover",               "Sadler",               "Firetruck",
    "Hustler",              "Intruder",             "Primo",                "Cargobob",             "Tampa",                "Sunrise",
    "Merit",                "Utility",              "Nevada",               "Yosemite",             "Windsor",              "Monster A","Monster B",
    "Uranus",               "Jester",               "Sultan",               "Stratum",              "Elegy",                "Raindance",
    "RC Tiger",             "Flash",                "Tahoma",               "Savanna",              "Bandito",              "Freight",
    "Trailer",              "Kart",                 "Mower",                "Duneride",             "Sweeper",              "Broadway",
    "Tornado",              "AT-400",               "DFT-30",               "Huntley",              "Stafford",             "BF-400",
    "Newsvan",              "Tug",                  "Trailer A",            "Emperor",              "Wayfarer",             "Euros",
    "Hotdog",               "Club",                 "Trailer B",            "Trailer C",            "Andromada",            "Dodo",
    "RC Cam",               "Launch",               "LS PD Car",            "SF PD Car",            "LV PD Car",            "Police Ranger",
    "Picador",              "S.W.A.T. Van",         "Alpha",                "Phoenix",              "Glendale",             "Sadler",
    "Luggage Trailer A",    "Luggage Trailer B",    "Stair Trailer",        "Boxville",             "Farm Plow",            "Utility Trailer"
};

PlaySoundForPlayersInRange(soundid, Float:range, Float:x, Float:y, Float:z)
{
	for(new i=0; i<MAX_PLAYERS; i++)
	{
	    if(IsPlayerConnected(i) && IsPlayerInRangeOfPoint(i,range,x,y,z))
	    {
		    PlayerPlaySound(i, soundid, x, y, z);
	    }
	}
}
