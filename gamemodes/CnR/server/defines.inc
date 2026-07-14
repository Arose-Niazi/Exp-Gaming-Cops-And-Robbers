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
	REFILL_DIALOG,          // M3 (stage 2) — cop /refill menu (HP/armour/weapons)
	BANK_DIALOG,            // M4 (stage 1) — bank menu (/bank /atm — balance + deposit/withdraw)
	BANK_DEPOSIT_DIALOG,    // M4 (stage 1) — deposit amount input
	BANK_WITHDRAW_DIALOG,   // M4 (stage 1) — withdraw amount input
	RADIO_DIALOG            // M4 (stage 2) — /radio preset stream URL picker (cmds/player.inc)
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

// ============================================================================
//  M4 (stage 1) — bank, taxes & insurance (the money core) — systems/bank.inc
//  Grounding: docs/GAMEPLAY-DESIGN.md §10, §11.2, §11.4 + docs/research/economy.md
//  §1 (money flow, bank interest, taxes, insurance).
// ============================================================================

// ---- Money safety rail -----------------------------------------------------
// Every money amount is validated positive/integer and clamped below this cap
// so no single deposit/withdraw/transfer/interest step can overflow the signed
// 32-bit cell (max 2,147,483,647). economy.md §1.3 notes CB caps a single
// withdraw at 99,999,999; we use a higher headroom cap here.
#define MAX_MONEY				2000000000	// hard ceiling for cash/bank per player (overflow guard)

// GameModeClock() advances 1 game-minute per real second, so one game-day =
// 24*60 = 1440 real seconds. Used to express insurance durations in game-days.
#define SECS_PER_GAME_DAY		1440

// ---- Bank interest (economy.md §1.3 — ~5-15% every 2 game days) ------------
// We pick 8% as a middle value of the researched 5-15% band. Paid on BankMoney
// on the game-clock day schedule (every BANK_INTEREST_DAYS game-days) via
// Bank_OnGameDay() hooked into GameModeClock's day transition — no new timer.
#define BANK_INTEREST_PCT		8			// % of BankMoney paid as interest (research band 5-15%)
#define BANK_INTEREST_DAYS		2			// pay interest every N game-days (economy.md §1.3)

// ---- /givecash (/pay) hand-cash transfer (economy.md §1.3, §10) ------------
// NOT a bank transfer: physical cash handed to a nearby player. Anti-abuse:
// positive integer amount, giver must hold it, proximity + cooldown, logged.
#define GIVECASH_RANGE			5.0			// on-foot reach to hand cash to a nearby player
#define GIVECASH_CD				10			// per-giver cooldown seconds (anti-spam) [OUR DESIGN]
#define GIVECASH_MIN			1			// minimum hand-cash amount

// ---- Taxes (economy.md §1.4 — every 2 game-days; GAMEPLAY §10) -------------
// Wealth tax: on cash+bank once it exceeds the threshold (donators pVip immune,
// economy.md §1.4). Income tax: on money earned since the last bill. Both run on
// the game-clock day schedule via Tax_OnGameDay(). Deduct bank-first then cash;
// any unpayable remainder accrues to TaxOwed and is chased next tick.
#define TAX_DAYS				2			// run the tax tick every N game-days (economy.md §1.4)
#define WEALTH_TAX_THRESHOLD	10000000	// wealth (cash+bank) above this is taxed (economy.md §1.4: ~$10M)
#define WEALTH_TAX_PCT			12			// wealth-tax rate on the amount OVER the threshold (economy.md §1.4)
#define INCOME_TAX_PCT			7			// income-tax rate on money earned since last bill (economy.md §1.4)

// ---- Insurance (economy.md §1.5/§1.6; GAMEPLAY §11.2) ----------------------
// Medical fee charged on death from cash-first-then-bank unless the player holds
// active Health Insurance (HealthInsExpiry > now). Life Insurance is a stored
// policy count that waives the medical fee AND preserves weapons on death.
#define MEDICAL_FEE				5000		// flat medical fee on death (economy.md §1.5) [OUR DESIGN — CB scales it]
#define HEALTH_INS_PRICE		15000		// /buyinsurance health cover price [OUR DESIGN]
#define HEALTH_INS_DAYS			5			// health cover duration in game-days (economy.md §1.6: 5 game-days)
#define LIFE_INS_PRICE			75000		// life policy price (economy.md §1.6: ~$75,000)
#define LIFE_INS_MAX			3			// max life policies held at once (economy.md §1.6)

// ============================================================================
//  M4 (stage 2) — lotto, money events, /ad + /advert, holdup base
//  Grounding: docs/GAMEPLAY-DESIGN.md §5.2 (holdup), §7 (/moneybag /moneyrush
//  /ad /advert /radio), §9.5 (lotto), §10.3 (the clock-tick draw fix), §11.4
//  (server_data.LottoJackpot) + docs/research/economy.md §4-6.
// ============================================================================

