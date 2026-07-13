# CrazyBob's Cops And Robbers — Full Site Transcription

**Research date:** 2026-07-14
**Source:** https://www.crazybobs.net/website/ — all pages reachable via WebFetch as of this sweep.
**Purpose:** Authoritative spec for the EXP Gaming C&R revival. Every section is tagged with its source URL. Verbatim numbers and command names are preserved throughout.

---

## Site Map

Every URL found, with one-line contents:

### Main navigation pages

| URL | Contents |
|-----|----------|
| `https://www.crazybobs.net/website/` | News / home page; navigation hub for all sections |
| `https://www.crazybobs.net/website/about-cnr` | History of CnR (started March 2006 as MTA, moved to SA-MP); dev team: CrazyBob, Mr.X, vick. |
| `https://www.crazybobs.net/website/cnr-10-codes` | Full 10-code list (10-0 through 10-161) verbatim |
| `https://www.crazybobs.net/website/cnr-commands` | Complete ~400-command reference grouped by category |
| `https://www.crazybobs.net/website/faq` | Registration, score/money basics, city-per-data, account rules |
| `https://www.crazybobs.net/website/how-play` | Overview of roles, wanted tiers, player-colour system |
| `https://www.crazybobs.net/website/maps` | Links to online/static maps; house map; satellite map |
| `https://www.crazybobs.net/website/missions` | Hub page listing 19 missions (each is a link to a sub-page) |
| `https://www.crazybobs.net/website/other` | Hub page listing sub-pages under "Other" category |
| `https://www.crazybobs.net/website/places` | Hub page listing 19 in-game venues |
| `https://www.crazybobs.net/website/rules` | 11 server rules verbatim |
| `https://www.crazybobs.net/website/skills` | 16 skills with brief descriptions and commands |
| `https://www.crazybobs.net/website/whats-new` | Changelog from Version 15 (Dec 2009) to Version 24.1 (Oct 2017) |
| `https://www.crazybobs.net/website/donate` | Donation page — PayPal only; explicitly states no perks granted |
| `https://www.crazybobs.net/website/connecting-to-crazybobs` | How to connect; servers s1/s2.crazybobs.net:7777 |

### Mission sub-pages (under /website/missions)

| URL | Contents |
|-----|----------|
| `https://www.crazybobs.net/website/airport-robbery` | Heist with truck/van/garbage truck; collect boxes; timer; hideout |
| `https://www.crazybobs.net/website/courier-delivery` | Smuggle illegal goods into city; 12 game-hour time limit; warrant on pickup |
| `https://www.crazybobs.net/website/domestic-disturbance` | Cop-only; drive from PD to a player-owned house; speed = more pay |
| `https://www.crazybobs.net/website/drug-delivery` | 5 deliveries to random locations; innocent status required; land vehicles only |
| `https://www.crazybobs.net/website/flower-delivery` | Deliver flowers to 5 players in 6 game hours; both cop and civ; cash bonus |
| `https://www.crazybobs.net/website/food-delivery` | Requires Mr.Whoopee/Hotdog/Pizzaboy; 5 deliveries; distance-scaled time |
| `https://www.crazybobs.net/website/holdup` | Rob 3 different stores; $10,000 min each; large completion bonus |
| `https://www.crazybobs.net/website/house-delivery` | Deliver items (some illegal) to random houses; 12 game-hour limit; civilian only |
| `https://www.crazybobs.net/website/illegal-immigrant-transport` | Pick up immigrants outside city; bring to city; wanted on completion; night reduces cop visibility |
| `https://www.crazybobs.net/website/lawn-mowing` | LV only; golf course; lawnmower; reach as many checkpoints as possible in 2 game hours |
| `https://www.crazybobs.net/website/paperboy` | Innocent civilian; 4-minute limit; deliver papers on foot to owned houses; $500/delivery |
| `https://www.crazybobs.net/website/pickpocket` | Rob $10,000 total from different players in 6.5 game hours; 3 city checkpoints |
| `https://www.crazybobs.net/website/race-challenges` | 28 challenges; `/challenge`; first free then cost; car/bike/boat/air/truck types |
| `https://www.crazybobs.net/website/routine-patrol` | Cop-only; 5 checkpoints; $2,500 each + $25,000 bonus = $37,500 total |
| `https://www.crazybobs.net/website/sexual-encounter` | Innocent civilian; sex shop start; 5 encounters in 5 game hours; Rapist/Prostitute skill boosts success |
| `https://www.crazybobs.net/website/tractor-mission` | CrazyBob's Farm (SF tractors only); 2 game hours; reach checkpoints; similar to Lawn Mowing |
| `https://www.crazybobs.net/website/trash-pickup` | Trashmaster vehicle; 5 checkpoints; bonus per checkpoint + all-checkpoint bonus under 6 hrs |
| `https://www.crazybobs.net/website/truck-delivery` | 14 truck types; `/delivery`; 12 game-hour limit; some illegal cargo = enhanced pay |
| `https://www.crazybobs.net/website/vehicle-theft` | 5 random vehicles; $15,000/car + $75,000 all-complete; last car = instant WL10; 15 game-hour limit |

### "Other" sub-pages

| URL | Contents |
|-----|----------|
| `https://www.crazybobs.net/website/bots` | Bot types: City Hall, PD, RPC, Transport, Prostitute, Drug Dealer, Driving Challenge bots |
| `https://www.crazybobs.net/website/clothes-accessories` | Clothing carry limit 35 items, wear 5; shops, house storage 20; free phone at 100 hours played |
| `https://www.crazybobs.net/website/cnr-markets` | Stock market; 21 stocks; trading hours 07:00–19:00; `/markets`, `/shares` |
| `https://www.crazybobs.net/website/crimes-jail-sentence` | Crime overview + jail options (serve/bail, escape, bribe, appeal to 15 jurors); crimes list is an image (not machine-readable) |
| `https://www.crazybobs.net/website/driving` | Driver Upgrade (Xoomer Gas Stations); `/driver [fare]`; Drivers lose car-jack protection; v18+ |
| `https://www.crazybobs.net/website/fishing` | Full fishing guide; tournaments; commands list; fishing rod + permits; bonus fish |
| `https://www.crazybobs.net/website/gambling` | Casino games (slots/blackjack/video poker via Y key); horse betting; scratch tickets; dice |
| `https://www.crazybobs.net/website/gps` | Navigation arrows; 5 saved locations; `/gps`, `/gpsset`, `/gpshide`, `/gpsresume`; auto-activates for missions |
| `https://www.crazybobs.net/website/drug-planting-and-hunting` | Drug growing mechanics; seeds; 5-plant limit; 20-min grow; hunting permits; deer traps |
| `https://www.crazybobs.net/website/houses` | 1500+ properties; own 2 / co-own 8 / rent 10; storage caps; alarm + super lock + pet security |
| `https://www.crazybobs.net/website/house-maid` | Enter owned house; earn ~$100/sec cleaning; $1,000 completion bonus + 1 score point |
| `https://www.crazybobs.net/website/game-items` | Full item catalogue (no prices shown on site) |
| `https://www.crazybobs.net/website/pimping` | Pimp-prostitute relationship; pimp pays medical fees; cut of sex/strip earnings |
| `https://www.crazybobs.net/website/player-races` | Wagered races between players; `/race`, `/racejoin`, `/raceinvite`; race owner starts countdown |
| `https://www.crazybobs.net/website/quick-strings` | 40+ dynamic variables ($veh, $loc, $cash etc.); 3 custom quick strings via `/$set` |
| `https://www.crazybobs.net/website/robberies` | All robbery types; crowbar; shoplifting; v23 rework details |
| `https://www.crazybobs.net/website/user-accounts` | Registration `/register`; city-specific data; `/newlife` resets money/houses/inventory; death = medical fees |
| `https://www.crazybobs.net/website/selling-vehicles` | Car Sell Crane; any civilian; Car Jacker advantage; damage = lower payout; `/sell`, `/vsi` |

### "Places" sub-pages

