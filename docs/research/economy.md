# CB:CNR Economy Research

Research for the EXP Gaming Cops And Robbers revival, area: **ECONOMY**.
Modelled on **CrazyBob's Cops And Robbers** (crazybobs.net, closed 31 Dec 2024
after 18 years). This document extracts concrete, implementable mechanics.

**Legend**
- **[VERIFIED]** — sourced from CB's live site, the CB Fandom wiki (via Wayback),
  CB forums, or a real SA-MP CnR gamemode. URL cited inline.
- **[INFERRED]** — reconstruction / our design proposal where sources were thin.
- **[DESIGN]** — our own concrete proposal for a feature CB never fully exposed
  publicly (or that we want to extend), grounded in the closest classic pattern.

**Primary sources used**
- CB commands page — https://www.crazybobs.net/website/cnr-commands
- CB how-to-play — https://www.crazybobs.net/website/how-play
- CB game-items — https://www.crazybobs.net/website/game-items
- CB houses — https://www.crazybobs.net/website/houses
- CB cnr-markets (stock market) — https://www.crazybobs.net/website/cnr-markets
- CB fishing — https://www.crazybobs.net/website/fishing
- CB gambling — https://www.crazybobs.net/website/gambling
- CB stores-and-shops — https://www.crazybobs.net/website/stores-and-shops
- CB Fandom wiki (via web.archive.org): Fishing, Street_Vendor, Taxes,
  Drug_Planting_and_Hunting, House_Maid, All_Making_Money_Methods, CrazyBob's_Farm
