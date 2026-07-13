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
	ADMIN_INFO_DIALOG2,     // M2 — second generic info dialog (async result formatting)
	COP_INFO_DIALOG,        // M3 (stage 2) — cop lists (/suspects /warrants /mostwanted /jaillist)
	REFILL_DIALOG           // M3 (stage 2) — cop /refill menu (HP/armour/weapons)
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

// ---- M3 : wanted-level system (design §2) ----
#define MAX_WANTED				10			// hard cap on pWanted (§2.1)
#define WANTED_SUSPECT_MIN		1			// yellow tier floor
#define WANTED_WARRANT_MIN		6			// orange tier floor (arrestable)
#define WANTED_MOSTWANTED_MIN	9			// dark-orange tier floor
#define WANTED_DECAY_SECS		180			// real-seconds per -1 level (§2.4) [OUR DESIGN]
#define VC_FREEZE_SECS			60			// /visualcontact decay-pause window (§2.4)
#define SPAWN_PROTECT_SECS		7			// spawn protection window (§2.5) [OUR DESIGN, 5-10s]

// Wanted tiers (GetWantedTier / GetPlayerWantedTier return values, §2.1).
enum
{
	WANTED_TIER_INNOCENT,	// 0 : WL0, white
	WANTED_TIER_SUSPECT,	// 1 : WL1-5, yellow
	WANTED_TIER_WARRANT,	// 2 : WL6-8, orange
	WANTED_TIER_MOSTWANTED	// 3 : WL9-10, dark orange
}

// Per-crime wanted values (design §2.3 crime table). Stage 2+ crime handlers
// pass these to GivePlayerWanted; the ones wired now live in the death/jack
// handlers in wanted.inc.
#define WANTED_KILL_CIVILIAN	4			// murder of an innocent (§2.3, +4..6 — baseline)
#define WANTED_KILL_COP			6			// killing a cop is worth more (§2.3 / task)
#define WANTED_KILL_SUSPECT		2			// killing another wanted player
#define WANTED_CARJACK			3			// stealing an occupied/player vehicle (§2.3)
#define WANTED_DRIVEBY			2			// drive-by shooting (§2.3 open Q9 — folded in)

// Admin-jail location (isolated: high virtual world + interior 0).
#define ADMIN_JAIL_VW			90210
#define ADMIN_JAIL_X			(197.6661)
#define ADMIN_JAIL_Y			(173.8215)
#define ADMIN_JAIL_Z			(1003.0234)
#define ADMIN_JAIL_A			(0.0)
#define ADMIN_JAIL_INT			3

// ---- M3 (stage 2) : cop command set (design §3.1) ----
#define ARREST_RANGE			4.0			// on-foot arrest reach (§3.1) [OUR DESIGN]
#define TICKET_RANGE			8.0			// ticket must be collected within <8m (§3.1)
#define VC_RANGE				250.0		// /visualcontact & /su suspect reach
#define ARREST_CASH_PER_WL		1500		// arrest cash bonus = this * wanted-at-arrest (§3.1)
#define ARREST_SCORE			1			// cop score per arrest (§3.1)
#define TICKET_FINE				500			// classic ticket collection bonus / fine (§3.1)
#define TICKET_SCORE			1			// cop score per ticket collected (§3.1)
#define TICKET_PAY_SECS			300			// unpaid ticket -> warrant escalation (§2.4)
#define TICKET_ESCALATE_WL		WANTED_WARRANT_MIN	// WL an unpaid ticket jumps to (§2.4)
#define BACKUP_MARKER_SECS		60			// how long the purple backup marker holds
#define REFILL_POINTS_MAX		10			// cop /refill point cap (§3.1)
#define REFILL_COOLDOWN_SECS	30			// PD refill cooldown (§3.1) [OUR DESIGN]
#define REFILL_HP_COST			1			// refill points spent on a health top-up
#define REFILL_ARMOUR_COST		2			// refill points spent on an armour top-up
#define REFILL_WEAPON_COST		3			// refill points spent on a weapon package
#define DONUT_CARRY_MAX			6			// /donut carry cap (§3.1)
#define DONUT_HEAL_PCT			50.0		// /donut heal amount (§3.1, +50% HP)