| URL | Contents |
|-----|----------|
| `https://www.crazybobs.net/website/airport` | 3 main airports + Verdant Meadows / Area 69 / Easter Basin Naval Station; Rustlers; skydiving; bot pilots |
| `https://www.crazybobs.net/website/ammunation` | 11 locations; cops instant access; civilians need Gun Permit; "weapons not sold by player dealers" |
| `https://www.crazybobs.net/website/bait-shop` | 6 locations (2 LS, 3 SF, 1 LV); fishing permits, rods, coolers, crowbar, seeds, hunting permits |
| `https://www.crazybobs.net/website/bank` | 3 major + 4 minor banks; deposit, tax refund, insurance, stock trading; robbery risk |
| `https://www.crazybobs.net/website/bar-and-dance-club` | Bars and dance clubs across 3 cities; alcohol; locations listed |
| `https://www.crazybobs.net/website/car-dealership` | 6 dealership locations; no vehicle list or prices on page |
| `https://www.crazybobs.net/website/car-sell-crane` | 3 locations; daily bonus vehicle; damage reduces payout |
| `https://www.crazybobs.net/website/casino` | 3 LV casinos (Caligula's, Four Dragons, Redsands); "known for daily robberies" |
| `https://www.crazybobs.net/website/church` | 8 churches; confess sins; donate; pray; buy holy wine; Flower Delivery mission start; wanted level reduction |
| `https://www.crazybobs.net/website/city-hall` | 3 cities; bot secretaries; life insurance; tax refunds; skill changes |
| `https://www.crazybobs.net/website/dm-stadium` | 3 stadiums; $5,000 entry; $1,000/death; 2500 ammo per life; no permanent consequences |
| `https://www.crazybobs.net/website/drug-refill-point` | 8 locations; fertilizer, seeds, deer traps, drug bags; only place to get drug bags |
| `https://www.crazybobs.net/website/gym` | 3 gyms (Ganton/Garcia/Redsands East); 3 fighting style options |
| `https://www.crazybobs.net/website/hospital` | 7 hospitals; condoms, chastity belt, adrenaline pill, skin change (once/day) |
| `https://www.crazybobs.net/website/hotels` | Room rental (daily cost); spawn point; car save add-on; safe deposit box; room service; max 10 rooms/life |
| `https://www.crazybobs.net/website/other-locations` | CrazyBob's Place (LS); SF Federal Mint; Jizzy's Pleasure Domes (SF); Inside Track Betting (LS) |
| `https://www.crazybobs.net/website/police-department` | 3 main PDs + 4 secondary; bot for armor/weapon refill; cop vehicles; bail payment |
| `https://www.crazybobs.net/website/regular-players-club` | 3 clubs (Rodeo LS / Financial SF / Emerald Isle LV); music; Regular Player status required |
| `https://www.crazybobs.net/website/restaurants-and-diners` | Well Stacked Pizza (9), Burger Shot (8), Cluckin' Bell (10), Diners (9), Donut Shop (3, cops only) |
| `https://www.crazybobs.net/website/stores-and-shops` | 24/7 (35 locations); ZIP/Binco/Prolaps/Sub Urban/Victim/Didier Sachs clothing; 6 barbers; 8 sex shops |
| `https://www.crazybobs.net/website/train-station` | 5 stations (2 LS, 1 SF, 2 LV); passenger + freight trains between cities |

### Pagination / news articles (not transcribed in full)

`/website/node?page=1` through `/website/node?page=20` — older news articles.
Notable articles: `/website/node/236` (End Of The Line), `/website/anniversary-party-1/2/3`, `/website/halloween2017`.

---

## Missions

**Source:** https://www.crazybobs.net/website/missions and all 19 sub-pages.

The missions index states: *"This section contains all the information about all the different missions in CnR and how to complete them."*

### 1. Airport Robbery

**Source:** https://www.crazybobs.net/website/airport-robbery

- **How to start:** Head to the current city's airport, enter the checkpoint, type `/robbery`.
- **Vehicle requirement:** Must be in a truck, van, or garbage truck.
- **Steps:** After initiating, a timer appears. Exit your vehicle and collect boxes using the Middle Mouse Button or `/box`. The more boxes collected, the more money at the end.
- **Wanted status:** You are wanted while collecting boxes.
- **Escape:** When the timer expires, return to your vehicle and reach the hideout within another time limit to receive payment and potentially reduce your wanted level.
- **Cancel:** `/cancel` at any time.
- **Available in:** Los Santos International Airport, San Fierro Easter Bay Airport, Las Venturas Airport.
- **Payout:** Not specified numerically — scales with boxes collected.
- **Score:** Not specified on page (general robberies = +1 per robbery from Score Guide).

### 2. Courier Delivery

**Source:** https://www.crazybobs.net/website/courier-delivery

- **How to start:** Type `/courier` as a civilian.
- **Mechanic:** "The pickup location is always far away from the city and the delivery destination is always in the city."
- **Time limit:** 12 game hours to complete.
- **Bonus:** Extra payment based on distance and time upon successful completion.
- **Vehicle:** Any vehicle.
- **Wanted:** You receive a warrant for smuggling on pickup. Police actively attempt to intercept.
- **Commands:** `/courier` to start; Sub-Mission Key or `/mission` to check status; `/cancel` to abandon.
- **Payout:** High (described as higher than truck delivery); exact number not stated.
- **Score:** +1 per delivery (from Score Guide).

### 3. Domestic Disturbance

**Source:** https://www.crazybobs.net/website/domestic-disturbance

- **Who:** Cops only.
- **How to start:** Visit the main Police Department in your current city, enter the checkpoint, type `/mission`, select the second option.
- **Objective:** Drive from the PD to a player-owned house somewhere in San Andreas within the time limit.
- **Reward:** "Get to the designated house as fast as you can, the faster you are, the bigger your bonus!" — cash scales with speed.
- **Cancel:** `/cancel` at any time.
- **Starting PD locations:**
  - Los Santos LSPD HQ: 1582.375, -1635.000
  - San Fierro SFPD HQ: -1621.750, 682.125
  - Las Venturas LVPD HQ: 2244.375, 2492.875
- **Payout:** Variable / speed-based; exact figures not stated.

### 4. Drug Delivery

**Source:** https://www.crazybobs.net/website/drug-delivery

- **Who:** Innocent civilians only (must have no wanted level to start).
- **How to start:** Enter the mission checkpoint. Available in all three cities:
  - Los Santos: Las Colinas
  - San Fierro: Calton Heights
  - Las Venturas: LVA Freight Depot
- **Objective:** Deliver drugs to 5 randomly selected locations in any order before the timer runs out. Land vehicles only.
- **Each checkpoint:** Reveals next location when reached. Press Sub-Mission Key or `/mission` to see destination list.
- **Payout:** $5,000 per checkpoint + $50,000 all-5 bonus (from Score Guide; site page does not repeat the numbers).
- **Score:** +1 per checkpoint (Score Guide).
- **Cooldown:** Once every 5 game hours (Score Guide).
- **Wanted effect:** +2 wanted per delivery if a cop is in sight (cop sight radius is smaller at night) (Score Guide). No warrant unless cop present 3 times.
- **Cancel:** `/cancel` at any time.

### 5. Flower Delivery

**Source:** https://www.crazybobs.net/website/flower-delivery

- **Who:** Both cops and civilians; innocent status assumed.
- **How to start:** Visit the Flower Delivery checkpoint in your city and enter `/mission`. Can also start from Churches.
- **Checkpoint locations:**
  - Los Santos: Jefferson
  - San Fierro: Hashbury
  - Las Venturas: South East
- **Objective:** Deliver flowers to 5 different players in under 6 game hours. Use the `/flowers` command to deliver.
- **Strategy:** Find players in populated areas like City Hall or PD.
- **Reward:** "A cash bonus and a warm fuzzy feeling inside." Exact amount not specified.
- **Cancel:** `/cancel` at any time.

### 6. Food Delivery

**Source:** https://www.crazybobs.net/website/food-delivery

- **Who:** Any innocent civilian.
- **Vehicle requirement:** Must be driving a Mr.Whoopee, Hotdog, or Pizzaboy.
- **How to start:** Type `/mission` while in the food delivery vehicle.
- **Objective:** Complete 5 food deliveries to 5 different locations within the city. Next location revealed after each delivery.
- **Time:** Extra time granted based on distance of next delivery.
- **Commands:** Sub-Mission Key or `/mission` during active delivery for current location; `/cancel` to abort.
- **Payout:** $2,500 per checkpoint + $25,000 all-complete bonus (from Score Guide; total = $37,500).
- **Score:** +1 per checkpoint (Score Guide). Also +$500/item to next-day pay (Score Guide).

### 7. Holdup (Mission variant)

**Source:** https://www.crazybobs.net/website/holdup

- **How to start:** Travel to the city's Holdup mission checkpoint, execute `/mission`.
- **Checkpoint locations:**
  - Los Santos: Market
  - San Fierro: Chinatown
  - Las Venturas: Linden Side
- **Objective:** Rob 3 different stores (24/7s, gas stations, food shops). Must steal minimum $10,000 from each.
- **Reward:** Money directly from each robbery plus "a large bonus upon completing the mission."
- **Warning:** "Robbing so many shops will get you a high warrant and lots of attention."
- **Cancel:** `/cancel` at any time.
- **Note:** This is a *mission-wrapper* around the `/holdup` robbery system (see Robberies section below for the underlying holdup mechanics).

### 8. House Delivery

**Source:** https://www.crazybobs.net/website/house-delivery

- **Who:** Civilians only (regardless of wanted status).
- **How to start:** Enter one of three checkpoints:
  - Los Santos: Ocean Docks (2165.500, -2279.500)
  - San Fierro: Doherty (-1821.250, -182.000)
  - Las Venturas: LVA Freight Depot (1380.250, 1026.500)
- **Objective:** Transport delivery items (including some illegal goods) to randomly selected houses.
- **Time limit:** 12 game hours.
- **Reward:** Bonus based on time and distance upon completion. Exact figure not stated.
- **Vehicle:** Any vehicle.
- **Commands:** Sub-Mission Key or `/mission` for current delivery location; `/cancel` to abort.

### 9. Illegal Immigrant Transport

**Source:** https://www.crazybobs.net/website/illegal-immigrant-transport

- **Who:** Innocent civilians.
- **How to start:** Enter mission checkpoint (outside main city).
  - Los Santos: East Los Santos
  - San Fierro: Hashbury
  - Las Venturas: Spiny Bed
- **Objective:** Pick up illegal immigrants at a location outside the main city, return them to the current city within 12 game hours.
- **Stealth mechanic:** Avoiding police during the mission prevents a wanted level during gameplay. "Police Officers have a reduced visibility radius at night."
- **Consequence:** Upon successful completion, you receive a wanted level for transporting illegal immigrants.
- **Commands:** `/mission` or Sub-Mission Key for info; `/cancel` to abort.
- **Payout:** Not specified numerically on page.

### 10. Lawn Mowing

**Source:** https://www.crazybobs.net/website/lawn-mowing

- **Location:** Las Venturas only — Yellow Bell Golf Course / Prickle Pine golf course.
- **Vehicle requirement:** Must be on a lawn mower.
- **How to start:** Go to the checkpoint at the golf course while on a mower, type `/mission`, select first option.
- **Objective:** Reach as many checkpoints as possible in 2 in-game hours.
- **Scoring:** "The higher the number of checkpoints, the higher you rank against other players." — competitive leaderboard.
- **Reward:** Bonus upon completion; specific figure not stated. "A safe and relaxing way to make money." "The only CnR mission fully endorsed by Hank Hill."
- **Cancel:** `/cancel` at any time (must then wait several minutes before restarting).

### 11. Paperboy

**Source:** https://www.crazybobs.net/website/paperboy

- **Who:** Innocent civilians.
- **How to start:** Visit mission checkpoint in current city:
  - Los Santos: Vinewood (845.250, -1042.000)
  - San Fierro: Esplanade East (-1536.250, 1045.500)
  - Las Venturas: Royal Casino (2250.125, 1466.750)
- **Objective:** Deliver as many papers as possible to owned houses around the city in 4 minutes (real-time).
- **Mechanic:** Walk on foot to checkpoints in front of houses.
- **Payout:** $500 per delivery + bonus rewards based on total deliveries.
- **Commands:** Sub-Mission Key or `/mission` for info; `/cancel` to exit.

### 12. Pickpocket (Mission)

**Source:** https://www.crazybobs.net/website/pickpocket

- **Who:** Civilians (Pickpocket skill mission specifically).
- **How to start:** Enter mission checkpoint:
  - Los Santos: Willowfield
  - San Fierro: Financial
  - Las Venturas: Redsands West
- **Objective:** Successfully rob a total amount of $10,000 from different players within 6.5 game hours.
- **Commands:** Sub-Mission Key or `/mission` to check progress; `/cancel` to abandon.
- **Note:** This is distinct from the regular `/rob` command; it is a structured mission with a monetary target.

### 13. Race Challenges

**Source:** https://www.crazybobs.net/website/race-challenges

- **Command:** `/challenge`
- **Cost:** First attempt free; subsequent attempts cost money.
- **Reward:** Winning a Top 10 ranking earns cash. +1 score per finish, +2 score for podium finish (Score Guide).
- **Types:**
  - Point To Point (fixed start)
  - Circuit (start determined by player's closest location)
- **Vehicle categories:** Land vehicles, trucks only (Trucker Challenge), bikes only, boats only, air vehicles (helicopters/planes).
- **28 challenges total** including:
  - San Andreas Tour (27.5 km land)
  - Red County Run (dirt)
  - Los Santos Circuit (3 km city)
  - Chiliad Climb / Admin Hill
  - Trucker Challenge (12 km trucks only)
  - Las Venturas Biker Endurance (9 km bikes)
  - Boating With Jaws (Very Hard)
  - Las Venturas Fly-By / San Fierro Fly-By / LS Helicopter Tour (air)
- **Difficulty range:** Easy to Very Hard.
- **Traffic:** integrated into challenge design.

### 14. Routine Patrol

**Source:** https://www.crazybobs.net/website/routine-patrol

- **Who:** Cops only.
- **How to start:** Visit PD checkpoint, type `/mission`, select first option. Land vehicles only.
- **Starting locations:**
  - Los Santos LSPD HQ: 1582.375, -1635.000
  - San Fierro SFPD HQ: -1621.750, 682.125
  - Las Venturas LVPD HQ: 2244.375, 2492.875
- **Objective:** Reach 5 sequential checkpoints before time expires. Timer for each checkpoint individually ("a short amount of time to reach a certain checkpoint").
- **Payout:** $2,500 per checkpoint + $25,000 completion bonus = **$37,500 total**.
- **Score:** +1 per checkpoint (Score Guide).
- **Cancel:** `/cancel` at any time.

### 15. Sexual Encounter

**Source:** https://www.crazybobs.net/website/sexual-encounter

- **Who:** Any innocent civilian; Rapist or Prostitute skill increases success chance.
- **How to start:** Visit a sex shop in-game and enter the `/mission` command.
- **Objective:** Complete sexual encounters with at least 5 characters within 5 game hours.
- **Cancel:** `/cancel` during active gameplay.
- **Payout:** Cash bonus; exact amount not stated.

### 16. Tractor Mission

**Source:** https://www.crazybobs.net/website/tractor-mission

- **Location:** CrazyBob's Farm. Can be done in any city but tractors only spawn in San Fierro.
- **Vehicle requirement:** Must be on a tractor.
- **How to start:** Approach the checkpoint near two tractors, type `/mis` or `/mission`.
- **Objective:** 2 in-game hours to reach as many checkpoints as possible.
- **Reward:** "Upon completion, you get a small bonus and if you can get the most, you will set a record."
- **Cancel:** `/cancel` at any time; must wait several minutes before restarting.
- **Structure:** Similar to the Lawn Mowing mission.

### 17. Trash Pickup

**Source:** https://www.crazybobs.net/website/trash-pickup

- **Vehicle requirement:** Must be driving a Trashmaster.
- **How to start:** Press Sub-Mission Key or type `/mission` while in a Trashmaster.
- **Objective:** Drive through 5 checkpoints scattered across the current city.
- **Reward:** Bonus per checkpoint + additional bonus if all 5 completed under 6 game hours.
- **Payout:** $2,500 per checkpoint + $25,000 all-complete bonus (from Score Guide).
- **Score:** +1 per checkpoint (Score Guide).
- **Auto-cancel condition:** Mission cancels if Trashmaster is lost.
- **Commands:** Sub-Mission Key or `/mission` during mission; `/cancel` anytime.

### 18. Truck Delivery

**Source:** https://www.crazybobs.net/website/truck-delivery

- **How to start:** Press Sub-Mission Key or type `/delivery` while in a delivery truck.
- **Vehicle types (14 supported):** Benson, Boxville, Cement Truck, DFT-30, Dumper, Dune, Flatbed, Linerunner, Mule, Packer, Pony, Roadtrain, Tanker, Yankee.
- **Time limit:** 12 game hours.
- **Reward:** Base pay + bonuses for distance, time completion, and trailer attachment. Some illegal cargo provides enhanced compensation.
- **Commands:**
  - `/mission` or Sub-Mission Key — get info about current delivery or pickup
  - `/truckmsg`, `/tm`, `/cb` — communicate with other truckers
  - `/cancel` — abandon (also auto-cancels if truck is lost)
- **Score:** +1 per delivery (Score Guide). Note: some cargo raises wanted level but does not issue a warrant (Score Guide).
- **Payout:** Cargo-based; exact formula not published.

### 19. Vehicle Theft ("Gone in 15 Hours")

**Source:** https://www.crazybobs.net/website/vehicle-theft

- **How to start:** Enter mission checkpoint:
  - Los Santos: Market
  - San Fierro: Downtown
  - Las Venturas: Redsands West
- **Objective:** Locate and deliver 5 randomly-assigned vehicles to a checkpoint within **15 game hours**.
- **Payouts (verbatim):**
  - **$15,000 bonus per vehicle delivered**
  - **$75,000 completion bonus** (all 5)
  - Total potential: **$150,000**
- **Last car warning:** "When you get into the last vehicle on your list, you receive Level 10 wanted level." Strategy: pick fastest or closest vehicle as your final.
- **Score:** +1 per car (Score Guide).
- **Commands:** `/cancel` to abort at any time.
- **Note:** "Not everyone knows all of the vehicle names" — must recognize vehicles by sight.

---

## Robberies and Crowbar

**Source:** https://www.crazybobs.net/website/robberies + https://www.crazybobs.net/website/game-items + https://www.crazybobs.net/website/holdup (from primary-sources.md)

### General Robbery Principles

- "A warrant for your Arrest will be issued when you commit a Robbery and the Police will attempt to prevent you from completing the robbery."
- **Version 23 rework:** Each robbery now has individualized settings including minimum/maximum times, bonuses, and success chances.
- **Alarm change (v23):** "The alarm for a robbery location will no longer go off immediately. Cops are notified of the robbery only when the alarm goes off." Alarms trigger immediately if law enforcement approaches.
- **Payouts scale with server population.**
- **Each robbery = +1 score** (Score Guide).

### Robbery Types

#### Store Holdups

**Command:** `/holdup` (`/hup`, `/storerob`, `/robstore`)

**Eligible locations:** 24/7 stores, Ammunation, Bars, Restaurants/Diners, Dance Clubs, Sex Shops, Inside Track Betting, Clothes stores.

**Mechanics (verbatim from primary-sources.md / Fandom Robbery wiki):**
- Must be on foot.
- Must be **looking at a store clerk** to begin.
- Instantly adds **+7 wanted** (Felon tier).
- Wait approximately **10 seconds** for the clerk to open the register.
- Then receive **$2,000–$6,000 per second** while the register is open.

**Crowbar effect on holdups:**
- Holding your crowbar increases the per-second amount AND reduces the register open time by a couple of seconds.
- Cop who enters the shop and presses MMB can arrest mid-holdup.
- Leave the cash zone and exit to finish.
- You can `/bribe` a cop afterward to go innocent (keeps crowbar + weapons).

#### Bank Robbery

**Command:** `/bankrob` (`/robbank`)

- **Available at:** All 3 city banks (LV, SF, LS).
- **Mechanics:** Wait approximately **30 seconds** at the checkpoint (cops are alerted during wait), then deliver the **safe to a random hideout** to receive payment.
- "Bank Robberies take money away from other players accounts." Banks close during and after a robbery.
- **Max taken per victim account: $100,000** (a small % of 0–$500,000 range drawn from random accounts, not all players).
- On a full server, expect **~$500k–$1M total**.
- Bank-insured players receive 75% back.
- **7 banks total:** Los Santos (620.250, -1258.000), San Fierro (-2021.000, 460.250), Las Venturas (2470.000, 2258.750), Fort Carson, Las Barrancas, Palomino Creek, and one additional.

#### Casino Robbery

**Command:** `/casinorob` (or `/robbery` at casino)

- **Available at:** All 5 casinos (3 in LV: Caligula's Palace, Four Dragons, Redsands Casino; 1 in SF; 1 in LS).
- Rob the interior, then transport the safe to a random hideout to crack it for payment.

#### CrazyBob's Place Robbery

**Command:** `/robbery`

- **Location:** Mulholland, Los Santos (near Vinewood mansion).
- 12 minutes to get to the hideout after successfully opening the safe.
- Must remain within checkpoint boundaries or face failure.

#### Airport Robbery

See Missions section above. Command: `/robbery` at airport checkpoint.

#### Special/Major Robberies (Version 23+)

- San Fierro Federal Mint — break in; 12 minutes to reach hideout.
- Los Santos SA-MP Office Tower
- Los Santos Observatory Mansion
- San Fierro Jizzy's Pleasure Domes — robbable location; also used for parties.
- Woozie's Private Club (SF)
- SF Cargo Ship
- SF Zombotech
- SF Drug Factory
- Las Venturas Methylamine at K.A.C.C. Fuels — find a barrel and reach hideouts.

#### House Robbery

**Command:** `/houserob` (`/hrob`, `/hsrob`)

- Break in while owner is online.
- Steal storage contents (money, drugs, condoms, flowers, seeds, deer traps, fish, clothing).
- Alarm and/or owner-present status pings police.
- House alarm: "alerts all owners, co-owners and the police anytime someone breaks into your house."
- House Super Lock: "makes it much more difficult for anyone to break into your house."
- Storage caps that can be stolen: 25 condoms, 25 flowers, 5000g drugs, 25 seeds, 25 deer traps, 10 fish, 20 clothing items.

#### Shoplifting

**Command:** `/shoplift`

At shop checkpoints. Steals items into your inventory.

### The Crowbar — Complete Mechanics

**Source:** https://www.crazybobs.net/website/robberies + https://www.crazybobs.net/website/game-items

**Game Items page (verbatim):**
> "Holding your Crowbar (/clotheswear) will lower your robbery time (robberies & holdups) and increase your chance of a successful robbery."

**Robberies page (verbatim):**
> "Holding your Crowbar will lower your robbery time and increase your chance of a successful robbery."

**Where to buy:** Truck Stops or Zero's RC Shop (Garcia, San Fierro).

**Bait Shop price:** $40,000 (from Street_Vendor wiki sample prices in economy.md).

**Activation:** Equip via `/clotheswear` (the crowbar is treated as a clothing/item slot, not a weapon slot).

**Remove:** `/crowbar` command.

**What exactly the crowbar does:**

1. **Holdups** — reduces the time it takes for the clerk to open the register (reduces that initial ~10 second wait "by a couple seconds") AND increases the per-second cash amount you receive from the register.
2. **Robberies (safe-type)** — lowers your robbery time (time required to open the safe / complete the robbery stage) AND increases your chance of a successful robbery. This applies to bank, casino, CrazyBob's Place, and special robberies.
3. **NOT a weapon** — the crowbar is an item/permit slot, not used as a melee weapon in the standard crime sense.

**Confiscation on arrest:**
> "Removed upon arrest" (Robberies page).
> From jail page (primary-sources.md): "Crowbar removed if crime history has a robbery violation. Exception: if you died to a cop's use of force, crowbar/rod are NOT removed — intentional."

**Cooldowns:** No explicit cooldown on the crowbar itself; the robbery activity cooldowns (if any) were not published as exact times.

**Wanted level effect of using crowbar:** No additional wanted-level penalty from the crowbar itself — the robbery it assists already issues a warrant.

---

## Skills

**Source:** https://www.crazybobs.net/website/skills

16 skills confirmed. "If at any point you would like to change skills, you can visit City Hall and have it changed for a price. If you wish to change your skin, you can go to the Hospital and have that changed for a price." Skin change: once per gameday. First skill change free, then ~$33,000 average.

**Law Enforcement family (4 skills):**

| Skill | Role | Key commands |
|-------|------|-------------|
| Police Officer | Arrest warrants, ticket suspects | `/arrest`, `/report`, `/cancellastreport`, `/ticket`, `/backup`, `/respond`, `/copmsg`, `/refill`, `/accept`, `/refuse`, `/jaillist`, `/suspects`, `/warrants`, `/mission` |
| Public Medic | Heal/cure anyone; force-cure STDs | `/medic`, `/cure`, `/healme`, `/cureme`, `/prices`, `/calls` |
| Police Technician | Supply cops weapons/ammo + repairs | `/weapons`, `/prices`, `/calls`, `/vehrepair` |
| Private Medic | Heal specific players + can `/infect` | `/medic`, `/healme`, `/cureme`, `/infect`, `/calls` |

**Civilian family (12 skills):**

| Skill | Advantage | Key commands |
|-------|-----------|-------------|
| Arms Dealer | Sell weapons to civilians (crime) | `/weapons`, `/prices`, `/calls` |
| Car Jacker | Steal locked cars; 5-min crane cooldown (vs 8 min base) | `/sell` |
| Con Artist | Rob up to $500,000; moderate success; tax evasion 9/10 | `/rob [nick/id]` |
| Drug Dealer | Sell drugs; carry 2500g (not 5000g — see note); grow faster | `/drugs`, `/prices`, `/calls` |
| Hitman | Fulfil hit contracts for the bounty | `/hits`, `/hit`, `/cancelhit` |
| Kidnapper | Lock passengers in vehicle; ransom; appears as Driver | `/kidnap`, `/kidnapall`, `/release`, `/releaseall`, `/fakeskill` |
| Mechanic | Repair vehicles + sell mods | `/mechanic`, `/prices`, `/calls`, `/vehrepair` |
| Pickpocket | Very high rob success; up to $100,000 per hit | `/rob [nick/id]` |
| Prostitute | Sell sex; strip; can pass STDs | `/sex`, `/strip`, `/calls` |
| Rapist | Higher rape success | `/rape [nick/id]` |
| Food Delivery | Sell food/drinks; can use Pizzaboy | `/food`, `/prices`, `/calls` |
| Street Vendor | Sell items to anyone (including cops) | `/items`, `/prices`, `/calls` |

**Note on Drug Dealer capacity:** The Fandom Drug_Dealer page says 2500g starts (from 500g base), going up to 5000g with a drug bag. The economy.md primary-sources.md notes both 2500g and 5000g are cited depending on source.

**Mechanic skill** appears on the official skills page but was listed as `[INFERRED - not confirmed as separate from Police Technician]` in prior research. It is confirmed here as a separate civilian skill.

---

## Rules (Complete)

**Source:** https://www.crazybobs.net/website/rules

1. **Admin Respect:** "Admins are there to make sure the game is fun for everyone." Follow admin instructions; no impersonating admins.
2. **Player Respect:** Treat as recreation; avoid insulting/deliberately disrupting others.
3. **No Deathmatching:** "Randomly attacking other players is not tolerated." Self-defense is permitted; excessive/random killing = removal.
4. **Cop Conduct:** Law enforcement cannot attack innocent civilians or other officers, steal civilian vehicles, or team-kill. Must protect non-criminals.
5. **Anti-Cheat:** Cheating and bug exploitation = bans and account disabling. Report exploitable bugs to admins privately.
6. **Mods Prohibited:** Only the Detailed Radar Mod is permitted.
6a. **Official Client:** Must use official SA-MP 0.3.7 from sa-mp.com; third-party clients blocked.
7. **No Advertising:** No server promotion, recruiting, or spam across any platform.
8. **No Pausing:** Idle players face automatic removal.
9. **No Quit Evasion:** Leaving to escape consequences = penalties.
10. **Traffic Laws:** Keep right, pass left, avoid ramming, yield to emergency vehicles.
11. **Faction Separation:** Law enforcement should not collaborate with criminals beyond accepting discretionary bribes.

---

## Bots

**Source:** https://www.crazybobs.net/website/bots

Bots identified by `[BOT]` tag. Interact via Sub-Mission button or Y key.

| Bot Type | Location | Function |
|----------|----------|----------|
| City Hall Bot | City Hall (each city) | Sell permits and other items |
| Police Department Bot | 3 main PDs | Civilians: pay fines / become temp cop; Cops: refill weapons and armor |
| Regular Players Club Bot | RPC (each city) | Sell food, drinks, and other items |
| Train Driver Bot | Train Stations | Provide train travel between cities |
| Bus Driver Bot | Airports/transit | Provide bus travel |
| Airplane Pilot Bot | Airports | Provide air travel between cities |
| Prostitute Bot | One per city | Sexual services |
| Drug Dealer Bot | One per city | Sell drugs; buy back player drugs |
| Driving Challenge Bot | 3 across SA (2 car, 1 helicopter) | Betting challenges; passengers stay in range until challenge ends |

Note: Store clerks, strippers, and blackjack dealers exist but are NOT classified as bots — they don't chat or respond to social commands.

---

## Clothes & Accessories

**Source:** https://www.crazybobs.net/website/clothes-accessories

- Carry up to **35 items** of clothing; wear up to **5 at a time**.
- House storage: up to **20 clothing items**.
- Shops: Clothes and Barber Shops with rotating weekly inventories.
- Players can preview items before purchasing.
- Resale: sell clothes back to stores after waiting several game days post-purchase.
- Trading: direct player-to-player clothing transactions supported.
- Customization: positioning, rotation, scale adjustments for worn items.
- **Free phone** once you have **100 hours played** for a city. Special items unlock through secret tasks.

**Commands:** `/clothes`, `/clotheswear`, `/clothesdiscard`, `/clothespos`, `/clothesprices`, `/clothessell`, `/clothesbuy`, `/clothesinfo`, `/clothesinv`.

**Six clothing chains:** ZIP (4), Binco (4), Prolaps (2), Sub Urban (3), Victim (3), Didier Sachs (1). Plus **6 barber shops** for hair + rare hats. Plus **8 sex shops** for adult items.

---

## GPS System

**Source:** https://www.crazybobs.net/website/gps

- Displays arrows on the road showing suggested route to destination.
- Auto-activates for mission-based destinations.
- Save up to 5 custom locations.
- Supports km/meters or miles/feet display.

**Commands:** `/gps`, `/gpshide`, `/gpsresume`, `/gpsset` (`/gpssettings`), `/gpsclear` (`/gpscclr`), `/gpsdestination` (`/gpsdest`, `/gpsloc`).

---

## Quick Strings

**Source:** https://www.crazybobs.net/website/quick-strings

40+ dynamic variables that expand in chat/commands:

**Location/vehicle:** `$veh` (current vehicle), `$loc` (current location), `$loc2` (secondary location).
**Player targeting:** `$ply` (closest player), `$civ` (nearest civilian), `$cop` (closest officer), `$law` (closest law enforcement), `$sus` (closest suspect), `$war` (closest warrant), `$drv` (closest driver), `$med` (closest medic).
**Status:** `$cash` (cash on hand), `$hlth` (current health), `$jtime` (jail time remaining), `$time` (game time), `$day` (game day), `$hour` (game hour).
**Context:** `$fish` (last caught fish), `$chal` (current race challenge), `$esc` (jail escape tracking), `$hotel` (current room location).
**Custom PM slots:** `$qp1`, `$qp2`, `$qp3` for saved PM nicknames.
**Custom strings:** `/$set (1-3) (text)` saves up to 95-char canned lines (no other quickstrings in custom strings). Referenced as `$s1`, `$s2`, `$s3`.

Race variables: `$rdest`, `$rdist`, `$rtime`.

---

## Driving (Driver Upgrade)

**Source:** https://www.crazybobs.net/website/driving

- Any civilian can be an "On Duty Driver" via specific vehicles + commands. Added in Version 18.
- **Vehicle requirement:** Taxi, Limo, Bus, Helicopter, or other multi-seat vehicles equipped with the **Driver Upgrade**.
- **Driver Upgrade:** Purchasable at **Xoomer Gas Stations** throughout the game.
- **Key mechanic:** "Drivers no longer have car-jacking protection" — driver-role players lose vehicle theft immunity.

**Commands:**
- `/driver [fare]` (`/drv`) — activate Driver status
- `/drivercalls` (`/dcalls`) — show available passenger requests
- `/drivermsg [message]` (`/dm [message]`) — driver-specific chat channel

---

## Pimping

**Source:** https://www.crazybobs.net/website/pimping

- "Pimping is a way for both prostitutes and civilians to benefit. Pimps offer protection by paying for the medical fees of their prostitutes."
- Pimps receive a cut of earnings from stripping and sex work; pay medical coverage for workers.
- Pimps receive alerts when workers are attacked or insulted.
- One player can manage multiple prostitutes.
- Mutual agreement required; prostitutes can also initiate pimp offers to civilians.

**Commands:**
- `/pimp [nick/id] [percentage]` — establish pimp-prostitute relationship with specified cut
- `/pimpinfo` (`/pimplist`) — track workers and commission percentages
- `/pimpcall` — display prostitute activity logs
- `/pimpmsg` — broadcast to all affiliated workers
- `/pimpleave` / `/pimpleaveall` — end arrangements
- `/pimpaccept` (`/pimpyes`) — accept pimp offer
- `/pimprefuse` (`/pimpno`) — reject pimp offer
- `/pimphelp` — in-game guidelines

---

## Player Races (Wagered)

**Source:** https://www.crazybobs.net/website/player-races

- Players establish competitive driving events with wagered stakes.
- Initiator sets the destination and bet amount, then invites nearby players.
- Each participant pays the established wager; victor claims the entire pool.
- Two outcome modes: conclude when all finish, or end on first arrival.
- "The race owner decides when to start the 10 second countdown for race start."

**Commands:**
- `/race` — create or access race menu
- `/racemsg [msg]` — communicate with competitors
- `/racejoin` — accept a race invitation
- `/racequit` — exit active race
- `/racestart` — initiate countdown (owner only)
- `/raceinvite [nick/id]` — invite participants
- `/racekick [nick/id]` — remove players (owner only)
- `/racelist` — display race roster
- `/racehelp` — command reference

---

## Selling Vehicles

**Source:** https://www.crazybobs.net/website/selling-vehicles

- Any civilian can sell vehicles; **Car Jackers** have advantages (faster selling, can defeat locks).
- Vehicle damage directly reduces the payout amount.
- **Process:** Enter a sellable vehicle (message confirms eligibility) → drive to nearest Car Sell Crane → press Sub-Mission Key or `/sell` within the checkpoint.
- **Crane locations:** 3 (one per city).
- **Daily bonus vehicle:** First to sell it gets extra compensation (`/vsi` to check value).

**Commands:**
- `/vsi` (`/vsellinfo`) — displays current bonus vehicle value, cooldown time until next sale, car dealership discounts
- `/sell` — complete vehicle sale at crane

---

## House Maid

**Source:** https://www.crazybobs.net/website/house-maid

- Visit any owned house and select the "House Maid Job" option when available.
- Earn **$100** (approximately per second spent cleaning inside the house).
- "The dirtier the house is, the longer you will have to stay inside to clean it completely."
- **Completion reward:** $1,000 bonus + **1 score point**.
- Requirement: access to an owned house with the job option available.

---

## User Accounts

**Source:** https://www.crazybobs.net/website/user-accounts

- Register with `/register`; login box appears at Class Selection Screen.
- Do NOT register multiple accounts (all will be deleted).
- **City-specific data:** progress in one city does not transfer to others.
- On death: pay medical fees determined by market conditions and total wealth; higher wealth = larger fees.
- `/newlife` — resets money, houses, inventory, and days alive while **preserving score**. Must die afterward to complete the reset.

**Commands:** `/register`, `/login`, `/stats`, `/morestats`, `/total`, `/newlife`.

---

## What's New (Changelog Summary)

**Source:** https://www.crazybobs.net/website/whats-new
Changelog runs from Version 15 (December 2009) to Version 24.1 (October 2017).

**Version 24.1 (October 2017):**
- Hit contract revisions: cancel hits within 2 game hours for a 25% fee; increased survival bonuses; notifications when contracted hits are completed.
- Added `/missing` command for law enforcement.
- Expanded Money Rush locations.

**Version 24 (August 2017):**
- Air delivery missions: civilians transport drugs via aircraft.
- Random law enforcement missions throughout gameplay.
- Separated courier and smuggling missions — smuggling variants trigger wanted levels.
- Kidnapper mechanics redesigned: revenue through cash theft and ransom scenarios.
- Vehicle enhancements: armor systems, tire modifications, hydraulic upgrades.

**Version 23 (May 2015) — Major overhaul:**
- "Y" as default interaction key for ATMs and NPCs.
- Revamped visual display system.
- Redesigned police training.
- Added casinos in San Fierro and Los Santos.
- Casino gaming introduced: slot machines, blackjack, video poker.
- Robberies comprehensively reworked with variable timers and dynamic difficulty.
- Hotels introduced for room rentals with car storage.
- Player races added as core feature.

**Version 21 (August 2013):**
- Model preview menus for vehicle purchases.
- Street sweeper income.
- New missions: Bible Salesman, Combine Harvester variants.
- Fishing tournaments introduced.
- Expanded to 5 fish types with dynamic "fish area" mechanics.

**Version 20 (September 2012):**
- Required complete character resets (preserved cumulative stats).
- Redesigned OST (on-screen text) display system.
- Revamped group functionality with leaders and configurable join settings.
- Rebuilt item purchase system with individual player pricing.
- Lawsuit mechanics: players can pursue civil cases.
- RV-based drug cooking with skill progression.
- UPS delivery missions.

**Version 19 (August 2011):**
- Added airport robberies, illegal immigrant transport, sexual encounter missions.
- Fishing redesigned with interactive reeling mechanics.
- Church system: prayer, confession, donation.
- Jail appeal system with randomly selected juries.

**Version 16+:** GPS system pathfinding.
**Version 15 (December 2009):** Earliest documented version.

---

## Locations Detail

### Banks (7 total)

**Source:** https://www.crazybobs.net/website/bank

**Major banks:**
- Los Santos Bank: 620.250, -1258.000
- San Fierro Bank: -2021.000, 460.250
- Las Venturas Bank: 2470.000, 2258.750

**Additional banks:** Fort Carson, Las Barrancas, Palomino Creek (+ one more implied).

**Services at Bank:** Money deposits, Tax Refund requests, Bank Insurance Policy management, Stock Market share trading.

### Police Departments

**Source:** https://www.crazybobs.net/website/police-department

**Main (3):** Los Santos (Pershing Square), San Fierro (Downtown), Las Venturas (Roca Escalante).
**Secondary (4):** Dillimore, Angel Pine, Fort Carson, El Quebrados.

### Drug Refill Points (8 total)

**Source:** https://www.crazybobs.net/website/drug-refill-point

LS Idlewood, LS East Los Santos, SF Doherty, LV Randolph Industrial Estate, Dillimore, Angel Pine, Hashbury, Las Payayadas.

"The only place to get a drug bag."

### Bait Shops (6 total)

**Source:** https://www.crazybobs.net/website/bait-shop

LS Santa Maria Pier, LS Red County Montgomery, SF Angel Pine, SF Flint County, SF Pier 69, LV Tierra Robada.

### Hospitals (6+ total)

**Source:** https://www.crazybobs.net/website/hospital

LS Market, LS Jefferson, SF Santa Flora, LV (near airport), plus Montgomery, Angel Pine, Fort Carson, El Quebrados.

Services: condoms, chastity belt, adrenaline pill, and more. Skin change once per day.

### Hotels

**Source:** https://www.crazybobs.net/website/hotels

- Room rental paid daily; quit inside = spawn there next session (same city).
- Car save add-on available.
- Safe deposit boxes for limited cash storage.
- Room service available: order food/drinks in room.
- Social: invite players to your room.
- Max **10 hotel rooms per character per life**.
- Different room types have varying daily rates.
- Access methods vary: main entrance, lobby, or room checkpoint.

**Commands:** `/hotels` (list current rentals), `/hotel` (room menu when inside).

### Churches (8 total)

**Source:** https://www.crazybobs.net/website/church

LS Jefferson, LS Temple/Graveyard, LS Red County/Palomino Creek; SF Ocean Flats, SF Downtown; LV South East, LV Old Venturas Strip, LV South Las Venturas (plus one more).

Services: confess sins, donate money, pray, buy holy wine, start Flower Delivery Mission. Donating "may reward you in the future" (vague). Wanted level reduction mechanic confirmed but mechanics not detailed on page.

### Casinos

**Source:** https://www.crazybobs.net/website/casino

**Las Venturas (3):** Caligula's Palace, Four Dragons Casino, Redsands Casino.
"Gambling facilities but much more known for their daily occurring robberies."
Note: Version 23 added casinos in San Fierro and Los Santos, bringing total to 5.

### DM Stadium

**Source:** https://www.crazybobs.net/website/dm-stadium

- **Entry fee:** $5,000
- **Death cost:** $1,000 per death
- **Ammo:** 2500 ammo for all weapons on each new life
- **Protection:** Retain lives and life insurance; deaths don't affect real character.
- **Legal DM:** "legally DM or have a fun competition."
- **Locations:** 3 (one per city).
- **Command:** `/dmrec` to view competitive records.

### Regular Players Club

**Source:** https://www.crazybobs.net/website/regular-players-club

- **Locations:** Rodeo (LS), Financial district (SF), Emerald Isle (LV).
- "Inside the club some smoothing music brings you to rest."
- Access restricted to **Regular Players** (awarded based on score + playtime per city).
- Bot sells food, drinks, and other items inside.
- No-crime zone (safe zone) — from crime-cop-loop.md research.

### Car Sell Crane

**Source:** https://www.crazybobs.net/website/car-sell-crane

- 3 locations (one per city).
- Not all cars can be sold (message confirms eligibility on entry).
- Daily bonus vehicle: first to sell it gets extra compensation.
- Damage penalty: more damage = less money.
- `/vsi` to check bonus vehicle and cooldown.

### Gym

**Source:** https://www.crazybobs.net/website/gym

- 3 locations: Ganton (LS), Garcia (SF), Redsands East (LV).
- "3 options to choose from" for fighting styles.
- Price not stated.

### Other Robbable Locations

**Source:** https://www.crazybobs.net/website/other-locations

- **CrazyBob's Place (LS, Vinewood):** 12 minutes to reach hideout after opening safe.
- **SF Federal Mint:** 12 minutes to reach hideout; stay within checkpoint.
- **Jizzy's Pleasure Domes (SF):** Robbable; also used for parties.
- **Inside Track Betting (LS):** Bet on horses AND rob the establishment.

### Restaurants and Diners

**Source:** https://www.crazybobs.net/website/restaurants-and-diners

| Restaurant | Count | Notes |
|-----------|-------|-------|
| Well Stacked Pizza Co. | 9 | Pizzas, snacks, donuts |
| Burger Shot | 8 | Burgers |
| Cluckin' Bell | 10 | Chicken |
| Diner | 9 | Steaks, romantic setting |
| Donut Shop | 3 | **Cops only**; Market (LS), Juniper Hollow (SF), Fort Carson (LV) |

"Each food refills your health differently."

### Stores and Shops

**Source:** https://www.crazybobs.net/website/stores-and-shops

- **24/7:** 35 locations (LS, SF, LV, rural); sell pets, chainsaws, trouts, fishing permits, general items; can sell fish here.
- **Clothing chains:** ZIP (4), Binco (4), Prolaps (2), Sub Urban (3), Victim (3), Didier Sachs (1).
- **Barber Shops:** 6 locations (hair + rare hats).
- **Sex Shops:** 8 locations ("erotic goods" + exclusive merchandise).

### Airport

**Source:** https://www.crazybobs.net/website/airport

- 3 main airports: LS International, SF Easter Bay, LV Airport.
- Secondary air bases: Verdant Meadows, Area 69, Easter Basin Naval Station — house rare vehicles including Rustlers.
- Helicopter Challenge at Verdant Meadows.
- Bot-piloted aircraft for inter-city travel; buy Plane Ticket; wait at Red Checkpoint.
- Skydiving available.

### Ammunation

**Source:** https://www.crazybobs.net/website/ammunation

- 11 locations across SA.
- Cops: instant access.
- Civilians: must have Gun Permit (City Hall / PD / Church / Downtown LS Ammunation).
- Stocks "weapons that aren't sold by Cop/Civilian weapon dealers."

### Car Dealership

**Source:** https://www.crazybobs.net/website/car-dealership

6 locations: LS Rodeo, Coutt And Schtuz (Jefferson LS), Shody's Used Autos (Redsands East LV), Grotti's Autos (Royal Casino LV), Wang's Auto (Kings SF), Otto's Auto (Downtown SF). No vehicle list or prices published on page.

### Train Station

**Source:** https://www.crazybobs.net/website/train-station

5 stations: LS Unity Station (El Corona), LS Market Station; SF Cranberry Station (Doherty); LV Yellow Bell Station (Prickle Pine), LV Linden Station (East LV). Fast transport between cities; passenger and freight trains.

---

## Stock Market (CnR Markets)

**Source:** https://www.crazybobs.net/website/cnr-markets

- **Trading hours:** 07:00–19:00 in-game time. Prices frozen outside these hours.
- **21 stocks:** Government Bonds, House Market, Fish Market, Police Department, Hospital, 24/7 Corporation, Cluckin' Bell, Well Stacked Pizza Co., Burger Shot, Ammunation, Sex Shops, Bars and Dance Clubs, Diners, Strip Clubs, Bait Shops, Off Track Betting, Gambling Industry, Drug Industry, Transport Industry, Clothing Industry, Commercial Market (unavailable in current version).
- **Max shares:** 100,000 per company.
- **Market access:** City Hall or any Bank.
- "Markets fluctuate through real in game supply and demand factors (ex: purchases at 24/7 help the 24/7 market)."
- Basic principle: "buy when prices are low and sell when they peak."

**Commands:**
- `/markets` (`/market`) — display current market status
- `/shares` (`/stocks`) — show owned shares (select number for detail)
- `/sharessell` (`/stockssell`) — sell shares

---

## Gambling

**Source:** https://www.crazybobs.net/website/gambling

- **Casino games (3):** Slot Machines, Blackjack, Video Poker — press Y at any machine.
- **Locations:** Any Casino, Jizzy's Pleasure Domes, dance clubs, strip clubs, bars. Higher stakes at the High Roller Casino.
- **Horse Racing:** 5 horses; odds **2:1 to 10:1**; races every **2 game hours**; command: `/horsebet`.
- **Scratch'n'Win tickets:** 24/7 Store, Xoomer Gas Station, casinos, City Hall, RPC; varying odds displayed.
- **Dice:** `/dice (nick/id) (amount)` — in casinos or player-to-player.
- **Settings:** `/casinoset` — controls text size, audio, input modes (mouse or keyboard).

---

## Fishing

**Source:** https://www.crazybobs.net/website/fishing

- "Fishing is one of the best ways to earn your money!"
- `/fish` from a boat.
- **Fishing Rod** (Bait Shops) — reduces fishing time + raises catch chance. Lost on jail without valid permit.
- **Fishing Permit** (24/7, Bait Shop, Street Vendors, City Hall) — carry up to **50**. Fish legally near shore; increases speed.
- **Fish Sales Permit** — needed to sell fish to players.
- **Bonus Fish:** Each game day a special bonus fish announced; first to catch it gets "a large amount of cash."
- **Tournaments:** Random; every couple days; 04:00–20:00 only; types: biggest fish / most of a type / highest quantity; rewards scale with participant count; just be fishing to join.

**Commands:** `/fish`, `/fishmsg`, `/fishhelp`, `/fishsell`, `/fishsellall`, `/fishbuy`, `/fishthrow`, `/fishrelease`, `/fishinventory`, `/cooler`, `/fishprices`, `/fisheat`, `/fishslap`, `/fishtour`, `/fishrecords`, `/fishinfo`, `/rod`.

---

## Drug Planting & Hunting

**Source:** https://www.crazybobs.net/website/drug-planting-and-hunting

- Any civilian can grow drugs; Drug Dealers get perks (faster growth, player-to-player sales).
- **Seeds:** From 24/7, Drug Refill Point, Bait Shops, or Street Vendors. Carry max **10 seeds**.
- **Planting:** Remote locations; not near checkpoints, water, or other plants. Max **5 active plants**. Red checkpoint visible ~50m; cops spot from further; visible from air.
- **Growth:** Fully mature = ~**20 minutes server time** → up to **200 grams**. Drug Dealers grow faster.
- **Threats:** Deer eat plants, hippies attack, other players harvest, plant dies if too large.
- **Fertilizer** (`/fertilize`, `/fert`, `/bait`) — accelerates growth but attracts more deer.
- **Harvest** (`/harvest`, `/harv`) — enter checkpoint; +1 score per plant (must have >50g). Can harvest others' plants.
- **Offline:** Plants do not grow when player is offline.
- **Deer:** Killing deer provides health + bonuses. No hunting permit → **+6 wanted**. Hunting permits: carry up to **20** (per game-items page). Deer Traps (`/trap`) provide defense.
- **Selling:** Only at Drug Refill Points via `/drugsell` (minimum 200g for +1 score point).

**Commands:** `/plant`, `/harvest` (`/harv`), `/trap`, `/plantgps` (`/pgps`), `/fertilize` (`/fert`), `/druginfo` (`/di`).

---

## Housing

**Source:** https://www.crazybobs.net/website/houses

- **1,500+ purchasable properties** across San Andreas.
- **Visit first** (max 1 game hour, must be innocent) before buying.
- **Ownership:** Own **2**, co-own **8**, rent **10** simultaneously.
- **House loss:** Lost on character death OR if unvisited for **2 real-world weeks**.
- **Pricing factors:** Location, distance from city, car-save availability, purchase history, market supply/demand, CnR Prime Rate. **10% discount for full-time police officers**.
- **Co-owners:** Full house/storage access except sale price, rent price, pet management.
- **Rent:** Auto-paid by tenants each game day at rate set when signing; price locked from contract date; owner can evict anytime.
- **Property tax:** Auto-paid every **2 gamedays**. Two consecutive unpaid taxes → house seized.

**Storage caps:**
- Money (unlimited)
- **25 condoms**
- **25 flowers**
- **5,000g drugs**
- **25 seeds**
- **25 deer traps**
- **10 fish**
- **20 clothing items**

**Security:**
- **House Alarm** — alerts owners, co-owners, police on break-in.
- **Super Lock** — much harder to break into.
- **Pets** — attack intruders; varying damage; daily food cost.

**Access methods:** Door knocking, invites, keys (10-key holder limit), break-ins (`/houserob`), password entry.

**Commands:** `/house`, `/houselist` (`/hlist`), `/keylist` (`/keys`), `/coowner` (`/housecoowner`), `/housekeys` (`/hkeys`), `/houseinvite` (`/hinvite`), `/storage` (`/housestorage`), `/fishstorage` (`/housefishstorage`), `/houserob` (`/hrob`).

---

## How To Play

**Source:** https://www.crazybobs.net/website/how-play

**Player color system (verbatim):**
- **White** — Innocent civilian
- **Blue** — Law enforcement (lighter shade = higher rank)
- **Purple** — Police requesting backup
- **Yellow** — Suspect (ticketable)
- **Orange** — Wanted suspect (arrestable)
- **Green** — On-duty Driver (innocent)

**Law Enforcement role:** "Protect the city and its innocent civilians from crime." Ticket suspects; arrest wanted suspects. Earn "points and money for arrests, tickets and assists."

**Civilian role:** Select a skill providing specialized advantages without restricting other activities. "Skills don't limit what you can do, only enhance particular activities." Earn from "any completed game activity."

**G key** — enter passenger seat.

---

## FAQ

**Source:** https://www.crazybobs.net/website/faq

- Change skill at City Hall for a fee; change skin at Hospital for a fee; temporary cop at PD.
- "Each city has its own data" — earnings don't transfer between cities.
- Accounts always save when registered. "It is impossible for your account to be logged into unless you have leaked your password."
- No refunds issued for compromised/scammed accounts.
- Report rule breakers: `/complain [Player name/ID] [Reason]` — alerts all online admins.
- Regular Player status: "players that play the server on a regular basis" — score + playtime based.
- Admins: "players that have been on CnR for a long time." "DO NOT ask to become an admin."
- Mods: only Detailed Radar Mod permitted; skin mods prohibited.

---

## Donation Page

**Source:** https://www.crazybobs.net/website/donate

- **Payment method:** PayPal only.
- "Donations cover the cost of server and website hosting."
- **Verbatim (critical):** "Donating any amount of money does not entitle you to any special treatment on the server or anywhere related to the server."
- Contributions are non-refundable.
- **No perks listed** — the donation page itself grants nothing. (Note: donator perks like no-wealth-tax and `/vehcolor` at $100 are confirmed from the Fandom wiki and forum threads, just not from this page.)

---

## About CnR

**Source:** https://www.crazybobs.net/website/about-cnr

- "CnR was started by CrazyBob in March 2006 as a 32 player server for MTA" — then migrated to SA-MP.
- Ran for 18+ years until closed 31 Dec 2024.
- Two 200-player servers supported by player donations.
- **Dev team:** CrazyBob (server operation + main scripting), Mr.X (server operation + external applications), vick. (scripting).
- Copyright: 2006–2026 CrazyBob.

---

## Deltas vs Existing Research

Changes or additions relative to `docs/research/{crime-cop-loop,economy,jobs-progression,primary-sources}.md`:

### Additions (new facts not in prior docs)

1. **19 missions with individual sub-pages** — prior research had only mission names from the `/website/missions` hub. This sweep adds:
   - **Paperboy:** 4-minute real-time limit; $500/delivery + bonus; innocent civilian only; on-foot delivery.
   - **Lawn Mowing:** LV only (Prickle Pine golf course); 2 in-game hours; competitive checkpoint leaderboard.
   - **Tractor Mission:** CrazyBob's Farm; SF tractors only; 2 in-game hours; similar to Lawn Mowing.
   - **Flower Delivery:** 6 game-hour limit; `/flowers` command to deliver; both cop and civ; cash bonus; can start from Churches.
   - **Sexual Encounter:** sex shop start; 5 encounters in 5 game hours; Rapist/Prostitute skill boosts success.
   - **House Delivery:** 12 game-hour limit; some illegal cargo; civ only; any vehicle.
   - **Illegal Immigrant Transport:** wanted on completion (not pickup); night reduces cop visibility; innocent required.
   - **Airport Robbery:** box-collection mechanic; MMB or `/box` to place; more boxes = more money; truck/van/garbage truck required.
   - **Holdup Mission:** wraps 3 store holdups; $10,000 min per store; large completion bonus.
   - **Vehicle Theft total potential earnings confirmed verbatim:** $15,000/car + $75,000 completion bonus = $150,000 for all 5.

2. **Mechanic skill confirmed** as a separate 16th civilian skill on the official skills page (prior research noted it as potentially not confirmed). "Repairs vehicles and sells modifications"; commands `/mechanic`, `/prices`, `/calls`, `/vehrepair`.

3. **DM Stadium** — 2500 ammo for all weapons on each new life (not previously noted in detail).

4. **House Maid** confirmed mechanics: ~$100/sec cleaning + $1,000 completion bonus + **1 score point** (prior research from wiki had the $100 rate but the score point is new from the official page).

5. **Hotel system details** — safe deposit boxes, room service, car save add-on, max 10 rooms per life — new details not in prior research.

6. **Pimping system** — pimp pays medical fees for prostitutes; receives cut of sex/strip earnings; pimps get alerts when workers attacked; both prostitutes and civilians can initiate.

7. **Player Races (wagered)** — separate from Race Challenges; `/race`, `/racejoin`, `/raceinvite`, `/racestart`; stakes pooled; owner controls countdown.

8. **Quick Strings** — 40+ variables including `$hotel`, `$fish`, `$chal`, `$esc`, `$qp1/2/3`; `/$set (1-3)` for custom strings; race variables `$rdest`, `$rdist`, `$rtime`.

9. **Driving (Driver Upgrade):** Drivers explicitly lose car-jacking protection when on duty. Driver Upgrade bought at Xoomer Gas Stations.

10. **Drug Refill Point is the ONLY place to get a drug bag** (explicitly stated on the place page — not previously sourced this way).

11. **Church confirmed** as Flower Delivery mission start point AND wanted level reduction source (confession), though exact reduction mechanics not on page.

12. **Clothes carry limit:** 35 items (carry) / 5 worn simultaneously (site says 35 vs economy.md which did not specify; house storage confirmed as 20).

13. **Free phone** reward after 100 hours played per city (new — not in any prior doc).

14. **Special items** unlock through secret tasks (teased but not described).

15. **Hospitals:** Skin change available once per day (not previously noted from official site).

16. **Train system:** 5 stations named — LS Unity Station (El Corona), LS Market Station, SF Cranberry Station (Doherty), LV Yellow Bell Station (Prickle Pine), LV Linden Station.

17. **Changelog (whats-new) confirms Version 23 as the robbery rework** including individualized robbery settings and casino additions in SF/LS. Version 24 confirms smuggling vs courier separation and kidnapper redesign to "cash theft and ransom scenarios."

18. **Hit contract: cancel within 2 game hours for 25% fee** (v24.1) — not in prior docs.

19. **Donut Shops** are cops-only with 3 locations: Market (LS), Juniper Hollow (SF), Fort Carson (LV). Sells donuts, armor pickups, cake, pie. (Prior research noted `/donut` command but not the shop locations.)

20. **35 24/7 locations** confirmed (prior research cited 35 from Street_Vendor wiki — now confirmed on official stores-and-shops page).

21. **Ammunition: 10 Ammunation branches** (not 11 as counted from the location list — minor counting discrepancy in the source itself).

22. **6 car dealerships** named (new: Coutt And Schtuz, Shody's Used Autos, Grotti's Autos, Wang's Auto, Otto's Auto).

23. **8 sex shops** confirmed (new count from stores-and-shops page).

24. **Inside Track Betting** is also robbable (in addition to horse betting) — from other-locations page.

### Corrections / Conflicts with existing docs

1. **crimes-jail-sentence page says 15 random jurors** for appeal (verbatim: "Appeal sentence to 15 random jurors"). The Fandom wiki (Jail page) says 12. Prior research noted this conflict. **The official site says 15 — this is the authoritative number.** Design as a configurable constant but default to 15 per the site.

2. **Donation page explicitly states NO perks** are granted for donating. Prior research (jobs-progression.md) cited donator perks (no wealth tax, `/vehcolor` $100, name colour) from forum/wiki. Those perks were likely real in-game but were never officially documented on the site — the site's official stance is no perks. Implementation should treat the perks as confirmed from community sources but NOT from the official site.

3. **Drug Dealer carry capacity:** The official skills page does not state a number. Prior research cited both 2500g (from Fandom Drug_Dealer wiki) and 5000g (from ToRro/Score Guide with drug bag). The drug-planting-and-hunting official page also does not state the cap. Treat 2500g as the base (confirmed from Fandom) and 5000g as the cap with a drug bag (confirmed from Score Guide). The Drug Refill Point is confirmed as the **only place to get a drug bag**.

4. **The Sexual Encounter mission** was not previously documented with its exact requirements (5 encounters, 5 game hours, sex-shop start). Prior research mentions it as a mission title only.

5. **Flower Delivery** was previously undocumented in mechanics. Now confirmed: 6-game-hour limit, 5 players, `/flowers` command, available to both cops and civilians.

6. **Airport Robbery** involves box collection (not just a checkpoint robbery as implied in prior research). The MMB (`/box`) mechanic for placing/collecting boxes is specific to this robbery type.

7. **Paperboy, Lawn Mowing, Tractor Mission, House Delivery, Illegal Immigrant Transport** — all newly documented from official site; none appeared in prior research docs beyond mission list names.

8. **Crowbar activation confirmed as `/clotheswear`** (equip as a clothing item, not a weapon). Prior research correctly identified it as an item but the official game-items page now confirms the exact activation method.

---

*End of document. Fetched and transcribed 2026-07-14. All content copyright CrazyBob 2006–2026.*
