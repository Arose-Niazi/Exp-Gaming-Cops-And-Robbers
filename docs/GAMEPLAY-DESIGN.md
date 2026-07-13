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
5. Player commands (weapons, moneybag, moneyrush, radio, lock, ad, jumpkick, shop, teleports, pm/reply, goto)
6. DJ / Owner / Scripter commands
7. Systems roster (housing, fishing, farming, stocks, lotto, duel, sniper, DMS, clothes, skills, anti-parachute, GPS, interiors, zones, fixes)
8. Economy integration table (stock market multipliers + market tick + lotto/clock fix)
9. DB schema additions (tables + columns)
10. Milestones M2–M7 (dependency-ordered)
11. **M2 command specs in full** (ready to implement)
12. Open questions for the owner
13. Executive summary

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
| Donator (VIP) | `pVip` | `players.Vip` | `/makedonator` | no wealth tax, `/vehcolor` $100, name colour *[jobs §5.2]* |
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
  2. `/appeal` — jury of `JURY_SIZE (12, config)` votes; innocent → time 0. Blocked
     if `<60s` left or a "serious" (murder) charge. *[§5]*
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
- Carry limits: Drug Dealer 5000g, others 500g *[primary §7]*.
- DB: `LSplayers.Drugs, DrugSeeds`. (Growing/`/plant`/`/harvest` = farming, §7.)

### 4.4 Kidnap / holdup (M4/M5 — spec here for completeness)

- **Holdup** `/holdup` `/hup`: at a shop actor (§ actors), looking at clerk. Set
  WL **+7**, ~10s clerk wait, then **$2,000–$6,000/sec**; crowbar boosts.
  *[primary §5]*. Lands in `cmds/robbery.inc` (M5).
- **Kidnap** `/kidnap [id] [ransom]` (`/kd`): disguise as Driver, lock victim,
  ransom ≤$50k, steal 20–80% on-hand at hideout; always warrant. `/release`,
  `/kidnapall`, `/ransom`. *[crime-cop-loop §2.6]* Lands in `cmds/kidnap.inc` (M5).

---

## 5. Player commands (from the roster)

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

## 6. DJ / Owner / Scripter commands

### 6.1 DJ (`pDj`) — `cmds/dj.inc`
| Command | Gate | Spec |
|---|---|---|
| `/djradio` | `pDj` | Broadcast a stream URL + now-playing text to all listeners (`PlayAudioStreamForPlayer` for all) + chat announce via `SendMessageToDJ`/global *[jobs §4.3]*. |

### 6.2 Owner (`SERVER_OWNER`) — `cmds/owner.inc`
| Command | Spec |
|---|---|
| `/atime` | Set/advance the game clock (`GameHour`/`GameMinute`). Guard the lotto/market tick (§8.3). |
| `/startmoneyrush` | Trigger the money-rush event (§5). Cooldown; can force-end. |
| `/makeadmin <id> <0-4>` | Set `players.Rank` to a staff tier ≤ HEAD_ADMIN; DB-write + reload. |
| `/makeregular <id> [0/1]` | Toggle `players.RegularPlayer`. |
| `/makedj <id> [0/1]` | Toggle `players.Dj`. |
| `/makedonator <id> [0/1]` | Toggle `players.Vip`. |

All grants: `UPDATE players SET … WHERE aID=…`, then refresh the online player's
`PlayerInfo`. **No name-based logic anywhere** (recorded decision).

### 6.3 Scripter (`SERVER_SCRIPTER`) — `cmds/scripter.inc`
| Command | Spec |
|---|---|
| `/addmoneybag` | Spawn a money bag at chosen/random location, set/random reward (testing/events) *[economy §5.2]*. |
| `/alotto` | Force/seed a lotto draw for testing (§8.3) *[economy §4.2]*. |
| `/addvehicle` | Spawn + persist a vehicle into `LSvehicles` (extends existing vehicle system). |
| `/addinterior` | Interior builder (already exists via `pAddingInterior` flow) — expose the entry command here. |
| `/lgoto <x> <y> <z>` | Coordinate teleport *[jobs §7.3]*. |
| `/makeowner <id>` | Set `players.Rank = SERVER_OWNER`. |
| `/makescripter <id>` | Set `players.Rank = SERVER_SCRIPTER`. |
| `/debug` | Toggle debug output (paths, wanted, market ticks). |

> Removed forever: name-bound `/getscripter` (PLANNED note; recorded security
> decision). Scripter is reached only via `/makescripter` (DB write).

---

## 7. Systems roster (from PLANNED "Systems" + "CB-style systems")

Each is a milestone deliverable; specs below are the target design, cited.

