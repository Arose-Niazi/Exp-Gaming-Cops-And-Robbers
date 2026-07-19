/*

	Feature / Help Browser — Stage C (FEATURE-EXPANSION-PLAN §2)
	Scripted by Arose Niazi.

	Implements:
		CMD:help (/info, /hlp, /features) — open the category list
		Help_ShowCategories(playerid)      — render MENU_HELP_CATS
		Help_ShowCommands(playerid, cat)   — render MENU_HELP_CMDS

	Content lives in two static arrays only:
		g_HelpCat[][e_HelpCat]  — 11 categories, each with a name + blurb
		g_HelpCmd[][e_HelpCmd]  — one row per documented player command

	Adding a command = one data row.  No new UI; reuses ShowTextDrawMenu /
	ShowTextDrawMenuItems / HideTextDrawMenu / MENU_HELP_CATS / MENU_HELP_CMDS.
	Menu routing (the two new switch cases) lives in players/menu.inc so the
	detail-dialog response (HELP_DETAIL_DIALOG) can return here.

	Naming:
		/help owns the aliases /info, /hlp, /features.
		/pinfo (/i) in cmds/player.inc is the player-info command — no clash.
		/showcommands (/commands) in cmds/admin.inc is the flat paginated list.

	RULES (CLAUDE.md):
		- Tabs for indentation.
		- CMD: / alias: per Pawn.CMD; no trailing semicolon on alias lines.
		- ASCII-only in ALL displayed strings (no em-dash, bullets, non-ASCII).
		- No DB, no new persistence.
		- 0 errors / 0 warnings.

*/

//==============================================================================
//	Category enum + data (category index = position in g_HelpCat)
//==============================================================================

// Category indices — must match the g_HelpCat row order exactly.
#define HCAT_GETTING_STARTED	0
#define HCAT_CRIME				1
#define HCAT_ROBBERIES			2
#define HCAT_COPS				3
#define HCAT_ECONOMY			4
#define HCAT_PROPERTY			5
#define HCAT_JOBS				6
#define HCAT_SKILLS				7
#define HCAT_FISHING_FARMING	8
#define HCAT_SOCIAL				9
#define HCAT_JAIL				10

enum e_HelpCat { HC_NAME[24], HC_BLURB[48] }
static const g_HelpCat[][e_HelpCat] =
{
	// 0 — HCAT_GETTING_STARTED
	{"Getting Started",    "Spawn, class, skills, money basics"},
	// 1 — HCAT_CRIME
	{"Crime",              "Rob, rape, drugs, holdups, jumpkick"},
	// 2 — HCAT_ROBBERIES
	{"Robberies",          "Bank, casino, house, shoplift, crw"},
	// 3 — HCAT_COPS
	{"Cops",               "Arrest, ticket, dispatch, 10-codes"},
	// 4 — HCAT_ECONOMY
	{"Economy",            "Bank, ATM, stocks, lotto, insurance"},
	// 5 — HCAT_PROPERTY
	{"Property",           "Houses, vehicle lock/unlock, theft"},
	// 6 — HCAT_JOBS
	{"Jobs & Missions",    "Deliveries, patrol, courier, races"},
	// 7 — HCAT_SKILLS
	{"Skills",             "Pickpocket, con-artist, medic..."},
	// 8 — HCAT_FISHING_FARMING
	{"Fishing & Farming",  "Fish rods, coolers, plant, harvest"},
	// 9 — HCAT_SOCIAL
	{"Social & Chat",      "PM, whisper, tip, ignore, radio"},
	// 10 — HCAT_JAIL
	{"Jail",               "Bail, appeal, bribe, escape, breakout"}
};

//==============================================================================
//	Command data (one row per documented player command)
//	Format: { category_index, "/cmd (aliases)", "one-line", "full detail" }
//	Keep each HCMD_LONG under 128 chars (fits the dialog well).
//	Keep HCMD_NAME under 24 chars and HCMD_SHORT under 40 chars.
//	ASCII-only: use '-' or '*' in place of any special character.
//==============================================================================

