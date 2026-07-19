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
	RADIO_DIALOG,           // M4 (stage 2) — /radio preset stream URL picker (cmds/player.inc)
	CROWBAR_BUY_DIALOG,     // M5 (stage 2) — retired in M6 (the crowbar is now a /clotheswear menu entry, systems/clothes.inc); id kept to preserve enum numbering
	HOUSE_MENU_DIALOG,      // M5 (stage 3) — /house owner menu (storage/lock/spawn/sell/rent, §9.1)
	HOUSE_BUY_DIALOG,       // M5 (stage 3) — /buyhouse confirm prompt (§9.1)
	HOUSE_STORE_DIALOG,     // M5 (stage 3) — /housestore deposit-money input (§9.1)
	HOUSE_WITHDRAW_DIALOG   // M5 (stage 3) — /housewithdraw money input (§9.1)
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

// ---- M5 (stage 1) : vehicle ownership — CB-simple model (design §9.12) -------
#define WANTED_GTA				2			// grand theft auto: taking an unoccupied owned/parked vehicle (§9.12)
#define VEH_RESTEAL_GRACE		180			// seconds you may re-take your own last-driven vehicle without a GTA crime (§9.12, ≈3 min)

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
//  M5 (stage 2) — full robbery system + crowbar — systems/robbery.inc
//  Grounding: docs/GAMEPLAY-DESIGN.md §5 (§5.1 shoplift, §5.2 holdup upgrade,
//  §5.3 bank, §5.4 casino, §5.5 house, §5.6 special/major, §5.7 crowbar) +
//  docs/research/crazybobs-site-full.md (§Robberies and Crowbar — exact numbers).
//  All payouts move through the Bank_* API; the bank rob physically drains
//  online victims' BANK balances (no minting) with a 75% gov-void insured refund.
// ============================================================================

// ---- The crowbar (clothing item, §5.7) -------------------------------------
#define CROWBAR_PRICE			40000		// $40,000 buy price via Bank_TakeCash (§5.7 VERIFIED)
#define CROWBAR_WAIT_REDUCTION	3			// seconds shaved off the ~10s clerk wait when equipped (§5.7 [OUR DESIGN])
#define CROWBAR_RATE_MIN		4000		// register floor per-second WITH a crowbar (§5.7 higher take)
#define CROWBAR_RATE_MAX		9000		// register ceiling per-second WITH a crowbar (§5.7 higher take)
#define CROWBAR_SAFE_REDUCTION	8			// seconds shaved off a safe-crack when equipped (§5.7 [OUR DESIGN])
#define CROWBAR_SUCCESS_BONUS	20			// +% success chance on a safe-crack when equipped (§5.7 [OUR DESIGN])

// Robbery-history bitmask flags (LSplayers.RobberyHistory) — a set bit means the
// player has committed that robbery type; the crowbar is confiscated on arrest
// if ANY of these is set (unless the arrest was a lethal cop takedown) (§5.7).
#define ROB_HOLDUP				1			// store holdup (§5.2)
#define ROB_BANK				2			// bank robbery (§5.3)
#define ROB_CASINO				4			// casino robbery (§5.4)
#define ROB_HOUSE				8			// house robbery (§5.5)
#define ROB_SPECIAL				16			// special/major robbery (§5.6)
#define ROB_ANY_MASK			(ROB_HOLDUP|ROB_BANK|ROB_CASINO|ROB_HOUSE|ROB_SPECIAL)

// ---- Shoplifting (§5.1) ----------------------------------------------------
#define SHOPLIFT_WANTED			2			// +2 wanted, low-risk item steal (§5.1 [OUR DESIGN])
#define SHOPLIFT_CD				120			// per-player cooldown seconds (§5.1 [OUR DESIGN])
#define SHOPLIFT_CASH_MIN		200			// petty item resale value floor (§5.1 [OUR DESIGN])
#define SHOPLIFT_CASH_MAX		900			// petty item resale value ceiling (§5.1 [OUR DESIGN])

// ---- Bank robbery (§5.3) ---------------------------------------------------
#define BANKROB_CP_SIZE			3.0			// bank/hideout dynamic checkpoint radius
#define BANKROB_ICON_MODEL		52			// map-icon model for banks (pickup/rob blip)
#define BANKROB_CRACK_SECS		30			// ~30s at the checkpoint to crack the safe (§5.3 VERIFIED)
#define BANKROB_WANTED_START	6			// set WL6 when the crack begins (§5.3, §2.3)
#define BANKROB_WANTED_CARRY	3			// +3 more (→9-10) once carrying the safe (§5.3 escalation)
#define BANKROB_MAX_PER_ACCT	100000		// max drained per victim bank account (§5.3 VERIFIED)
#define BANKROB_YIELD_MAX		1000000		// full-server total yield ceiling (§5.3 VERIFIED ~$500k-1M)
#define BANKROB_INSURED_PCT		75			// % of a victim's loss refunded if bank-insured (§5.3 VERIFIED, gov/void source)
#define BANKROB_HIDEOUT_SECS	720			// real-seconds to reach the hideout with the safe (12 min, §5.6 window)
#define BANKROB_BRANCH_CD		3600		// per-branch real-second cooldown after a rob (§5.3 [OUR DESIGN] 60 min)

// ---- Casino / special robberies (§5.4/§5.6) --------------------------------
#define CASINOROB_CP_SIZE		3.0			// casino/special dynamic checkpoint radius
#define CASINOROB_CRACK_SECS	40			// safe-crack seconds at a casino (§5.4 [OUR DESIGN])
#define SPECIALROB_CRACK_SECS	50			// safe-crack seconds at a special/major site (§5.6 [OUR DESIGN])
#define ROB_HIDEOUT_SECS		720			// 12-min real-time hideout window (§5.6 VERIFIED)
#define ROB_SITE_CD				1800		// per-site real-second cooldown [OUR DESIGN] 30 min
#define CASINOROB_YIELD_MIN		250000		// casino payout floor (~50-75% of bank, §5.4 [OUR DESIGN])
#define CASINOROB_YIELD_MAX		600000		// casino payout ceiling (§5.4 [OUR DESIGN])
#define SPECIALROB_YIELD_MIN		400000	// special/major payout floor (§5.6 [OUR DESIGN])
#define SPECIALROB_YIELD_MAX		900000	// special/major payout ceiling (§5.6 [OUR DESIGN])
#define ROB_WANTED_START		6			// set WL6 when a safe-crack begins (§5.4/§5.6, §2.3)
#define ROB_WANTED_CARRY		3			// +3 more (→9-10) once carrying the safe
#define ROB_CRACK_BASE_SUCCESS	70			// base % chance a safe-crack succeeds; crowbar adds CROWBAR_SUCCESS_BONUS (§5.7 [OUR DESIGN])