// ---- Lotto (systems/lotto.inc — design §9.5 / §10.3) -----------------------
// Pick-a-number lotto: buy a ticket for the game-day, drawn at game-hour 18
// inside GameModeClock's hour transition (NOT a real-time timer — the legacy
// "lottery timer bug" fix, §10.3). Jackpot lives in server_data.LottoJackpot.
#define LOTTO_MAX				125			// pick range 1..125 (§9.5)
#define LOTTO_TICKET_PRICE		5000		// ticket price via Bank_TakeCash (§9.5)
#define LOTTO_DRAW_HOUR			18			// draw fires at this game-hour (§9.5 / §10.3)
#define LOTTO_JACKPOT_BASE		1000000		// jackpot floor after a paid-out/rolled draw (§9.5 / §11.4)
#define LOTTO_WIN_SCORE			5			// score to each winner (§9.5, "+5 score")
#define LOTTO_PLAY_SCORE		1			// participation score to a winner (§9.5, "+1 participation")

// ---- Money bag (systems/moneybag.inc — design §7 / economy §5) -------------
// One hidden bag at a time; the finder is paid a random reward. Zone hint is
// coarse (warmer/colder by distance). Bag times out + relocates.
#define MONEYBAG_REWARD_MIN		250000		// reward floor  random(250k..2M) (§7)
#define MONEYBAG_REWARD_MAX		2000000		// reward ceiling random(250k..2M) (§7)
#define MONEYBAG_PICKUP_RANGE	3.0			// reach to grab the bag on foot [OUR DESIGN]
#define MONEYBAG_TIMEOUT_SECS	600			// bag relocates if unclaimed for this long [OUR DESIGN]
#define MONEYBAG_HINT_CD		5			// per-player /moneybag hint cooldown seconds [OUR DESIGN]
#define MONEYBAG_HINT_WARM		60.0		// within this: "very warm"  [OUR DESIGN]
#define MONEYBAG_HINT_HOT		150.0		// within this: "warm"       [OUR DESIGN]
#define MONEYBAG_HINT_MILD		400.0		// within this: "cold"       [OUR DESIGN]
#define MONEYBAG_MODEL			1550		// money-bag pickup model (Money) [OUR DESIGN]

// ---- Money rush (systems/moneyrush.inc — design §7/§8.2 / economy §6.2) ----
// Owner-started cash-rain event: dynamic pickups worth random(1k..25k) grabbed
// via Bank_GiveCash, PvP allowed during, per-event leaderboard, cooldown.
#define MONEYRUSH_CASH_MIN		1000		// pickup cash floor   random(1k..25k) (§7)
#define MONEYRUSH_CASH_MAX		25000		// pickup cash ceiling random(1k..25k) (§7)
#define MONEYRUSH_DURATION		180			// event length seconds [OUR DESIGN]
#define MONEYRUSH_COOLDOWN		600			// cooldown before the next rush [OUR DESIGN]
#define MONEYRUSH_PICKUPS		40			// dynamic pickups scattered per event [OUR DESIGN]
#define MONEYRUSH_PICKUP_MODEL	1212		// money pickup model (Money) [OUR DESIGN]
#define MONEYRUSH_SPREAD		120.0		// pickups scatter radius around the arena [OUR DESIGN]
// Arena centre (Los Santos — Glen Park, an open flat area) [OUR DESIGN].
#define MONEYRUSH_X				(1990.0)
#define MONEYRUSH_Y				(-1440.0)
#define MONEYRUSH_Z				(13.5)

// ---- /ad adrenaline + /advert paid ad (cmds/player.inc — design §7/§4.2/§4.3)
// /ad = adrenaline pill: full heal + STD cure-all + overdose clear; carry up to
// ADRENALINE_MAX. /advert = paid server-wide coloured line.
#define ADRENALINE_MAX			3			// adrenaline pills carried (§7, "carry 3")
#define ADRENALINE_START		1			// starter pill granted on first /ad (acquisition deferred)
#define ADVERT_COST				200			// /advert cost via Bank_TakeCash (§7)
#define ADVERT_COOLDOWN			90			// /advert cooldown seconds (§7)
#define ADVERT_MAX_CHARS		100			// /advert text cap (§7)

// ---- Store holdup base (systems/holdup.inc — design §5.2) -------------------
// On-foot near a store/24-7 clerk actor: +7 wanted, ~10s clerk wait, then pays
// $2000-6000/sec for a short duration. Full robbery system + crowbar bonus M5.
#define HOLDUP_WANTED			7			// wanted set on holdup start (§5.2, +7 Felon)
#define HOLDUP_CLERK_WAIT		10			// seconds the clerk waits before the register opens (§5.2)
#define HOLDUP_PAYOUT_SECS		6			// seconds the register pays out for [OUR DESIGN]
#define HOLDUP_RATE_MIN			2000		// register payout floor per second (§5.2)
#define HOLDUP_RATE_MAX			6000		// register payout ceiling per second (§5.2)
#define HOLDUP_STORE_CD			300			// per-player holdup cooldown seconds (§5.2 [OUR DESIGN])
#define HOLDUP_ACTOR_RANGE		4.0			// reach to a store clerk actor to start [OUR DESIGN]