### 7.1 Housing (`systems/housing.inc`, M5) *[economy §3]*
Buy `/buyhouse` at house pickup (bank value; police -10%); own 2 / co-own 8 /
rent 10; storage (money dodges wealth tax → 6% storage tax); car-save; `/house`,
`/houses/hlist`, `/rent/rlist`, `/hotel(s)`, `/houserob/hrob`, `/housekeys`,
`/housecoowner`. DB: `houses` table (§9). House prices market-scaled by
`HOUSE_MARKET` + prime rate (§8).

### 7.2 Fishing (`systems/fishing.inc`, M6) *[economy §8]*
`/fish` on a boat; Fishing Rod ($20,000) faster+higher catch; Fishing Permit
(carry 50, 1 fish/permit, avoids wanted near cops); `/fishsellall` at 24/7/Bait;
`/fishsell/fp/fishbuy` (Fish Sales Permit $20,000). `fishSellPrice = weightLb *
rarity * FISH_MARKET rate`; selling **feeds** the Fish stock (supply pushes price
down, factor 0.15). Bonus fish 05:00, tournaments 04:00–20:00. DB: `LSplayers`
fish cols + `fish_catch` log.

### 7.3 Farming / drug growing (`systems/farming.inc`, M6) *[economy §9]*
Illegal: seeds ($1,200) → `/plant` → ~20 min grow → up to 200g → `/harvest` →
`/drugsell` at Refill Point; max 5 plants; deer/hippie threats; Hunting Permit
(kill deer w/o = +6 wanted); `/fertilize` faster but attracts deer; `/pgps`,
`/plantinfo`. Legit: CrazyBob's Farm job (harvester/plow route, $500–$2,000/field).
Feeds `DRUG` stock. DB: `plants` table (§9).

### 7.4 Stock market (`systems/stocks.inc`, M7) *[economy §7]* — **headline feature**
21 stocks; trading hours game **07:00–19:00** (frozen outside); `/markets`,
`/shares/stocks`, `/sharessell`; own ≤100,000 shares/company; 1% trading fee.
Price formula + activity-fed pool (§8). Market tick on the **game clock** each
report period (game day). DB: `stocks`, `stock_holdings`, `stock_reports` (§9).

### 7.5 Lotto (`systems/lotto.inc`, M4) *[economy §4]*
`/lotto` pick 1–125, draws **game 18:00**; jackpot base 1,000,000 + ticket sales
+ rollover; win → split + **+5 score**, +1 participation *[primary §8]*. Ticket
$5,000. **Draw bound to the game-clock hour transition** (§8.3 — the bug fix).
DB: `LSplayers.LottoNumber`, `server_data.LottoJackpot`.

### 7.6 DM systems (M6) *[jobs §6]*
- **DM Stadium (DMS)** `systems/dms.inc`: `/enter`/`/exit`, $5,000 entry,
  $1,000/death, fixed weapon kit, isolated VW, Top10 streak +1 + cash. No life
  loss.
- **Sniper arena** `systems/sniper.inc`: same framework, sniper-only kit.
- **Duel** `systems/duel.inc`: `/duel [id] [stake]` → `/accept`, escrow stake,
  isolated VW, countdown, winner takes pot; quit mid-duel = forfeit.
- **Teleport DM zones** (§5 `/drylake` etc.): DM flag → **anti-parachute**
  (strip weapon 46 on enter + per-tick), no wanted/arrest inside.

### 7.7 Clothes (`systems/clothes.inc`, M6) *[economy §2.5]*
6 chains (ZIP/Binco/…); buying an outfit writes `pSkinsSelected[MAX_PEDS]`
(existing array) — purchase = unlock+equip that ped/skin; Barber = hair.

### 7.8 Skills & fighting styles (`systems/skills.inc`, M6) *[jobs §2]*
Weapon skills: `SetPlayerSkillLevel(...,999)` on spawn (classic feel).
Fighting styles: 3 gyms (Ganton/Garcia/Redsands East), 3 styles, small fee $500;
persist `pFightStyle` (DB), re-apply on spawn.

### 7.9 GPS destination categories (M6) — engine exists, handlers stubbed
`server/gps.inc` already has the category enum (`GPS_ROBBERY, GPS_MISSION,
GPS_C_CITY_HALL, GPS_C_PD, GPS_C_HOSPITAL, GPS_C_BANK, GPS_C_24_7, GPS_C_BAIT,…`).
Wire each category to the nearest matching **actor** (the 26 actor types already
map: Bank→GPS_C_BANK, PD→GPS_C_PD, Hospital→GPS_C_HOSPITAL, 24-7→GPS_C_24_7, etc.).

### 7.10 Interiors / zones gameplay hooks (M6)
Interiors builder exists (`pAddingInterior`); zones exist (466). Add **DM-zone
flags** + **safe-zone (no-crime) flags** to zone/area records for §2.5.