- Reference gamemode: `PatrickGTR/sf-cnr` (irresistible CnR, dev "Lorenc") —
  https://github.com/PatrickGTR/sf-cnr — used for the concrete **stock market
  pricing model** and shop item table (a modern open CnR, not CB's own source).

> Prices below are CB values sampled ~2019 by wiki editors and **fluctuate with
> the market/prime rate** — treat them as baselines, not constants.

---

## 1. Money Flow

### 1.1 How you earn (the loop) [VERIFIED]
Every completed activity grants **points (score) + money**. Civilians earn from
missions, robberies, selling vehicles/guns/drugs/items/fish, services, fishing,
farming, house-maid, gambling. Police earn from arrests, tickets, and assists.
(how-play, All_Making_Money_Methods). Money can live in three places: **pocket
(cash)**, **bank**, or **house storage** (how-play).

### 1.2 Paycheck / income interval [INFERRED / DESIGN]
CB does not publish a fixed "paycheck." Instead income is **event-driven** (you
get paid per action) plus **bank interest** every 2 game days (see 1.3). There is
no evidence of a flat GTA-style hourly payday.
- **[DESIGN]** For our revival: keep event-driven payouts as primary. Optionally
  add a small **activity-scaled paycheck** on the game clock only for *active*
  players (anti-AFK), e.g. `$X * min(streak, cap)` each game hour — but the CB-
  faithful choice is bank interest, not a payday. Our legacy `pMoney` var already
  exists in `gamemodes/CnR/players/player_vars.inc`; add `pBankMoney`.

### 1.3 Bank accounts [VERIFIED]
- Commands: `/deposit` `/dep`, `/withdraw` `/wit`, `/moneyinfo` `/bankinfo` `/$i`,
  `/atm`. Cash transfer: `/givecash /gc /sendcash /sendmoney /givemoney /$`
  (must be close enough). (commands page)
- Deposit/withdraw available at **Bank, City Hall, or Regular Players Club**
  (commands page); ATMs scattered in the world.
- **Interest: ~5-15% of banked money every 2 game days** — and interest works
  best if you *don't* pay taxes (i.e. tax bites into it). (forum: "What is bank
  interest?", https://forums.crazybobs.net/viewtopic.php?f=12&t=70923)
- **Bank insurance**: if you hold **$10,000+ in the bank** you can buy bank
  insurance for **$25,000**; if the bank is robbed, insurance returns **75%** of
  what was stolen from your account. (bank page / forum)
- **Tax refund**: request at the bank; returns some income tax you paid, minus a
  fee. (bank page)
- Reference impl caps a single withdraw at 99,999,999 and supports `MAX`/`ALL`
  quick-deposit keywords (`sf-cnr .../player/bank.pwn`) — good UX to copy.

### 1.4 Taxes (the drain that balances the economy) [VERIFIED]
Four recurrent taxes, **all charged every 2 game days** (wiki: Taxes):
| Tax | Rate | Notes |
|-----|------|-------|
| Wealth Tax | **12%** | On total wealth (bank + cash + stocks) once it exceeds **$10,000,000**. NOT refundable. IRL donors ($10+) are immune. |
| Income Tax | **7%** | On all money earned since last tax bill. Refundable via tax refund. |
| Storage Tax | **6%** | On house-storage money. Half of wealth tax, and storage money is *excluded* from wealth tax — so storing money is the tax-dodge. |
| Property Tax | — | On owned houses; miss it twice in a row → lose a house to cover it. |

**Prime Rate** governs the whole economy: tax rates and item/house/stock prices
all move with it; **higher prime rate = lower goods prices** and higher house
prices. (Taxes wiki; houses page; forum). This is our master multiplier.

### 1.5 Cash vs bank on death / arrest [VERIFIED + INFERRED]
- On death you pay **Medical Fees** (a large chunk of money) and can **lose your
  weapons** — unless you hold **Life Insurance** (see 3.4). (game-items)
- **[INFERRED]** Cash-in-hand is the vulnerable pool (medical fees, robbery,
  jail). Banked money is safe from medical fees/pickpockets but exposed to bank
  robbery (mitigated by bank insurance) and wealth tax. House storage is safest
  vs tax but lootable in a house robbery.
- **[DESIGN]** Recommended rule set: medical fee = % of *cash* (capped); Life
  Insurance waives it and preserves weapons; banked/stored money untouched by
  death. Jail can seize a bail-sized cash cut. This matches every signal above.

### 1.6 Insurance [VERIFIED]
- **Life Insurance** — bought at **City Halls / Street Vendors** (~**$75,000** at
  Hospital sample). Prevents the large medical-fee money loss on death and
  **saves all your weapons**. Hold up to **3 policies** at once. Price is
  market-driven and **rises the more often you die**. (game-items, Street_Vendor)
- **Health Insurance** — bought at **Hospitals / Medics**. Gives **free medical
  items** (heals, cures, condoms) for **5 game days**. Check with `/ii`
  (`/insuranceinfo`). (game-items, commands page)
- **Bank Insurance** — see 1.3.

---

## 2. Shops

CB shop prices **vary with the items market / prime rate**; cheaper items barely
move, expensive items swing a lot. (Street_Vendor wiki). Buy menu: `/buy`
`/purchase` `/purch` opens the nearest seller's menu. Vendors refill via
`/refill /rf`. (commands page)

### 2.1 24/7 [VERIFIED — Street_Vendor wiki sample prices]
Sells pets, chainsaws, trouts, fishing permits, and general items; you can also
**sell fish here** (35 locations across all cities — stores-and-shops).

| Item | Price | Effect |
|------|-------|--------|
| Sprunk / Cola | $23 | drink (thirst/small heal) |
| Condom | $400 | STD protection |
| Drug Plant Seed | $1,200 (5-pack $4,000) | grow drugs |
| Fishing Rod | $20,000 | faster fishing, higher catch chance |
| Body Armor | $800 | armor/vest |
| Baseball Bat | $800 | melee |
| Shovel | $800 | tool |
| Flowers | $800 | low-dmg melee / gift |
| Parachute | $2,400 | skydiving safety |
| Pistol (100 ammo) | $4,000 | weapon (needs Gun Permit as civ) |
| Shotgun (100 ammo) | $8,000 | weapon (needs Gun Permit as civ) |

### 2.2 Ammunation / weapon dealer [VERIFIED]
`/weapons /weapon /weap /weaponsell /wsell /ws` — offers weapons to a player
(Arms Dealer skill) or calls an arms dealer. Player gun-dealers **can** sell to
others; selling arms without a **Weapon Sales Permit** gives a wanted level.

| Ammunation item | Price |
|-----------------|-------|
| Body Armor | $800 |
| Gun Permit | $12,000 |
| Baseball Bat | $800 |
| Chainsaw | $20,000 |
| Pistol (50 / 100 ammo) | $2,400 / $4,000 |
| Shotgun (50 / 100 ammo) | $6,000 / $8,000 |
| Weapon Sales Permit | $40,000 |

Ammunation locations: Downtown LS Ammunation etc. **Gun Permit** required for a
civilian to buy weapons; sold at City Halls, Police Departments, Churches, and
the Downtown LS Ammunation. (game-items)

### 2.3 Other shops [VERIFIED — Street_Vendor wiki]
- **Bait Shop**: Moonshine $350, Seeds $1,200/5-pack $4,000, **Fish Cooler
  $20,000**, Trout $400, Turtle $25,500, **Fishing Rod $20,000**, Crowbar
  $40,000, Shovel $800, Pistol100 $4,000, Shotgun100 $8,000, **Fish Sales Permit
  $20,000**, Fishing Permit 5/10/25 fish = $8,000/$12,000/$20,000, Hunting Permit
  5/10 deer = $4,000/$6,000.
- **Hospital**: Condom $400 (5-pack $1,200), **Life Insurance $75,000**, Chastity
  Belt $16,000, **Adrenaline Pill $24,000**, Cane $800, Vigora 14g $13,000,
  Fancy White Powder 28g $21,666.
- **Sex Shop**: Condom $400, 3/5 packs $800/$1,200, Chastity Belt $16,000,
  dildos/vibrators $1,200 each, Flowers $800, Vigora 14g $13,000, plus STDs
  found nowhere else. (stores-and-shops)
- **Drug Refill Point**: Seeds, Shovel $800, Shotgun100 $8,000, **Drug Sales
  Permit $40,000**; you **sell freshly grown drugs only here** (`/drugsell`).
- **Truck Stop (Xoomer)**: Vehicle Repair Kit $12,000, Crowbar $40,000,
  Parachute $2,400, **Free Vehicle Credit $8,666** (only from Street Vendor /
  Mechanic / here).
- **Bar / Dance Club / Strip Club**: DM Juice $117, Beer $117, Whiskey $224-233,
  Moonshine $350-$16,000, Pool Cue $800, Fancy White Powder 28g $21,666.
- **Gym**: DM Juice $106, Body Armor $725, Golf Club/Bat/Cane $725, Pistol100
  $3,625.
- **Restaurants/Diners**: Sprunk $21, Beer $106; food delivery via `/food`
  `/pizza`, drinks via `/beer /drinks /drink /booze` (Food Delivery skill).

### 2.4 Consumables / effect items [VERIFIED]
Sprunk/food = thirst/hunger/small heal; Condom = STD protection; Chastity Belt
= STD immunity; Adrenaline Pill; Parachute = safe skydiving; Fishing Rod +
Cooler; Free Vehicle Credit; Vehicle Repair Kit / ACME Insta-Fix; Crowbar;
Public Transit Card; Gun/Fish/Drug/Weapon Sales Permits; Fishing/Hunting Permits.
(game-items, Street_Vendor)

### 2.5 Clothes shops [VERIFIED]
Six chains: **ZIP, Binco, Prolaps, Sub Urban, Victim, Didier Sachs** — buy/sell
**clothes, watches, hats**. **Barber Shops** sell hair + rare hats. Player-to-
player selling: `/clothesprice` sets prices, `/clothessell` advertises.
(stores-and-shops, commands page)
- **Hook to our `pSkinsSelected` system**: buying a skin/outfit in a clothes shop
  should write into the player's `pSkinsSelected[MAX_PEDS]` (see
  `players/player_vars.inc`); each shop chain unlocks a skin subset; a purchase
  = unlocking + equipping that ped/outfit. **[DESIGN]**