// Robbery in-progress stages (pRobStage). The store holdup keeps its own
// pHoldingUp/pHoldupStage machine; these drive the safe-type sequence.
#define ROBSTAGE_NONE			0			// no safe-type robbery running
#define ROBSTAGE_CRACKING		1			// standing at the site cracking the safe
#define ROBSTAGE_CARRYING		2			// safe cracked — deliver it to the hideout

// Robbery "site kind" (pRobKind) — which payout/history flag a running safe-type
// robbery resolves to.
#define ROBKIND_BANK			0
#define ROBKIND_CASINO			1
#define ROBKIND_SPECIAL			2

// ============================================================================
//  M5 (stage 3) — housing system — systems/housing.inc
//  Grounding: docs/GAMEPLAY-DESIGN.md §9.1 (housing spec), §5.5 (house robbery),
//  §11.3 (houses table) + docs/research/{crazybobs-site-full.md §Housing,
//  economy.md §3}. All money moves through the Bank_* API (no minting); the
//  house robbery MOVES house-storage money into the robber's hand (no mint).
// ============================================================================

#define MAX_HOUSES				900			// hard array bound (DB `houses` rows loaded on init; 621 seeded LS+countryside)
#define MAX_OWNED_HOUSES		2			// own-cap (§9.1 VERIFIED — own 2 / co-own 8 / rent 10)
#define HOUSE_ENTER_RANGE		2.5			// on-foot reach at an entrance pickup for /enter//buyhouse (§9.1)
#define HOUSE_PICKUP_MODEL		1273		// green house pickup at the entrance (money-house icon)
#define HOUSE_ICON_OWNED		31			// map-icon: an owned house (blue house)
#define HOUSE_ICON_FORSALE		32			// map-icon: a for-sale house (green house)
#define HOUSE_ICON_RANGE		150.0		// map-icon streamer stream distance
#define HOUSE_LABEL_RANGE		18.0		// entrance 3D-label draw distance

// Prices [OUR DESIGN — CB never published a table]: sane market range.
#define HOUSE_PRICE_MIN			250000		// $250k floor (rural shack) (§9.1 [OUR DESIGN])
#define HOUSE_PRICE_MAX			5000000		// $5M ceiling (Mulholland estate) (§9.1 [OUR DESIGN])
#define HOUSE_SELL_PCT			70			// % of price the bank pays on a /sellhouse to server (§3.4 "below market")
#define HOUSE_POLICE_DISCOUNT	10			// % discount for full-time police on /buyhouse (§9.1/§3.1 VERIFIED)
#define HOUSE_VALUE_BUMP_PCT	3			// each purchase raises the house value by this % (§3.1 "each sale raises the value")

// Super Lock upgrade (§5.5) — a paid anti-robbery upgrade that lowers break-in success.
#define HOUSE_SUPERLOCK_PRICE	100000		// $100k to fit a super lock (§5.5 [OUR DESIGN])

// Rent (§3.4) — owner sets a per-game-day rent; tenants auto-pay each game-day
// they pass in-server. Kept MINIMAL for M5: single-renter, charged on the day tick.
#define HOUSE_RENT_MIN			500			// min per-game-day rent an owner may set [OUR DESIGN]
#define HOUSE_RENT_MAX			50000		// max per-game-day rent an owner may set [OUR DESIGN]

// House storage (§9.1/§5.5). Money is unlimited; a simple item count is kept for
// M5 (the full per-item stash — drugs/condoms/flowers/… — is deferred to M6).
#define HOUSE_STORE_ITEMS_MAX	20			// simple stored-item count cap (§5.5 clothing cap as the proxy) [M5 minimal]

// House robbery (§5.5). The robber steals house-storage MONEY (moved, no mint) +
// a slice of stored items. A super lock / pet defense roll can foil the break-in.
#define HOUSEROB_RANGE			4.0			// on-foot reach of the target house entrance for /houserob (§5.5)
#define HOUSEROB_WANTED			6			// +6 wanted warrant on a house-rob attempt (§5.5 [OUR DESIGN])
#define HOUSEROB_CD				300			// per-robber cooldown seconds between house robs [OUR DESIGN] 5 min
#define HOUSEROB_BASE_SUCCESS	65			// base % a break-in succeeds (§5.5 [OUR DESIGN])
#define HOUSEROB_SUPERLOCK_PEN	35			// -% success when the house has a super lock (§5.5 "much harder") [OUR DESIGN]
#define HOUSEROB_PET_PEN		15			// -% success + pet bite when the house has a pet (§5.5 pets) [OUR DESIGN]
#define HOUSEROB_PET_DAMAGE		25.0		// HP a foiled break-in loses to a house pet (§5.5) [OUR DESIGN]
#define HOUSEROB_MONEY_PCT		60			// % of stored money the robber gets away with (§5.5 [OUR DESIGN])

// Inactivity decay (§3.2 VERIFIED — an unvisited house is lost after 2 real weeks).
// We use REAL time (LastVisited DATETIME → unix seconds); the check runs on the
// low-frequency game-day tick. A decayed house reverts to for-sale (owner cleared).
#define HOUSE_DECAY_SECS		1209600		// 2 real-world weeks in seconds (§3.2 VERIFIED)

// House interior virtual worlds start here so each house gets its own isolated VW
// (VW = HOUSE_VW_BASE + house index), well clear of the interior builder's range.
#define HOUSE_VW_BASE			20000

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
	// ---- M5 (stage 4) — the 3 M5 robbery missions (append, never renumber). ----
	MISSION_VEHICLE_THEFT,		// 4 — civ, 5 marked cars → chop-shop CP, $15k/car + $75k bonus, LAST car = WL10 (§6.19)
	MISSION_HOLDUP_MISSION,		// 5 — civ, chain 3 store holdups ($10k+ each) + completion bonus (§6.7)
	MISSION_AIRPORT_ROBBERY,	// 6 — civ, truck/van/garbage truck, /box box-collection mechanic, boxes→payout (§6.1)
	// ---- M6 (stage 4) — the final ~12 missions (append, never renumber). --------
	MISSION_COURIER_DELIVERY,	// 7 — civ, any vehicle, 5 city drops, warrant on pickup, distance pay, 12 game-hr window (§6.2)
	MISSION_HOUSE_DELIVERY,		// 8 — civ, any vehicle, 5 house drops (some illegal → wanted), 12 game-hr window (§6.8)
	MISSION_ILLEGAL_IMMIGRANT,	// 9 — innocent civ, pickup+return route, WANTED ON COMPLETION (not pickup), night hides cops (§6.9)
	MISSION_LAWN_MOWING,		// 10 — civ, needs a mower, timed mow route (as-many-CPs), completion bonus (§6.10)
	MISSION_PAPERBOY,			// 11 — innocent civ, ON FOOT, 4-min REAL-TIME limit, $500/delivery + count bonus (§6.11)
	MISSION_PICKPOCKET,			// 12 — civ (Pickpocket skill scales), rob $10k target from players via /pickpocket (§6.12)
	MISSION_SEXUAL_ENCOUNTER,	// 13 — innocent civ (Rapist/Prostitute scale), 5 encounters via /encounter, 5 game-hr (§6.15)
	MISSION_TRACTOR,			// 14 — civ, needs a tractor, wraps the /farmjob crop system at CrazyBob's Farm (§6.16)
	MISSION_FLOWER_DELIVERY,	// 15 — cop OR civ, deliver flowers to 5 players via /flowers, 6 game-hr (§6.5)
	MISSION_RACE_CHALLENGE,		// 16 — all players, /challenge race framework (28 races as data), +1 finish/+2 podium (§6.13)
	MISSION_ROUTINE_PATROL,		// 17 — COP, 5 sequential PD checkpoints, $2.5k/cp + $25k bonus = $37.5k (§6.14)
	MISSION_DOMESTIC_DISTURBANCE,// 18 — COP, drive PD → a player-owned house, faster = bigger bonus (§6.3)
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