### 7.11 Fixes (M2/M3)
- **Fire death "grilled"** *[roster]*: in `OnPlayerDeath`, if `reason` is a fire
  weapon (37 Flamethrower / 42 / WEAPON_FIRE 44), format the death/kill message
  as "grilled." Lands in `CnR.pwn` death handler.
- **GameModeClock lottery-timer bug** *[roster + economy §4.2]*: see §8.3.

---

## 8. Economy integration table  *(the "one economy" model)*  *[economy §7.3]*

### 8.1 Prime rate + per-domain multiplier (single source of truth)
Central `systems/economy.inc`:
```
mult(domain) = clamp( stockPrice(domain) / IPO_PRICE(domain), 0.5, 2.0 )
primeRateFactor              // moves all prices; higher prime = lower goods, higher houses
shopPrice(item)  = base * mult(item.stock)
weaponPrice      = base * mult(AMMUNATION)
housePrice       = base * mult(HOUSE_MARKET) * primeRateFactor
fishSellPrice    = weightLb * rarity * mult(FISH_MARKET)
```

### 8.2 Which prices the market multiplies (the ask)
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

### 8.3 Market tick + lotto draw tied to GameModeClock (**the clock bug fix**)
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

## 9. DB schema additions  (`scriptfiles/cnr.sql`)

Follow the existing style (InnoDB, per-city tables mirror `LSplayers`, FKs to
`players.aID`). SF/LV clone later. **No `players` structural change needed for
SCRIPTER** (Rank is INT). Additions:

### 9.1 `players` (global) — none required (Rank INT already holds 7).

### 9.2 `LSplayers` — add columns (per-city gameplay state)
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
  ADD `PlayTimeMins`   INT NOT NULL DEFAULT 0;   -- for auto-Regular
```

### 9.3 New tables
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

### 9.4 `server_data` — add
```sql
ALTER TABLE `server_data`
  ADD `LottoJackpot` INT NOT NULL DEFAULT 1000000,
  ADD `PrimeRate`    DOUBLE NOT NULL DEFAULT 1.0;
