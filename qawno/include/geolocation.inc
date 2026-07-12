/*

	GeoIP Include by Whitetiger
	-> SQLite GeoIP Include
	
	Credits: Whitetiger, RaekwonDaChef, Y_Less, Andreas Gohr.
	
	MaxMind, GeoIP and related marks are registered trademarks of MaxMind, Inc.
	
	---------------------------------------------------------------------------------------
	
*/

#if defined _geolocation_included
  #endinput
#endif


#define _geolocation_included

#tryinclude <a_samp>
#tryinclude <a_http>
#tryinclude <sscanf2>

/*
	---------------------------------------------------------------------------------------
	Callbacks
	---------------------------------------------------------------------------------------
*/

/*
	public OnProxyResponse(playerid, bool:proxy);

*/

/*
	---------------------------------------------------------------------------------------
	NATIVES
	---------------------------------------------------------------------------------------
*/

/*
	native GetPlayerCountry(playerid, string[], const len = sizeof(string));
	native GetPlayerISP(playerid, string[], const len = sizeof(string));
	native GetPlayerCity(playerid, string[], const len = sizeof(string));
	native GetPlayerGMT(playerid);
	native GetPlayerProxy(playerid);
*/

/*
	---------------------------------------------------------------------------------------
	Variables
	---------------------------------------------------------------------------------------
*/


#if !defined GeoIP_MainFile
	#define GeoIP_MainFile      "geoip.db"
#endif

new DB:geoip_db;


stock GetPlayerCountry(playerid, string[], const len = sizeof(string)) {
	new ip[24];
	GetPlayerIp(playerid, ip, sizeof(ip));

	GetIPCountry(ip, string, len);

	return true;
}

stock GetPlayerProxy(playerid) {
	new ip[24];
	GetPlayerIp(playerid, ip, sizeof(ip));
	
	return GetIPProxy(ip, playerid);
}


public OnGameModeInit()
{
    #if defined Geo_OnGameModeInit
        Geo_OnGameModeInit();
	#endif
	geoip_db = DB_Open(GeoIP_MainFile);
	return 1;
}

public OnGameModeExit()
{
    #if defined Geo_OnGameModeExit
        Geo_OnGameModeExit();
	#endif
	DB_Close(geoip_db);
	return 1;
}

/*

	---------------------------------------------------------------------------------------
	INTERNAL FUNCTIONS
	---------------------------------------------------------------------------------------

	- stock GetIPCountry(ip[], dest[], len = sizeof(dest))
	- stock GetIPProxy(ip[])
	- stock ip2long(ip[])

*/


stock GetIPCountry(const ip[], dest[], const len = sizeof(dest)) {
	new tmp[90];
	tmp = ip2long(ip);
	
	new string[500];
	format(string, sizeof(string), "SELECT loc.country FROM geolocation loc, geoblocks blk WHERE blk.startIpNum < %s AND blk.endIpNum > %s AND loc.locId = blk.locId LIMIT 1;", tmp, tmp);
	
	new DBResult:_result;
    _result = DB_ExecuteQuery(geoip_db, string);
    if(DB_GetRowCount(_result) >= 1)
    {
  		DB_GetFieldStringByName(_result,"country",dest,len);
    }
    DB_FreeResultSet(_result);
	
	if(!strlen(dest)) format(dest, len, "Unknown");
	
	return true;
}

new IPInfo[MAX_PLAYERS][MAX_PLAYER_NAME];

stock GetIPProxy(ip[], playerid) {

	if(IsPlayerConnected(playerid) && strlen(ip) > 0) {
		new str[128];
		GetIPCountry(ip, str, sizeof(str));

		if(strlen(str) > 0 && !strcmp("Anonymous Proxy",str)) {
		    //CallLocalFunction("OnProxyResponse", "ii", playerid, true);
		    return true;
		} else {

			format(IPInfo[playerid], MAX_PLAYER_NAME, "%s", ip);
		    format(str, sizeof(str), "www.ipqualityscore.com/api/ip_lookup.php?KEY=onekz5t6ksv8aidtrhcj7bhjit7u0&IP=%s", ip);
		    HTTP(playerid, HTTP_GET, str, "", "OnProxyRecieved");
		}
	}
	
	
	return false;
}

forward OnProxyRecieved(index, response_code, data[]);
public OnProxyRecieved(index, response_code, data[]) {

	if(IsPlayerConnected(index)) {
	
		new ip[MAX_PLAYER_NAME];
		GetPlayerIp(index, ip, sizeof(ip));
		
		if(strlen(IPInfo[index]) > 0 && strlen(ip) > 0 && !strcmp(IPInfo[index], ip, true)) {
			CallLocalFunction("OnProxyResponse", "ii",  index, strval(data));
		}
	}
	IPInfo[index] = "";
}

stock ip2long(const ip[]) {
	new ips[4];
	sscanf(ip, "p<.>dddd", ips[0], ips[1], ips[2], ips[3]);
	new tmp[90];
	format(tmp, sizeof(tmp), "((16777216 * %d) + (65536 * %d) + (256 * %d) + %d)", ips[0], ips[1], ips[2], ips[3]);
	// we use a string here so it will not pass the 32-bit integer limits, the math is done later in the sql query
	return tmp;
}


forward OnProxyResponse(playerid, bool:proxy);


#if defined _ALS_OnGameModeInit
    #undef OnGameModeInit
#else
    #define _ALS_OnGameModeInit
#endif
#define OnGameModeInit Geo_OnGameModeInit
#if defined Geo_OnGameModeInit
    forward Geo_OnGameModeInit();
#endif

#if defined _ALS_OnGameModeExit
    #undef OnGameModeExit
#else
    #define _ALS_OnGameModeExit
#endif
#define OnGameModeExit Geo_OnGameModeExit
#if defined Geo_OnGameModeExit
    forward Geo_OnGameModeExit();
#endif