// ============================================================================
//  M5 (stage 4) — kidnap + the 3 M5 robbery missions — systems/kidnap.inc +
//  systems/missions.inc. Grounding: docs/GAMEPLAY-DESIGN.md §4.4 (kidnap/holdup),
//  §6.19 Vehicle Theft "Gone in 15 Hours", §6.7 Holdup Mission, §6.1 Airport
//  Robbery (box-collection) + docs/research/{crazybobs-site-full.md,
//  primary-sources.md} (Kidnapper skill: ransom ≤$50k + 20-80% on-hand steal;
//  Vehicle Theft verbatim $15k/car + $75k bonus = $150k; last car = WL10).
//  All payouts move through the Bank_* API. The 3 missions are framework rows.
// ============================================================================

// ---- Vehicle Theft "Gone in 15 Hours" (§6.19) ------------------------------
// 5 randomly-marked target cars delivered to a chop-shop checkpoint. The route
// framework drives the 5 chop-shop drops (one per car); each drop = $15k, the
// final delivery pays the $75k completion bonus (=$150k total) AND the last-car
// entry triggers instant WL10 (Most Wanted). 15 game-hour cooldown.
#define MVEHTHEFT_CP_PAY		15000		// $15,000 bonus per vehicle delivered (§6.19 VERIFIED)
#define MVEHTHEFT_BONUS			75000		// $75,000 all-5 completion bonus (§6.19 VERIFIED, total $150k)
#define MVEHTHEFT_COOLDOWN_HRS	15			// 15 game-hour cooldown (§6.19 — the "15 hours" window/gate)
#define MVEHTHEFT_WANTED_PER_CP	1			// prior cars raise wanted moderately [OUR DESIGN]; last car forces WL10 (§6.19)

// ---- Holdup Mission (§6.7) — chain 3 store holdups --------------------------
// A mission wrapper: rob 3 different stores (min $10k each is the design floor;
// the underlying /holdup register drives the actual take) + a completion bonus.
// The route framework marks 3 store-holdup checkpoints; each reached checkpoint
// runs a scripted register take (mission-owned, mirrors the /holdup rate band)
// and requires the $10k minimum. 5 game-hour cooldown.
#define MHOLDUP_STORES			3			// rob 3 different stores (§6.7 VERIFIED)
#define MHOLDUP_MIN_PER_STORE	10000		// minimum $10,000 stolen from each store (§6.7 VERIFIED)
#define MHOLDUP_CP_PAY			10000		// guaranteed per-store take (the $10k floor; §6.7) — mission-owned register
#define MHOLDUP_CP_BONUS_MAX		8000	// extra random register take on top of the floor per store [OUR DESIGN]
#define MHOLDUP_BONUS			40000		// large completion bonus for all 3 (§6.7 "large bonus") [OUR DESIGN]
#define MHOLDUP_COOLDOWN_HRS	5			// 5 game-hour cooldown (high-reward mission, §6.7)
#define MHOLDUP_WANTED_PER_CP	7			// +7 per store holdup → cumulative high warrant (§6.7 / §5.2)

// ---- Airport Robbery (§6.1) — box-collection mechanic ----------------------
// Truck/van/garbage-truck gated. On start a collect-timer runs; the robber exits
// the vehicle and grabs boxes (MMB / /box) at scattered box pickups. When the
// timer expires they drive the loot to a hideout checkpoint; payout scales with
// boxes collected. Wanted while collecting. NOT a plain checkpoint route.
#define MAIRPORT_COLLECT_SECS	120			// real seconds to collect boxes before the timer expires [OUR DESIGN]
#define MAIRPORT_HIDEOUT_SECS	180			// real seconds to reach the hideout with the loot [OUR DESIGN]
#define MAIRPORT_BOX_COUNT		12			// box pickups scattered around the airport apron per run [OUR DESIGN]
#define MAIRPORT_BOX_SPREAD		60.0		// scatter radius of the box pickups around the airport [OUR DESIGN]
#define MAIRPORT_BOX_RANGE		3.0			// on-foot reach to grab a box with MMB / /box [OUR DESIGN]
#define MAIRPORT_BOX_PICKUP_MODEL	1558	// box object model for the pickups (Box)
#define MAIRPORT_HIDEOUT_CP_SIZE	4.0			// hideout delivery checkpoint radius
#define MAIRPORT_PAY_PER_BOX	9000		// cash per box delivered (boxes → payout, §6.1) [OUR DESIGN]
#define MAIRPORT_BONUS			25000		// flat completion bonus on delivery [OUR DESIGN]
#define MAIRPORT_COOLDOWN_HRS	5			// 5 game-hour cooldown (§6.1 [OUR DESIGN])
#define MAIRPORT_WANTED_START	7			// set WL7 on start / while collecting (§6.1 [OUR DESIGN])

// Airport mission phases (pAirportPhase) — bespoke box mechanic, not the route CP.
#define AIRPHASE_NONE			0			// not running the airport box mechanic
#define AIRPHASE_COLLECT		1			// timer running, grabbing boxes on foot (wanted)
#define AIRPHASE_DELIVER		2			// timer expired, drive the loot to the hideout

// ============================================================================
//  M6 (stage 4) — the final ~12 missions — systems/missions.inc (framework rows)
//  Grounding: docs/GAMEPLAY-DESIGN.md §6.2 Courier, §6.3 Domestic Disturbance,
//  §6.5 Flower, §6.8 House Delivery, §6.9 Illegal Immigrant, §6.10 Lawn Mowing,
//  §6.11 Paperboy, §6.12 Pickpocket, §6.13 Race Challenges, §6.14 Routine Patrol,
//  §6.15 Sexual Encounter, §6.16 Tractor + docs/research/crazybobs-site-full.md
//  (exact payouts/structure/cooldowns). All payouts move through the Bank_* API.
//  Most are plain framework rows (route + one switch arm); a few are bespoke
//  target/real-time mechanics (Paperboy real-time deadline, Flower/Pickpocket/
//  Sexual target-based, Race checkpoint route, Domestic single-house destination).
// ============================================================================

