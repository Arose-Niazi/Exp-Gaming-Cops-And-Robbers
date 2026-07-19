/*

	Cop dispatch / 10-code / department-radio core for Cops And Robbers script
	Scripted by Arose Niazi.
	Stage B (FEATURE-EXPANSION-PLAN §3 - THE FULL COP SYSTEM). Shared cop-comms
	infrastructure the /d channel and the auto-dispatch events build on. Included
	AFTER wanted/jail (and reached from robbery/holdup/housing/wanted via
	CallLocalFunction, since those are included later) but BEFORE cmds/cop.inc so
	the cop commands call these helpers directly.

	This module owns:
	  §3.2  Dispatch_Alert()        - the central [DISPATCH] fan-out (priority
	                                  colour, server-wide or radius, cop-chat gate,
	                                  critical priority overrides the gate). Records
	                                  the last dispatch origin for /respond.
	  §3.3  g_TenCodes[]            - the curated ~24-code table.
	        Cop_Expand10Code()      - one-pass expansion of a leading 10-NN token +
	                                  the $loc/$sus/$veh/$time quick-strings, applied
	                                  to every cop-channel message.
	  §3.1  g_CopLastSuspect[]      - per-cop last-targeted suspect (drives $sus);
	                                  set by /vc //su //report //arrest //ticket etc.

	Public API (FUNCTION publics - cross-module via CallLocalFunction):
		Dispatch_Alert(originPlayer, priority, const code[], const text[], Float:radius)
		                                     : fan a [DISPATCH] line to cops (§3.2)
		Dispatch_SetLastSuspect(copid, suspectid) : record $sus for a cop
		Dispatch_OnDisconnect(playerid)      : reset the per-slot last-suspect

	Direct-call helpers (defined here, called by cop.inc which is included next):
		Cop_Expand10Code(dest[], const src[], playerid)  : 10-code + quick-string pass
		Dispatch_IsCritical(priority)                    : bool

	Functions Being Called:
		misc:     IsCop, Rob_SuccessChance (unused here)
		wanted:   (none - this is called FROM wanted.inc)
*/

// Dispatch priorities + scope (DISPATCH_LOW/NORMAL/HIGH/CRITICAL, DISPATCH_ALL)
// and the WL9 radius live in server/defines.inc so wanted.inc (included before
// this module) can pass DISPATCH_CRITICAL to Dispatch_Alert. The priority COLOURS
// stay here with the dispatch code (nothing else reads them).

// Priority colours (opaque). NORMAL reuses a warrant orange; HIGH a hotter
// red-orange; CRITICAL a bright red; LOW the cop blue.
#define COLOR_DISPATCH_LOW		COLOR_COP_BLIP
#define COLOR_DISPATCH_NORMAL	0xFF9A19FF
#define COLOR_DISPATCH_HIGH		0xFF5A1EFF
#define COLOR_DISPATCH_CRITICAL	0xFF2222FF

//==============================================================================
//	Last dispatch origin (for /respond) + per-cop last suspect (for $sus)
//==============================================================================

// World position of the most recent dispatch call, so /respond can drop a red
// call checkpoint on it (§3.4). Session-only, single latest call (mirrors CB's
// single "red call checkpoint").
new Float:g_LastDispatchPos[3];

// The last suspect each cop targeted (via /vc //su //report //arrest //ticket).
// Drives the $sus quick-string. -1 = none. Reset per-connect (Dispatch_OnDisconnect).
new g_CopLastSuspect[MAX_PLAYERS] = { -1, ... };

// Record that `copid` just targeted `suspectid` (for $sus expansion). Called
// from the cop commands that resolve a suspect. Guards the ids so a stale/NPC
// target is not stored.
FUNCTION Dispatch_SetLastSuspect(copid, suspectid)
{
	if(copid < 0 || copid >= MAX_PLAYERS) return 0;
	if(!IsPlayerConnected(suspectid) || IsPlayerNPC(suspectid)) return 0;
	g_CopLastSuspect[copid] = suspectid;
	return 1;
}

// Reset the per-slot last-suspect on disconnect (mirrors Cop_OnDisconnect).
FUNCTION Dispatch_OnDisconnect(playerid)
{
	g_CopLastSuspect[playerid] = -1;
	return 1;
}

//==============================================================================
//	The curated 10-code table (§3.3) - ~24 codes
//==============================================================================