---

## 3. Housing

### 3.1 Buying [VERIFIED — houses page]
- **1,500+ purchasable properties** across San Andreas. **Visit first** (max 1
  game hour, must be innocent) before buying.
- **Bank sale** = current market value; **Police get a 10% discount**.
  **Owner sale** = seller sets the price.
- Price factors: location (populated = pricier), distance from current city,
  car-save availability, **purchase frequency (each sale raises the value)**,
  housing supply/demand, and **CnR Prime Rate** (higher = pricier houses).
- **[INFERRED]** No single published price table; ranges from a few tens of
  thousands (rural shacks) up to millions (Mulholland / Whitewood estates).
  `/buyhouse` at the house pickup is the classic entry point (our PLANNED note).

### 3.2 Ownership limits [VERIFIED]
Own **2 houses**, co-own **8**, rent **10** at once. A house is yours until you
**lose your life** (die permanently?) — and an unvisited house is lost after
**2 weeks real-time**.

### 3.3 Ownership perks [VERIFIED]
- **Spawn**: quit inside your own house → respawn there on rejoin.
- **Storage**: money, drugs, condoms, flowers, seeds, traps, **fish**, and up to
  **20 clothing pieces**. Storing money dodges wealth tax (only 6% storage tax).
- **Car Save**: park in driveway before quitting; wait **6+ game hours** before
  rejoining to reclaim it.