// ---- Courier Delivery (§6.2) — smuggle far→city, warrant on pickup ----------
// Any land vehicle; 5 city drops; 12 game-hr window (= the cooldown). Warrant on
// pickup (set at start). Distance-scaled bonus baked into the per-cp/bonus pay.
#define MCOURIER_CP_PAY			6000		// $ per drop [OUR DESIGN — "higher than truck delivery" (§6.2), above MTRUCK]
#define MCOURIER_BONUS			45000		// completion bonus (distance/time payment folded in) [OUR DESIGN]
#define MCOURIER_COOLDOWN_HRS	12			// 12 game-hr window/cooldown (§6.2 VERIFIED time limit)
#define MCOURIER_WANTED_START	4			// smuggling warrant on PICKUP (§6.2 VERIFIED — set at start)

// ---- House Delivery (§6.8) — deliver to houses, some cargo illegal ----------
// Any land vehicle; 5 house drops (reuse M5 housing coords); 12 game-hr window.
// A random subset of drops carry illegal cargo → +wanted if a cop is in sight.
#define MHOUSEDEL_CP_PAY		3500		// $ per house drop [OUR DESIGN — between food ($2.5k) and courier ($6k)]
#define MHOUSEDEL_BONUS			30000		// completion bonus (time/distance folded in) [OUR DESIGN]
#define MHOUSEDEL_COOLDOWN_HRS	3			// 3 game-hr cooldown (§6.8 [OUR DESIGN]; 12-hr window is the throttle)
#define MHOUSEDEL_WANTED_PER_CP	2			// +2 wanted on an ILLEGAL drop if a cop is in sight (§6.8 [OUR DESIGN])
#define MHOUSEDEL_ILLEGAL_PCT	40			// % chance any given drop carries illegal cargo (§6.8 "some illegal") [OUR DESIGN]

// ---- Illegal Immigrant Transport (§6.9) — WANTED ON COMPLETION not pickup ---
// Innocent civ; land vehicle; pickup outside city then return — a 5-CP route (the
// last CP is the city drop). Night reduces cop "sight". Wanted issued only at the
// FINAL drop (§6.9 VERIFIED), never on pickup.
#define MIMMIG_CP_PAY			4000		// $ per leg [OUR DESIGN — smuggling premium over truck]
#define MIMMIG_BONUS			40000		// completion bonus [OUR DESIGN]
#define MIMMIG_COOLDOWN_HRS		4			// 4 game-hr cooldown (§6.9 [OUR DESIGN]; 12-hr window)
#define MIMMIG_WANTED_COMPLETE	5			// wanted issued ON COMPLETION for transporting illegals (§6.9 VERIFIED)
#define MIMMIG_NIGHT_START		20			// game-hour night window start (cops see less) (§6.9 VERIFIED night effect)
#define MIMMIG_NIGHT_END		6			// game-hour night window end
#define MIMMIG_COPSIGHT_DAY		80.0		// cop sight radius by day [OUR DESIGN — matches MISSION_COPSIGHT_RANGE]
#define MIMMIG_COPSIGHT_NIGHT	35.0		// reduced cop sight radius at night (§6.9 VERIFIED) [OUR DESIGN]
#define MIMMIG_WANTED_ENROUTE	2			// +2 wanted mid-route if a cop is in sight AND it is day (stealth, §6.9) [OUR DESIGN]

// ---- Lawn Mowing (§6.10) — timed mow route, competitive checkpoint count -----
// LV golf course; needs a lawn mower; reach as many CPs as possible (we run a
// fixed route + a completion bonus scaled by count). Built-in restart wait on cancel.
#define MLAWN_CP_PAY			800			// $ per mowed checkpoint [OUR DESIGN — low-stakes "relaxing" job (§6.10)]
#define MLAWN_BONUS				8000		// completion bonus for the full route [OUR DESIGN]
#define MLAWN_COOLDOWN_HRS		2			// 2 game-hr cooldown (§6.10 "wait several minutes"/built-in) [OUR DESIGN]
#define MLAWN_MOWER_MODEL		572			// Mower (SA-MP vehicle model 572) — the lawn-mower gate (§6.10 VERIFIED)

// ---- Paperboy (§6.11) — ON FOOT, 4-MINUTE REAL-TIME limit, $500/delivery -----
// Innocent civ; walk to house-front checkpoints; a real-time (gettime) deadline on
// the shared tick, NOT a game-hour window. $500/delivery + a count bonus.
#define MPAPERBOY_CP_PAY		500			// $500 per delivery (§6.11 VERIFIED)
#define MPAPERBOY_BONUS			5000		// completion bonus for the full route [OUR DESIGN — "bonus based on total"]
#define MPAPERBOY_COOLDOWN_HRS	1			// 1 game-hr cooldown (§6.11 "~10 min real-time" — game-hr is ~10 real-min) [OUR DESIGN]
#define MPAPERBOY_TIME_SECS		240			// 4-minute REAL-TIME limit (§6.11 VERIFIED — gettime deadline on the tick)

// ---- Pickpocket Mission (§6.12) — rob a $10k target from players -------------
// Target-based (bespoke, not a route): /pickpocket a nearby non-cop; the running
// take counts toward a $10k goal within a window. Pickpocket skill scales the take.
#define MPICKPOCKET_TARGET		10000		// total $ to steal from players to complete (§6.12 VERIFIED)
#define MPICKPOCKET_COOLDOWN_HRS	3		// 3 game-hr cooldown (§6.12 [OUR DESIGN])
#define MPICKPOCKET_WINDOW_HRS	6			// 6.5 game-hr window rounded to 6 — the run window (§6.12 VERIFIED ~6.5)
#define MPICKPOCKET_RANGE		3.0			// on-foot reach to pickpocket a target (mirrors ROB_RANGE) [OUR DESIGN]
#define MPICKPOCKET_CD_SECS		20			// per-target anti-spam cooldown seconds [OUR DESIGN]
#define MPICKPOCKET_BASE_PCT		15		// % of a target's on-hand cash taken per hit (base) [OUR DESIGN]
#define MPICKPOCKET_SKILL_PCT	35			// % taken per hit WITH the Pickpocket skill (scales, §6.12) [OUR DESIGN]
#define MPICKPOCKET_BASE_SUCCESS	60		// base success chance % (a failed hit alerts the target) [OUR DESIGN]
#define MPICKPOCKET_SKILL_SUCCESS	90		// success chance % WITH the Pickpocket skill (§6.12 "most effective") [OUR DESIGN]
#define MPICKPOCKET_WANTED		2			// +2 wanted per successful pickpocket (§2.3 /rob) [VERIFIED underlying]
#define MPICKPOCKET_COMPLETE_BONUS	15000	// completion bonus once the $10k target is met [OUR DESIGN]