// ---- M3 (stage 2) : jail (design §3.2) ----
// Real LSPD (Pershing Square) cell interior — isolated in a high virtual world.
#define JAIL_VW					90310
#define JAIL_INT				6
#define JAIL_X					(219.6404)
#define JAIL_Y					(110.2582)
#define JAIL_Z					(999.0156)
#define JAIL_A					(90.0)
// Spawn-back-outside point (Pershing Square PD front).
#define JAIL_RELEASE_X			(1554.8619)
#define JAIL_RELEASE_Y			(-1675.5304)
#define JAIL_RELEASE_Z			(16.1953)
#define JAIL_RELEASE_A			(90.0)

#define JAIL_BASE_SECS			60			// base sentence (§3.2 formula) [OUR DESIGN]
#define JAIL_PER_WL_SECS		30			// + wantedAtArrest * this (§3.2)
#define JAIL_PER_SERIOUS_SECS	45			// + seriousCrimeCount * this (§3.2)
#define BAIL_PER_SEC			200			// bail = sentence seconds * this (§3.2)
#define JAIL_MIN_SERVE_SECS		30			// minimum served before /parole is allowed (§3.2)

#define BRIBE_MIN				1000		// /bribe floor (§3.2)
#define BRIBE_MAX				15000		// /bribe ceiling (§3.2)
#define BRIBE_CUT_SECS			45			// accepted bribe cuts this many jail seconds
#define BRIBE_REFUSE_ADD_SECS	30			// refused bribe adds this many jail seconds

#define JURY_SIZE				15			// appeal jury size (§3.2, config constant)
#define APPEAL_MIN_JURY			2			// min online jurors to run an appeal
#define APPEAL_MIN_LEFT_SECS	60			// cannot appeal with < this remaining (§3.2)

#define ESCAPE_WL				10			// /escape sets Most Wanted (§3.2)
#define ESCAPE_RECAP_ADD_SECS	120			// re-caught escapee sentence extension (§3.2)
#define BREAKOUT_RANGE			5.0			// /breakout reach to a jailed player

// ---- M3 (stage 3) : player crime commands (design §4) ---------------------

// Civilian skill ids (pSkill / LSplayers.Skill). Set at City Hall via /skill in
// M6 (§4 header, jobs §1.3); until then everyone is SKILL_NONE and the crime
// handlers use their base tier. The rob/rape/drug handlers read these to unlock
// the higher caps + success rates, so the M6 skill work is a data change only.
enum
{
	SKILL_NONE,			// 0 — no specialisation (base rob cap, base rape/drug behaviour)
	SKILL_PICKPOCKET,	// 1 — rob cap $100k, very high success (§4.1)
	SKILL_CONARTIST,	// 2 — rob cap $500k, robs 10-50% on-hand (§4.1)
	SKILL_RAPIST,		// 3 — high rape success + can pass STDs they don't carry (§4.2)
	SKILL_DRUGDEALER	// 4 — 5000g drug carry cap vs 500g (§4.3)
}

// /rob (/rb) — rob a player (§4.1). Only cash-in-hand is robbable.
#define ROB_RANGE				3.0			// on-foot reach for /rob (§4.1)
#define ROB_CD					30			// per-victim cooldown seconds (§4.1)
#define ROB_WANTED				2			// wanted gained per rob (§2.3 table)
#define ROB_SCORE				1			// robber score per successful rob (§4.1)
#define ROB_PCT_MIN				10			// min % of victim on-hand cash taken (§4.1)
#define ROB_PCT_MAX				50			// max % of victim on-hand cash taken (§4.1)
// Skill caps — base ($50k) now; Pickpocket/Con-Artist caps are M6 skills, so the
// cap is parameterised via ROB_CAP_FOR() which the M6 skill work overrides.
#define ROB_CAP_BASE			50000		// non-rob skill cap (§4.1) [owner-ratify §12.1]
#define ROB_CAP_PICKPOCKET		100000		// Pickpocket cap (M6 skill)
#define ROB_CAP_CONARTIST		500000		// Con Artist cap (M6 skill)