- **Legal protections**: some crimes (urination, farting) are legal in your own
  house; you may legally kill wanted criminals inside.
- **House pets**: cost pet food, auto-deducted from pocket cash daily.
- **House Maid** (`/maid`-style mini-job): enter a player-owned house and clean;
  you earn **$100 repeatedly up to a max** per cleaning session. (House_Maid wiki)

### 3.4 Selling / rent / robbery [VERIFIED]
- **Sell** to bank (below market) or list "For Sale By Owner" at your price.
- **Rent**: owner sets rent per gameday; **tenants auto-pay each gameday** they
  pass in-server; contract price is locked at signup; owner can evict anytime.
  Player commands: `/rent /rents /rentlist /rlist /rentl`. **Hotels**: `/hotel`,
  `/hotels`. House menu: `/house`; owned list: `/houses /houselist /hlist`.
- **House Robbery** (`/houserob /hsrob /hrob`): break in while owner online;
  steal storage contents; alarm/owner-present pings police.

### 3.5 Business ownership [VERIFIED for existence; INFERRED for income]
CB has businesses tied to the **stock market** (see §7): 24/7 Corporation,
Cluckin' Bell, Well Stacked Pizza, Burger Shot, Ammunation, Sex Shops, Bars &
Dance Clubs, Diners, Strip Clubs, Bait Shops, Off Track Betting, Gambling,
Drug/Transport/Clothing industries. **[INFERRED]** Player-facing business income
in CB is expressed as **owning shares** of these companies (dividends), rather
than physical business plots.
- **[DESIGN — from `sf-cnr`]** For a physical-business option: business types
  Weed/Meth/Coke/Weapon; each has a **business bank** with `/withdraw`; product
  is exported by vehicles for a payout `export_value * (MAX_DROPS - exported) *
  0.25` (0.3 if the finisher is a cop); every payout feeds the matching stock
  (`business.pwn`). Members up to 8 per business.

---

## 4. Lotto

### 4.1 Number lotto [VERIFIED — commands page]
- `/lotto` — "Play the lottery which draws at **18:00 every game day**. Select a
  **number 1-125**."
- **[VERIFIED]** Real wins were large — YouTube evidence of a **~9.8 million**
  jackpot payout (search result: "Lotto Win 1 | 9.8 Million").
- **[INFERRED]** Ticket cost is small/flat and the **jackpot grows from unclaimed
  draws + ticket sales** (classic rollover). If nobody picked the drawn number,
  jackpot rolls to the next game day.

### 4.2 Draw ties to the game clock — our legacy bug note [VERIFIED context]
CB draws exactly at game time **18:00**. Our PLANNED-FEATURES notes a
"gamemodeclock lottery timer bug" — because the draw is bound to the in-game
clock, not a real-time timer. **[DESIGN]** Implement the draw inside the
game-clock tick (fire once when the hour transitions to 18, guard against
double-fire and against skipped hours when the clock jumps). Scripter command
`/alotto` exists (PLANNED) to force/seed a draw for testing.