enum e_HelpCmd { HCMD_CAT, HCMD_NAME[32], HCMD_SHORT[48], HCMD_LONG[144] }
static const g_HelpCmd[][e_HelpCmd] =
{
	//--------------------------------------------------------------------------
	//	0 — Getting Started
	//--------------------------------------------------------------------------
	{ HCAT_GETTING_STARTED,
	  "/showcommands",
	  "Paginated full command list",
	  "Opens a paginated dialog listing all commands. Alias: /commands. No login needed." },

	{ HCAT_GETTING_STARTED,
	  "/pinfo (/i)",
	  "View class/skill/level of a player",
	  "/pinfo [id] - shows class, skill, level, zone and wanted level. Blank = yourself." },

	{ HCAT_GETTING_STARTED,
	  "/stats (/sts)",
	  "Show your current-life stats",
	  "/stats [id] - money, bank, score, kills, wanted, cop rank, robberies done/failed." },

	{ HCAT_GETTING_STARTED,
	  "/time (/day)",
	  "Show the current game day and clock",
	  "Displays the in-game day of week, hour and minute. No argument needed." },

	{ HCAT_GETTING_STARTED,
	  "/locate (/loc)",
	  "Show a player's current zone",
	  "/locate [id] - prints the named zone the player is in. Blank = yourself." },

	{ HCAT_GETTING_STARTED,
	  "/getid (/id)",
	  "Resolve a partial name to a player id",
	  "/getid <partial name> - lists all online players whose name contains that text." },

	{ HCAT_GETTING_STARTED,
	  "/gps",
	  "Open the GPS destination menu",
	  "Opens a menu of map categories. Pick a destination and a route is drawn on your map." },

	{ HCAT_GETTING_STARTED,
	  "/gpsoff (/cleargps)",
	  "Clear the GPS route",
	  "Removes any active GPS route and map marker set by /gps." },

	{ HCAT_GETTING_STARTED,
	  "/ad",
	  "Use an adrenaline pill",
	  "Consumes one carried adrenaline pill: full heal, cures all STDs, clears overdose." },

	{ HCAT_GETTING_STARTED,
	  "/advert",
	  "Send a paid server-wide ad line",
	  "/advert <text> - broadcasts a coloured ad to all players. Costs $200, 90s cooldown." },

	{ HCAT_GETTING_STARTED,
	  "/skill (/skills, /skl)",
	  "Browse or choose a civilian skill",
	  "At City Hall: pick Pickpocket, Con Artist, Rapist or Drug Dealer to specialise." },

	{ HCAT_GETTING_STARTED,
	  "/fightstyle (/fs, /gym)",
	  "Change your fighting style",
	  "At a gym marker: pick Boxing, Kung Fu or Capoeira to change your melee style." },

	{ HCAT_GETTING_STARTED,
	  "/complain",
	  "Report a cheater to online staff",
	  "/complain <reason> - sends a message to all on-duty staff/admins. Not cop-only." },

	//--------------------------------------------------------------------------
	//	1 — Crime
	//--------------------------------------------------------------------------
	{ HCAT_CRIME,
	  "/rob (/rb)",
	  "Rob a nearby player's cash",
	  "/rob <id> - steals 10-50% of a player's cash-on-hand within 3m. +2 wanted. 30s CD." },

	{ HCAT_CRIME,
	  "/rape (/ra)",
	  "Attempt to rape a nearby player",
	  "/rape <id> - chance-based within 2m. May infect with an STD. +2 wanted. 45s CD." },

	{ HCAT_CRIME,
	  "/usecondom (/uc)",
	  "Use a condom for infection protection",
	  "Equip a carried condom before /rape - drops STD infection chance from ~45% to ~8%." },

	{ HCAT_CRIME,
	  "/takedrugs (/td)",
	  "Take drugs from your inventory",
	  "/takedrugs <grams> - enter a click mini-game. Heals over time; risk overdose or bad dose." },

	{ HCAT_CRIME,
	  "/holdup (/hup, /storerob)",
	  "Hold up a store/24-7 clerk",
	  "On foot next to a store clerk actor. Wait ~10s then collect cash. +7 wanted. 5-min CD." },

	{ HCAT_CRIME,
	  "/jumpkick",
	  "Melee jump-kick a player in front",
	  "On foot: deals 8 HP melee damage to the player directly in front of you. 5s CD." },

	{ HCAT_CRIME,
	  "/kidnap (/kd)",
	  "Kidnap a nearby player",
	  "/kidnap <id> - cuffs and takes control of a player; demand ransom via /ransom." },

	{ HCAT_CRIME,
	  "/release (/rel)",
	  "Release your kidnap victim",
	  "Let your hostage go free. /ransom <amount> to demand cash first." },

	{ HCAT_CRIME,
	  "/ransom",
	  "Demand ransom for your kidnap victim",
	  "/ransom <amount> - sends a ransom request to the victim. They /payransom or wait." },

	{ HCAT_CRIME,
	  "/payransom (/pr)",
	  "Pay a ransom demand",
	  "/payransom <amount> - pay the kidnapper to be released if you have the cash." },

	{ HCAT_CRIME,
	  "/uncuff",
	  "Free yourself from cuffs (costly)",
	  "Attempt to break free while cuffed. Costs time and wanted level." },

	{ HCAT_CRIME,
	  "/fakeskill",
	  "Disguise your skill class display",
	  "Show a different skill to /pinfo viewers until you die or relog." },

	//--------------------------------------------------------------------------
	//	2 — Robberies
	//--------------------------------------------------------------------------
	{ HCAT_ROBBERIES,
	  "/shoplift",
	  "Shoplift an item from a store",
	  "Near a store: steal an item worth $200-$900. +2 wanted. 2-min CD. Success improves with rob XP." },

	{ HCAT_ROBBERIES,
	  "/bankrob (/robbank)",
	  "Start a bank safe-crack",
	  "At a bank checkpoint: 30s crack, then carry the safe to a hideout. +6 wanted. Crowbar helps." },

	{ HCAT_ROBBERIES,
	  "/casinorob",
	  "Rob the casino safe",
	  "At the casino checkpoint: 40s crack + carry. +6 wanted. ~$250k-600k yield." },

	{ HCAT_ROBBERIES,
	  "/robbery",
	  "Rob a special/major site",
	  "At a special-site checkpoint: 50s crack + carry. +6 wanted. ~$400k-900k yield." },

	{ HCAT_ROBBERIES,
	  "/houserob (/hrob, /hsrob)",
	  "Break into a player's house",
	  "/houserob <house_id> - steals 60% of stored money + items. +6 wanted. 5-min CD." },

	{ HCAT_ROBBERIES,
	  "/stoprobbery (/stoprob)",
	  "Abort your safe-crack or carry",
	  "Cancels a running bank/casino/special robbery. Safe is dropped, cooldown still runs." },

	{ HCAT_ROBBERIES,
	  "/buycrowbar",
	  "Buy a crowbar from the vendor",
	  "Costs $40,000 at the crowbar vendor. Improves safe-crack speed and success chance." },

	{ HCAT_ROBBERIES,
	  "/crowbar",
	  "Check crowbar status",
	  "Shows whether you have a crowbar equipped and what bonuses it gives." },

	{ HCAT_ROBBERIES,
	  "/clotheswear (/clothes, /cw)",
	  "Browse the clothes and gear shop",
	  "At the clothes shop: buy the crowbar, change clothes, or pick accessories." },

	//--------------------------------------------------------------------------
	//	3 — Cops
	//--------------------------------------------------------------------------
	{ HCAT_COPS,
	  "/arrest (/ar)",
	  "Arrest a warranted suspect",
	  "On foot within 4m of a WL6+ suspect: jail them, earn WL-scaled cash + 1 score." },

	{ HCAT_COPS,
	  "/ticket (/tk)",
	  "Issue a minor traffic/crime fine",
	  "Issue a $500 fine to a suspect within 8m. They /payticket within 5 min or get WL6." },

	{ HCAT_COPS,
	  "/payticket (/pt)",
	  "Pay a ticket fine issued to you",
	  "Pay the $500 fine a cop gave you within 5 min to avoid escalation to WL6." },

	{ HCAT_COPS,
	  "/vc (/visualcontact)",
	  "Tag a suspect for a wanted pause",
	  "/vc <id> - within 250m: marks the suspect and pauses their wanted-level decay for 60s." },

	{ HCAT_COPS,
	  "/su",
	  "Scan a player's wanted level",
	  "/su <id> - sends the suspect's wanted level and last crime to cop chat." },

	{ HCAT_COPS,
	  "/report (/rp)",
	  "Report a crime you witnessed",
	  "/report <text> - sends a [REPORT] line to all cops on the department channel." },

	{ HCAT_COPS,
	  "/clr (/cancelreport)",
	  "Cancel your last report",
	  "Withdraws the last report you sent via /report." },

	{ HCAT_COPS,
	  "/backup (/bk)",
	  "Request backup at your location",
	  "Fires a [DISPATCH] 10-78 to all cops with a map marker at your position. 60s marker." },

	{ HCAT_COPS,
	  "/respond (/yes)",
	  "Respond to the last dispatch call",
	  "Fires [DISPATCH] 10-23 responding and sets a red checkpoint at the last dispatch origin." },

	{ HCAT_COPS,
	  "/d (/dept, /copchat)",
	  "Send a message on the cop channel",
	  "/d <text> - broadcasts [DEPT] to all cops. Supports 10-codes and $loc/$sus/$veh." },

	{ HCAT_COPS,
	  "/copmsg (/cm)",
	  "Send a one-shot cop channel message",
	  "Same as /d - alias retained for compatibility." },

	{ HCAT_COPS,
	  "/copchaton",
	  "Enable receiving cop-channel msgs",
	  "Re-enables the [DEPT] / [DISPATCH] channel if you toggled it off with /copchatoff." },

	{ HCAT_COPS,
	  "/copchatoff",
	  "Mute the cop-channel feed",
	  "Silences [DEPT] / [DISPATCH] messages except CRITICAL alerts (most-wanted proximity)." },

	{ HCAT_COPS,
	  "/suspects (/sus, /wanted)",
	  "List wanted players",
	  "Opens a dialog of all online players with a wanted level above 0." },

	{ HCAT_COPS,
	  "/warrants (/war)",
	  "List players with arrest warrants",
	  "Shows players at WL6+ (orange tier - immediately arrestable)." },

	{ HCAT_COPS,
	  "/mostwanted (/mw)",
	  "List the most-wanted players",
	  "Shows players at WL9-10 (dark-orange tier). Priority targets for dispatch." },

	{ HCAT_COPS,
	  "/jaillist (/jl)",
	  "List currently jailed players",
	  "Shows all players inside the jail with their remaining sentence time." },

	{ HCAT_COPS,
	  "/robberies (/robs)",
	  "List active robberies/holdups",
	  "Shows in-progress safe-cracks, holdups and safe-carries with suspect and zone." },

	{ HCAT_COPS,
	  "/freeze (/fr, /pullover)",
	  "Order a suspect to comply",
	  "/freeze <id> - within 12m: briefly freezes a WL1+ suspect for 3 seconds." },

	{ HCAT_COPS,
	  "/crimes (/crime)",
	  "View a player's last 5 crimes",
	  "/crimes <id> - shows the crime log ring buffer for any player (cop + self access)." },

	{ HCAT_COPS,
	  "/vehrepair (/vehfix)",
	  "Repair your cop vehicle on foot",
	  "On foot next to your PD vehicle: repair it for $15,000." },

	{ HCAT_COPS,
	  "/coprank",
	  "Check your cop rank and progress",
	  "Shows your rank (0-10 Recruit-Commissioner), arrests to next rank, /refill tier." },

	{ HCAT_COPS,
	  "/refill (/rf)",
	  "Restock health, armour or weapons",
	  "At a PD refill point: spend refill points on HP (+1pt), armour (+2pt) or weapons (+3pt)." },

	{ HCAT_COPS,
	  "/donut",
	  "Eat a donut to restore health",
	  "Carry up to 6 donuts (acquired from PD). Each restores 50% HP. Cop-only." },

	{ HCAT_JAIL,
	  "/accept (/ac)",
	  "Accept a parole or other request",
	  "Cop: accept an inmate's /parole request to release them." },

	{ HCAT_JAIL,
	  "/refuse (/ref)",
	  "Refuse a parole request",
	  "Cop: reject an inmate's /parole request to keep them jailed." },

	//--------------------------------------------------------------------------
	//	4 — Economy
	//--------------------------------------------------------------------------
	{ HCAT_ECONOMY,
	  "/bank (/bankinfo)",
	  "Open the bank account menu",
	  "At a bank: see balance, deposit and withdraw. Earns 8% interest every 2 game-days." },

	{ HCAT_ECONOMY,
	  "/atm",
	  "Use a nearby ATM",
	  "At an ATM machine: quick-access deposit and withdraw without visiting the bank." },

	{ HCAT_ECONOMY,
	  "/deposit (/dep)",
	  "Deposit cash into your bank",
	  "/deposit <amount> - move cash-on-hand into your bank account. Must be at a bank/ATM." },

	{ HCAT_ECONOMY,
	  "/withdraw (/wd, /wit)",
	  "Withdraw cash from your bank",
	  "/withdraw <amount> - move bank money to cash-on-hand. Must be at a bank/ATM." },

	{ HCAT_ECONOMY,
	  "/balance (/bal)",
	  "Check your bank balance",
	  "Prints your current bank balance and cash-on-hand. Can be used anywhere." },

	{ HCAT_ECONOMY,
	  "/givecash (/pay, /gc)",
	  "Hand cash to a nearby player",
	  "/givecash <id> <amount> - physical cash transfer within 5m. 10s cooldown." },

	{ HCAT_ECONOMY,
	  "/tip (/givetip)",
	  "Tip cash to a nearby player",
	  "/tip <id> <amount> - send a social cash tip to a player within proximity." },

	{ HCAT_ECONOMY,
	  "/buyinsurance (/buyins)",
	  "Buy health or life insurance",
	  "Health: $15,000 / 5 game-days (waives medical fee). Life: $75,000 (preserves weapons on death)." },

	{ HCAT_ECONOMY,
	  "/lotto",
	  "Buy a lottery ticket",
	  "/lotto <1-125> - pick a number for $5,000. Draw fires at game-hour 18 each day." },

	{ HCAT_ECONOMY,
	  "/markets (/market)",
	  "Browse stock market listings",
	  "Lists all 21 tradeable stocks with current price, trend and your holdings." },

	{ HCAT_ECONOMY,
	  "/shares (/stocks)",
	  "Buy shares in a stock",
	  "/shares <symbol> <quantity> - buy at the current market price. Holdings earn dividends." },

	{ HCAT_ECONOMY,
	  "/sharessell (/stockssell)",
	  "Sell shares you hold",
	  "/sharessell <symbol> <quantity> - sell at the current price; profit/loss shown." },

	{ HCAT_ECONOMY,
	  "/markethistory",
	  "View a stock's price history",
	  "/markethistory <symbol> - last N price ticks shown as a trend table." },

	{ HCAT_ECONOMY,
	  "/moneybag (/mb)",
	  "Hunt for the hidden money bag",
	  "/moneybag - get a hot/warm/cold hint. Find it for a $250k-$2M reward. 5s CD." },

	{ HCAT_ECONOMY,
	  "/moneyrush (/mr)",
	  "Info on the current money rush",
	  "During a money rush event: shows status. Pickups scattered across Glen Park." },

	//--------------------------------------------------------------------------
	//	5 — Property
	//--------------------------------------------------------------------------
	{ HCAT_PROPERTY,
	  "/buyhouse",
	  "Buy the house you are standing at",
	  "Within 2.5m of a for-sale house entrance: purchase it if you can afford it." },

	{ HCAT_PROPERTY,
	  "/sellhouse",
	  "Sell one of your houses",
	  "/sellhouse <house_id> - sell back to the server for 70% of the purchase price." },

	{ HCAT_PROPERTY,
	  "/house",
	  "Open your house management menu",
	  "At your own house: lock/unlock, set spawn, store/withdraw money, rent settings." },

	{ HCAT_PROPERTY,
	  "/houses (/hlist, /houselist)",
	  "List all houses you own or rent",
	  "Opens a dialog showing your owned and rented houses with their values." },

	{ HCAT_PROPERTY,
	  "/enter",
	  "Enter a nearby house or interior",
	  "Step within 2.5m of an entrance pickup and type /enter to go inside." },

	{ HCAT_PROPERTY,
	  "/exit",
	  "Leave an interior you are inside",
	  "Return to the street from inside a house or interior." },

	{ HCAT_PROPERTY,
	  "/housestore (/hstore)",
	  "Deposit money into your house safe",
	  "/housestore <amount> - move cash into your house. Only accessible inside your house." },

	{ HCAT_PROPERTY,
	  "/housewithdraw (/hwd)",
	  "Withdraw money from your house",
	  "/housewithdraw <amount> - take cash out of your house safe while inside." },

	{ HCAT_PROPERTY,
	  "/housestoreitem",
	  "Store an item in your house",
	  "/housestoreitem <item> - add a counted item to your house storage (up to 20 items)." },

	{ HCAT_PROPERTY,
	  "/lockhouse (/hlock)",
	  "Lock your house door",
	  "Prevents visitors from entering your house. Toggle with /unlockhouse." },

	{ HCAT_PROPERTY,
	  "/unlockhouse (/hunlock)",
	  "Unlock your house door",
	  "Allows players to enter your house (e.g. tenants). Toggle with /lockhouse." },

	{ HCAT_PROPERTY,
	  "/superlock",
	  "Fit a super lock on your house",
	  "Costs $100,000. Makes house robbery 35% less likely to succeed." },

	{ HCAT_PROPERTY,
	  "/setrent",
	  "Set a daily rent for your house",
	  "/setrent <amount> - set a per-game-day rent ($500-$50,000) for a tenant." },

	{ HCAT_PROPERTY,
	  "/renthouse",
	  "Rent a house from its owner",
	  "/renthouse <house_id> - become a tenant and pay the owner's rent each game-day." },

	{ HCAT_PROPERTY,
	  "/stoprenting",
	  "Stop renting a house",
	  "End your tenancy. You will lose access to the interior." },

	{ HCAT_PROPERTY,
	  "/sethousespawn",
	  "Set your house as spawn point",
	  "Inside your owned house: respawn inside it after death instead of at the default spot." },

	{ HCAT_PROPERTY,
	  "/clearhousespawn",
	  "Clear your house spawn point",
	  "Reset to the default class spawn. You will no longer respawn inside your house." },

	{ HCAT_PROPERTY,
	  "/lock (/lk)",
	  "Lock your owned vehicle",
	  "Locks your parked owned vehicle so only you (and your co-owners) can enter it." },

	{ HCAT_PROPERTY,
	  "/unlock (/ulk)",
	  "Unlock your owned vehicle",
	  "Unlocks your owned vehicle so passengers can get in." },

	//--------------------------------------------------------------------------
	//	6 — Jobs & Missions
	//--------------------------------------------------------------------------
	{ HCAT_JOBS,
	  "/missions (/work, /jobs)",
	  "Browse and start a mission",
	  "/missions - opens the mission list. Pick Drug Delivery, Truck, Trash, Food and more." },

	{ HCAT_JOBS,
	  "/quitmission (/cancel)",
	  "Abandon your current mission",
	  "Cancel a mission in progress. No pay; the cooldown may still apply." },

	{ HCAT_JOBS,
	  "/challenge (/race)",
	  "Start or join a race challenge",
	  "/challenge <id> - enter one of 28 race courses. Finish for score; podium = double." },

	{ HCAT_JOBS,
	  "/flowers",
	  "Deliver a flower to a player",
	  "/flowers <id> - hand a flower to a nearby player as part of the flower delivery mission." },

	{ HCAT_JOBS,
	  "/pickpocket (/pp)",
	  "Pickpocket a target player",
	  "Part of the Pickpocket mission: steal $10k goal total from players via this command." },

	{ HCAT_JOBS,
	  "/encounter",
	  "Perform a social encounter",
	  "Part of the Sexual Encounter mission: 5 consensual encounters via this command." },

	{ HCAT_JOBS,
	  "/box",
	  "Pick up or drop an airport box",
	  "Part of the Airport Robbery mission: collect boxes with a truck/van." },

	{ HCAT_JOBS,
	  "/farmjob",
	  "Start a tractor farm job",
	  "Near a tractor at CrazyBob's Farm: start the timed crop-run mission." },

	{ HCAT_JOBS,
	  "/cancelfarmjob",
	  "Cancel the tractor farm job",
	  "Abandon the current tractor farm-job run. No reward." },

	{ HCAT_JOBS,
	  "/dms",
	  "Enter the DM (deathmatch) arena",
	  "/dms <arena_id> - enter a designated free-for-all arena. No wanted for kills inside." },

	{ HCAT_JOBS,
	  "/sniper",
	  "Enter a sniper arena",
	  "/sniper <id> - a long-range only sniper match. No wanted for arena kills." },

	{ HCAT_JOBS,
	  "/duel",
	  "Challenge another player to a duel",
	  "/duel <id> - 1v1 fight in the duel arena. Loser pays a stake to the winner." },

	{ HCAT_JOBS,
	  "/deny",
	  "Decline a duel invitation",
	  "Reject a pending /duel challenge from another player." },

	{ HCAT_JOBS,
	  "/leave",
	  "Leave an arena",
	  "Exit the DM, sniper or duel arena you are currently in." },

	//--------------------------------------------------------------------------
	//	7 — Skills
	//--------------------------------------------------------------------------
	{ HCAT_SKILLS,
	  "/skill (/skills, /skl)",
	  "Choose or view your civilian skill",
	  "At City Hall: pick Pickpocket (rob cap $100k), Con Artist ($500k), Rapist or Drug Dealer." },

	{ HCAT_SKILLS,
	  "/fightstyle (/fs, /gym)",
	  "Change your fighting style",
	  "At a gym (Ganton/Garcia/Redsands East): Boxing, Kung Fu or Capoeira." },

	{ HCAT_SKILLS,
	  "/rob (/rb) - Pickpocket",
	  "Higher cap + best success with PP skill",
	  "Pickpocket skill raises your /rob cap to $100k and gives the best rob success curve." },

	{ HCAT_SKILLS,
	  "/rob (/rb) - Con Artist",
	  "Very high cap with Con Artist skill",
	  "Con Artist skill raises your /rob cap to $500k and takes 10-50% of victim cash." },

	{ HCAT_SKILLS,
	  "/rape - Rapist skill",
	  "Higher rape success + STD pass",
	  "Rapist skill gives a higher /rape success chance and lets you pass STDs you carry." },

	{ HCAT_SKILLS,
	  "/takedrugs - Drug Dealer",
	  "Carry up to 5,000g with drug bag",
	  "Drug Dealer skill raises your carry cap (2,500g base; 5,000g with a drug bag)." },

	//--------------------------------------------------------------------------
	//	8 — Fishing & Farming
	//--------------------------------------------------------------------------
	{ HCAT_FISHING_FARMING,
	  "/fish",
	  "Cast your fishing rod",
	  "/fish <bait_type> - at a fishing spot with a rod equipped. Catch varies by bait." },

	{ HCAT_FISHING_FARMING,
	  "/fishsellall (/fsell)",
	  "Sell all fish in your cooler",
	  "At the fish market: sell the entire contents of your cooler for market prices." },

	{ HCAT_FISHING_FARMING,
	  "/cooler (/fishinventory)",
	  "View your fish cooler contents",
	  "Lists all fish you have caught and their current market values." },

	{ HCAT_FISHING_FARMING,
	  "/rod",
	  "Equip or inspect your fishing rod",
	  "/rod - show your rod tier and switch to the rod as your active item." },

	{ HCAT_FISHING_FARMING,
	  "/baitshop (/bait)",
	  "Browse bait types at the bait shop",
	  "At the Bait Shop: buy Worm, Bug or Special bait. Each affects catch quality." },

	{ HCAT_FISHING_FARMING,
	  "/plant",
	  "Plant a drug crop",
	  "/plant <seed_type> - at a farm plot: start a crop growing cycle (takes game-days)." },

	{ HCAT_FISHING_FARMING,
	  "/harvest (/harv)",
	  "Harvest a mature drug crop",
	  "At your plot when the crop is ready: collect the drug grams and sell via /drugsell." },

	{ HCAT_FISHING_FARMING,
	  "/fertilize (/fert)",
	  "Fertilize a crop to speed growth",
	  "At your plot: apply fertilizer to cut the time to maturity." },

	{ HCAT_FISHING_FARMING,
	  "/destroyplant",
	  "Destroy a drug plant",
	  "Remove one of your in-ground plants. You lose the investment." },

	{ HCAT_FISHING_FARMING,
	  "/plantgps (/pgps)",
	  "Get a GPS route to your plants",
	  "/plantgps - sets a GPS marker to your nearest planted crop." },

	{ HCAT_FISHING_FARMING,
	  "/drugsell (/ds)",
	  "Sell harvested drugs",
	  "At a drug dealer NPC: sell grams from your inventory for cash." },

	{ HCAT_FISHING_FARMING,
	  "/buyseeds",
	  "Buy crop seeds at the farm",
	  "At the seed vendor near CrazyBob's Farm: buy seeds to /plant." },

	{ HCAT_FISHING_FARMING,
	  "/hunt",
	  "Hunt wild animals",
	  "At a hunting zone: stalk and shoot animals for pelts. Sell at the market." },

	//--------------------------------------------------------------------------
	//	9 — Social & Chat
	//--------------------------------------------------------------------------
	{ HCAT_SOCIAL,
	  "/pm (/msg, /m)",
	  "Send a private message",
	  "/pm <id> <text> - private message to a player. They can /nopm to block PMs." },

	{ HCAT_SOCIAL,
	  "/reply (/r)",
	  "Reply to the last PM you received",
	  "/reply <text> - sends a PM back to whoever last messaged you." },

	{ HCAT_SOCIAL,
	  "/nopm",
	  "Toggle receiving private messages",
	  "Block or unblock private messages from other players." },

	{ HCAT_SOCIAL,
	  "/ignore (/mute)",
	  "Ignore all chat from a player",
	  "/ignore <id> - hides that player's public and private messages until you /ignore again." },

	{ HCAT_SOCIAL,
	  "/w (/whisper)",
	  "Whisper nearby - local proximity",
	  "/w <text> - sends to players within 15m only. NOT a private message; proximity chat." },

	{ HCAT_SOCIAL,
	  "/say (/s)",
	  "Explicit global chat line",
	  "/say <text> - broadcast to all players, like normal chat. Use when in a custom channel." },

	{ HCAT_SOCIAL,
	  "/tip (/givetip)",
	  "Send a cash tip to a nearby player",
	  "/tip <id> <amount> - hand a social tip to a nearby player." },

	{ HCAT_SOCIAL,
	  "/radio (/cnrradio)",
	  "Pick a music stream to play",
	  "Opens a preset stream list. Last option stops your personal audio stream." },

	{ HCAT_SOCIAL,
	  "/djradio",
	  "DJ: start a server-wide stream",
	  "/djradio <url> - DJ-tier: broadcast a stream URL to all online players." },

	{ HCAT_SOCIAL,
	  "/stopdjradio",
	  "DJ: stop the server-wide stream",
	  "DJ-tier: end the currently broadcasting DJ stream for all players." },

	{ HCAT_SOCIAL,
	  "/advert",
	  "Paid server-wide advertisement",
	  "/advert <text> - $200 for a coloured broadcast line. 90s cooldown, 100 char limit." },

	{ HCAT_SOCIAL,
	  "/a",
	  "Send a message to admin chat",
	  "/a <text> - broadcasts to all online staff/admins. Must be Admin+ rank." },

	{ HCAT_SOCIAL,
	  "/complain",
	  "Report a cheater to online staff",
	  "/complain <reason> - forwards your report to all on-duty staff as an admin-chat alert." },

	//--------------------------------------------------------------------------
	//	10 — Jail
	//--------------------------------------------------------------------------
	{ HCAT_JAIL,
	  "/bail (/pb, /paybail)",
	  "Pay bail to leave jail",
	  "Pay the bail amount (sentence * $200) to be released immediately. Bank or cash." },

	{ HCAT_JAIL,
	  "/parole (/pl)",
	  "Request early parole",
	  "After 30s served: ask a cop to grant parole. They /accept or /refuse." },

	{ HCAT_JAIL,
	  "/appeal (/jailappeal)",
	  "Appeal your sentence to a jury",
	  "Start a jury vote among at least 2 online players: /innocent or /guilty decides." },

	{ HCAT_JAIL,
	  "/innocent",
	  "Vote innocent in a jail appeal",
	  "Cast your vote to acquit the appealing inmate. Requires the appeal to be active." },

	{ HCAT_JAIL,
	  "/guilty",
	  "Vote guilty in a jail appeal",
	  "Cast your vote to uphold the sentence. Requires the appeal to be active." },

	{ HCAT_JAIL,
	  "/bribe",
	  "Bribe a cop to reduce sentence",
	  "/bribe <$1k-$15k> - offer a bribe. Accepted = -45s sentence; refused = +30s." },

	{ HCAT_JAIL,
	  "/escape (/esc)",
	  "Attempt a jail break",
	  "Try to escape from jail. Sets your wanted to WL10 (Most Wanted) on success." },

	{ HCAT_JAIL,
	  "/breakout (/bo, /brk)",
	  "Break a friend out of jail",
	  "/breakout <id> - on foot within 5m of a jailed player: attempt to free them." }
};