// /rape (/ra) — proximity, chance-based, may infect (§4.2).
#define RAPE_RANGE				2.0			// proximity target reach (§4.2)
#define RAPE_CD					45			// base cooldown seconds (Rapist shorter, M6)
#define RAPE_DAMAGE				15.0		// HP damage dealt on a rape (even through condom)
#define RAPE_SUCCESS_BASE		55			// base success chance % (§4.2) [OUR DESIGN]
#define RAPE_INFECT_BASE		45			// base infection chance % on success (§4.2)
#define RAPE_INFECT_CONDOM		8			// infection chance % when victim has a condom (§4.2)
#define RAPE_WANTED				2			// wanted gained per rape (§2.3)
#define RAPE_WANTED_INFECT		1			// extra wanted when it infects (§2.3, "+1 if infects")
#define RAPE_SCORE				1			// rapist score per successful rape (§4.2)

// STD system (systems/std.inc) — pSTD bitmask, tiers Chlamydia..Mary Lou (§4.2).
#define MAX_STD_TYPES			5			// number of STD tiers (bit count in the mask)
#define STD_CHLAMYDIA			(1 << 0)	// tier 0 — mildest
#define STD_GONORRHEA			(1 << 1)	// tier 1
#define STD_SYPHILIS			(1 << 2)	// tier 2
#define STD_HERPES				(1 << 3)	// tier 3
#define STD_MARYLOU				(1 << 4)	// tier 4 — worst
#define STD_ALL					(STD_CHLAMYDIA | STD_GONORRHEA | STD_SYPHILIS | STD_HERPES | STD_MARYLOU)
#define STD_DRAIN_SECS			6			// seconds between HP-drain ticks (§4.2) [OUR DESIGN]
#define STD_DRAIN_PER_TIER		2.0			// HP drained per infected tier each tick (§4.2)

// /takedrugs (/td) + drug system (systems/drugs.inc) — §4.3.
#define DRUG_CARRY_MAX			500			// non-dealer carry cap (grams, §4.3)
#define DRUG_CARRY_MAX_DEALER	5000		// Drug Dealer carry cap (M6 skill, §4.3)
// §16 #2: Drug Dealer cap split — base without a drug bag, doubled with one. The
// bag is acquired via M6 refill points; the split is inert until pHasDrugBag can
// be set (pHasDrugBag defaults 0), but the structure ships now.
#define DRUG_DEALER_BASE		2500		// Drug Dealer carry cap without a drug bag (§16 #2)
#define DRUG_DEALER_WITH_BAG	5000		// Drug Dealer carry cap with a drug bag (§16 #2)
#define DRUG_MAX_SINGLE			60			// max single intake before overdose (§4.3)
#define DRUG_CLICK_GRAMS		5			// grams consumed per click in click-mode (§4.3)
#define DRUG_EFFECT_SECS_PER_G	5			// ~5s of buzz effect per gram (§4.3)
#define DRUG_HEAL_TICK_SECS		2			// small heal every ~1.5-2s while buzzed (§4.3)
#define DRUG_HEAL_PER_TICK		1.5			// HP healed per heal tick (§4.3)
#define DRUG_OD_DMG_PER_SEC		2.0			// overdose slow-death HP loss per second (§4.3)
#define DRUG_BADCHANCE			8			// % chance a dose is "bad" (§4.3)
#define DRUG_BAD_STD_CHANCE		35			// % a bad dose also gives an STD (§4.3)

// Drug buzz stages (bottom-right HUD text, §4.3 — Low/Medium/High/Very High).
enum
{
	BUZZ_NONE,		// 0
	BUZZ_LOW,		// 1
	BUZZ_MEDIUM,	// 2
	BUZZ_HIGH,		// 3
	BUZZ_VERYHIGH	// 4
}

// /jumpkick — small melee to a player directly in front (design §5 / jobs §7.1).
#define JUMPKICK_RANGE			2.0			// reach in front of the player (§5)
#define JUMPKICK_DAMAGE			8.0			// melee HP damage on a landed jump-kick (§5)
#define JUMPKICK_CD				5			// per-player cooldown seconds [OUR DESIGN]

#define isNull(%1) \
                ((!(%1[0])) || (((%1[0]) == '\1') && (!(%1[1]))))