// ============================================================================
//  M4 (stage 3) — reusable mission framework + 4 core missions — systems/missions.inc
//  Grounding: docs/GAMEPLAY-DESIGN.md §6 (missions intro), §6.4 Drug Delivery,
//  §6.18 Truck Delivery, §6.17 Trash Pickup, §6.6 Food Delivery, §6.20 (milestone
//  placement) + docs/research/crazybobs-site-full.md (§Missions, exact payouts).
//
//  The framework is DATA-DRIVEN: every mission is a row in MissionReg[] (id, name,
//  type civ/cop, cooldown in game-hours, requires-vehicle + accepted model set,
//  payout-per-checkpoint, all-complete bonus, per-checkpoint wanted-if-cop) plus a
//  small route table (MissionRoute[]) of candidate checkpoint coords the launcher
//  picks from. M5/M6 add a mission in ~30 lines: append a MissionReg row, add its
//  route coords, and (only if it needs bespoke logic) a couple of switch arms.
// ============================================================================

// ---- Mission ids (index into MissionReg[]) — APPEND new missions, never renumber.
enum
{
	MISSION_DRUG_DELIVERY,		// 0 — civ, 5 CP, $5k/cp + $50k bonus, 5 game-hr CD, +2 wanted/cp if cop near (§6.4)
	MISSION_TRUCK_DELIVERY,		// 1 — civ, needs a delivery truck, 5 CP route, 12 game-hr window/CD (§6.18)
	MISSION_TRASH_PICKUP,		// 2 — civ, needs a Trashmaster, 5 CP, $2.5k/cp + $25k bonus (§6.17)
	MISSION_FOOD_DELIVERY,		// 3 — civ, needs a food vehicle, 5 CP, $2.5k/cp + $25k bonus (§6.6)
	MISSION_COUNT				// keep last — number of registered missions (array sizes)
}

// ---- Mission type flags (MissionReg[][mType]) — gate civ-vs-cop entry (§6). ----
#define MISSION_TYPE_CIVILIAN	0			// civilians only (IsCivilian gate)
#define MISSION_TYPE_COP		1			// cops only (IsCop gate)

// ---- Framework tunables ----------------------------------------------------
#define MISSION_MAX_CHECKPOINTS	6			// max checkpoints a route can have (array bound; all 4 use 5)
#define MISSION_MAX_ROUTE_POOL	10			// candidate points per mission the launcher samples from
#define MISSION_CP_SIZE			5.0			// race-checkpoint radius (delivery routes)
#define MISSION_START_CP_SIZE	4.0			// on-foot/vehicle reach at a launcher marker [OUR DESIGN]
#define MISSION_OFFROUTE_DIST	3000.0		// abort if this far from the active checkpoint (stuck/teleport guard)
#define MISSION_COPSIGHT_RANGE	80.0		// a cop within this (line-of-city sight) counts as "in sight" (§6.4) [OUR DESIGN]
#define MISSION_START_DIALOG	5301		// /missions launcher list dialog id (unique — avoids the defines.inc enum)

// ---- Game-time cooldown clock ----------------------------------------------
// Missions cool down in GAME hours, not real seconds. We stamp completion with a
// monotonic game-hour = g_GameDayCounter*24 + GameHour (never rewinds on the
// game-week rollover because g_GameDayCounter is monotonic). A mission is ready
// when (nowGameHour - lastStamp) >= its cooldown, or the stamp is 0 (never done).
#define MISSION_HOURS_PER_DAY	24

// ---- Per-mission payouts / cooldowns (VERIFIED from §6 + Score Guide) -------
// Drug Delivery (§6.4): $5,000/cp + $50,000 all-5 bonus = $75k; 5 game-hr CD.
#define MDRUG_CP_PAY			5000
#define MDRUG_BONUS				50000
#define MDRUG_COOLDOWN_HRS		5
#define MDRUG_WANTED_PER_CP		2			// +2 wanted per checkpoint if a cop is in sight (§6.4)

// Truck Delivery (§6.18): base pay per delivery; 12 game-hr window/cooldown.
// Payout formula not published — [OUR DESIGN] a per-cp base + a completion bonus
// comparable to the trash/food loop (trucks are longer hauls, so a touch higher).
#define MTRUCK_CP_PAY			4000
#define MTRUCK_BONUS			30000
#define MTRUCK_COOLDOWN_HRS		12

// Trash Pickup (§6.17): $2,500/cp + $25,000 all-5 bonus = $37,500; 2 game-hr CD.
#define MTRASH_CP_PAY			2500
#define MTRASH_BONUS			25000
#define MTRASH_COOLDOWN_HRS		2

// Food Delivery (§6.6): $2,500/cp + $25,000 all-5 bonus = $37,500; 2 game-hr CD.
#define MFOOD_CP_PAY			2500
#define MFOOD_BONUS				25000
#define MFOOD_COOLDOWN_HRS		2

#define isNull(%1) \
                ((!(%1[0])) || (((%1[0]) == '\1') && (!(%1[1]))))