// ---- Sexual Encounter (§6.15) — complete 5 encounters via /encounter --------
// Target-based (bespoke): /encounter a nearby non-cop; 5 successful encounters in
// the window completes. Rapist/Prostitute skill boosts the per-encounter success.
#define MSEX_ENCOUNTERS			5			// 5 encounters to complete (§6.15 VERIFIED)
#define MSEX_COOLDOWN_HRS		3			// 3 game-hr cooldown (§6.15 [OUR DESIGN])
#define MSEX_WINDOW_HRS			5			// 5 game-hr window (§6.15 VERIFIED)
#define MSEX_RANGE				3.0			// on-foot reach to a target for /encounter [OUR DESIGN]
#define MSEX_CD_SECS			20			// per-encounter anti-spam cooldown seconds [OUR DESIGN]
#define MSEX_BASE_SUCCESS		55			// base per-encounter success chance % [OUR DESIGN]
#define MSEX_SKILL_SUCCESS		85			// success chance % WITH the Rapist/Prostitute skill (§6.15 VERIFIED boost) [OUR DESIGN]
#define MSEX_PAY_PER_ENCOUNTER	3000		// $ paid per successful encounter [OUR DESIGN]
#define MSEX_COMPLETE_BONUS		20000		// completion bonus for all 5 encounters [OUR DESIGN]

// ---- Tractor Mission (§6.16) — wraps the /farmjob crop system ----------------
// Needs a tractor; the launcher hands off to the M6-stage-2 /farmjob at CrazyBob's
// Farm (a plow/harvest field route). The mission row carries the launcher marker +
// cooldown only; the run itself is farming.inc's field-checkpoint loop.
#define MTRACTOR_COOLDOWN_HRS	2			// 2 game-hr cooldown (§6.16 built-in wait) [OUR DESIGN]
#define MTRACTOR_MODEL			531			// Tractor (SA-MP vehicle model 531) — the tractor gate (§6.16 VERIFIED)

// ---- Flower Delivery (§6.5) — deliver flowers to 5 players via /flowers ------
// COP OR CIV; target-based (bespoke): /flowers a nearby player; 5 distinct players
// in the window completes. No wanted. Can start at the mission marker or a church.
#define MFLOWER_DELIVERIES		5			// 5 different players to complete (§6.5 VERIFIED)
#define MFLOWER_COOLDOWN_HRS	2			// 2 game-hr cooldown (§6.5 [OUR DESIGN])
#define MFLOWER_WINDOW_HRS		6			// 6 game-hr window (§6.5 VERIFIED)
#define MFLOWER_RANGE			4.0			// on-foot reach to hand flowers to a player [OUR DESIGN]
#define MFLOWER_PAY_PER			1500		// $ paid per flower delivered [OUR DESIGN]
#define MFLOWER_COMPLETE_BONUS	12000		// completion bonus for all 5 deliveries [OUR DESIGN — "a cash bonus"]

// ---- Race Challenges (§6.13) — /challenge race framework --------------------
// A checkpoint race against a par time. First attempt free, then a fee. +1 finish,
// +2 podium (par-beat). 28 races on the official site — we ship a representative
// data set spanning land/bike/boat categories; add more by appending g_RaceReg rows.
#define MRACE_COOLDOWN_HRS		1			// 1 game-hr cooldown between race runs [OUR DESIGN — races are repeatable]
#define MRACE_RETRY_FEE			5000		// $ for a second+ attempt at a race (first is free, §6.13 VERIFIED) [OUR DESIGN amount]
#define MRACE_FINISH_PAY		10000		// Top-10/finish cash (§6.13 "Top 10 earns cash") [OUR DESIGN amount]
#define MRACE_PODIUM_BONUS		15000		// extra cash for a podium (beat the par time) finish [OUR DESIGN]
#define MRACE_CP_SIZE			8.0			// race-checkpoint radius (bigger for driving speed) [OUR DESIGN]
#define MRACE_MAX_CHECKPOINTS	8			// max checkpoints in a single race route (array bound) [OUR DESIGN]
#define MRACE_SELECT_DIALOG		5305		// /challenge race picker dialog id (unique) [OUR DESIGN]
#define MRACE_CATEGORY_LAND		0			// land vehicles (§6.13)
#define MRACE_CATEGORY_BIKE		1			// bikes only (§6.13)
#define MRACE_CATEGORY_BOAT		2			// boats only (§6.13)

// ---- Routine Patrol (§6.14, COP) — 5 sequential PD checkpoints --------------
// Cop-only; land vehicle; 5 sequential checkpoints; $2,500/cp + $25,000 bonus =
// $37,500 (VERIFIED verbatim). No wanted. 2 game-hr cooldown.
#define MPATROL_CP_PAY			2500		// $2,500 per checkpoint (§6.14 VERIFIED)
#define MPATROL_BONUS			25000		// $25,000 completion bonus → $37,500 total (§6.14 VERIFIED)
#define MPATROL_COOLDOWN_HRS	2			// 2 game-hr cooldown (§6.14 [OUR DESIGN])

// ---- Domestic Disturbance (§6.3, COP) — drive PD → a player-owned house -----
// Cop-only; a SINGLE destination (a random player-owned house, reusing M5 housing
// coords); faster arrival = bigger bonus. No wanted. The bonus decays over a par
// time. Bespoke single-CP handler (not a multi-drop route).
#define MDOMESTIC_COOLDOWN_HRS	2			// 2 game-hr cooldown (§6.3 [OUR DESIGN])
#define MDOMESTIC_BASE_PAY		15000		// guaranteed base bonus for reaching the house [OUR DESIGN]
#define MDOMESTIC_SPEED_MAX		25000		// max additional speed bonus (arrive within par) [OUR DESIGN]
#define MDOMESTIC_PAR_SECS		90			// real-seconds "par" — full speed bonus if you beat it (§6.3 "faster=bigger") [OUR DESIGN]
#define MDOMESTIC_TIMEOUT_SECS	300			// real-seconds before the call is abandoned (no bonus) [OUR DESIGN]
#define MDOMESTIC_CP_SIZE		4.0			// destination checkpoint radius [OUR DESIGN]