//==============================================================================
//	State — per-player category index remembered for "Back" from detail dialog
//==============================================================================
new g_HelpLastCat[MAX_PLAYERS];

// Getter called from CnR.pwn's OnDialogResponse (HELP_DETAIL_DIALOG Back branch).
// Returns the last category viewed so Help_ShowCommands can be called via CLF.
stock Help_GetLastCat(playerid) { return g_HelpLastCat[playerid]; }

//==============================================================================
//	Help_ShowCategories — render MENU_HELP_CATS (plain stock, called only from
//	CMD:help which is in the same file so no forward needed)
//==============================================================================

Help_ShowCategories(playerid)
{
	new nCats = sizeof(g_HelpCat);
	ShowTextDrawMenu(playerid, MENU_HELP_CATS, "~y~Feature Guide", nCats, nCats);
	ShowTextDrawMenuItems(playerid, 0, "~w~Pick a category number", "", "", 0, 0);
	for(new i = 0; i < nCats; i++)
	{
		new col2buf[48];
		format(col2buf, sizeof(col2buf), "~w~%s", g_HelpCat[i][HC_BLURB]);
		ShowTextDrawMenuItems(playerid, i + 1, "", g_HelpCat[i][HC_NAME], col2buf, 0, i);
	}
	return 1;
}

