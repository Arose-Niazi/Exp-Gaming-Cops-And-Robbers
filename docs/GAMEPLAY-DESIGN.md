# EXP Gaming Cops And Robbers — Gameplay Design Spec

> **What this is:** the implementable design that coding agents build from. Every
> feature/command in `docs/PLANNED-FEATURES.md` gets a concrete mechanic here:
> syntax, params, prices, cooldowns, wanted effects, rank gate, DB columns, and
> the **exact module file** it lands in. Grounded in `docs/research/*.md`
> (cited as e.g. *[crime-cop-loop §4.2]*, *[economy §7.2]*, *[jobs §3.1]*,
> *[primary §13]*). Where research was thin, choices are marked **[OUR DESIGN]**.
>
> **Money scale note.** CB cash figures are *ratios/intent*, not literal targets
> *[economy §preamble]*. Our starting bank is `$50,000` (`OnPlayerRegister`).
> All prices below are our tuned baseline; the owner ratifies them (§Open
> Questions).
>
> **Conventions this spec follows** (from `CLAUDE.md` + the existing tree):
> - Commands: **Pawn.CMD** `CMD:` / `alias:`. Cross-module publics use the
>   `FUNCTION` macro (`forward public` + `public`) and `CallLocalFunction`.
> - MySQL R41 async (`mysql_pquery` + `cache_*`), per-city tables (`LSplayers`),
>   global `players` for account/rank.
> - New command files go under **`gamemodes/CnR/cmds/`**; new systems under
>   **`gamemodes/CnR/systems/`** (both are new dirs — add their includes to
>   `CnR.pwn`'s include block, wrapped in `#if defined WINDOWS_COMPILER`).
> - `MAX_PLAYERS 100`, ranks `SERVER_PLAYER(0)…SERVER_OWNER(6)` +
>   `RETIRED_STAFF(1)`; `pDj/pVip/pRegularPlayer` are bools.
> - Build must stay **0 errors / 0 warnings**; recompile + commit `CnR.amx`.

---

## 0. Table of contents

1. Rank tiers & permission model (SCRIPTER, DONATOR/REGULAR/DJ)
2. Wanted-level system (core dependency)
3. Cop / arrest / jail system (CB parity — minimal set)
4. Crime commands (`/rob`, `/rape`, `/takedrugs`, drugs, kidnap, holdup)
5. **Robberies system** (shoplifting, store holdup, bank, casino, house, special/v23, crowbar)
6. **Missions system** (all 19 missions: civilian, cop-only, competitive)
7. Player commands (weapons, moneybag, moneyrush, radio, lock, ad, jumpkick, shop, teleports, pm/reply, goto)
8. DJ / Owner / Scripter commands
9. Systems roster (housing, fishing, farming, stocks, lotto, duel, sniper, DMS, clothes, skills, anti-parachute, GPS, interiors, zones, fixes)
10. Economy integration table (stock market multipliers + market tick + lotto/clock fix)
11. DB schema additions (tables + columns)
12. Milestones M2–M7 (dependency-ordered)
13. **M2 command specs in full** (ready to implement)
14. Open questions for the owner
15. Executive summary
16. **Code follow-ups from site research** (constants/behaviours to adjust in already-built stages)

---

## 1. Rank tiers & permission model

### 1.1 Staff rank enum change (`server/defines.inc`)

The roster requires a **SCRIPTER tier above OWNER**. Add it to the existing enum
(order matters — append, do not renumber existing rows, so stored `Rank` ints
stay valid):

```pawn
enum
{
    SERVER_PLAYER,       // 0
    RETIRED_STAFF,       // 1
    SERVER_MODERATOR,    // 2
    SERVER_ADMIN,        // 3
    SERVER_HEAD_ADMIN,   // 4
    SERVER_MANAGER,      // 5
    SERVER_OWNER,        // 6
    SERVER_SCRIPTER      // 7  <-- NEW, highest tier
};
```

- `players.Rank` already stores 0–6; SCRIPTER = 7 needs no schema change (INT).
- **Rank grants are DB-driven ONLY.** No name/hash backdoors — this is a recorded
  security decision (`CLAUDE.md`; PLANNED-FEATURES note on `/getscripter`). The
  legacy April-Fools name check in `login_register.inc`
  (`if(day==1 && month==4) … "Server Moderator"`) is a **cosmetic login label
  only** and grants no powers; leave it or remove it, but never let it set
  `pRank`.
- `/makescripter` and `/makeowner` (scripter cmds, §6) are the only way to reach
  6/7 in-game, and they write `players.Rank` then re-load.

### 1.2 Non-staff flags (already bools — keep as-is)

| Flag | Var | DB col | Granted by | Meaning |
|---|---|---|---|---|
| Donator (VIP) | `pVip` | `players.Vip` | `/makedonator` | no wealth tax, `/vehcolor` $100, name colour *[jobs §5.2]*. **CAVEAT:** The official crazybobs.net donation page explicitly states "Donating any amount of money does not entitle you to any special treatment on the server." The perks above are sourced from community docs (Fandom wiki, forum threads), NOT from the official site. They are likely real in CB but were never officially sanctioned. Our design may grant them as admin-assigned VIP perks (via `/makedonator`) separate from any real-money donation flow *[crazybobs-site-full §Corrections #2]*. |
| Regular | `pRegularPlayer` | `players.RegularPlayer` | auto (score+playtime) or `/makeregular` | RPC access, Rustler *[jobs §5.1]* |
| DJ | `pDj` | `players.Dj` | `/makedj` | `/djradio` broadcast *[jobs §4.3]* |

**Permission helper [OUR DESIGN]** — add to `server/misc.inc` so every command
gates identically:
```pawn
IsStaff(playerid, rank)   return (PlayerInfo[playerid][pRank] >= rank);
IsCop(playerid)           // pTeam == POLICE || SHERIFF || FBI || UC_COP
IsCivilian(playerid)      // pTeam == CIVIL || UC_COP-appears-civ
```
`ERROR_NOT_ADMIN` already exists for the failure message.

---

## 2. Wanted-level system  *(core dependency — build first in M3)*

Source: *[crime-cop-loop §2–3, §9]*, *[primary §2]*. The spine of the whole mode.
Lands in **`systems/wanted.inc`** (new).

### 2.1 Model

- `pWanted` int 0–10 (add to `pInfo` + `LSplayers.Wanted`).
- **Three tiers** *[primary §2]*:
  | WL | Tier | Colour | Cop action |
  |---|---|---|---|
  | 0 | Innocent | White name | none |
  | 1–5 | Suspect | Yellow | ticket only |
  | 6–8 | Warrant | Orange | arrest (on foot) |
  | 9–10 | Most Wanted | Dark Orange | arrest OR lethal takedown (+2 cop score) |
- **Colours** are the readability contract — keep verbatim *[crime-cop-loop §1]*.
  Add to `colors.inc`: `COLOR_WANTED_YELLOW 0xFFFF0000`,
  `COLOR_WANTED_ORANGE 0xFF751900`, `COLOR_WANTED_DARKORANGE 0xCC330000`. Apply
  via `SetPlayerColor` + `SetPlayerMarkerForPlayer` on every WL change.
- **Backup marker** purple for cops who `/backup` *[crime-cop-loop §4.1]*.

### 2.2 API (FUNCTION publics so crime handlers in other modules can call)

```pawn
FUNCTION GivePlayerWanted(playerid, amount, const reason[]);  // stack, clamp 0..10
FUNCTION SetPlayerWanted(playerid, level, const reason[]);    // absolute (escape=10, holdup=+7)
FUNCTION GetPlayerWanted(playerid);
```
Every crime handler calls `GivePlayerWanted`. `reason[]` feeds the cop crime
log / future Discord bridge via `IRC_SendMessage` (no-op today).

### 2.3 Per-crime wanted values *[crime-cop-loop §2.3, §2.2 verified anchors]*

| Crime | WL effect | Source |
|---|---|---|
| Pickpocket / petty `/rob` | +2 | INFERRED §2.3 |
| `/rape` | +2 (+1 if it infects) | INFERRED §2.3 |
| Store holdup | **set +7** (Felon) | VERIFIED *[primary §5]* |
| Shoplift | +2 | INFERRED |
| Car theft (jacked/locked) | +3 | INFERRED |
| Sell drugs/weapons **in cop sight** | +1 | VERIFIED *[primary §2]* |
| Drug delivery per checkpoint (cop in sight) | +2 | VERIFIED |
| Bank/casino/mint robbery | set **6** on start, climbs to 9–10 carrying safe | INFERRED |
| Kidnapping | **always warrant** (+1/civ, +6/cop kidnapped) | VERIFIED *[primary §2]* |
| Murder of innocent (white) | +4..+6, flag "serious" | INFERRED |
| Drunk driving | set **5** | VERIFIED |
| Kill deer w/o hunting permit | +6 | VERIFIED |
| Kill hippy | instant warrant (set 6) | VERIFIED |
| Jail escape | set **10** | VERIFIED |
| Hitman contract kill | ≥ 6 | VERIFIED |

### 2.4 Decay *[crime-cop-loop §3]*

- WL decays 1 level every **`WANTED_DECAY_SECS` (default 180 real-seconds)**
  **[OUR DESIGN number]** — CB says "slowly," no number *[primary §15]*.
- Decay **paused** while any cop holds `/visualcontact` on this player
  (`pVCFreezeUntil` timestamp).
- **Losing all pursuing cops during a robbery reduces WL** *[crime-cop-loop §2.5]*.
- Confess at church / bribe / long time no cop → reduce (church + bribe in M5/M4).
- **Ticket-unpaid → warrant escalation:** a ticketed Suspect who doesn't pay in
  `TICKET_PAY_SECS (default 300)` jumps to WL6 *[primary §2]*.

### 2.5 DM protection / safe zones *[crime-cop-loop §3, §7]*

- **Innocent (white) players are damage-protected** in **safe zones** (PD, bank,
  City Hall, hospital, RPC). Track a `DMZoneFlag` per streamer area (§7 zones).
- Random attack on a white player auto-adds wanted to the aggressor so cops can
  respond (channel DM into the loop) *[crime-cop-loop §7 design]*.
- Spawn protection timer **5–10 s** on fresh spawn (no damage in/out) **[OUR DESIGN]**.

---

## 3. Cop / arrest / jail system  *(CB parity, M3 — minimal cop set)*

The roster lists no cop commands, but arrest/jail are required for CnR parity
*(task requirement)*. Lands in **`cmds/cop.inc`** + **`systems/jail.inc`**.
Source: *[crime-cop-loop §4–5]*, *[primary §3–4]*.

### 3.1 Minimal cop command set

| Command | Alias | Gate | Spec |
|---|---|---|---|
| `/arrest` | `/ar` | cop | Nearest warranted (WL6+) suspect **on foot** within `ARREST_RANGE 4.0`. Disarm, jail, cop +1 score + WL-scaled cash. Cannot arrest in-vehicle or WL<6. *[§4.2]* |
| `/ticket` | `/tk` | cop | Nearest **Suspect (WL1–5, not warranted)** within range; sets a ticket, cop must stay `<8m` to collect; collect → cop +$500 +1 score, suspect must `/payticket`. *[§4.3]* |
| `/freeze` `/pullover` | `/fr` `/pu` | cop | Order nearest suspect to comply (freeze 5 s / message). *[§4.1]* |
| `/report` | `/rp` | cop | `<id> <reason>` raise suspect WL (serious crimes only, abuse=ban). *[§2.4]* |
| `/cancellastreport` | `/clr` | cop | undo last `/report`, restore WL. *[§2.4]* |
| `/visualcontact` | `/vc` | cop | freeze target WL decay `VC_FREEZE_SECS 60`. *[§2.4]* |
| `/backup` | `/bk` | cop | request backup, purple marker + broadcast to cops. *[§4.1]* |
| `/copmsg` | `/cm` | cop | PM to all cops. *[§4.1]* |
| `/suspects` | `/sus` | cop | list WL1–5. *[§4.1]* |
| `/warrants` | `/war` | cop | list WL6+ (WL, loc, veh). *[§4.1]* |
| `/mostwanted` | `/mw` | cop | list WL9–10. *[§4.1]* |
| `/parole` | `/pl` | cop | free-release a jailed player who served minimum. *[§5]* |
| `/accept` `/refuse` | `/ac` `/ref` | cop | resolve an offered `/bribe`. *[§5]* |
| `/refill` | `/rf` | cop, at PD | spend Refill Points (max 10) on HP/armour/weapons; weapon tier gated by cop rank. *[§4.3, jobs §3.2]* |
| `/donut` | — | cop, at donut shop | +50% HP, carry max 6. *[§4.1]* |

**Cop rank ladder** (separate from staff rank) `pCopRank` 0–10 Recruit→Commissioner
*[primary §4]*; rises with arrests+tickets, gates `/refill` weapon tiers. DB:
`LSplayers.CopRank`. `/refill` weapon availability by rank is the clearest
level-gated perk *[jobs §3.2]*.

**Cop rewards** *[crime-cop-loop §4.3]*: ticket +$500 +1; arrest +1 + WL-scaled
cash (`ARREST_CASH_PER_WL 1500 * wl` **[OUR DESIGN]**); WL10 kill +2; COTD +$25k
+1; patrol mission $2,500/cp + $25k all = $37,500.

**Cop hard rules** (script-enforced) *[crime-cop-loop §4.4]*: cannot arrest WL0;
cannot ticket WL0 or WL6+; no shooting jailed players (block damage in jail VW);
killing innocent/cop → lose a cop rank.

### 3.2 Jail  (`systems/jail.inc`)  *[crime-cop-loop §5, primary §3]*

- On arrest: teleport to city PD jail (LS Pershing Square), interior/VW isolated,
  disarm, set `pJailed=1`, `pJailTime`, `pBail`.
- **Cell counts**: LS 3 cells *[primary §3]*.
- **Sentence formula [OUR DESIGN — image-only in CB]**:
  `pJailTime = JAIL_BASE(60s) + wantedAtArrest*30s + seriousCrimeCount*45s`;
  `pBail = pJailTime * BAIL_PER_SEC(200)`.
- **Four ways out** *[§5]*:
  1. Serve full → `/bail` (`/pb`) pay bail, or cop `/parole` (free after minimum).
  2. `/appeal` — jury of `JURY_SIZE (15, config)` votes; innocent → time 0. Blocked
     if `<60s` left or a "serious" (murder) charge. *[§5]*
     > **CODE FOLLOW-UP:** The constant `JURY_SIZE` must be set to **15** in
     > `systems/jail.inc`. The official crazybobs.net crimes-jail-sentence page states
     > verbatim "Appeal sentence to 15 random jurors" *[crazybobs-site-full §Corrections #1]*.
     > Earlier spec draft used 12 (from the Fandom wiki). The official site is authoritative.
     > Do NOT change this constant until the M3 review pass — log it in §16.
  3. `/bribe [amount]` a nearby cop, `$1,000–$15,000`; cop `/accept`/`/refuse`.
     Accept → time+bail cut; refuse → time+bail up, -1 score, "attempted bribe."
     **Most Wanted (WL10) cannot bribe.** *[§5]*
  4. `/escape` (`/esc`) — set **WL10** + "Jail Escape" broadcast; re-caught →
     chained (no re-escape). `/breakout` (`/bo`) frees another player. *[§5]*
- Jail is a rape hotspot — `/rape` works in cells; carry drugs/condoms *[§5]*.
- DB: `LSplayers`: `Jailed, JailTime, Bail, JailReason, EscapeChained, SeriousCrimes`.

---

## 4. Crime commands (player-facing crimes)

Lands in **`cmds/crimes.inc`** + **`systems/drugs.inc`** + **`systems/std.inc`**.
Each handler calls `GivePlayerWanted` (§2.2). Skill perks read `pSkill`
(add enum `pSkill` to `pInfo`; DB `LSplayers.Skill`; changed at City Hall via
`/skill` in M6) *[jobs §1.3]*.

### 4.1 `/rob` `/rb` — rob a player  *[primary §6]*

- Syntax: `/rob [id]` (blank = nearest, `ROB_RANGE 3.0`). Target must be on foot,
  not in Bank/City Hall/Ammunation, not already robbed this cooldown.
- **Caps by skill** *[primary §6]*: non-rob skill **$50,000**, Pickpocket
  **$100,000** (high success), Con Artist **$500,000** (moderate, robs 10–50% of
  victim on-hand cash). Only **cash-in-hand** is robbable (bank is safe).
- Wanted: **+2**. Cooldown `ROB_CD 30s` per victim. Robber +1 score + cash.
- DB: none new (uses `pMoney`); score in `LSplayers.Score`.

### 4.2 `/rape` `/ra`  *[crime-cop-loop §6.1, primary §7]*

- Syntax: `/rape [id]` (blank = nearest, `RAPE_RANGE 2.0`). Deals damage even
  through condom; may infect victim with an STD.
- **Rapist skill:** high success + can pass STD they don't carry; anyone holding
  several STDs rapes at ~100%. Cooldown `RAPE_CD` (Rapist shorter).
- Wanted **+2** (+1 if infects). +1 score.
- **STD system (`systems/std.inc`)**: `pSTD` bitmask, tiers Chlamydia→Mary Lou;
  STDs drain HP over time (timer) and kill if untreated. Cures: condom (reduces
  infect chance), chastity belt (prevents rape, breakable), **Adrenaline pill
  (`/ad` per roster) fully heals + cures all**, Public Medic force-cure. Drugs
  stall the drain. *[crime-cop-loop §6.1]* DB: `LSplayers.STDs, Condoms,
  ChastityBelt`.

### 4.3 `/takedrugs` + drug system  (`systems/drugs.inc`)  *[crime-cop-loop §6.2, primary §7]*

- `/takedrugs [amount]` (`/td`): consume held drugs. No amount → click-mode 5g/click,
  F exits. Heals over time (~5s effect/gram, small heal/1.5–2s). Four buzz stages.
- **Overdose > 60g at once → slow death**; adrenaline pill cancels. Max single 60g.
- **Bad drugs**: random chance → fall anim, buzz reset, chance of STD, -1 score.
- Carry limits: Drug Dealer **2,500g base** (confirmed Fandom Drug_Dealer wiki) /
  **5,000g with a drug bag** (confirmed Score Guide); others **500g** *[primary §7]*.
  **Drug bag is obtainable ONLY from Drug Refill Points** (confirmed verbatim on
  official site: "The only place to get a drug bag") *[crazybobs-site-full §Drug Refill Points]*.
  Prior spec text citing 5000g as a flat limit is amended accordingly.
- DB: `LSplayers.Drugs, DrugSeeds`. (Growing/`/plant`/`/harvest` = farming, §7.)

### 4.4 Kidnap / holdup (M4/M5 — spec here for completeness)

- **Holdup** `/holdup` `/hup`: at a shop actor (§ actors), looking at clerk. Set
  WL **+7**, ~10s clerk wait, then **$2,000–$6,000/sec**; crowbar boosts.
  *[primary §5]*. Lands in `cmds/robbery.inc` (M5).
- **Kidnap** `/kidnap [id] [ransom]` (`/kd`): disguise as Driver, lock victim,
  ransom ≤$50k, steal 20–80% on-hand at hideout; always warrant. `/release`,
  `/kidnapall`, `/ransom`. *[crime-cop-loop §2.6]* Lands in `cmds/kidnap.inc` (M5).

---

## 5. Robberies system  *(M5 — lands in `cmds/robbery.inc` + `systems/robbery.inc`)*

Source: *[crazybobs-site-full §Robberies and Crowbar]* (VERIFIED throughout, cited per type).
**General principles** (all robbery types):
- A **warrant for arrest** is issued when a robbery begins; police are notified immediately
  (or when the alarm triggers — see v23 alarm change below). *[crazybobs-site-full §General Robbery Principles]*
- **Version 23 rework:** Every robbery type has individualized min/max timers, bonuses, and
  success chances. Payouts scale with online server population. *[crazybobs-site-full §General Robbery Principles]*
- **Alarm delay (v23):** The alarm no longer triggers immediately. Police are notified only
  when it goes off. However, if any law enforcement enters the area, the alarm triggers
  immediately. *[crazybobs-site-full §General Robbery Principles]*
- **Score:** Each completed robbery = **+1 score** *[crazybobs-site-full §General Robbery Principles]*.
- All robbery commands call `GivePlayerWanted` (§2.2). DB column `LSplayers.RobberyHistory`
  (bitmask of recent robbery type flags — needed for crowbar confiscation logic).

---

### 5.1 Shoplifting (`/shoplift`)

**Command:** `/shoplift`  **Module:** `cmds/robbery.inc`

- Available at store/shop checkpoints (24/7 stores, Clothes shops, etc.).
- Steals items into the player's inventory rather than cash.
- **Wanted:** +2 **[OUR DESIGN — site says warrant issued; exact WL not published]**.
- DB state: none beyond `LSplayers.Wanted`; item added to `pInventory`.
- **VERIFIED** *[crazybobs-site-full §Shoplifting]*.

---

### 5.2 Store Holdup (`/holdup`)

**Command:** `/holdup` (aliases: `/hup`, `/storerob`, `/robstore`)
**Module:** `cmds/robbery.inc`

**Eligible locations** *[crazybobs-site-full §Store Holdups]*:
24/7 Stores, Ammunation, Bars, Restaurants/Diners, Dance Clubs, Sex Shops,
Inside Track Betting, Clothes stores.

**Mechanics (VERIFIED)** *[crazybobs-site-full §Store Holdups]*:
- Player must be **on foot** and **looking at a store clerk** (actor) to initiate.
- Instantly sets wanted to **+7** (Felon tier — pushes into Warrant range). **VERIFIED.**
- Clerk waits **~10 seconds** before opening the register. (Crowbar reduces this wait by
  a couple of seconds — see §5.7.)
- Once register is open: player receives **$2,000–$6,000 per second**. **VERIFIED.**
  Crowbar increases the per-second amount (see §5.7).
- A cop who enters the shop and presses MMB can interrupt and arrest mid-holdup.
- Leave the cash zone and exit the store to finish.
- Player may `/bribe` a cop afterward to go innocent (keeps crowbar + weapons on bribe accept).

**Cooldown:** **[OUR DESIGN]** — CB never published a numeric cooldown; use `HOLDUP_STORE_CD 300s`.

**DB state:** `LSplayers.Wanted`, `LSplayers.RobberyHistory` (flag: `ROB_HOLDUP`).

---

### 5.3 Bank Robbery (`/bankrob`)

**Command:** `/bankrob` (alias: `/robbank`)  **Module:** `cmds/robbery.inc`

**Available banks (7 total)** *[crazybobs-site-full §Bank Robbery, §Banks]*:
- Los Santos: `620.250, -1258.000`
- San Fierro: `-2021.000, 460.250`
- Las Venturas: `2470.000, 2258.750`
- Secondary: Fort Carson, Las Barrancas, Palomino Creek, + one additional.

**Mechanics (VERIFIED)** *[crazybobs-site-full §Bank Robbery]*:
- Wait **~30 seconds** at the bank checkpoint (cops are alerted during this wait).
- Safe is then available — deliver it to a **randomly-selected hideout** to receive payment.
- Bank Robberies take money from other players' bank accounts (not a fixed cash pool).
  - **Max taken per victim account: $100,000** *[VERIFIED]*.
  - On a full server, total yield is approximately **$500,000–$1,000,000** *[VERIFIED]*.
  - Bank-insured players receive **75% of stolen funds back** *[VERIFIED]*.
- Banks **close** during and immediately after a robbery (no deposits/withdrawals).
- Cops alerted immediately when the 30-second wait begins (alarm is not delayed for bank robs).

**Wanted:** **[OUR DESIGN]** set **WL6** at start of 30-second wait, escalates to WL9–10 while
  carrying the safe (refer to §2.3 table; bank/casino/mint row).

**DB state:** `LSplayers.Wanted`, `LSplayers.RobberyHistory` (flag: `ROB_BANK`).
New server-state table column: `server_data.BankRobCooldown` per bank (ISO datetime of last
successful rob — **[OUR DESIGN]** prevent back-to-back on same branch; suggest 60-min real cooldown).

---

### 5.4 Casino Robbery (`/casinorob`)

**Command:** `/casinorob` (or `/robbery` at casino checkpoint)  **Module:** `cmds/robbery.inc`

**Available at 5 casinos (VERIFIED)** *[crazybobs-site-full §Casino Robbery, §Casinos]*:
- Las Venturas (3): Caligula's Palace, Four Dragons Casino, Redsands Casino.
- San Fierro (1): added in Version 23.
- Los Santos (1): added in Version 23.

**Mechanics (VERIFIED)** *[crazybobs-site-full §Casino Robbery]*:
- Rob the casino interior.
- Transport the **safe to a random hideout** to crack it for payment.
- Casinos are "known for their daily occurring robberies" — flavour/frequency note.

**Payout:** Not numerically specified on site. **[OUR DESIGN]** — tune comparable to bank rob
  at 50–75% of bank yield to preserve balance. Owner ratifies.

**Wanted:** Same escalation model as bank rob (§5.3). **[OUR DESIGN]**

**DB state:** `LSplayers.RobberyHistory` (flag: `ROB_CASINO`).

---

### 5.5 House Robbery (`/houserob`)

**Command:** `/houserob` (aliases: `/hrob`, `/hsrob`)  **Module:** `cmds/robbery.inc`

**Mechanics (VERIFIED)** *[crazybobs-site-full §House Robbery]*:
- Can only rob a house while the **owner is online**.
- Break-in steals items from the house's storage contents.
- **Items that can be stolen (storage caps):**
  | Item | Max storable (robbable) |
  |------|------------------------|
  | Condoms | 25 |
  | Flowers | 25 |
  | Drugs | 5,000g |
  | Seeds | 25 |
  | Deer Traps | 25 |
  | Fish | 10 |
  | Clothing items | 20 |
  | Money | Unlimited (all storaged money) |
- **House Alarm** — alerts all owners, co-owners, and police when triggered *[VERIFIED]*.
- **Super Lock** — makes it much harder for anyone to break into the house; reduces
  success chance / increases break-in time *[VERIFIED]*.
- **Pets** — house pets attack intruders; deal varying damage per pet type *[VERIFIED]*.

**Crowbar interaction:** Holding crowbar reduces break-in time and improves success chance (§5.7).

**Wanted:** **[OUR DESIGN]** warrant on attempt (+6, Warrant tier); higher if owner or cop
  intercepts. DB: `LSplayers.RobberyHistory` (flag: `ROB_HOUSE`).

---

### 5.6 Special / Major Robberies (Version 23+)

**Command:** `/robbery` at location-specific checkpoint  **Module:** `cmds/robbery.inc`

All v23 special robberies share the mechanic: break into the location, open the safe/collect
the target, then reach a **random hideout within 12 minutes** *[VERIFIED — CrazyBob's Place
and Federal Mint explicitly state 12-minute window]* *[crazybobs-site-full §Special/Major Robberies]*.

| Location | City | Notes |
|----------|------|-------|
| CrazyBob's Place | LS (Mulholland/Vinewood) | 12-min hideout window; stay within checkpoint boundary |
| SF Federal Mint | SF | 12-min hideout window; break-in required |
| LS SA-MP Office Tower | LS | Special robbery type |
| LS Observatory Mansion | LS | Special robbery type |
| Jizzy's Pleasure Domes | SF | Also used for parties/gambling |
| Woozie's Private Club | SF | Special robbery type |
| SF Cargo Ship | SF | Special robbery type |
| SF Zombotech | SF | Special robbery type |
| SF Drug Factory | SF | Special robbery type |
| K.A.C.C. Fuels (Methylamine) | LV | Find the barrel; reach hideouts |
| Inside Track Betting | LS | Also a horse-betting venue |
| Airport Robbery | All 3 cities | See §6.1 (Mission-type, box-collection) |

**VERIFIED** (list and 12-min window) *[crazybobs-site-full §Special/Major Robberies]*.
Payout for each: **[OUR DESIGN]** — not published numerically; tune as part of M5 balancing.

**Wanted / DB state:** Same warrant model as bank rob. `LSplayers.RobberyHistory` (flag: `ROB_SPECIAL`).

---

### 5.7 The Crowbar (Clothing Item)

**Module:** `systems/robbery.inc` (item effect applied in holdup/robbery handlers)

**VERIFIED throughout** *[crazybobs-site-full §The Crowbar — Complete Mechanics]*.

#### Where to buy
- **Truck Stops** (across San Andreas) *[VERIFIED]*
- **Zero's RC Shop** (Garcia, San Fierro) *[VERIFIED]*
- **Price: $40,000** *[VERIFIED — Bait Shop/economy cross-ref]*

Also sold at Bait Shops (same price). **[OUR DESIGN — bait shop as additional vendor]**

#### Activation / slot
The crowbar occupies a **clothing item slot**, NOT a weapon slot. Equip via:
- **`/clotheswear`** — official confirmed activation command *[VERIFIED crazybobs-site-full §Crowbar]*
- **`/crowbar`** — remove/toggle crowbar *[VERIFIED]*

It is **not a melee weapon** and does not occupy a weapon slot.

#### Dual effect on crime (VERIFIED)
| Robbery context | Crowbar effect |
|----------------|---------------|
| **Store Holdup** | Reduces the ~10-second clerk register-open wait by a couple of seconds; AND increases the per-second cash amount received from the register |
| **Safe-type robberies** (bank, casino, CrazyBob's Place, special) | Lowers the time required to open the safe / complete the robbery stage; AND increases the chance of a successful robbery |

Official site (Game Items page, verbatim): *"Holding your Crowbar (/clotheswear) will lower your robbery time (robberies & holdups) and increase your chance of a successful robbery."* *[VERIFIED]*

#### Confiscation on arrest
- **Removed on arrest** if `LSplayers.RobberyHistory` contains a robbery violation *[VERIFIED]*.
- **Exception:** If the player was **killed by a cop using lethal force** (not arrested), the
  crowbar and fishing rod are **NOT removed** — this is intentional CB design *[VERIFIED]*.
  Implement via: check death reason in `OnPlayerDeath`; if killer is a cop (WL10 context),
  skip crowbar confiscation. Arrest path always confiscates (if history flag set).

#### Implementation notes
- `pCrowbarEquipped` bool in `pInfo`; DB: `LSplayers.CrowbarEquipped TINYINT(1)`.
- On equip: apply clothing attachment (visual); set flag; robbery handlers read flag.
- Holdup handler: `clerkWaitTime = (pCrowbarEquipped ? (BASE_CLERK_WAIT - CROWBAR_WAIT_REDUCTION) : BASE_CLERK_WAIT)`; `registerPerSec = (pCrowbarEquipped ? HIGH_REGISTER_RATE : BASE_REGISTER_RATE)`. **[OUR DESIGN — exact reduction values; tune in M5]**
- Safe-crack handler: `safeOpenTime -= (pCrowbarEquipped ? CROWBAR_SAFE_REDUCTION : 0)`; modify success probability. **[OUR DESIGN]**
- Arrest handler: `if(PlayerInfo[playerid][pCrowbarEquipped] && HasRobberyHistory(playerid)) { PlayerInfo[playerid][pCrowbarEquipped] = 0; /* confiscate */ }`

---

### 5.8 Robberies DB additions

New columns for `LSplayers` (add in §11 / M5 schema):
```sql
ALTER TABLE `LSplayers`
  ADD `CrowbarEquipped`   TINYINT(1) NOT NULL DEFAULT 0,
  ADD `RobberyHistory`    INT NOT NULL DEFAULT 0;  -- bitmask: ROB_HOLDUP|ROB_BANK|ROB_CASINO|ROB_HOUSE|ROB_SPECIAL
```

New server-state columns for `server_data`:
```sql
ALTER TABLE `server_data`
  ADD `BankRobLastLS`     DATETIME NULL DEFAULT NULL,
  ADD `BankRobLastSF`     DATETIME NULL DEFAULT NULL,
  ADD `BankRobLastLV`     DATETIME NULL DEFAULT NULL;
```

---

## 6. Missions system  *(M4/M5/M6 — see milestone note at end of section)*

Source: *[crazybobs-site-full §Missions]* and individual sub-pages — all 19 confirmed VERIFIED.
**General mechanic notes (all missions):**
- Cancel any mission with **`/cancel`** at any time.
- GPS auto-activates for mission destinations *[crazybobs-site-full §GPS System]*.
- Monitor mission status with the **Sub-Mission Key** or **`/mission`**.
- Missions land in **`systems/missions.inc`** (new) with individual mission handler includes
  under `gamemodes/CnR/systems/missions/` **[OUR DESIGN — file layout]**.

---

### 6.1 Airport Robbery (Mission)

**Command:** `/robbery` at airport checkpoint  **Trigger:** Be in a truck, van, or garbage truck at the airport

**Start locations (VERIFIED)** *[crazybobs-site-full §Airport Robbery]*:
- Los Santos International Airport
- San Fierro Easter Bay Airport
- Las Venturas Airport

**Structure:**
1. Enter checkpoint → type `/robbery` (must be in truck/van/garbage truck).
2. A timer appears. Exit vehicle, collect **boxes** using Middle Mouse Button or **`/box`**.
3. Player is **wanted while collecting boxes** — cops will intervene.
4. When timer expires, return to vehicle and **drive to the hideout** within a second time limit.
5. Reach hideout = receive payment proportional to boxes collected + wanted level may reduce.

**Payout:** Scales with boxes collected; exact formula not published. *[VERIFIED structure; payout [OUR DESIGN]]*.
**Score:** +1 (from General Robbery Score Guide).
**Wanted:** Active while collecting boxes *[VERIFIED]*. **[OUR DESIGN]** set WL7 on start.
**Cooldown:** **[OUR DESIGN]** 5 game hours.

---

### 6.2 Courier Delivery

**Command:** `/courier`  **Who:** Civilians only

**Structure (VERIFIED)** *[crazybobs-site-full §Courier Delivery]*:
- Pickup location is always **far from the city**; delivery destination is always **inside the city**.
- Time limit: **12 game hours**.
- Receive a smuggling warrant **on pickup** (cops attempt to intercept during transport).
- Extra payment based on distance and time upon successful delivery.

**Payout:** High — described as higher than truck delivery; exact number not on official page.
*[VERIFIED structure; payout scale VERIFIED relative; exact number [OUR DESIGN]]*.
**Score:** +1 per delivery *[VERIFIED — Score Guide]*.
**Wanted:** Warrant on pickup *[VERIFIED]*.
**Cooldown:** **[OUR DESIGN]** 3 game hours.

---

### 6.3 Domestic Disturbance (Cop-only)

**Who:** Cops only  **Command:** `/mission` at PD (select second option)

**Start locations (VERIFIED)** *[crazybobs-site-full §Domestic Disturbance]*:
- LSPD HQ: `1582.375, -1635.000`
- SFPD HQ: `-1621.750, 682.125`
- LVPD HQ: `2244.375, 2492.875`

**Structure (VERIFIED):**
- Drive from the PD to a **player-owned house** anywhere in San Andreas within the time limit.
- Payout scales with speed — faster arrival = bigger bonus *[VERIFIED]*.

**Payout:** Variable/speed-based; exact figures not stated *[VERIFIED structure; exact scale [OUR DESIGN]]*.
**Score:** +1 per completion **[OUR DESIGN]**.
**Wanted:** None (cop mission).

---

### 6.4 Drug Delivery

**Who:** Innocent civilians only (WL0 required to start)  **Trigger:** Enter mission checkpoint

**Start locations (VERIFIED)** *[crazybobs-site-full §Drug Delivery]*:
- Los Santos: Las Colinas
- San Fierro: Calton Heights
- Las Venturas: LVA Freight Depot

**Structure (VERIFIED):**
- Deliver drugs to **5 randomly-selected locations** in any order before the timer expires.
- Land vehicles only.
- Each checkpoint reveals the next destination.

**Payout (VERIFIED — Score Guide):** **$5,000 per checkpoint + $50,000 all-5 bonus**.
**Score:** +1 per checkpoint *[VERIFIED]*.
**Cooldown:** Once every **5 game hours** *[VERIFIED — Score Guide]*.
**Wanted:** **+2 per delivery if a cop is in sight** (cop sight radius smaller at night); no warrant unless cop sees you **3 times** *[VERIFIED — Score Guide]*.

---

### 6.5 Flower Delivery

**Who:** Both cops and civilians (innocent assumed)  **Command:** `/mission` at checkpoint or Church

**Start locations (VERIFIED)** *[crazybobs-site-full §Flower Delivery]*:
- Los Santos: Jefferson
- San Fierro: Hashbury
- Las Venturas: South East
- **Also:** Can start from any of the 8 Churches.

**Structure (VERIFIED):**
- Deliver flowers to **5 different players** in under **6 game hours** using **`/flowers`** command.
- Strategy: find players at City Hall, PD, or other populated areas.

**Payout:** Cash bonus upon completion; exact amount not stated on official page *[VERIFIED structure; exact [OUR DESIGN]]*.
**Score:** +1 per completion **[OUR DESIGN]**.
**Wanted:** None *[VERIFIED — cop-accessible mission]*.
**Cooldown:** **[OUR DESIGN]** 2 game hours.

---

### 6.6 Food Delivery

**Who:** Any innocent civilian  **Command:** `/mission` while in a food delivery vehicle

**Vehicle requirement (VERIFIED):** Mr.Whoopee, Hotdog, or Pizzaboy.

**Structure (VERIFIED)** *[crazybobs-site-full §Food Delivery]*:
- Complete **5 food deliveries** to 5 different locations in the current city.
- Each delivery reveals the next location.
- Extra time granted based on distance of next delivery.

**Payout (VERIFIED — Score Guide):** **$2,500 per checkpoint + $25,000 all-5 bonus = $37,500 total**.
**Score:** +1 per checkpoint; also +$500/item to next-day pay *[VERIFIED — Score Guide]*.
**Wanted:** None.
**Cooldown:** **[OUR DESIGN]** 2 game hours.

---

### 6.7 Holdup Mission

**Command:** `/mission` at Holdup mission checkpoint  **Trigger:** Travel to checkpoint first

**Start locations (VERIFIED)** *[crazybobs-site-full §Holdup (Mission)]*:
- Los Santos: Market
- San Fierro: Chinatown
- Las Venturas: Linden Side

**Structure (VERIFIED):**
- Rob **3 different stores** (24/7s, gas stations, food shops).
- Must steal a **minimum of $10,000 from each store**.
- "Robbing so many shops will get you a high warrant and lots of attention."
- Cash received directly from each robbery + **large completion bonus** *[VERIFIED — "large bonus upon completing the mission"]*.

**Note:** This is a **mission wrapper** around the `/holdup` robbery system (§5.2). The underlying
holdup mechanics (WL+7, clerk wait, register rate) apply to each of the 3 stores.

**Payout:** $10,000+ per store (3 stores) + mission completion bonus; exact bonus *[OUR DESIGN]*.
**Wanted:** +7 per holdup → cumulative high warrant after 3 stores *[VERIFIED structure]*.
**Cooldown:** **[OUR DESIGN]** 5 game hours (high-reward mission).

---

### 6.8 House Delivery

**Who:** Civilians only (regardless of wanted status)  **Command:** Enter checkpoint

**Start locations (VERIFIED)** *[crazybobs-site-full §House Delivery]*:
- Los Santos: Ocean Docks `2165.500, -2279.500`
- San Fierro: Doherty `-1821.250, -182.000`
- Las Venturas: LVA Freight Depot `1380.250, 1026.500`

**Structure (VERIFIED):**
- Transport delivery items (including some **illegal goods**) to randomly-selected houses.
- Time limit: **12 game hours**.
- Any vehicle.
- Reward: bonus based on time and distance upon completion.

**Payout:** Variable; exact figure not stated *[VERIFIED structure; scale [OUR DESIGN]]*.
**Wanted:** Carrying illegal cargo may trigger if cop intercepts **[OUR DESIGN]**.
**Cooldown:** **[OUR DESIGN]** 3 game hours.

---

### 6.9 Illegal Immigrant Transport

**Who:** Innocent civilians  **Command:** Enter mission checkpoint

**Start locations (VERIFIED)** *[crazybobs-site-full §Illegal Immigrant Transport]*:
- Los Santos: East Los Santos
- San Fierro: Hashbury
- Las Venturas: Spiny Bed

**Structure (VERIFIED):**
- Pick up illegal immigrants **outside the main city**.
- Return them to the current city within **12 game hours**.
- **Stealth mechanic:** Avoiding police during transport prevents a wanted level mid-mission.
  Cops have a **reduced visibility radius at night** *[VERIFIED]*.
- Upon **successful completion**, receive a wanted level for transporting illegal immigrants *[VERIFIED]*.

**Payout:** Not specified numerically on official page *[VERIFIED structure; exact [OUR DESIGN]]*.
**Wanted:** Issued **on completion** (not pickup) *[VERIFIED — important distinction from prior assumption]*.
**Cooldown:** **[OUR DESIGN]** 4 game hours.

---

### 6.10 Lawn Mowing

**Location:** Las Venturas only — Yellow Bell Golf Course / Prickle Pine golf course
**Vehicle:** Lawn mower (must be on one)  **Command:** `/mission` at golf course checkpoint (select first option)

**Structure (VERIFIED)** *[crazybobs-site-full §Lawn Mowing]*:
- Reach as many checkpoints as possible in **2 in-game hours**.
- Competitive leaderboard: higher checkpoint count = higher rank against other players.
- Completion bonus; "a safe and relaxing way to make money."
- If cancelled, must wait several minutes before restarting.

**Payout:** Bonus upon completion; scale tied to checkpoint count *[VERIFIED structure; exact [OUR DESIGN]]*.
**Score:** +1 per completion **[OUR DESIGN]**.
**Wanted:** None.
**Cooldown:** Built-in (must wait several minutes after cancel) *[VERIFIED]*.

---

### 6.11 Paperboy

**Who:** Innocent civilians  **Command:** Enter mission checkpoint

**Start locations (VERIFIED)** *[crazybobs-site-full §Paperboy]*:
- Los Santos: Vinewood `845.250, -1042.000`
- San Fierro: Esplanade East `-1536.250, 1045.500`
- Las Venturas: Royal Casino `2250.125, 1466.750`

**Structure (VERIFIED):**
- Deliver as many newspapers as possible to **owned houses** around the city.
- Time limit: **4 minutes (real-time)** *[VERIFIED — unusually a real-time limit, not game-time]*.
- On **foot** — walk to checkpoints in front of houses.
- Bonus rewards based on total deliveries.

**Payout (VERIFIED):** **$500 per delivery** + bonus based on total count.
**Wanted:** None (innocent-only mission).
**Cooldown:** **[OUR DESIGN]** 10 minutes real-time (short mission, fast cooldown).

---

### 6.12 Pickpocket Mission

**Who:** Civilians (Pickpocket skill most effective)  **Command:** Enter mission checkpoint

**Start locations (VERIFIED)** *[crazybobs-site-full §Pickpocket (Mission)]*:
- Los Santos: Willowfield
- San Fierro: Financial
- Las Venturas: Redsands West

**Structure (VERIFIED):**
- Rob a total of **$10,000 from different players** within **6.5 game hours**.
- Structured target-based mission distinct from regular `/rob` command.

**Payout:** Collected money + completion reward **[OUR DESIGN — exact bonus not stated]**.
**Wanted:** +2 per successful rob (§2.3); warrant if caught *[VERIFIED — underlying /rob mechanic]*.
**Cooldown:** **[OUR DESIGN]** 3 game hours.

---

### 6.13 Race Challenges

**Command:** `/challenge`  **Who:** All players

**Structure (VERIFIED)** *[crazybobs-site-full §Race Challenges]*:
- **28 challenges** total across San Andreas.
- **First attempt: free**; subsequent attempts cost money *[VERIFIED]*.
- Winning a Top 10 ranking earns cash *[VERIFIED]*.
- **Race types:** Point To Point (fixed start) and Circuit (start = closest player location).
- **Vehicle categories:** Land, Trucks only (Trucker Challenge), Bikes only, Boats only, Air (helicopters/planes).

**Notable challenges (VERIFIED):**
| Challenge | Distance / Notes |
|-----------|-----------------|
| San Andreas Tour | 27.5 km, land |
| Trucker Challenge | 12 km, trucks only |
| Las Venturas Biker Endurance | 9 km, bikes only |
| Los Santos Circuit | 3 km city circuit |
| Red County Run | Dirt / off-road |
| Chiliad Climb | Off-road hill climb |
| Boating With Jaws | Boat, Very Hard |
| Las Venturas Fly-By | Air vehicles |
| San Fierro Fly-By | Air vehicles |
| LS Helicopter Tour | Helicopter |
| Admin Hill | Special challenge |

**Score:** +1 per finish; **+2 for podium finish** *[VERIFIED — Score Guide]*.
**Payout:** Top 10 earnings (amount not stated).
**Wanted:** None.

---

### 6.14 Routine Patrol (Cop-only)

**Who:** Cops only  **Command:** `/mission` at PD checkpoint (select first option)

**Start locations (VERIFIED)** *[crazybobs-site-full §Routine Patrol]*:
- LSPD HQ: `1582.375, -1635.000`
- SFPD HQ: `-1621.750, 682.125`
- LVPD HQ: `2244.375, 2492.875`

**Structure (VERIFIED):**
- Reach **5 sequential checkpoints** before each individual timer expires.
- Land vehicles only.

**Payout (VERIFIED):** **$2,500 per checkpoint + $25,000 completion bonus = $37,500 total**.
**Score:** +1 per checkpoint *[VERIFIED — Score Guide]*.
**Wanted:** None (cop mission).
**Cooldown:** **[OUR DESIGN]** 2 game hours.

---

### 6.15 Sexual Encounter

**Who:** Any innocent civilian (Rapist or Prostitute skill increases success chance)
**Command:** `/mission` at a sex shop

**Structure (VERIFIED)** *[crazybobs-site-full §Sexual Encounter]*:
- Complete **5 sexual encounters** within **5 game hours**.
- Rapist or Prostitute skill boosts the per-encounter success probability.

**Payout:** Cash bonus upon completion; exact amount not stated *[VERIFIED structure; exact [OUR DESIGN]]*.
**Wanted:** None at start; individual encounter may add WL **[OUR DESIGN — /rape mechanics apply]**.
**Cooldown:** **[OUR DESIGN]** 3 game hours.

---

### 6.16 Tractor Mission

**Location:** CrazyBob's Farm (tractors spawn in San Fierro but mission available in any city)
**Vehicle:** Tractor  **Command:** `/mis` or `/mission` at farm checkpoint

**Structure (VERIFIED)** *[crazybobs-site-full §Tractor Mission]*:
- Reach as many checkpoints as possible in **2 in-game hours**.
- Completion = small bonus; record for most checkpoints.
- Similar structure to Lawn Mowing (§6.10).
- If cancelled, must wait several minutes before restarting.

**Payout:** Small bonus + optional record *[VERIFIED; exact [OUR DESIGN]]*.
**Wanted:** None.
**Cooldown:** Built-in wait on cancel *[VERIFIED]*.

---

### 6.17 Trash Pickup

**Vehicle:** Trashmaster  **Command:** Sub-Mission Key or `/mission` while in a Trashmaster

**Structure (VERIFIED)** *[crazybobs-site-full §Trash Pickup]*:
- Drive through **5 checkpoints** scattered across the current city.
- Mission auto-cancels if the Trashmaster is lost.
- Bonus per checkpoint + additional bonus if all 5 completed under **6 game hours**.

**Payout (VERIFIED — Score Guide):** **$2,500 per checkpoint + $25,000 all-5 bonus** (same structure as Food Delivery and Routine Patrol).
**Score:** +1 per checkpoint *[VERIFIED]*.
**Wanted:** None.
**Cooldown:** **[OUR DESIGN]** 2 game hours.

---

### 6.18 Truck Delivery

**Command:** Sub-Mission Key or **`/delivery`** while in a delivery truck

**Supported vehicle types (14, VERIFIED)** *[crazybobs-site-full §Truck Delivery]*:
Benson, Boxville, Cement Truck, DFT-30, Dumper, Dune, Flatbed, Linerunner, Mule,
Packer, Pony, Roadtrain, Tanker, Yankee.

**Structure (VERIFIED):**
- Time limit: **12 game hours**.
- Base pay + bonuses for distance, time completion, trailer attachment.
- Some **illegal cargo** provides enhanced pay but may raise wanted level (no warrant
  issued unless cop sees you) *[VERIFIED — Score Guide note]*.
- Auto-cancels if truck is lost.

**Commands:** `/mission` or Sub-Mission Key for delivery info; `/truckmsg`, `/tm`, `/cb`
  for trucker chat; `/cancel` to abandon.

**Payout:** Cargo-based; formula not published *[VERIFIED structure; formula [OUR DESIGN]]*.
**Score:** +1 per delivery *[VERIFIED]*.
**Wanted:** Illegal cargo: wanted if cop in sight *[VERIFIED — "raises wanted level but not warrant"]*.
**Cooldown:** None explicit (12-game-hour per-run limit is the natural throttle) *[VERIFIED]*.

---

### 6.19 Vehicle Theft ("Gone in 15 Hours")

**Command:** Enter mission checkpoint

**Start locations (VERIFIED)** *[crazybobs-site-full §Vehicle Theft]*:
- Los Santos: Market
- San Fierro: Downtown
- Las Venturas: Redsands West

**Structure (VERIFIED):**
- Locate and deliver **5 randomly-assigned vehicles** to a checkpoint.
- Time limit: **15 game hours** *[VERIFIED]*.
- **CRITICAL last-car warning:** Entering the **last vehicle on the list** triggers instant
  **WL10 (Most Wanted)** *[VERIFIED]*. Strategy: save fastest/closest vehicle for last.
- "Not everyone knows all of the vehicle names" — vehicles identified by sight.

**Payout (VERIFIED — verbatim from official site):**
- **$15,000 bonus per vehicle delivered**
- **$75,000 completion bonus** (all 5)
- **Total potential: $150,000**

**Score:** +1 per car *[VERIFIED]*.
**Wanted:** WL10 on last car entry *[VERIFIED]*; prior 4 cars raise wanted moderately **[OUR DESIGN]**.
**Cooldown:** **[OUR DESIGN]** 5 game hours (high-value mission).

---

### 6.20 Missions milestone placement

Missions are a large, independent system. Recommended milestone placement:

| Mission group | Milestone | Rationale |
|---------------|-----------|-----------|
| **Routine Patrol, Domestic Disturbance** (cop-only) | **M3** | Core CnR cop loop; minimal civilian dependency |
| **Drug Delivery** | **M4** | Requires wanted system (M3) + drug system (M3); part of money loop |
| **Truck Delivery, Trash Pickup, Food Delivery** (vehicle-based payouts) | **M4** | Straightforward job loop; drives economy testing |
| **Vehicle Theft, Holdup Mission** | **M5** | Depend on robbery system (M5) and wanted escalation |
| **Bank Robbery, Airport Robbery** | **M5** | Part of robbery milestone |
| **Paperboy, Flower Delivery, Lawn Mowing, Tractor Mission** | **M6** | Ambient/low-stakes; fill out job variety |
| **Courier Delivery, House Delivery, Illegal Immigrant Transport** | **M6** | Smuggling/transport; needs housing (M5) for House Delivery |
| **Pickpocket Mission, Sexual Encounter** | **M6** | Skill-gated; part of Skills milestone |
| **Race Challenges** | **M6** | Standalone; no crime/economy dependency |

---

## 7. Player commands (from the roster)

Unless noted, lands in **`cmds/player.inc`**. All require `pLoggedIn`
(enforced globally in `OnPlayerCommandReceived`).

| Command | Alias | Module | Spec |
|---|---|---|---|
| `/weapons` | `/ws` | `cmds/shop.inc` | Weapon dealer / Ammunation buy menu (textdraw). Civ needs **Gun Permit ($12,000)** *[economy §2.2]*. Prices market-scaled by `AMMUNATION` stock (§8). Arms Dealer player sells to others (+wanted if cop near, no permit). Bodyarmor $800, Gun Permit $12,000, Pistol100 $4,000, Shotgun100 $8,000, Chainsaw $20,000, Weapon Sales Permit $40,000. |
| `/moneybag` | — | `systems/moneybag.inc` | If carrying a bag → drop it; else → show active hidden-bag **zone hint** (coarse), warmer/colder as you approach *[economy §5]*. First to pickup → reward `random(250k..2M)` + 1 score, announced. Timeout relocates. |
| `/moneyrush` | `/mr` | `systems/moneyrush.inc` | Join the active money-rush event (started by owner `/startmoneyrush`). Cash-rain pickups `random(1k..25k)`, PvP allowed, per-event leaderboard *[economy §6.2]*. |
| `/radio` | `/cnrradio` | `cmds/player.inc` | Dialog of preset stream URLs → `PlayAudioStreamForPlayer`; "stop" → `StopAudioStreamForPlayer` *[jobs §4.3]*. |
| `/rape` | `/ra` | `cmds/crimes.inc` | §4.2 |
| `/rob` | `/rb` | `cmds/crimes.inc` | §4.1 |
| `/lock` | `/lk` | `cmds/vehicle.inc` | Lock owned vehicle (alarm) — `SetVehicleParamsForPlayer` doors for all but owner/coowners; Car Jacker skill defeats locks *[jobs §7.2]*. |
| `/unlock` | `/ulk` | `cmds/vehicle.inc` | Deactivate. |
| `/takedrugs` | `/td` | `systems/drugs.inc` | §4.3 |
| `/ad` | — | `cmds/player.inc` | **[OUR DESIGN naming]** Roster says "paid advertisement," but in CB `/ad`=**adrenaline** *[jobs §4.2]*. **Decision: implement paid advert as `/advert`; keep `/ad`=adrenaline** (full heal + cure all disease + OD save; carry 3). `/advert [text]`: server-wide coloured line, cost **$200**, cooldown **90s**, cap 100 chars, block URLs/other-server names *[jobs §4.2]*. Owner ratifies (§Open Q). |
| `/jumpkick` | — | `cmds/player.inc` | `ApplyAnimation` jump-kick; small melee dmg to player directly in front `<2m` *[jobs §7.1]*. |
| `/shop` | — | `cmds/shop.inc` | 24/7 shop menu at a store actor: Sprunk $23, Condom $400, Seeds $1,200, Body Armor $800, Parachute $2,400, etc. *[economy §2.1]*. Prices market-scaled by `SUPA_SAVE` stock (§8). |
| `/drylake` | `/d` | `cmds/teleport.inc` | Teleport Dry Lake (DM zone flag on). §7.6 |
| `/sfairport` | `/sfa` | `cmds/teleport.inc` | Teleport SF Airport. |
| `/bayside` | `/bs` | `cmds/teleport.inc` | Teleport Bayside. |
| `/lossantosdm` | `/lsadm` | `cmds/teleport.inc` | Teleport LS DM zone (DM flag on, anti-parachute). |
| `/palominocreek` | `/pc` | `cmds/teleport.inc` | Teleport Palomino Creek. |
| `/pm` | — | `cmds/messages.inc` | `<id> <text>` private message; honour `/nopm`, `/ignore`, flood throttle *[jobs §4.1]*. **M2.** |
| `/reply` | `/r` | `cmds/messages.inc` | reply to last PM sender (`pLastPMFrom`). **M2.** |
| `/goto` | — | `cmds/admin.inc` | **Staff only** (`SERVER_MODERATOR+`) *[jobs §7.3]* — never all players (breaks the chase loop). Teleport to player. **M2.** |

**Teleport commands** all: block if jailed/wanted-in-pursuit, set VW/interior 0,
apply DM-zone flag if destination is a DM zone (enables anti-parachute + suppresses
arrest/wanted while inside). *[jobs §6.2–6.3]*

---

## 8. DJ / Owner / Scripter commands

### 8.1 DJ (`pDj`) — `cmds/dj.inc`
| Command | Gate | Spec |
|---|---|---|
| `/djradio` | `pDj` | Broadcast a stream URL + now-playing text to all listeners (`PlayAudioStreamForPlayer` for all) + chat announce via `SendMessageToDJ`/global *[jobs §4.3]*. |

### 8.2 Owner (`SERVER_OWNER`) — `cmds/owner.inc`
| Command | Spec |
|---|---|
| `/atime` | Set/advance the game clock (`GameHour`/`GameMinute`). Guard the lotto/market tick (§10.3). |
| `/startmoneyrush` | Trigger the money-rush event (§7). Cooldown; can force-end. |
| `/makeadmin <id> <0-4>` | Set `players.Rank` to a staff tier ≤ HEAD_ADMIN; DB-write + reload. |
| `/makeregular <id> [0/1]` | Toggle `players.RegularPlayer`. |
| `/makedj <id> [0/1]` | Toggle `players.Dj`. |
| `/makedonator <id> [0/1]` | Toggle `players.Vip`. |

All grants: `UPDATE players SET … WHERE aID=…`, then refresh the online player's
`PlayerInfo`. **No name-based logic anywhere** (recorded decision).

### 8.3 Scripter (`SERVER_SCRIPTER`) — `cmds/scripter.inc`
| Command | Spec |
|---|---|
| `/addmoneybag` | Spawn a money bag at chosen/random location, set/random reward (testing/events) *[economy §5.2]*. |
| `/alotto` | Force/seed a lotto draw for testing (§10.3) *[economy §4.2]*. |
| `/addvehicle` | Spawn + persist a vehicle into `LSvehicles` (extends existing vehicle system). |
| `/addinterior` | Interior builder (already exists via `pAddingInterior` flow) — expose the entry command here. |
| `/lgoto <x> <y> <z>` | Coordinate teleport *[jobs §7.3]*. |
| `/makeowner <id>` | Set `players.Rank = SERVER_OWNER`. |
| `/makescripter <id>` | Set `players.Rank = SERVER_SCRIPTER`. |
| `/debug` | Toggle debug output (paths, wanted, market ticks). |

> Removed forever: name-bound `/getscripter` (PLANNED note; recorded security
> decision). Scripter is reached only via `/makescripter` (DB write).

---

## 9. Systems roster (from PLANNED "Systems" + "CB-style systems")

Each is a milestone deliverable; specs below are the target design, cited.

### 9.1 Housing (`systems/housing.inc`, M5) *[economy §3]*
Buy `/buyhouse` at house pickup (bank value; police -10%); own 2 / co-own 8 /
rent 10; storage (money dodges wealth tax → 6% storage tax); car-save; `/house`,
`/houses/hlist`, `/rent/rlist`, `/hotel(s)`, `/houserob/hrob`, `/housekeys`,
`/housecoowner`. DB: `houses` table (§9). House prices market-scaled by
`HOUSE_MARKET` + prime rate (§8).

### 9.2 Fishing (`systems/fishing.inc`, M6) *[economy §8]*
`/fish` on a boat; Fishing Rod ($20,000) faster+higher catch; Fishing Permit
(carry 50, 1 fish/permit, avoids wanted near cops); `/fishsellall` at 24/7/Bait;
`/fishsell/fp/fishbuy` (Fish Sales Permit $20,000). `fishSellPrice = weightLb *
rarity * FISH_MARKET rate`; selling **feeds** the Fish stock (supply pushes price
down, factor 0.15). Bonus fish 05:00, tournaments 04:00–20:00. DB: `LSplayers`
fish cols + `fish_catch` log.

### 9.3 Farming / drug growing (`systems/farming.inc`, M6) *[economy §9]*
Illegal: seeds ($1,200) → `/plant` → ~20 min grow → up to 200g → `/harvest` →
`/drugsell` at Refill Point; max 5 plants; deer/hippie threats; Hunting Permit
(kill deer w/o = +6 wanted); `/fertilize` faster but attracts deer; `/pgps`,
`/plantinfo`. Legit: CrazyBob's Farm job (harvester/plow route, $500–$2,000/field).
Feeds `DRUG` stock. DB: `plants` table (§11).

### 9.4 Stock market (`systems/stocks.inc`, M7) *[economy §7]* — **headline feature**
21 stocks; trading hours game **07:00–19:00** (frozen outside); `/markets`,
`/shares/stocks`, `/sharessell`; own ≤100,000 shares/company; 1% trading fee.
Price formula + activity-fed pool (§10). Market tick on the **game clock** each
report period (game day). DB: `stocks`, `stock_holdings`, `stock_reports` (§11).

### 9.5 Lotto (`systems/lotto.inc`, M4) *[economy §4]*
`/lotto` pick 1–125, draws **game 18:00**; jackpot base 1,000,000 + ticket sales
+ rollover; win → split + **+5 score**, +1 participation *[primary §8]*. Ticket
$5,000. **Draw bound to the game-clock hour transition** (§10.3 — the bug fix).
DB: `LSplayers.LottoNumber`, `server_data.LottoJackpot`.

### 9.6 DM systems (M6) *[jobs §6]*
- **DM Stadium (DMS)** `systems/dms.inc`: `/enter`/`/exit`, $5,000 entry,
  $1,000/death, fixed weapon kit, isolated VW, Top10 streak +1 + cash. No life
  loss.
- **Sniper arena** `systems/sniper.inc`: same framework, sniper-only kit.
- **Duel** `systems/duel.inc`: `/duel [id] [stake]` → `/accept`, escrow stake,
  isolated VW, countdown, winner takes pot; quit mid-duel = forfeit.
- **Teleport DM zones** (§7 `/drylake` etc.): DM flag → **anti-parachute**
  (strip weapon 46 on enter + per-tick), no wanted/arrest inside.

### 9.7 Clothes (`systems/clothes.inc`, M6) *[economy §2.5]*
6 chains (ZIP/Binco/…); buying an outfit writes `pSkinsSelected[MAX_PEDS]`
(existing array) — purchase = unlock+equip that ped/skin; Barber = hair.
**Crowbar is a clothing-slot item** (§5.7); `systems/clothes.inc` manages the slot
interaction; `systems/robbery.inc` reads the equip flag for crime effects.

### 9.8 Skills & fighting styles (`systems/skills.inc`, M6) *[jobs §2]*
Weapon skills: `SetPlayerSkillLevel(...,999)` on spawn (classic feel).
Fighting styles: 3 gyms (Ganton/Garcia/Redsands East), 3 styles, small fee $500;
persist `pFightStyle` (DB), re-apply on spawn.

### 9.9 GPS destination categories (M6) — engine exists, handlers stubbed
`server/gps.inc` already has the category enum (`GPS_ROBBERY, GPS_MISSION,
GPS_C_CITY_HALL, GPS_C_PD, GPS_C_HOSPITAL, GPS_C_BANK, GPS_C_24_7, GPS_C_BAIT,…`).
Wire each category to the nearest matching **actor** (the 26 actor types already
map: Bank→GPS_C_BANK, PD→GPS_C_PD, Hospital→GPS_C_HOSPITAL, 24-7→GPS_C_24_7, etc.).

### 9.10 Interiors / zones gameplay hooks (M6)
Interiors builder exists (`pAddingInterior`); zones exist (466). Add **DM-zone
flags** + **safe-zone (no-crime) flags** to zone/area records for §2.5.

### 9.11 Fixes (M2/M3)
- **Fire death "grilled"** *[roster]*: in `OnPlayerDeath`, if `reason` is a fire
  weapon (37 Flamethrower / 42 / WEAPON_FIRE 44), format the death/kill message
  as "grilled." Lands in `CnR.pwn` death handler.
- **GameModeClock lottery-timer bug** *[roster + economy §4.2]*: see §10.3.

---

## 10. Economy integration table  *(the "one economy" model)*  *[economy §7.3]*

### 10.1 Prime rate + per-domain multiplier (single source of truth)
Central `systems/economy.inc`:
```
mult(domain) = clamp( stockPrice(domain) / IPO_PRICE(domain), 0.5, 2.0 )
primeRateFactor              // moves all prices; higher prime = lower goods, higher houses
shopPrice(item)  = base * mult(item.stock)
weaponPrice      = base * mult(AMMUNATION)
housePrice       = base * mult(HOUSE_MARKET) * primeRateFactor
fishSellPrice    = weightLb * rarity * mult(FISH_MARKET)
```

### 10.2 Which prices the market multiplies (the ask)
| Price domain | Multiplied by stock | Feeds pool on activity (factor) |
|---|---|---|
| **24/7 items** (`/shop`) | `SUPA_SAVE` | buy at 24/7 → +0.25 *[economy §7.2]* |
| **Weapons** (`/weapons`, Ammunation) | `AMMUNATION` | weapon buy → +0.25 |
| **Houses** (`/buyhouse`) | `HOUSE_MARKET` × prime | each sale raises value |
| **Fish** (`/fishsellall/fishsell`) | `FISH_MARKET` | selling fish → +0.15 (supply pushes down) |
| **Crops/drugs** (`/drugsell`) | `DRUG` | drug sale → factor (dumping lowers) |
| Vehicles (dealer) | `VEHICLE_DEALERSHIP` | buy +0.01, mods +0.025, jack-sell +0.25 |
| Arrests/bail/tickets | `GOVERNMENT` | +0.1 |

Every relevant transaction calls
`StockMarket_UpdateEarnings(stockid, cashAmount, factor)` (clamp POOL ≥ 0).

### 10.3 Market tick + lotto draw tied to GameModeClock (**the clock bug fix**)
*[economy §4.2, §7.2, §11]* — The legacy "lottery timer bug" is that the draw is
bound to the in-game clock, not a real-time timer. **Fix:** fire both the lotto
draw and the stock report **inside the game-clock hour-transition** in
`GameModeClock()` (`CnR.pwn`), guarded against double-fire and skipped hours:

```pawn
// inside GameModeClock(), when GameMinute hits 0 (hour transitions):
static lastLottoHour = -1, lastMarketDay = -1;
if (GameHour == 18 && lastLottoHour != GameDay) { Lotto_Draw(); lastLottoHour = GameDay; }
if (GameHour == 0  && lastMarketDay != GameDay) { StockMarket_Tick(); lastMarketDay = GameDay; }
// trading window: prices frozen unless 07 <= GameHour < 19
```
- Guard vars reset per game-week rollover.
- `/alotto` (scripter) calls `Lotto_Draw()` directly for testing.
- Horse races every 2 game hours (odd hours) hang off the same transition.

---

## 11. DB schema additions  (`scriptfiles/cnr.sql`)

Follow the existing style (InnoDB, per-city tables mirror `LSplayers`, FKs to
`players.aID`). SF/LV clone later. **No `players` structural change needed for
SCRIPTER** (Rank is INT). Additions:

### 11.1 `players` (global) — none required (Rank INT already holds 7).

### 11.2 `LSplayers` — add columns (per-city gameplay state)
```sql
ALTER TABLE `LSplayers`
  ADD `BankMoney`       INT NOT NULL DEFAULT 0,
  ADD `Score`          INT NOT NULL DEFAULT 0,
  ADD `Wanted`         INT NOT NULL DEFAULT 0,
  ADD `Skill`          INT NOT NULL DEFAULT 0,   -- pSkill enum
  ADD `CopRank`        INT NOT NULL DEFAULT 0,   -- 0..10
  ADD `Jailed`         TINYINT(1) NOT NULL DEFAULT 0,
  ADD `JailTime`       INT NOT NULL DEFAULT 0,
  ADD `Bail`           INT NOT NULL DEFAULT 0,
  ADD `JailReason`     VARCHAR(64) NOT NULL DEFAULT '',
  ADD `EscapeChained`  TINYINT(1) NOT NULL DEFAULT 0,
  ADD `SeriousCrimes`  INT NOT NULL DEFAULT 0,
  ADD `Drugs`          INT NOT NULL DEFAULT 0,
  ADD `DrugSeeds`      INT NOT NULL DEFAULT 0,
  ADD `STDs`           INT NOT NULL DEFAULT 0,   -- bitmask
  ADD `Condoms`        INT NOT NULL DEFAULT 0,
  ADD `ChastityBelt`   TINYINT(1) NOT NULL DEFAULT 0,
  ADD `FishPermits`    INT NOT NULL DEFAULT 0,
  ADD `HuntPermits`    INT NOT NULL DEFAULT 0,
  ADD `GunPermit`      TINYINT(1) NOT NULL DEFAULT 0,
  ADD `FightStyle`     INT NOT NULL DEFAULT 4,
  ADD `LottoNumber`    INT NOT NULL DEFAULT 0,
  ADD `TaxOwed`        INT NOT NULL DEFAULT 0,
  ADD `LifeInsurance`  INT NOT NULL DEFAULT 0,
  ADD `HealthInsExpiry` INT NOT NULL DEFAULT 0,
  ADD `PlayTimeMins`   INT NOT NULL DEFAULT 0,   -- for auto-Regular
  -- Robbery system (M5, §5):
  ADD `CrowbarEquipped` TINYINT(1) NOT NULL DEFAULT 0,
  ADD `RobberyHistory`  INT NOT NULL DEFAULT 0,  -- bitmask: ROB_HOLDUP|ROB_BANK|ROB_CASINO|ROB_HOUSE|ROB_SPECIAL
  -- Drug bag (M3, §16 follow-up):
  ADD `HasDrugBag`      TINYINT(1) NOT NULL DEFAULT 0;
```

### 11.3 New tables (from M2 onwards)
```sql
CREATE TABLE `houses` ( ID INT AUTO_INCREMENT, OwnerAID INT, Price INT,
  X FLOAT, Y FLOAT, Z FLOAT, Interior INT, VirtualWorld INT, ForSale TINYINT(1),
  Rent INT, StorageMoney INT, StorageJSON TEXT, LastVisited DATETIME,
  PRIMARY KEY(ID) ) ENGINE=InnoDB;

CREATE TABLE `stocks` ( ID INT, Name VARCHAR(40), Price DOUBLE, Pool DOUBLE,
  AvailableShares INT, IPOPrice DOUBLE, IPOShares INT, MaxShares INT,
  PRIMARY KEY(ID) ) ENGINE=InnoDB;

CREATE TABLE `stock_holdings` ( aID INT, StockID INT, Shares INT,
  PRIMARY KEY(aID,StockID), CONSTRAINT fk_sh_aID FOREIGN KEY(aID)
  REFERENCES players(aID) ON DELETE CASCADE ) ENGINE=InnoDB;

CREATE TABLE `stock_reports` ( StockID INT, Day INT, Price DOUBLE,
  PRIMARY KEY(StockID,Day) ) ENGINE=InnoDB;   -- keep last 30 periods

CREATE TABLE `plants` ( ID INT AUTO_INCREMENT, OwnerAID INT, X FLOAT, Y FLOAT,
  Z FLOAT, Grams INT, PlantedAt DATETIME, PRIMARY KEY(ID) ) ENGINE=InnoDB;

CREATE TABLE `bans` ( ID INT AUTO_INCREMENT, Type TINYINT, Value VARCHAR(256),
  Reason VARCHAR(128), AdminName VARCHAR(24), Date DATETIME, Expiry DATETIME NULL,
  PRIMARY KEY(ID), KEY idx_value (Value) ) ENGINE=InnoDB;  -- native ban system (M2)
```

### 11.4 `server_data` — add
```sql
ALTER TABLE `server_data`
  ADD `LottoJackpot`  INT NOT NULL DEFAULT 1000000,
  ADD `PrimeRate`     DOUBLE NOT NULL DEFAULT 1.0,
  -- Bank robbery per-branch cooldowns (M5, §5.3):
  ADD `BankRobLastLS` DATETIME NULL DEFAULT NULL,
  ADD `BankRobLastSF` DATETIME NULL DEFAULT NULL,
  ADD `BankRobLastLV` DATETIME NULL DEFAULT NULL;
```

---

## 12. Milestones M2–M7  (dependency-ordered, each ships compile-green)

| M | Theme | Depends on | Ships |
|---|---|---|---|
| **M2** | **Command/rank/moderation infra** | M1 (done) | SCRIPTER tier, PM/reply, full admin suite, ajail **FIXED**, aduty, aka/ips (+offline), **native ban system** (replaces legacy Bans FS), teleport cmds + DM-zone flags, `/showcommands`, owner/scripter grant cmds. **See §13 for every spec.** |
| **M3** | **Core CnR loop** | M2 | Wanted system (§2), cop set + arrest (§3.1), jail (§3.2 — `JURY_SIZE=15`), `/rob` `/rape` (§4.1–4.2), STD/drugs core (§4.2–4.3), fire "grilled" fix, spawn protection, safe/DM zones. Routine Patrol + Domestic Disturbance missions (§6.3, §6.14). **See §16 for JURY_SIZE follow-up.** |
| **M4** | **Money & events + core missions** | M3 | Bank (`/deposit`/`/withdraw`/`/atm`/`/givecash`), lotto (§9.5) + **clock-tick fix (§10.3)**, moneybag/moneyrush (§7), `/ad` adrenaline + `/advert`, taxes/insurance, holdup base (§5.2). Drug Delivery + Truck Delivery + Trash Pickup + Food Delivery missions (§6.4, §6.18, §6.17, §6.6). |
| **M5** | **Property & bigger crime + robbery missions** | M4 | Housing (§9.1), kidnap (§4.4), **full robberies system (§5: holdup/bank/casino/house/special + crowbar §5.7)**, vehicle lock/unlock + ownership. Vehicle Theft + Holdup Mission + Airport Robbery missions (§6.19, §6.7, §6.1). Crowbar as clothing/item dependency on M5 clothes stub. |
| **M6** | **Jobs, activities, arenas + remaining missions** | M5 | Skills+`/skill`, fighting styles, clothes (§9.7 — crowbar slot live), fishing (§9.2), farming (§9.3), DMS/duel/sniper + anti-parachute (§9.6), GPS categories (§9.9), radio/DJ, jumpkick, interiors/zones hooks. Remaining missions: Paperboy, Flower Delivery, Lawn Mowing, Tractor Mission, Courier Delivery, House Delivery, Illegal Immigrant Transport, Pickpocket Mission, Sexual Encounter, Race Challenges (§6.2, §6.5, §6.8–§6.13, §6.15–§6.16). |
| **M7** | **Stock market + full economy** | M4,M6 | Stocks (§9.4), prime-rate wiring, market multipliers on all domains (§10.2), dividends/bankruptcy, business shares. |

Each milestone is independently shippable and must build 0/0.

---

## 13. M2 — full command specs (start implementing now)

New files: `cmds/messages.inc` (PM), `cmds/admin.inc`, `cmds/teleport.inc`,
`cmds/owner.inc`, `cmds/scripter.inc`, `systems/bans.inc` (native ban), and the
`SERVER_SCRIPTER` enum addition. Add all to `CnR.pwn` include block
(both `WINDOWS_COMPILER` branches). Register `pLastPMFrom`, `pAdminDuty`,
`pShowCommands`, `pPMFloodTime` in `pInfo`.

### 13.1 Messaging
| Command | Alias | Gate | Spec |
|---|---|---|---|
| `/pm` | `/msg` `/m` | player | `/pm <id> <text>`. Send to target (unless target `/nopm` or has ignored sender). Echo to both; store `pLastPMFrom[target]=sender`. **Flood throttle:** reject if `<PM_FLOOD_MS 1500` since last PM. Staff see PMs via a spy flag (optional). `OnPlayerCommandPerformed` already special-cases `pm` — keep that. |
| `/reply` | `/r` | player | `/reply <text>` → PM `pLastPMFrom`; error if none. |
| `/nopm` | — | player | toggle `pNoPM`; DB `LSplayers`? → keep session-only (no DB needed M2). |
| `/ignore` | `/mute` | player | `/ignore <id>` toggle mute of a player's chat/PM (session array). |

### 13.2 Admin suite (from the roster)
Gate at `SERVER_MODERATOR` unless noted; use `SendAdminAnonmityMessage` for
anonymity (exists) and `p_AdminNick`.
| Command | Gate | Spec |
|---|---|---|
| `/a <text>` | MOD | Admin chat → `SendMessageToModerators`. |
| `/showcommands` | player | Toggle `pShowCommands`; also **`/showcommands` prints the command list** (dialog paginated). Re-enable the commented `OnPlayerCommandPerformed` echo loop guarded by `pShowCommands`. |
| `/ajail <id> [mins] [reason]` | MOD | **FIXED.** Admin-jail: teleport to admin-jail interior/VW, freeze, set `pAdminJailed`+timer. Legacy was "not working" — reimplement cleanly with a per-player timer that auto-releases and a `/ajailed` list. Store nothing in DB (session) or `LSplayers.AdminJailUntil` for persistence across relog. |
| `/ajailed` | MOD | List currently admin-jailed players + remaining time. |
| `/aduty` | MOD | Toggle `pAdminDuty` (on-duty admin: shows admin name colour, enables anonymity off). |
| `/aka <id>` | MOD | Show **also-known-as** (all usernames sharing this player's IP/serial) — query `players` by `Latest_IP`/`Latest_Serial`. |
| `/ips <id>` | MOD | Show IP history for an online player. |
| `/offlineaka <name>` | MOD | `/aka` for an offline account (query by name → IP/serial → matches). |
| `/offlineips <name>` | MOD | `/ips` for an offline account. |
| `/setarmour <id> <0-100>` | MOD | `SetPlayerArmour`. |
| `/sethealth <id> <0-100>` | MOD | `SetPlayerHealth`. |
| `/setvirtualworld <id> <vw>` | ADMIN | `SetPlayerVirtualWorld`. |
| `/setinterior <id> <int>` | ADMIN | `SetPlayerInterior`. |
| `/aflip <id>` | MOD | Flip target's vehicle upright. |
| `/move <id>` | MOD | Teleport target to admin (bring). |
| `/goto <id>` | MOD | Teleport admin to target. |
| `/disarm <id>` | MOD | `ResetPlayerWeapons`. |
| `/arm <id> <weap> [ammo]` | ADMIN | `GivePlayerWeapon`. |
| `/rangearm <weap> [ammo] [range]` | ADMIN | Arm all players in range. |
| `/rangedisarm [range]` | ADMIN | Disarm all in range. |
| `/count <secs>` | MOD | Broadcast a countdown. |
| `/areacount [range]` | MOD | Count players in range. |
| `/aslap <id>` | MOD | Small damage + Z-bump. |
| `/osearch <id>` | MOD | Show target's weapons/cash/inventory (offline-safe: online only in M2). |
| `/healthhack <id>` | MOD | View HP-hack suspicion flags (anti-cheat list; heuristic in M3) *[crime-cop-loop §8]*. In M2 stub the list view. |
| `/aimbotters` | MOD | View aimbot suspicion list (stub in M2) *[crime-cop-loop §8]*. |
| `/clearaimbotters` | ADMIN | Clear the aimbot flag list. |

> `/healthhack`, `/aimbotters`, `/clearaimbotters`, `/osearch` are on the roster's
> admin list — wire the **command shells + flag storage** in M2; the detection
> heuristics land in M3 (`OnPlayerTakeDamage`) *[crime-cop-loop §8 — command names
> INFERRED]*.

### 13.3 Native ban system (`systems/bans.inc`) — replaces legacy Bans FS
Recorded decision: `CallForChecking` short-circuits to `BanCheckDone(playerid,0)`;
legacy `Bans.pwn` has burned credentials — **never reuse them**.
- **`bans` table** (§11.3): Type (0 name, 1 IP, 2 serial), Value, Reason, AdminName,
  Date, Expiry.
- **`CallForChecking`** → async `SELECT` from `bans` matching name/IP/serial →
  `BanCheckDone(playerid, banned)` (banned=1 shows `BAN_DIALOG` then kicks).
- Commands (MOD+): `/ban <id> [reason]`, `/unban <name>`, `/unbannick <name>`,
  `/unbanid <banID>`, `/offlineban <name>`. All log via `IRC_SendMessage`
  (no-op today → future Discord bridge). No IRC dependency.

### 13.4 Teleport commands + DM-zone flags (`cmds/teleport.inc`)
Implement `/drylake`, `/sfairport`, `/bayside`, `/lossantosdm`, `/palominocreek`
(+ aliases). Each: `SetPlayerPos` to a fixed coord table, VW/interior 0. Attach a
**`bool:pInDMZone`** flag; entering a DM-flagged destination enables anti-parachute
(strip weapon 46, block re-give) and suppresses wanted/arrest inside (§9.6). Block
teleport while jailed or in active pursuit (`pVCFreezeUntil` set by a cop).

### 13.5 Owner/scripter grant commands
Implement §8.2 (`/atime`, `/makeadmin`, `/makeregular`, `/makedj`, `/makedonator`,
`/startmoneyrush` shell) and §8.3 (`/makeowner`, `/makescripter`, `/lgoto`,
`/debug`, `/addvehicle` shell). All grants: `UPDATE players` + refresh online
`PlayerInfo`. `/startmoneyrush` may be a shell in M2 (full event in M4).

---

## 14. Open questions for the owner

1. **Economy tuning** — ratify starting bank ($50k), price baselines (§7/§10),
   robbery caps ($50k/$100k/$500k by skill), jail sentence/bail constants,
   wanted-decay rate (180s), and the wealth-tax threshold (CB used $7M–$10M
   *[economy §1.4, primary §15]*).
2. **`/ad` naming** — accept `/ad`=adrenaline + `/advert`=paid ad *[jobs §4.2]*,
   or override `/ad` to mean the paid advert? (Spec assumes the former.)
3. **LS-only vs all cities now** — which features ship LS-only for M2–M6 vs wait
   for SF/LV? (Housing, stocks, RPC, DMS are per-city; SF/LV tables aren't
   populated. Recommend LS-only through M7.)
4. **Regular auto-award thresholds** — score + playtime formula for auto-Regular
   *[jobs §5.1]* (CB never published numbers *[primary §15]*).
5. **Jury size** — ~~12 (wiki) or 15 (official)?~~ **RESOLVED: 15** per the official
   crazybobs.net site *[crazybobs-site-full §Corrections #1]*. `JURY_SIZE = 15`
   in `systems/jail.inc`. See §16 for the code follow-up task.
6. **Weapon-skill model** — max 999 on spawn (classic) or level-gated progression?
   *[jobs §2.1]*
7. **Donator strength** — CB-faithful (tax break + `/vehcolor` + name colour) or
   stronger VIP (spawn armour, lounge)? *[jobs §5.2]*. Note: the official CB site
   grants **no perks** to donors *[crazybobs-site-full §Donations]*. Any VIP perks
   we add are **[OUR DESIGN]** and should be framed as admin-granted status, not
   tied to real-money donations in the server UI.
8. **Anti-cheat auto-ban** — keep evidence-gated/manual (CB policy) or allow
   auto-ban on strong heuristics? *[crime-cop-loop §8]*
9. **Drive-by** — add as a distinct crime or fold into attack+wanted? *[§2.3]*
10. **Discord bridge target** — wire the `IRC_Send*` no-ops to Discord now or later?

---

## 15. Executive summary

1. **Every** roster command/feature is specified with syntax, price, cooldown,
   wanted effect, rank gate, DB column, and target module file, each cited to
   `docs/research/*.md` or marked `[OUR DESIGN]`.
2. **New tiers:** add `SERVER_SCRIPTER (7)` above OWNER in the rank enum;
   DONATOR/REGULAR/DJ stay bools. **All grants DB-driven only — zero name-based
   backdoors** (recorded security decision).
3. **Wanted system (`systems/wanted.inc`)** is the core dependency: stacking 0–10,
   3 tiers with verbatim colours, per-crime values, decay (paused by `/visualcontact`),
   ticket→warrant escalation, safe/DM zones.
4. **Minimal cop set + jail** (arrest/ticket/report/vc/backup/parole/refill +
   bail/appeal/bribe/escape/breakout) delivers CB arrest→jail parity.
   **Jury size confirmed as 15** per official site; `JURY_SIZE` constant must reflect
   this — see §16 for code follow-up.
5. **Crime commands** (`/rob` skill-capped $50k/$100k/$500k, `/rape`+STDs,
   `/takedrugs` with >60g overdose + adrenaline save) reconstruct the CB
   crime/drug/STD triangle.
6. **Robberies system (§5):** 6 robbery types (shoplifting, store holdup, bank,
   casino, house, special/v23) fully spec'd with wanted values, cooldowns, DB state,
   and CB-verified payouts. **Crowbar (§5.7)** is a clothing-slot item ($40,000),
   dual-effect (reduces timer + increases per-second yield/success chance); confiscated
   on arrest if robbery history is flagged; kept if killed by cop lethal force.
7. **Missions system (§6):** All 19 CB missions spec'd with start locations,
   structure, verified payouts, wanted effects, and cooldowns. Cop-only missions
   (Routine Patrol, Domestic Disturbance) in M3; high-value crime missions (Vehicle
   Theft, Holdup, Airport Robbery) in M5; remaining 11 missions in M6.
8. **Economy is one system:** a prime-rate + per-stock multiplier scales 24/7
   items (SUPA_SAVE), weapons (AMMUNATION), houses (HOUSE_MARKET×prime), fish
   (FISH_MARKET), crops (DRUG); activity feeds each stock's pool.
9. **Clock fix:** lotto (18:00) and the stock report tick fire inside
   `GameModeClock()`'s guarded hour-transition — resolving the legacy lottery-timer
   bug and giving the market its tick.
10. **DB additions:** ~27 `LSplayers` gameplay columns (adds `CrowbarEquipped`,
    `RobberyHistory`) + `houses/stocks/stock_holdings/stock_reports/plants/bans`
    tables + `server_data` jackpot/prime/bank-rob-cooldown cols.
11. **M2 (ready now):** SCRIPTER tier, PM/reply, full admin suite, **ajail FIXED**,
    aduty, aka/ips (+offline), **native ban system replacing the legacy Bans FS**,
    teleport cmds + DM-zone flags, `/showcommands`, owner/scripter grants — each
    spec'd in §13 for immediate implementation.
12. **M3–M7** are dependency-ordered and each ships compile-green: core loop →
    money/events + core missions → property/big crime + robbery missions →
    jobs/arenas + remaining missions → stock market. Open questions (§14) isolate
    exactly what only the owner can decide (prices, city scope, naming, thresholds).
    **JURY_SIZE and drug bag source** are resolved (see §16).

---

## 16. Code follow-ups from site research

These are constants or behaviours in **already-built or M3-targeted** code that the
official crazybobs.net site has now clarified. Apply during the **M3 review pass**.
Do NOT edit the source files now — this section is the authoritative TODO list.

| # | File | Change | Source | Priority |
|---|------|--------|--------|----------|
| 1 | `systems/jail.inc` | Change `#define JURY_SIZE 12` → `#define JURY_SIZE 15`. The official site (crimes-jail-sentence page) states verbatim "Appeal sentence to 15 random jurors." The Fandom wiki said 12 — the official site is authoritative. | *[crazybobs-site-full §Corrections #1]* | HIGH — apply in M3 before jail system ships |
| 2 | `systems/drugs.inc` | Drug Dealer carry capacity: confirm base is **2,500g** (Fandom Drug_Dealer), not a flat 5,000g. 5,000g is the cap **with a drug bag obtained only from Drug Refill Points**. If any constant currently reads `DRUG_DEALER_MAX 5000` without a drug bag check, split it: `DRUG_DEALER_BASE 2500` / `DRUG_DEALER_WITH_BAG 5000`. Add `pHasDrugBag` bool to `pInfo` and `LSplayers`. | *[crazybobs-site-full §Corrections #3, §Drug Refill Points]* | HIGH — needed before drug system ships in M3 |
| 3 | `cmds/robbery.inc` (new in M5) | When implementing `/holdup`: the crowbar is NOT a weapon-slot item. It must be read from `pCrowbarEquipped` (clothing flag, §5.7) — not from `GetPlayerWeapon` or any weapon-slot check. Confiscation on arrest: only confiscate if `LSplayers.RobberyHistory` has a robbery flag; if death cause was a cop's lethal force (`OnPlayerDeath`, killer is cop, WL10 context), skip confiscation. | *[crazybobs-site-full §The Crowbar — Complete Mechanics]* | M5 — block on robbery.inc implementation |
| 4 | `systems/jail.inc` | Crowbar confiscation logic must check `LSplayers.RobberyHistory` bitmask before removing. This is distinct from the fishing rod (which is only confiscated if player lacks a permit). Both items are NOT removed if the player was killed by a cop (vs arrested). Wire this in `OnPlayerDeath` + arrest handler. | *[crazybobs-site-full §The Crowbar — Complete Mechanics]* | M5 — coordinate with robbery.inc |
| 5 | `systems/wanted.inc` | Illegal Immigrant Transport mission: wanted is issued on **completion** (successful drop-off), NOT on pickup. If any placeholder comment or stub in the mission handler suggests a wanted-on-pickup model, correct it. | *[crazybobs-site-full §Illegal Immigrant Transport]* | M6 — when mission ships |
| 6 | `systems/missions.inc` (new in M4/M5/M6) | Airport Robbery is a **box-collection mechanic** (MMB / `/box`) in a truck/van/garbage truck, not a simple checkpoint robbery. The trigger is `/robbery` at the airport, not a general checkpoint walk-in. Payout scales with boxes collected. Do not implement as a standard checkpoint mission. | *[crazybobs-site-full §Airport Robbery, §Corrections #6]* | M5 — when airport robbery ships |
| 7 | Any file referencing donation / VIP perks | Donator perks (no wealth tax, `/vehcolor`, name colour) are **not from the official site** — they are community-sourced. In player-facing UI strings, do NOT describe these as "donation rewards" or connect them to real-money payments. They are admin-granted VIP status only. | *[crazybobs-site-full §Donation Page, §Corrections #2]* | M6 — before VIP/donator system ships |