// ============================================================================
//  M5 (stage 4) — kidnap crime (§4.4) — systems/kidnap.inc
//  /kidnap (/kd) a nearby non-cop: hold them (force into your vehicle / cuff on
//  foot), set a ransom (≤$50k), always earn a warrant (+1 civ / +6 cop kidnapped),
//  drive them to a hideout to steal 20-80% of their on-hand cash. Victim freed by
//  cops (/uncuff proximity), by paying the ransom, by escaping (strays too far),
//  or on the kidnapper's death/arrest/disconnect. Kidnapper skill (ransom scaling)
//  is an M6 skill — the base ransom applies until pSkill can be SKILL_KIDNAPPER.
// ============================================================================
#define KIDNAP_RANGE			4.0			// on-foot reach to grab / release a kidnap target (§4.4)
#define KIDNAP_CD				60			// per-kidnapper cooldown seconds between grabs [OUR DESIGN]
#define KIDNAP_RANSOM_MAX		50000		// ransom ceiling per victim (§4.4 / Kidnapper skill VERIFIED ≤$50k)
#define KIDNAP_RANSOM_DEFAULT	25000		// default ransom if none supplied [OUR DESIGN, half the cap]
#define KIDNAP_STEAL_MIN_PCT	20			// min % of victim on-hand cash stolen at the hideout (§4.4 VERIFIED)
#define KIDNAP_STEAL_MAX_PCT	80			// max % of victim on-hand cash stolen at the hideout (§4.4 VERIFIED)
#define KIDNAP_GRAB_BONUS		1500		// small cash bonus paid to the kidnapper on a successful grab (§4.4 "small bonus") [OUR DESIGN]
#define KIDNAP_WANTED_CIV		1			// +1 wanted when a civilian is kidnapped (§4.4 VERIFIED, always a warrant)
#define KIDNAP_WANTED_COP		6			// +6 wanted when a cop is kidnapped (§4.4 VERIFIED)
#define KIDNAP_ESCAPE_DIST		60.0		// victim strays this far from the kidnapper → escapes (§4.4) [OUR DESIGN]
#define KIDNAP_SCORE			1			// +1 kidnapper score per successful ransom/steal (§4.4 / Score Guide)
#define KIDNAP_HIDEOUT_RANGE	5.0			// reach of a hideout to trigger the cash steal [OUR DESIGN]

// Kidnapper skill (M6 — set at City Hall via /skill). Until then everyone is
// SKILL_NONE and the base ransom applies. The id is APPENDED to the skill enum
// so the M6 skill work is a data change only (mirrors the pickpocket/rapist plan).
#define SKILL_KIDNAPPER			5			// Kidnapper — appears as Driver, ransom scaling (M6 skill, §4.4)

// ============================================================================
//  M6 (stage 1) — skills, fighting styles & clothes (design §9.8/§9.7/§5.7,
//  jobs-progression §1.2/§2, crazybobs-site-full §Skills/§Gym/§Clothes).
//  systems/skills.inc (/skill), systems/fightstyle.inc (/fightstyle),
//  systems/clothes.inc (/clotheswear). These ACTIVATE the M3-M5 skill hooks
//  (rob cap, rape success, drug carry cap, kidnap ransom scaling, carjack-lock
//  defeat) — all of which already read pSkill — plus persist FightStyle/HasDrugBag.
// ============================================================================

// ---- Skill ids (pSkill / LSplayers.Skill) — set at City Hall via /skill -----
// The first six ids are the enum in this file (SKILL_NONE..SKILL_DRUGDEALER) plus
// SKILL_KIDNAPPER above; the remaining civilian skills are appended here as plain
// defines (a data change only — each effect already reads pSkill). Cop-family
// skills are NOT chooseable civilian skills; they follow the team/agency (jobs §1.3).
#define SKILL_CARJACKER			6			// Car Jacker — defeats vehicle locks (§9.12 / vehicle_own.inc), faster crane sell
#define SKILL_HITMAN			7			// Hitman — hit contracts (M7+ hits system; effect deferred, choosable now)
#define SKILL_ARMSDEALER		8			// Arms Dealer — sell weapons to civilians (M7 vendor system; choosable now)
#define SKILL_STREETVENDOR		9			// Street Vendor — sell items to anyone incl. cops (M7 vendor system)
#define SKILL_PROSTITUTE		10			// Prostitute — /sex + strip (M7 vendor system)
#define SKILL_FOODDELIVERY		11			// Food Delivery — sell food/drinks (M7 vendor system)
#define SKILL_MAX				11			// highest valid civilian skill id (bounds the /skill dialog)

// /skill re-pick (jobs §1.2 — "change skill at City Hall, for a price"). Changeable
// with a cost + a cooldown so it is not free-swapped mid-fight.
#define SKILL_CHANGE_COST		25000		// $ charged to CHANGE an existing skill (first pick is free) [OUR DESIGN]
#define SKILL_CHANGE_CD			300			// seconds between skill changes (5 min re-pick cooldown) [OUR DESIGN]
#define SKILL_ACTOR_RANGE		6.0			// proximity to a City Hall actor / marker to use /skill (§9.8) [OUR DESIGN]
#define SKILL_CHANGE_DIALOG		5401		// /skill picker list dialog id (unique — avoids the defines.inc enum)
// Kidnapper skill ransom scaling (§4.4 — the M6 "ransom scaling" hook). A Kidnapper
// can demand up to 3x the base ceiling and earns a bigger grab bonus.
#define KIDNAP_SKILL_RANSOM_MULT	3		// Kidnapper skill ransom ceiling multiplier (§4.4 ransom scaling) [OUR DESIGN]
#define KIDNAP_SKILL_GRAB_BONUS		4000	// Kidnapper skill grab bonus (vs KIDNAP_GRAB_BONUS base) [OUR DESIGN]
// Weapon skills (§9.8 resolved: classic feel — max all weapon skills on spawn).
#define WEAPON_SKILL_LEVEL		999			// SetPlayerSkillLevel level applied to every weapon skill on spawn (Hitman tier, §2.1)

// ---- Fighting styles at the gym (§9.8 / jobs §2.2) — systems/fightstyle.inc --
// SA-MP FIGHTING_STYLE_* ids: NORMAL(4) BOXING(5) KUNGFU(6) KNEEHEAD(7) GRABKICK(15)
// ELBOW(16). CB offered 3 options at 3 gyms (Ganton/Garcia/Redsands East). We expose
// the 3 visually-distinct melee styles (Boxing/Kung-Fu/Knee-head) for a small fee.
#define FIGHTSTYLE_DEFAULT		4			// FIGHTING_STYLE_NORMAL — the LSplayers.FightStyle default (matches §11.2 DEFAULT 4)
#define FIGHTSTYLE_FEE			500			// small fee per style change (§9.8 / jobs §2.2 "$500") [DESIGN]
#define FIGHTSTYLE_GYM_RANGE	8.0			// proximity to a gym marker to use /fightstyle [OUR DESIGN]
#define FIGHTSTYLE_DIALOG		5402		// /fightstyle picker list dialog id (unique)

// ---- Clothes & accessories (§9.7 / §5.7 / clothes-accessories) — clothes.inc -
// /clotheswear is the full clothes menu: skins (pSkinsSelected, peds.inc), the
// crowbar accessory ($40,000, pCrowbarEquipped — unified with the M5 /buycrowbar),
// and the drug bag ($15,000 — doubles the Drug Dealer carry cap, activates pHasDrugBag).
#define CLOTHES_SHOP_RANGE		8.0			// proximity to a clothes-shop marker (Binco/ProLaps/Victim/…) [OUR DESIGN]
#define CLOTHES_SKIN_PRICE		8000		// $ to buy+wear a not-yet-owned civilian skin (owned skins re-worn free) [OUR DESIGN]
#define DRUGBAG_PRICE			15000		// $ for a drug bag — doubles the Drug Dealer carry cap (§16 #2 / §4.3) [OUR DESIGN]
#define CLOTHES_MENU_DIALOG		5403		// /clotheswear top menu dialog id (unique)
#define CLOTHES_SKINS_DIALOG	5404		// /clotheswear -> skins list dialog id (unique)