```

---

## 10. Milestones M2–M7  (dependency-ordered, each ships compile-green)

| M | Theme | Depends on | Ships |
|---|---|---|---|
| **M2** | **Command/rank/moderation infra** | M1 (done) | SCRIPTER tier, PM/reply, full admin suite, ajail **FIXED**, aduty, aka/ips (+offline), **native ban system** (replaces legacy Bans FS), teleport cmds + DM-zone flags, `/showcommands`, owner/scripter grant cmds. **See §11 for every spec.** |
| **M3** | **Core CnR loop** | M2 | Wanted system (§2), cop set + arrest (§3.1), jail (§3.2), `/rob` `/rape` (§4.1–4.2), STD/drugs core (§4.2–4.3), fire "grilled" fix, spawn protection, safe/DM zones. |
| **M4** | **Money & events** | M3 | Bank (`/deposit`/`/withdraw`/`/atm`/`/givecash`), lotto (§7.5) + **clock-tick fix (§8.3)**, moneybag/moneyrush (§5), `/ad` adrenaline + `/advert`, taxes/insurance, holdup base. |
| **M5** | **Property & bigger crime** | M4 | Housing (§7.1), kidnap (§4.4), robberies (bank/casino/holdup full), vehicle lock/unlock + ownership, `/goto` refinement. |
| **M6** | **Jobs, activities, arenas** | M5 | Skills+`/skill`, fighting styles, clothes, fishing (§7.2), farming (§7.3), DMS/duel/sniper + anti-parachute (§7.6), GPS categories (§7.9), radio/DJ, jumpkick, interiors/zones hooks. |
| **M7** | **Stock market + full economy** | M4,M6 | Stocks (§7.4), prime-rate wiring, market multipliers on all domains (§8.2), dividends/bankruptcy, business shares. |

Each milestone is independently shippable and must build 0/0.

---

## 11. M2 — full command specs (start implementing now)

New files: `cmds/messages.inc` (PM), `cmds/admin.inc`, `cmds/teleport.inc`,
`cmds/owner.inc`, `cmds/scripter.inc`, `systems/bans.inc` (native ban), and the
`SERVER_SCRIPTER` enum addition. Add all to `CnR.pwn` include block
(both `WINDOWS_COMPILER` branches). Register `pLastPMFrom`, `pAdminDuty`,
`pShowCommands`, `pPMFloodTime` in `pInfo`.

### 11.1 Messaging
| Command | Alias | Gate | Spec |
|---|---|---|---|
| `/pm` | `/msg` `/m` | player | `/pm <id> <text>`. Send to target (unless target `/nopm` or has ignored sender). Echo to both; store `pLastPMFrom[target]=sender`. **Flood throttle:** reject if `<PM_FLOOD_MS 1500` since last PM. Staff see PMs via a spy flag (optional). `OnPlayerCommandPerformed` already special-cases `pm` — keep that. |
| `/reply` | `/r` | player | `/reply <text>` → PM `pLastPMFrom`; error if none. |
| `/nopm` | — | player | toggle `pNoPM`; DB `LSplayers`? → keep session-only (no DB needed M2). |
| `/ignore` | `/mute` | player | `/ignore <id>` toggle mute of a player's chat/PM (session array). |

### 11.2 Admin suite (from the roster)
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

### 11.3 Native ban system (`systems/bans.inc`) — replaces legacy Bans FS
Recorded decision: `CallForChecking` short-circuits to `BanCheckDone(playerid,0)`;
legacy `Bans.pwn` has burned credentials — **never reuse them**.
- **`bans` table** (§9.3): Type (0 name, 1 IP, 2 serial), Value, Reason, AdminName,
  Date, Expiry.
- **`CallForChecking`** → async `SELECT` from `bans` matching name/IP/serial →
  `BanCheckDone(playerid, banned)` (banned=1 shows `BAN_DIALOG` then kicks).
- Commands (MOD+): `/ban <id> [reason]`, `/unban <name>`, `/unbannick <name>`,
  `/unbanid <banID>`, `/offlineban <name>`. All log via `IRC_SendMessage`
  (no-op today → future Discord bridge). No IRC dependency.

### 11.4 Teleport commands + DM-zone flags (`cmds/teleport.inc`)
Implement `/drylake`, `/sfairport`, `/bayside`, `/lossantosdm`, `/palominocreek`
(+ aliases). Each: `SetPlayerPos` to a fixed coord table, VW/interior 0. Attach a
**`bool:pInDMZone`** flag; entering a DM-flagged destination enables anti-parachute
(strip weapon 46, block re-give) and suppresses wanted/arrest inside (§7.6). Block
teleport while jailed or in active pursuit (`pVCFreezeUntil` set by a cop).

### 11.5 Owner/scripter grant commands
Implement §6.2 (`/atime`, `/makeadmin`, `/makeregular`, `/makedj`, `/makedonator`,
`/startmoneyrush` shell) and §6.3 (`/makeowner`, `/makescripter`, `/lgoto`,
`/debug`, `/addvehicle` shell). All grants: `UPDATE players` + refresh online
`PlayerInfo`. `/startmoneyrush` may be a shell in M2 (full event in M4).

---

## 12. Open questions for the owner

1. **Economy tuning** — ratify starting bank ($50k), price baselines (§5/§8),
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
5. **Jury size** — 12 (wiki) or 15 (official)? Config constant `JURY_SIZE`.
6. **Weapon-skill model** — max 999 on spawn (classic) or level-gated progression?
   *[jobs §2.1]*
7. **Donator strength** — CB-faithful (tax break + `/vehcolor` + name colour) or
   stronger VIP (spawn armour, lounge)? *[jobs §5.2]*
8. **Anti-cheat auto-ban** — keep evidence-gated/manual (CB policy) or allow
   auto-ban on strong heuristics? *[crime-cop-loop §8]*
9. **Drive-by** — add as a distinct crime or fold into attack+wanted? *[§2.3]*
10. **Discord bridge target** — wire the `IRC_Send*` no-ops to Discord now or later?

---

## 13. Executive summary (10 lines)

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
5. **Crime commands** (`/rob` skill-capped $50k/$100k/$500k, `/rape`+STDs,
   `/takedrugs` with >60g overdose + adrenaline save) reconstruct the CB
   crime/drug/STD triangle.
6. **Economy is one system:** a prime-rate + per-stock multiplier scales 24/7
   items (SUPA_SAVE), weapons (AMMUNATION), houses (HOUSE_MARKET×prime), fish
   (FISH_MARKET), crops (DRUG); activity feeds each stock's pool.
7. **Clock fix:** lotto (18:00) and the stock report tick fire inside
   `GameModeClock()`'s guarded hour-transition — resolving the legacy lottery-timer
   bug and giving the market its tick.
8. **DB additions:** ~25 `LSplayers` gameplay columns + `houses/stocks/
   stock_holdings/stock_reports/plants/bans` tables + `server_data` jackpot/prime.
9. **M2 (ready now):** SCRIPTER tier, PM/reply, full admin suite, **ajail FIXED**,
   aduty, aka/ips (+offline), **native ban system replacing the legacy Bans FS**,
   teleport cmds + DM-zone flags, `/showcommands`, owner/scripter grants — each
   spec'd in §11 for immediate implementation.
10. **M3–M7** are dependency-ordered and each ships compile-green: core loop →
    money/events → property/big crime → jobs/arenas → stock market. Open questions
    (§12) isolate exactly what only the owner can decide (prices, city scope,
    naming, thresholds).