//==============================================================================
//	Help_ShowCommands — render MENU_HELP_CMDS for category cat.
//	Declared with FUNCTION macro so menu.inc and CnR.pwn can call it via
//	CallLocalFunction("Help_ShowCommands", "ii", playerid, cat).
//==============================================================================

FUNCTION Help_ShowCommands(playerid, cat)
{
	new row = 0;
	new nCmds = sizeof(g_HelpCmd);

	// Title from the category name
	new titleBuf[32];
	format(titleBuf, sizeof(titleBuf), "~y~%s", g_HelpCat[cat][HC_NAME]);
	ShowTextDrawMenu(playerid, MENU_HELP_CMDS, titleBuf, MAX_TEXTDRAW_ROWS - 1, 0);
	ShowTextDrawMenuItems(playerid, 0, "~w~Pick a command for details", "", "", 0, 0);

	for(new i = 0; i < nCmds; i++)
	{
		if(g_HelpCmd[i][HCMD_CAT] != cat) continue;
		row++;
		if(row > (MAX_TEXTDRAW_ROWS - 1)) break;	// leave row 0 for the header; cap at 24 selectable
		// column2: short blurb
		new blurbBuf[44];
		format(blurbBuf, sizeof(blurbBuf), "~w~%s", g_HelpCmd[i][HCMD_SHORT]);
		ShowTextDrawMenuItems(playerid, row, "", g_HelpCmd[i][HCMD_NAME], blurbBuf, 0, i);
	}

	// Store how many selectable rows so menu.inc validates within bounds
	Menu_Options[playerid] = row;
	// Remember which category we are viewing (for the Back button in HELP_DETAIL_DIALOG)
	g_HelpLastCat[playerid] = cat;
	return 1;
}