// ============================================================================
//  M6 (stage 2) — fishing & farming — systems/fishing.inc + systems/farming.inc
//  Grounding: docs/GAMEPLAY-DESIGN.md §9.2 (fishing), §9.3 (farming/drug growing),
//  §10.2 (economy: fish × FISH_MARKET, drugs × DRUG — market mults are M7 stubs),
//  §11.2 (FishPermits/HuntPermits/DrugSeeds columns), §11.3 (plants table) +
//  docs/research/economy.md §8-9 + docs/research/crazybobs-site-full.md (Fishing,
//  Drug Planting & Hunting, Bait Shop). All money moves through the Bank_* API;
//  drug sales feed pDrugs (Drug_GiveGrams); drug farming respects wanted/cops.
// ============================================================================

// ---- Bait Shop pricing (economy.md §2.3 — VERIFIED sample prices) ----------
#define FISHING_ROD_PRICE		20000		// fishing rod — faster + higher catch chance (§9.2 VERIFIED $20,000)
#define FISH_COOLER_PRICE		20000		// fish cooler — hold more fish (§9.2 VERIFIED $20,000)
#define FISH_SALES_PERMIT_PRICE	20000		// fish sales permit — sell to players (§9.2 VERIFIED $20,000)
// Fishing permits sold in bundles (Bait Shop): 5/10/25 = $8,000/$12,000/$20,000.
#define FISH_PERMIT_BUNDLE_5	8000		// 5-fish permit bundle price (§9.2 VERIFIED)
#define FISH_PERMIT_BUNDLE_10	12000		// 10-fish permit bundle price (§9.2 VERIFIED)
#define FISH_PERMIT_BUNDLE_25	20000		// 25-fish permit bundle price (§9.2 VERIFIED)
#define FISH_PERMIT_MAX			50			// max fishing permits carried (§9.2 VERIFIED "carry up to 50")
// Hunting permits sold in bundles (Bait Shop): 5/10 deer = $4,000/$6,000.
#define HUNT_PERMIT_BUNDLE_5	4000		// 5-deer hunting permit bundle price (§9.3 VERIFIED)
#define HUNT_PERMIT_BUNDLE_10	6000		// 10-deer hunting permit bundle price (§9.3 VERIFIED)
#define HUNT_PERMIT_MAX			20			// max hunting permits carried (§9.3 VERIFIED "carry up to 20")
// Drug seeds (24/7/Bait/Refill/Street Vendor): $1,200 each, 5-pack $4,000.
#define DRUG_SEED_PRICE			1200		// single drug seed price (§9.3 VERIFIED)
#define DRUG_SEED_BUNDLE_5		4000		// 5-pack drug seed price (§9.3 VERIFIED)
#define DRUG_SEED_MAX			10			// max seeds carried (§9.3 VERIFIED "carry max 10")

// ---- Fishing (systems/fishing.inc — design §9.2) ---------------------------
#define FISH_SPOT_RANGE			6.0			// on-foot reach to a pier/fishing spot to /fish [OUR DESIGN]
#define FISH_CATCH_SECS			5			// seconds to reel in a catch after /fish; rod halves it (§9.2)
#define FISH_CATCH_SECS_ROD		3			// reel-in seconds WITH a rod (faster, §9.2 VERIFIED "reduces time")
#define FISH_COOLER_CAP			10			// fish held with a cooler (matches the house-storage fish cap, §9.2)
#define FISH_NOCOOLER_CAP		2			// fish held without a cooler [OUR DESIGN — cooler is the upgrade]
#define FISH_WEIGHT_MIN			2			// minimum catch weight (lb) [OUR DESIGN]
#define FISH_WEIGHT_MAX			85			// maximum catch weight (lb) — heavy catches are rare (§8.2 baby-seal ~80lb)
#define FISH_BASE_PRICE			8			// $ per (pound × rarity) base — pre-market-mult (§8.2, trout ~$400 anchor) [OUR DESIGN]
#define FISH_ROD_RARITY_BONUS	1			// a rod nudges the rarity roll up by this much (§9.2 "higher catch chance")
#define FISH_SKILL_RARITY_BONUS	1			// Fisherman skill nudges the rarity roll up (§9.2 skill perk)
#define FISH_NOPERMIT_WANTED	1			// +1 wanted for fishing near a cop with no permit (§9.2 VERIFIED)
#define FISH_COPSIGHT_RANGE		60.0		// a cop within this counts as "watching you fish" [OUR DESIGN]
#define FISH_BONUS_HOUR			5			// bonus fish announced/claimable at game-hour 05:00 (§9.2 VERIFIED)
#define FISH_BONUS_REWARD		250000		// large cash reward to the first bonus-fish catcher (§9.2 "large amount") [OUR DESIGN]

// Fish rarity tiers (species class → rarity multiplier + name). Higher tier = rarer
// + bigger multiplier; the rare top tier (Baby Seal) is the huge payout (§8.2).
enum
{
	FISH_RARITY_COMMON,		// 0 — trout/boot/tin can (×1)
	FISH_RARITY_UNCOMMON,	// 1 — bass/catfish (×2)
	FISH_RARITY_RARE,		// 2 — turtle/marlin (×5)
	FISH_RARITY_LEGENDARY	// 3 — baby seal / killer fish (×12) — the big payout (§8.2)
}
#define FISH_RARITY_TIERS		4			// number of rarity tiers (bounds the rarity roll)
// Rarity roll: random(100) < these thresholds → tier. Legendary is deliberately rare.
#define FISH_ROLL_LEGENDARY		3			// < this % → legendary (baby seal, §8.2)
#define FISH_ROLL_RARE			15			// < this % → rare
#define FISH_ROLL_UNCOMMON		45			// < this % → uncommon; else common