enum e_TenCode { TC_CODE[8], TC_MEANING[48] }
new const g_TenCodes[][e_TenCode] =
{
	{"10-4",   "Affirmative"},				{"10-20",  "Location: $loc"},
	{"10-23",  "Arrived on scene"},			{"10-7",   "Murder"},
	{"10-17",  "Robbery"},					{"10-78",  "Backup needed at $loc"},
	{"10-80",  "In pursuit with $sus $loc"},{"10-33",  "Help immediately $loc"},
	{"10-97",  "Test comms"},				{"10-99",  "Wanted/Stolen"},
	{"10-114", "Most Wanted"},				{"10-130", "Robbery/Hold Up"},
	{"10-131", "Casino Robbery"},			{"10-132", "Bank Robbery"},
	{"10-51",  "Suspect on foot"},			{"10-138", "Suspect in a vehicle"},
	{"10-70",  "Lost visual $loc"},			{"10-85",  "Suspect in custody"},
	{"10-52",  "Ambulance needed $loc"},	{"10-32",  "Units needed"},
	{"10-13",  "Follow me"},				{"10-12",  "Stand by"},
	{"10-22",  "Disregard"},				{"10-100", "5 min break"}
};

//==============================================================================
//	Quick-string helpers ($loc / $sus / $veh / $time)
//==============================================================================

// Resolve a cop's quick-string sources into short display strings. $loc is the
// cop's zone; $sus the last-targeted suspect's name; $veh the cop's current
// vehicle model name; $time the game clock. All ASCII, safe fall-backs.
static Dispatch_QuickLoc(playerid, dest[], size)
{
	if(isNull(PlayerInfo[playerid][pZoneName])) format(dest, size, "unknown");
	else format(dest, size, "%s", PlayerInfo[playerid][pZoneName]);
}

static Dispatch_QuickSus(playerid, dest[], size)
{
	new s = (playerid >= 0 && playerid < MAX_PLAYERS) ? g_CopLastSuspect[playerid] : -1;
	if(s != -1 && IsPlayerConnected(s) && !IsPlayerNPC(s)) format(dest, size, "%s", PlayerInfo[s][pUserName]);
	else format(dest, size, "a suspect");
}

static Dispatch_QuickVeh(playerid, dest[], size)
{
	new vid = GetPlayerVehicleID(playerid);
	if(vid != 0 && vid != INVALID_VEHICLE_ID)
	{
		new model = GetVehicleModel(vid);
		if(model >= 400 && model <= 611) { format(dest, size, "%s", VehicleNames[model - 400]); return; }
	}
	format(dest, size, "on foot");
}

static Dispatch_QuickTime(dest[], size)
	format(dest, size, "%02d:%02d", GameHour, GameMinute);

// Replace every occurrence of `token` in `str` with `with` (bounded, ASCII).
// A small in-place pass - the strings are short (< 144) so this is cheap.
static Dispatch_ReplaceToken(str[], const token[], const with[], size)
{
	new pos, tlen = strlen(token);
	if(tlen == 0) return;
	while((pos = strfind(str, token, false)) != -1)
	{
		strdel(str, pos, pos + tlen);
		strins(str, with, pos, size);
	}
}

//==============================================================================
//	Cop_Expand10Code (§3.3) - one pass over a cop-channel message
//==============================================================================

// Expand a cop-channel message in place:
//   1. If the message STARTS with a standalone "10-NN" token that matches the
//      curated table, rewrite that token to "10-NN (Meaning)".
//   2. Expand the $loc/$sus/$veh/$time quick-strings anywhere in the message
//      (including inside a code's expanded Meaning).
// `dest` receives the expanded text; `src` is left untouched. `playerid` is the
// sending cop (source of the quick-string data).
Cop_Expand10Code(dest[], const src[], playerid)
{
	// Work in a local buffer sized for a chat line + the widest expansion.
	new work[176];
	format(work, sizeof(work), "%s", src);

	// (1) Leading 10-NN token -> "10-NN (Meaning)". Only a token at the very
	// start (optionally the whole message) is expanded, so mid-sentence "10-4"
	// mentions are not mangled.
	if(work[0] == '1' && work[1] == '0' && work[2] == '-')
	{
		// Read the leading token up to the first space.
		new token[8], i = 0;
		while(i < 7 && work[i] != ' ' && work[i] != '\0') { token[i] = work[i]; i++; }
		token[i] = '\0';

		for(new c = 0; c < sizeof(g_TenCodes); c++)
		{
			if(strcmp(token, g_TenCodes[c][TC_CODE], false) != 0) continue;
			// Rebuild: "10-NN (Meaning)" + the remainder of the message (from index i,
			// which is the space after the token, or the end of the string).
			new rebuilt[176], tail[160];
			strmid(tail, work, i, strlen(work), sizeof(tail));	// keep the trailing " ..." incl. the leading space
			format(rebuilt, sizeof(rebuilt), "%s (%s)%s", token, g_TenCodes[c][TC_MEANING], tail);
			format(work, sizeof(work), "%s", rebuilt);
			break;
		}
	}

	// (2) Quick-strings anywhere (the code Meaning above may itself contain them).
	new q[MAX_ZONE_NAME];
	if(strfind(work, "$loc", true) != -1)  { Dispatch_QuickLoc(playerid, q, sizeof(q));  Dispatch_ReplaceToken(work, "$loc", q, sizeof(work)); }
	if(strfind(work, "$sus", true) != -1)  { Dispatch_QuickSus(playerid, q, sizeof(q));  Dispatch_ReplaceToken(work, "$sus", q, sizeof(work)); }
	if(strfind(work, "$veh", true) != -1)  { Dispatch_QuickVeh(playerid, q, sizeof(q));  Dispatch_ReplaceToken(work, "$veh", q, sizeof(work)); }
	if(strfind(work, "$time", true) != -1) { Dispatch_QuickTime(q, sizeof(q));           Dispatch_ReplaceToken(work, "$time", q, sizeof(work)); }

	format(dest, 176, "%s", work);
	return 1;
}