//==============================================================================
//	Help_ShowDetailDialog — show the MSGBOX detail for one command row.
//	Declared with FUNCTION macro so menu.inc can call it via
//	CallLocalFunction("Help_ShowDetailDialog", "ii", playerid, cmdRow).
//==============================================================================

FUNCTION Help_ShowDetailDialog(playerid, cmdRow)
{
	new dlgTitle[48], dlgBody[256];
	format(dlgTitle, sizeof(dlgTitle), "%s", g_HelpCmd[cmdRow][HCMD_NAME]);
	format(dlgBody, sizeof(dlgBody), "%s\n\n%s", g_HelpCmd[cmdRow][HCMD_SHORT], g_HelpCmd[cmdRow][HCMD_LONG]);
	ShowPlayerDialog(playerid, HELP_DETAIL_DIALOG, DIALOG_STYLE_MSGBOX, dlgTitle, dlgBody, "Back", "");
	return 1;
}

//==============================================================================
//	CMD:help — open the feature guide browser
//	Owns aliases: /info, /hlp, /features
//	Does NOT own /pinfo or /i — those belong to cmds/player.inc (Stage D)
//	Does NOT own /commands or /showcommands — those are the flat admin list
//==============================================================================

CMD:help(playerid, params[])
{
	#pragma unused params
	Help_ShowCategories(playerid);
	return 1;
}
alias:help("info", "hlp", "features")