// ---- Farming: drug planting (systems/farming.inc — design §9.3) ------------
#define MAX_PLANTS				256			// hard array bound (DB `plants` rows loaded on init) [OUR DESIGN]
#define MAX_PLANTS_PER_PLAYER	5			// max active plants per player (§9.3 VERIFIED "Max 5 active plants")
#define DRUG_PLANT_GROW_SECS	1200		// ~20 minutes of ACCRUED (online) grow time to full (§9.3 VERIFIED "~20 minutes")
#define DRUG_PLANT_MAX_GRAMS	200			// full-grown yield (§9.3 VERIFIED "up to 200 grams")
#define DRUG_PLANT_HARVEST_MIN	50			// minimum grams to harvest for +1 score (§9.3 VERIFIED ">50g")
#define DRUG_PLANT_RANGE		4.0			// on-foot reach to /harvest / interact with a plant (§9.3)
#define DRUG_PLANT_SPACING		15.0		// plants must be at least this far apart / from water (§9.3 "not near other plants") [OUR DESIGN]
#define DRUG_PLANT_CP_SIZE		2.0			// red checkpoint radius on a plant (§9.3 "red checkpoint")
#define DRUG_PLANT_CP_MODEL		19		    // plant object model near the checkpoint (a bush/plant) [OUR DESIGN]
#define DRUG_PLANT_ICON_RANGE	60.0		// how far the plant is spottable — cops see it from further (§9.3)
#define DRUG_PLANT_FERT_MULT	2			// fertilizer multiplies grow speed by this (§9.3 "accelerates growth")
#define DRUG_DEALER_GROW_MULT	2			// Drug Dealers grow faster (§9.3 VERIFIED "Drug Dealers grow faster")
#define DRUG_PLANT_WANTED		2			// +2 wanted if a cop is in sight when planting/harvesting (§9.3 illegal) [OUR DESIGN]
#define DRUG_PLANT_COPSIGHT		60.0		// a cop within this "sees" the illegal farming [OUR DESIGN]
#define DRUG_SELL_MIN_GRAMS		200			// minimum grams sold at a refill point for +1 score (§9.3 VERIFIED "minimum 200g")
#define DRUG_SELL_PRICE_PER_G	14			// $ per gram of grown drugs sold at a refill point (pre-market-mult) [OUR DESIGN — Vigora 14g/$13k anchor]
#define DRUG_SELL_RANGE			6.0			// on-foot reach to a Drug Refill Point to /drugsell [OUR DESIGN]

// ---- Farming: hunting (deer) — design §9.3 (minimal) -----------------------
#define HUNT_RANGE				20.0		// reach in front of the player to /hunt a deer [OUR DESIGN]
#define HUNT_NOPERMIT_WANTED	6			// +6 wanted for killing a deer with no hunting permit (§9.3 VERIFIED "+6 wanted")
#define HUNT_HEAL				25.0		// HP a successful hunt restores (§9.3 VERIFIED "provides health") [OUR DESIGN]
#define HUNT_CASH_MIN			1500		// meat/bonus cash floor per deer [OUR DESIGN]
#define HUNT_CASH_MAX			4000		// meat/bonus cash ceiling per deer [OUR DESIGN]
#define HUNT_CD					30			// per-player /hunt cooldown seconds [OUR DESIGN]

// ---- Legal crop job: CrazyBob's Farm (Flint County, §9.3) ------------------
// A plow/harvest loop of checkpoints at CrazyBob's Farm for legit low-yield cash.
// This is the FARM crop SYSTEM (/farmjob); the Tractor MISSION (stage 4) wraps it.
#define FARM_JOB_FIELDS			5			// number of field checkpoints in a farm-job route (§9.3)
#define FARM_JOB_CP_SIZE		6.0			// field checkpoint radius (drive the harvester over it)
#define FARM_JOB_PAY_MIN		500			// $ per field completed floor (§9.3 VERIFIED "$500-$2,000/field")
#define FARM_JOB_PAY_MAX		2000		// $ per field completed ceiling (§9.3 VERIFIED)
#define FARM_JOB_BONUS			5000		// completion bonus for all fields [OUR DESIGN]
#define FARM_JOB_CD				120			// per-player farm-job cooldown seconds after finishing/cancel [OUR DESIGN]
#define FARM_JOB_RANGE			8.0			// on-foot reach to the CrazyBob's Farm start marker [OUR DESIGN]
// CrazyBob's Farm start marker (Flint County, south of Easter Bay Intl — §9.3).
#define FARM_X					(-315.0)
#define FARM_Y					(-1420.0)
#define FARM_Z					(25.7)

// ---- Fishing/farming dialog ids (unique — outside the defines.inc enum) -----
#define BAIT_SHOP_DIALOG		5501		// Bait Shop buy menu (rod/cooler/permits/seeds, §9.2/§9.3)
#define REFILL_SELL_DIALOG		5502		// Drug Refill Point sell/seed menu (§9.3)

//==============================================================================
//	M6 (stage 3) : DM / sniper / duel arenas (design §9.6 / jobs §6)
//==============================================================================
// Arena kinds (also the pArena value; ARENA_NONE = not in an arena).
#define ARENA_NONE				0
#define ARENA_DMS				1			// DM Stadium free-for-all (§6.1)
#define ARENA_SNIPER			2			// sniper-only arena (§6.5)
#define ARENA_DUEL				3			// 1v1 duel (§6.4)

// Economy (jobs §6.1 — VERIFIED $5,000 entry / $1,000 per death).
#define ARENA_ENTRY_FEE			5000		// DMS/sniper entry fee (Bank_TakeCash)
#define ARENA_DEATH_COST		1000		// charged per death inside DMS/sniper
#define ARENA_LIFE_AMMO			2500		// ammo per weapon per life (crazybobs dm-stadium page)

// Isolated virtual worlds so arena PvP never touches the city (§6.4). Each arena
// kind gets its own base VW; duels add the challenger id for a private world.
#define ARENA_VW_DMS			7100
#define ARENA_VW_SNIPER			7200
#define ARENA_VW_DUEL_BASE		7300		// duel VW = base + challenger id (unique per pair)

// DM Stadium (LS) — the existing /lossantosdm warp spot is the LS forum roof arena.
#define DMS_X					(1544.8907)
#define DMS_Y					(-1353.6543)
#define DMS_Z					(329.4738)
#define DMS_A					(0.0)

// Sniper arena — an isolated rooftop-style spread (LS, reuse the DM stadium area
// but its own VW so the two never mix).
#define SNIPER_X				(1476.5)
#define SNIPER_Y				(-1348.9)
#define SNIPER_Z				(329.4738)
#define SNIPER_A				(90.0)

// Duel arena — two facing spawn points on the flat LS forum roof (same confirmed-
// flat ground as the DM stadium; each pair gets an isolated VW so they never mix).
#define DUEL_X1					(1524.0)
#define DUEL_Y1					(-1353.65)
#define DUEL_Z1					(329.4738)
#define DUEL_A1					(90.0)
#define DUEL_X2					(1565.0)
#define DUEL_Y2					(-1353.65)
#define DUEL_Z2					(329.4738)
#define DUEL_A2					(270.0)

#define DUEL_STAKE_MAX			10000000	// sanity cap on a duel stake (both must afford it)
#define DUEL_OFFER_SECS			30			// seconds a duel challenge stays open before it lapses
#define DUEL_COUNTDOWN_SECS		3			// 3-2-1 freeze before the duel goes live

// Safe / no-crime zones (design §2.5 / §9.10) — hospital & RPC bar interiors are
// no-crime. Radius the on-foot proximity check uses to set pInSafeZone.
#define SAFE_ZONE_RADIUS		25.0

#define isNull(%1) \
                ((!(%1[0])) || (((%1[0]) == '\1') && (!(%1[1]))))