//==============================================================================
//	Dispatch_Alert (§3.2) - the central [DISPATCH] fan-out
//==============================================================================

// Is a priority "critical" (delivered even to cops with cop-chat off)?
Dispatch_IsCritical(priority)
	return (priority >= DISPATCH_CRITICAL) ? 1 : 0;

// Colour for a priority.
static Dispatch_Colour(priority)
{
	switch(priority)
	{
		case DISPATCH_CRITICAL: return COLOR_DISPATCH_CRITICAL;
		case DISPATCH_HIGH:     return COLOR_DISPATCH_HIGH;
		case DISPATCH_NORMAL:   return COLOR_DISPATCH_NORMAL;
	}
	return COLOR_DISPATCH_LOW;
}

// Emit "[DISPATCH] <code> <text> - <zone>" in a priority colour to every cop who
// has cop-chat ON (critical priority ignores the toggle). If radius<=0 the alert
// is server-wide, else only cops within `radius` of the origin's position receive
// it. Records the origin as the last dispatch call (for /respond). `originPlayer`
// may be a valid player (the suspect/officer the alert is about) or, if -1, the
// alert is location-less (uses no zone / no origin filter).
FUNCTION Dispatch_Alert(originPlayer, priority, const code[], const text[], Float:radius)
{
	new Float:ox, Float:oy, Float:oz, zoneName[MAX_ZONE_NAME];
	new bool:haveOrigin = (originPlayer >= 0 && originPlayer < MAX_PLAYERS && IsPlayerConnected(originPlayer));
	if(haveOrigin)
	{
		GetPlayerPos(originPlayer, ox, oy, oz);
		if(isNull(PlayerInfo[originPlayer][pZoneName])) format(zoneName, sizeof(zoneName), "the city");
		else format(zoneName, sizeof(zoneName), "%s", PlayerInfo[originPlayer][pZoneName]);
		// Record the origin as the last dispatch call (for /respond's checkpoint).
		g_LastDispatchPos[0] = ox;
		g_LastDispatchPos[1] = oy;
		g_LastDispatchPos[2] = oz;
	}
	else format(zoneName, sizeof(zoneName), "the city");

	new colour = Dispatch_Colour(priority), string[176];
	if(!isNull(code) && !isNull(text))
		format(string, sizeof(string), ""COL_LIGHTBLUE"[DISPATCH] "COL_WHITE"%s %s - %s", code, text, zoneName);
	else if(!isNull(code))
		format(string, sizeof(string), ""COL_LIGHTBLUE"[DISPATCH] "COL_WHITE"%s - %s", code, zoneName);
	else
		format(string, sizeof(string), ""COL_LIGHTBLUE"[DISPATCH] "COL_WHITE"%s - %s", text, zoneName);

	new bool:critical = (Dispatch_IsCritical(priority) != 0);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || !PlayerInfo[i][pLoggedIn]) continue;
		if(!IsCop(i)) continue;
		// Cop-chat gate: cops who muted the channel skip non-critical dispatch.
		if(!critical && !PlayerInfo[i][pCopChatOn]) continue;
		// Radius gate: only cops within `radius` of the origin (server-wide if <=0
		// or no origin). Uses the origin player's world position.
		if(radius > 0.0 && haveOrigin && !IsPlayerInRangeOfPoint(i, radius, ox, oy, oz)) continue;
		SendClientMessage(i, colour, string);
	}
	return 1;
}