### 4.3 Scratch'n'Win + other gambling [VERIFIED — gambling page]
- **Scratch'n'Win** tickets at 24/7, gas stations, casinos, City Hall, RP Club;
  each ticket lists its own odds + payout beside the price.
- **Horse betting** `/horsebet`: 5 horses, odds **2:1 → 10:1**, race **every 2
  game hours** (Inside Track Betting / Woozie's / machines).
- **Casino games** (press **Y**): Slots, Blackjack, Video Poker; `/casinosettings`
  `/casinoset`. **Dice**: `/dice (nick/id) (amount)`.

### 4.4 Proposed lotto model [DESIGN]
```
TICKET_PRICE      = 5,000            // flat, market-scaled by prime rate
NUMBER_RANGE      = 1..125
DRAW_TIME         = game hour == 18
JACKPOT_BASE      = 1,000,000
jackpot += sum(ticket_prices_this_day) + rollover_from_unclaimed
On draw: pick winning = random(1..125).
  winners = players whose ticket number == winning
  if winners: split jackpot equally, announce each payout in chat; reset to BASE
  else: rollover jackpot to next game day
```

---

## 5. Money Bag

### 5.1 CB behaviour [VERIFIED — search summary + commands page]
- `/moneybag` on the commands page is listed as **"Drop a money bag"** — i.e. a
  player who is *carrying* a money bag can drop it. (commands page)
- CB's money-bag/"money rush" is a **hidden lost-cash event**: the Mafia loses a
  bag at a **random server-chosen location**; not every game day (once or a few
  times per game week). Use **`/mrush`** to see if one is active and where.
  Finder gets **+1 point + a cash amount that is hidden until picked up**; the
  amount is then **announced in chat for everyone**. Beware — players kill the
  finder to steal it. (Money_Rush search summary)

### 5.2 Our commands [DESIGN — maps to legacy]
- **`/moneybag`** (player) — locate/hunt the active hidden bag. Give an
  approximate zone hint (see §5.3), or, if the player holds a bag, drop it.
- **`/addmoneybag`** (scripter) — spawn a money bag at a chosen/random location
  with a set or randomized reward for testing/events.

### 5.3 Proposed hidden-bag model [DESIGN]
```
Event start (random, e.g. 1 in N game-hours or admin-triggered):
  pick random point from a curated location list
  reward = random(REWARD_MIN=250,000 .. REWARD_MAX=2,000,000)   // hidden
  announce globally: "A money bag has been dropped somewhere in <zone>!"
Hints: /moneybag tells the player the ZONE name (coarse), not exact coords.
       Optionally warmer/colder distance hints as they approach.
Pickup: first player to reach the pickup gets reward + 1 score; announce amount.
        Bag can be dropped (/moneybag while carrying) and re-picked (steal loop).
Timeout: if unclaimed after T game-hours, bag despawns / relocates.
```

---

## 6. Money Rush

### 6.1 CB analog [VERIFIED]
The Money Rush **is** CB's lost-Mafia-money event (see §5.1). `/moneyrush /mrush
/mr` "Displays Money Rush (Lost Mafia Money) Information." (commands page). So in
CB, money bag == money rush. Our project splits them into two features:
`/moneybag` (single hidden bag hunt) and `/moneyrush` (a rain/multi-pickup event).

### 6.2 Proposed money-rush model [DESIGN — since we want a distinct rain event]
Owner command **`/startmoneyrush`** triggers a timed event; players join with
**`/moneyrush`**.
```
On /startmoneyrush:
  choose an arena/zone; announce globally with countdown.
  For DURATION (e.g. 3 real minutes) spawn CASH_PICKUPS at random points in-zone
  on a short interval; each pickup = random(1,000 .. 25,000).
  Players run around grabbing pickups -> instant cash; PvP allowed (chaos).
  Track a per-event leaderboard; announce top 3 at the end.
Cooldown before another rush can start. Scripter/admin can force-end.
```
This is the classic "money rain" SA-MP event; CB itself leaned on the single
hidden bag, so this is our extension in the CB spirit.

---

## 7. Stock Market

**CB absolutely had one** — this is a headline CnR feature, not a bolt-on.

### 7.1 CB behaviour [VERIFIED — cnr-markets page]
- Commands: `/markets` `/market` (current market + interest rates), `/shares`
  `/stocks` (your holdings; pick a number for detail), `/sharessell`
  `/stockssell` (sell). Accessed at **City Hall or any Bank**.
- **Trading hours: game time 07:00-19:00**; prices **frozen** outside that window.
- **21 stocks**: Government Bonds, House Market, Fish Market, Police Department,
  Hospital, 24/7 Corporation, Cluckin' Bell, Well Stacked Pizza, Burger Shot,
  Ammunation, Sex Shops, Bars & Dance Clubs, Diners, Strip Clubs, Bait Shops,
  Off Track Betting, Gambling, Drug, Transport, Clothing, Commercial (n/a).
- Own up to **100,000 shares** per company. **"Buy low, sell high."** Prices move
  on **real in-game supply & demand** — e.g. buying at 24/7 stores pushes the
  24/7 stock. A **Prime Rate** feeds the whole thing.

**This confirms our headline goal**: market prices are *driven by* player economic
activity and in turn *scale* item/weapon/house/fish prices via the prime rate.

### 7.2 Concrete, implementable model [DESIGN — verified pattern from `sf-cnr`]
The `sf-cnr` gamemode (`.../features/stocks/stocks.pwn`) implements the exact
"activity feeds a pool, pool sets the price" mechanic. Core pieces:

**Per-stock config:** `MAX_SHARES`, `POOL_FACTOR`, `PRICE_FACTOR`, `IPO_SHARES`,
`IPO_PRICE`, `MAX_PRICE`. **Report period = 1 day**, keep **last 30 periods** for
history/graphs.

**Price formula (the heart of it):**
```
new_price = (POOL / POOL_FACTOR) * PRICE_FACTOR + PRICE_FLOOR      // PRICE_FLOOR=1.0
new_price *= floatpower(0.5, MAX_SHARES / IPO_SHARES - 1.0)        // dilution scaling
```
`POOL` is a running reservoir of net economic activity for that company.

**Activity feeds the pool** — every relevant transaction calls:
```
StockMarket_UpdateEarnings(stockid, cashAmount, factor):
    POOL += cashAmount * factor      // clamp POOL >= 0
```
Real call-sites in `sf-cnr` (copy this wiring):
| Action | Stock | factor |
|--------|-------|--------|
| Buy at 24/7 / Supa Save | SUPA_SAVE | 0.25 |
| Vehicle purchase | VEHICLE_DEALERSHIP | 0.01 |
| Vehicle mods | VEHICLE_DEALERSHIP | 0.025 |
| Car jacking sold | VEHICLE_DEALERSHIP | 0.25 |
| Arrest / bail / tickets | GOVERNMENT | 0.1 |
| Firefighter payout | GOVERNMENT | 0.15 |
| Ammunation (gang facility) | AMMUNATION | 0.25 |
| Trucking delivery | TRUCKING | 1.0 |
| Mining | MINING | 0.5 |
| Pilot delivery | AVIATION | (dividend alloc) |
| Meth cook | CLUCKIN_BELL | 0.3 |
| House burglary fence | PAWN_STORE | 1.0 |
| Blackjack win/loss | CASINO | 0.05 (+/-) |

**Trading:** buy/sell at market price with a **1% trading fee**
(`STOCK_MARKET_TRADING_FEE`). Buying draws from `AVAILABLE_SHARES`; selling posts
a sell order at last price. **Dividends**: pay each shareholder
`pool * (shares_owned / total_shares)` on the report tick. **Bankruptcy**: if a
stock drops **-80%**, holders lose **20%** and it resets.

**Market tick:** run on the **game clock** each report period (game day). On tick:
shift history down, compute `new_price` from the accumulated `POOL`, reset the
day's pool, pay dividends. This is the natural home for our game-clock timer
(same clock the lotto uses).

### 7.3 Prices affecting items / weapons / houses / fish [DESIGN — our headline ask]
Expose a global **market multiplier per domain** derived from stock price vs its
IPO price, e.g.:
```
mult(domain) = clamp( stockPrice(domain) / IPO_PRICE(domain), 0.5, 2.0 )
shopPrice(item)  = base * mult(item.stock)          // 24/7 -> SUPA_SAVE, gun -> AMMUNATION
weaponPrice      = base * mult(AMMUNATION)
housePrice       = base * mult(HOUSE_MARKET) * primeRateFactor
fishSellPrice    = base * mult(FISH_MARKET)         // ties fishing to the market (see §8)
```
Layer the **Prime Rate** on top as CB does (higher prime = lower goods, higher
houses). This yields exactly the "one economy where everything moves together"
behaviour CB was known for.

---

## 8. Fishing

### 8.1 CB mechanics [VERIFIED — fishing page + Fishing wiki]
- **`/fish`** while on a boat. "One of the most lucrative ways players earn money."
- **Fishing Rod** (clothing item, $20,000 at Bait Shops / 24/7) — **reduces
  fishing time + increases catch chance**. Rods are **removed when jailed** if you
  lack a valid fishing permit.
- **Fishing Permit** (24/7, Bait Shop, Street Vendor, City Hall) — hold up to
  **50**, **1 fish per permit**; lets you **fish faster** and avoids a wanted
  level for fishing near cops. Sold in bundles: 5/10/25 fish = $8,000/$12,000/
  $20,000 (Bait Shop).
- **Fish Cooler** ($20,000) — lets you carry **more fish** but costs **$9,500/day
  tax**; you lose it if you can't pay. (Fishing wiki)
- **Bonus fish**: announced each game day at **05:00**; first to catch it gets a
  **large cash reward**. (Fishing wiki)
- **Fishing tournaments**: random, ~once every couple days, game time **04:00-
  20:00**; types = biggest fish / most of a type / most fish; reward scales with
  participant count; just be fishing to join. (Fishing wiki)
- **Fishing boats** (buy around SA): Coastguard, Dingy, Jetmax, Marquis, Reefer,
  Speeder, Squallo, Tropic. (Fishing wiki)
- **Dangerous creatures** exist while fishing ("Killer Fish" wiki page). (Fishing)

### 8.2 Selling fish [VERIFIED]
- To shops: **`/fishsellall`** at a **24/7 or Bait Shop**.
- To players (needs **Fish Sales Permit**, $20,000): **`/fishsell /fsell`** to
  offer, **`/fishprices /fp`** to set your price, **`/fishbuy`**, **`/fishgive`**.
- **Price = per-pound value driven by the Fish Market**. Wiki example: a **Baby
  Seal over 80 Lb** at market ~**100** (`/market`) sells for **~$500,000** — so
  catch **weight × rarity × market rate** = payout. (fishing forum guide;
  Fishing wiki). Trout baseline ~$400 (Street_Vendor sample).

### 8.3 Fish price → market hook [DESIGN]
Fishing already reads the **Fish Market** stock (`/market`). Wire it both ways:
```
fishSellPrice = weightLb * rarityMultiplier * fishMarketRate
// selling fish should also FEED the market so heavy selling depresses it:
StockMarket_UpdateEarnings(E_STOCK_FISH_MARKET, saleValue, 0.15)  // supply pushes price
```
This makes fish a genuine market commodity: scarcity raises price, dumping lowers
it — the CB "supply & demand" promise.

---

## 9. Farming

### 9.1 CB drug planting = the farming loop [VERIFIED — Drug_Planting_and_Hunting]
CB's "farming" is **growing drugs** (any civilian; Drug Dealers get bonuses):
- **Seeds** from 24/7 / Drug Refill Point / Bait Shop / Street Vendor
  ($1,200 each, 5 for $4,000). Carry up to **10 seeds**.
- **`/plant`** at a valid spot (not near checkpoints, water, or other plants).
  Plant = a red checkpoint visible ~50 m; grows more visible over time; **cops &
  aircraft spot crops from further**. Max **5 plants** at once.
- Growth: a fully mature plant takes **~20 min real-time** and yields up to
  **200 grams**. **`/fertilize /fert`** grows faster but **attracts deer**.
  **`/plantgps /pgps`** finds/monitors plants. **`/druginfo /di`** shows inventory.
- **Threats**: deer eat plants, hippies attack, other players harvest, or a plant
  dies if it grows too large. Defend with **Deer Traps** (`/trap`) or by killing
  deer — but killing deer without a **Hunting Permit** (5/10 deer = $4,000/$6,000)
  gives a wanted level. Killing deer refills health + a bonus.
- **`/harvest /harv`** to collect. Plants/seeds **save on quit** (registered
  players) but **do not grow while offline**.
- **Sell** freshly grown drugs **only at Drug Refill Points** via **`/drugsell
  /ds`** ($/gram). Player-to-player: `/givedrugs`, `/drugs` (dealer offers).

### 9.2 CrazyBob's Farm — the crop job [VERIFIED]
There is a literal farm at **Flint County** (south of Easter Bay Intl) with
**[BOT]CrazyFarmer** offering **Farmer Missions**: **Crop Harvesting** and
**Field Plowing** (legit paid jobs), alongside other missions like Lawn Mowing.
(CrazyBob's_Farm wiki, mission list). Concrete payouts weren't archived.

### 9.3 Proposed farming design [DESIGN]
Mirror CB exactly and add a legit crop job:
- **Illegal (weed/drug) farming**: seeds → `/plant` → grow timer (~20 min) →
  `/harvest` → sell at refill points; deer/theft threats; hunting permits;
  fertilizer trade-off. Feeds the **Drug** stock via
  `StockMarket_UpdateEarnings(E_STOCK_DRUG, saleValue, factor)`.
- **Legit farming (CrazyBob's Farm job)**: take a mission from the farmer BOT at
  Flint County; drive a harvester/plow along a route of checkpoints; get paid per
  field completed (e.g. $500-$2,000 scaled by field size + a completion bonus).
  Feeds a **Transport/Agriculture** stock. Classic SA-MP "harvest job" pattern —
  a safe, low-yield income floor for new players, contrasting the risky drug farm.

---

## 10. Quick command map (economy) [VERIFIED — commands page]
Bank: `/deposit /dep`, `/withdraw /wit`, `/moneyinfo /bankinfo /$i`, `/atm`,
`/givecash /gc /$`. Shops: `/buy /purchase`, `/refill /rf`, `/prices`. Weapons:
`/weapons /ws`. Items: `/items /is`, food `/food /pizza`, drinks `/beer /booze`.
Drugs: `/drugsell /ds`, `/plant`, `/harvest /harv`, `/fertilize /fert`, `/trap`,
`/plantgps /pgps`, `/druginfo /di`. Fishing: `/fish`, `/fishsellall`,
`/fishsell /fsell`, `/fishprices /fp`, `/fishbuy`, `/fishgive`. Housing:
`/house`, `/houses /hlist`, `/rent /rlist`, `/hotel(s)`, `/houserob /hrob`.
Lotto/gambling: `/lotto`, `/horsebet`, `/dice`, `/casinosettings`. Stocks:
`/markets /market`, `/shares /stocks`, `/sharessell /stockssell`. Insurance:
`/insuranceinfo /ii`. Events: `/moneybag`, `/moneyrush /mrush /mr`. Legal fees:
`/bail /pb`, `/payticket /pay`, `/bribe /br`, `/tip`.

---

## 11. Implementation notes for our codebase
- `player_vars.inc` currently has only `pMoney`. Add: `pBankMoney`,
  `pHouseStorageMoney`, `pStocks[MAX_STOCKS]`, permit counters
  (`pFishPermits`, `pHuntPermits`, `pGunPermit`), insurance flags/expiry, plus
  `pLottoNumber` and `pTaxOwed`.
- Bind **lotto draw (18:00)** and the **stock-market report tick** to the same
  game-clock hour-transition handler; guard against double-fire / skipped hours
  (this is the "gamemodeclock lottery timer bug" in PLANNED-FEATURES).
- Central **prime-rate** value + per-domain **market multipliers** should be the
  single source of truth that shop/weapon/house/fish pricing all read.
- SQL: `scriptfiles/cnr.sql` already exists — extend with tables for
  `bank_accounts`, `stocks`/`stock_reports`, `houses`, `lotto`, and permits.
