/*
	Colors for Cops And Robbers script
	Scripted by Arose Niazi.
	Scripting started on 18th October 2017
	Last Updated: ----
	
	Credits:
		color-hex.com
		
	Functions:
		
	Functions Being Called:
		
*/

#define COL_SERVER "{39a1c3}"
#define COLOR_SERVER 0x39a1c375
#define IRC_SERVER_COLOR "1,7"

#define COL_SERVER_INFO "{ffc220}"
#define COLOR_SERVER_INFO 0xffc22075

#define COL_WHITE "{FFFFFF}"
#define COLOR_WHITE 0xFFFFFF75


#define COL_LIGHTBLUE "{00B6FF}"
#define COL_RED "{FF0000}"
#define COL_GREEN "{99FF00}"
#define COL_SKYBLUE "{00B6FF}"
#define COL_PURPLE "{800080}"
#define COL_ORANGE "{FF7519}"
#define COL_WHITE "{FFFFFF}"
#define COL_YELLOW "{FFFF00}"
#define COL_BLUE "{4285f4}"

#define COLOR_ERROR COLOR_RED
#define IRC_ERROR_COLOR "0,4"

#define COLOR_RED 0xFF000075

#define IRC_SPECIAL_COLOR "0,1"

#define IRC_ADMIN_COLOR_INFO "6,8"
#define COLOR_ADMIN_INFO 0xd8b2d875
#define COL_ADMIN_INFO "{d8b2d8}"

#define COLOR_CONNECT 0xFD7C6E75
#define IRC_CONNECT_COLOR "7,1"

#define IRC_NORMAL_COLOR "12"

#define COLOR_DJ COLOR_PASTEL_PINK
#define IRC_COLOR_DJ "13,1"


#define IRC_RADIO_COLOR "10"

#define IRC_LIST_COLOR "8,7"

#define COLOR_PASTEL_PINK 0xFFB6C175

// ---- M3 : wanted-level tier colours (design §2.1 — verbatim readability
// contract). SetPlayerColor/SetPlayerMarkerForPlayer alpha is forced opaque so
// the radar blip reads clearly; the low byte (0xFF) is the alpha channel.
#define COL_WANTED_YELLOW		"{FFFF00}"		// suspect (WL 1-5)
#define COL_WANTED_ORANGE		"{FF7519}"		// warrant (WL 6-8)
#define COL_WANTED_DARKORANGE	"{CC3300}"		// most wanted (WL 9-10)
#define COLOR_WANTED_YELLOW		0xFFFF00FF		// suspect name/blip
#define COLOR_WANTED_ORANGE		0xFF7519FF		// warrant name/blip
#define COLOR_WANTED_DARKORANGE	0xCC3300FF		// most-wanted name/blip
#define COLOR_INNOCENT			0xFFFFFFFF		// white civilian name/blip
#define COLOR_COP_BLIP			0x2E64FEFF		// law-enforcement blue blip
#define COLOR_BACKUP_PURPLE		0x800080FF		// cop requesting backup (stage 2 /backup)

// ---- M3 (stage 3) : crime / drug / STD feedback colours -------------------
#define COLOR_CRIME				0xFF7519FF		// crime action confirmation (robber/rapist)
#define COLOR_VICTIM			0xFF6347FF		// victim notification (robbed / raped)
#define COLOR_DISEASE			0x9ACD32FF		// STD infection / drain warnings
#define COLOR_DRUG				0xC080FFFF		// drug buzz / take-drugs feedback
#define COL_DISEASE				"{9ACD32}"
#define COL_DRUG				"{C080FF}"