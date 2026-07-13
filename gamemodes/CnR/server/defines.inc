/*

	Defines for Cops And Robbers script
	Scripted by Arose Niazi.
	Scripting started on 11th October 2017
	Last Updated: 18/10/2017
	
	Credits:
		
	
	Functions:
		FUNCTION AnyFunciton(voids) : forwards, and public in same line.
		isNull(string)
		
	Functions Being Called:
		
*/

#if defined WINDOWS_COMPILER
	#include "CnR\server\colors"
#else
	#include "CnR/server/colors"
#endif

#define FUNCTION%1(%2) 			forward public %1(%2); 	public %1(%2)

#define COMMUNITY_NAME "EXP GAMING"
#define SERVER_BOT "Goku"

enum 
{
	LS,
	SF,
	LV
	
};

#define MAX_ZONE_NAME 			50
#define MAX_ZONES 				466
#define MAX_POLICE_ZONES 		14
#define INVALID_ZONE_ID 		-1

#undef MAX_PLAYERS
#define MAX_PLAYERS 			100

#define MAX_PED_NAME 			24
#define MAX_PEDS				270

#define MAX_SERIAL_SIZE			256
#define MAX_IP_SIZE				16
#define MAX_EMAIL_SIZE 			129
#define MAX_HASH_SIZE 			129

#define MAX_NODS 				800

#define IRC_SERVER "irc.mg-s.us"
#define IRC_PORT (6667)
#define IRC_ECHANNEL "#cnr.echo"
#define IRC_ACHANNEL "#cnr.admins"
#define IRC_MCHANNEL "#cnr.management"
#define IRC_RADIO "#mg.radio"
#define IRC_RADIO_DJ "#RadioHeadDjs"
#define IRC_BAN_CHANNEL "#bans"
#define ECHO_IRC_CHANNEL "%"IRC_ECHANNEL
#define ECHO_MANAGER_ADMIN "~"IRC_ACHANNEL
#define ECHO_MANAGER_NORMAL "~"IRC_ECHANNEL

#define TABLE_PLAYERS "players"
#define SERVER_TABLE "server_data"

enum
{
	CIVIL,
	POLICE,
	SHERIFF,
	FBI,
	UC_COP
}

enum
{
	MALE,
	FEMALE,
	OTHER
};

enum
{
	REGISTER_DIALOG_PASSWORD,
	REGISTER_DIALOG_EMAIL,
	LOGIN_DIALOG,
	BAN_DIALOG,
	COMMANDS_DIALOG,        // M2 — paginated command list (/showcommands for players)
	ADMIN_INFO_DIALOG,      // M2 — generic read-only info dialog (aka/ips/osearch/ajailed/flag lists)
	ADMIN_INFO_DIALOG2      // M2 — second generic info dialog (async result formatting)
};

enum
{
	SERVER_PLAYER,       // 0
	RETIRED_STAFF,       // 1
	SERVER_MODERATOR,    // 2
	SERVER_ADMIN,        // 3
	SERVER_HEAD_ADMIN,   // 4
	SERVER_MANAGER,      // 5
	SERVER_OWNER,        // 6
	SERVER_SCRIPTER      // 7  <-- highest tier (appended, do not renumber)
};

enum 
{
	NO_MENU,
	MENU_INFO,
	MENU_ADD_INT_TYPE,
	MENU_EDIT_INTERIORS_LIST,
	MENU_EDIT_INT_SETTING,
	MENU_EDIT_INT_SETTING_NAME,
	MENU_SAMP_TOWER,
	MENU_GRIN_TOWER,
	MENU_EDIT_ACTORS_LIST,
	MENU_ADD_ACTOR_TYPE,
	MENU_ADD_ACTOR_TYPE_2,
	MENU_EDIT_ACTOR_SETTING,
	MENU_GPS_OPTIONS,
	MENU_GPS_LIST
};

enum
{
	SIMPLE_INTERIOR,
	REGULAR_ONLY,
	STAFF_ONLY,
	COPS_ONLY,
	TELEPORTS,
	ROBBERY,
	POLICE_INT
};

enum 
{
	VEHICLE_TYPE_CIVILIAN,
	VEHICLE_TYPE_PUBLIC,
	VEHICLE_TYPE_COP
};

enum 
{
	LOW_RIDER,
	WAGOON,
	SALOON,
	JEEP,
	COVERTABLE,
	INDUSTERIAL
};

enum 
{
	ELEVATOR_STATE_IDLE,
	ELEVATOR_STATE_WAITING,
	ELEVATOR_STATE_MOVING
};

enum
{
	UP_WARDS,
	DOWN_WARDS
};

#define INTERIOR_VW 10000

#define ERROR_NOT_ADMIN "You don't match the rank to use this command."
#define INVALID_COMMAND "This command is not valid, use /commands for the list."
#define ERROR_WRONG_CHANNEL "This command is not available in this channel."

// ---- M2 command / moderation infra ----
#define PM_FLOOD_MS 			1500		// min gap between private messages (ms)
#define MAX_ADMIN_JAIL_MINS		1440		// hard cap for /ajail duration
#define PARACHUTE_WEAPON		46			// weapon id stripped inside DM zones

// Admin-jail location (isolated: high virtual world + interior 0).
#define ADMIN_JAIL_VW			90210
#define ADMIN_JAIL_X			(197.6661)
#define ADMIN_JAIL_Y			(173.8215)
#define ADMIN_JAIL_Z			(1003.0234)
#define ADMIN_JAIL_A			(0.0)
#define ADMIN_JAIL_INT			3

#define isNull(%1) \
                ((!(%1[0])) || (((%1[0]) == '\1') && (!(%1[1]))))