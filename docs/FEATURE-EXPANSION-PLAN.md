# EXP Gaming Cops And Robbers — Feature Expansion Plan (post-M7)

> **What this is:** the staged, implementable spec for the four owner-requested
> additions on top of the shipped M2–M7 build: (1) a **robbery success grind**,
> (2) an in-game **info/help browser** on the existing textdraw menu, (3) a
> **full cop system** (cop chat, dispatch/alerts, 10-codes, missing cop cmds),
> and (4) **missing CB commands** (`/w` and friends). Grounded in the actual
> code (real hook points cited with file + line) and in
> `docs/research/cb-commands-full.md` + `docs/GAMEPLAY-DESIGN.md`.
>
> **Conventions (from `CLAUDE.md` + the tree):** Pawn.CMD `CMD:` / `alias:`;
> cross-module publics use the `FUNCTION` macro + `CallLocalFunction`; MySQL R41
> async (`mysql_pquery` + `cache_*`); LSplayers is the per-city persistence table
> with the single shared `Jail_SavePlayer` UPDATE; build stays **0 errors / 0
> warnings**; recompile + commit `CnR.amx`. New cmd files → `gamemodes/CnR/cmds/`,
> new systems → `gamemodes/CnR/systems/`, includes wrapped in
> `#if defined WINDOWS_COMPILER`.

---

## EXECUTIVE SUMMARY (12 lines)

1. **Robbery grind** adds one persisted stat `SuccessfulRobberies` (+ `FailedRobberies`) to LSplayers, driving a shared `Rob_GrindBonus()` that maps rep onto a slow diminishing curve.
2. The curve starts at a low base per robbery type and climbs to a cap only after **~250–300** successful robberies (e.g. safe-crack: 45% → 90% cap; `/rob` base 55% → 90% cap).
3. Grind hooks replace three hardcoded rolls: `Crime_RobSuccessFor()` (crime.inc:120), the safe-crack `chance` (robbery.inc:335), and `House_RobSteal`'s `HOUSEROB_BASE_SUCCESS` gate; `/holdup` and `/shoplift` gain a rep-scaled fail roll they currently lack.
4. A **failed** robbery costs a short cooldown + small wanted, yields no loot, and increments `FailedRobberies`; success increments `SuccessfulRobberies` and persists via `Jail_SavePlayer`.
5. **Info browser** reuses `ShowTextDrawMenu`/`ShowTextDrawMenuItems` with two new menu ids (`MENU_HELP_CATS`, `MENU_HELP_CMDS`): a category list → a per-category command list → an MSGBOX detail dialog.
6. All help content lives in one static data table in a new `cmds/help.inc` (`g_HelpCat[]` + `g_HelpCmd[]`), so adding a command is a one-line data edit, no UI code.
7. **Cop chat** becomes an always-on toggled channel `/d` (department) built on the existing `Cop_Broadcast`, plus a persisted `pCopChatOn` mute toggle.
8. **Dispatch/alerts** centralise on a new `Dispatch_Alert()` that fans a formatted `[DISPATCH]` line to cops (server-wide or radius), fired on robbery start, holdup, WL9+ suspect proximity, and `/backup`/`/respond`.
9. **10-codes** ship as a curated ~24-code subset expanded inline in any cop-channel message via a `Cop_Expand10Code()` pass plus `$loc/$sus/$veh` quick-strings.
10. **Missing cop cmds**: `/respond` (`/yes`), `/robberies` (`/robs`), `/freeze` (`/fr`,`/pu`), `/crimes`, `/vehrepair`, `/coprank` — each specced onto existing systems.
11. **Missing player/social cmds**: `/w` (`/whisper`, LOCAL proximity chat — NOT PM), `/say`, `/getid`, `/info`(`/i`), `/stats`, `/complain`, `/locate`(`/loc`), `/giverep`, `/tip` — mostly `cmds/messages.inc` + `cmds/player.inc`.
12. Ships in **four dependency-ordered, compile-green stages**: **A** robbery grind, **B** cop chat + dispatch + 10-codes + missing cop cmds, **C** info browser, **D** remaining player/social cmds. Owner decisions flagged in §6.

---

## 1. ROBBERY SUCCESS GRIND

### 1.1 The stat

New persisted columns on `LSplayers`:

```sql
ALTER TABLE `LSplayers`
  ADD `SuccessfulRobberies` INT NOT NULL DEFAULT 0,   -- the grind counter (drives success %)
  ADD `FailedRobberies`     INT NOT NULL DEFAULT 0;    -- informational / anti-farm telemetry
```

- Add both to the CREATE TABLE in `scriptfiles/cnr.sql`.
- `PlayerInfo` enum (`players/player_vars.inc`): add `pSuccessfulRobberies`,
  `pFailedRobberies`.
- **Load** (`loading_data.inc` `LoadUserData`, after the last `cache_get_value_int`,
  e.g. after `CoolerValue`):
  ```pawn
  cache_get_value_int(0, "SuccessfulRobberies", PlayerInfo[playerid][pSuccessfulRobberies]);
  cache_get_value_int(0, "FailedRobberies",     PlayerInfo[playerid][pFailedRobberies]);
  ```
- **Save**: extend the single `Jail_SavePlayer` UPDATE (`jail.inc`) SET list with
  `, SuccessfulRobberies=%d, FailedRobberies=%d` and add the two args before
  `PlayerInfo[playerid][pID]`. **The format buffer is 832 chars (comment on
  jail.inc:108) — the two new `=%d` clauses add ~40 chars; widen the buffer to
  ~900 to keep headroom** (verify at implementation time).

> Naming call: spec uses `SuccessfulRobberies` (self-documenting) rather than
> the shorter `RobberyRep` the inventory floated. Owner may prefer `RobberyRep`
> as the column name — see §6.

### 1.2 The success-chance formula (the curve)

We want: fail often at first, and a **slow grind over hundreds** to a hard cap
that is only *approached* near ~250–300 successful robberies — never a quick win.

Use a **diminishing-returns curve** (not linear) so early reps feel meaningful
but the last few percent take hundreds of robberies. Define, per robbery type, a
`base%` (chance at 0 successes) and a `cap%` (asymptote). The bonus added to base
is a saturating function of `n = SuccessfulRobberies`:

```
bonus(n) = (cap - base) * n / (n + K)
success%(n) = base + bonus(n)          // clamp to cap
```

`K` is the **half-saturation constant** — the number of successful robberies at
which you are exactly halfway from base to cap. Pick **K = 80**. That yields a
long tail: at n=80 you're at the midpoint, and you need n≈240 to reach 75% of the
way and n≈720 to reach 90% of the way — i.e. the last stretch is a real grind but
the practical cap is effectively reached (within ~2–3%) around n≈250–300.

**Integer-safe Pawn implementation** (shared helper, lands in `systems/robbery.inc`
so every module reaches it via `CallLocalFunction`, or as a `stock` in
`server/misc.inc`):

```pawn
#define ROB_GRIND_K   80    // half-saturation: reps to reach the base→cap midpoint

// Returns the success % for `n` successful robberies given base/cap.
FUNCTION Rob_SuccessChance(base, cap, n)
{
    if(n < 0) n = 0;
    new span = cap - base;
    new bonus = (span * n) / (n + ROB_GRIND_K);   // 0..span, saturating
    new chance = base + bonus;
    if(chance > cap) chance = cap;
    return chance;
}
```

**Per-type base/cap** (all `[OUR DESIGN]`, owner-tunable — §6):

| Robbery type            | base% | cap% | current constant it replaces |
|-------------------------|-------|------|------------------------------|
| `/rob` (no skill)       | 55    | 90   | `Crime_RobSuccessFor` returns **70** (crime.inc:127) |
| `/rob` (Con Artist)     | 45    | 88   | returns **60** (crime.inc:125) |
| `/rob` (Pickpocket)     | 65    | 95   | returns **90** (crime.inc:124) |
| Safe-crack (bank/casino/special) | 45 | 90 | `ROB_CRACK_BASE_SUCCESS` **70** (defines.inc:538; used robbery.inc:335) |
| House rob               | 40    | 85   | `HOUSEROB_BASE_SUCCESS` **65** (defines.inc:593) |
| `/holdup`               | 60    | 92   | *currently no fail roll* (holdup.inc — always succeeds) |
| `/shoplift`             | 65    | 95   | *currently no fail roll* (robbery.inc — always pays) |

Crowbar/skill bonuses stay **additive on top** of the curve result (clamp final
to 97 so nothing is a guaranteed success):
`final = min(97, Rob_SuccessChance(base,cap,n) + crowbarBonus)`.

**Chance table** (safe-crack row, base 45 / cap 90, K=80 — the reference curve):

| Successful robberies `n` | success % (no crowbar) | with crowbar (+20, clamp 97) |
|---|---|---|
| 0   | 45 | 65 |
| 25  | 56 | 76 |
| 50  | 62 | 82 |
| 100 | 70 | 90 |
| 200 | 77 | 97 |
| 300 | 80 | 97 |
| 500 | 84 | 97 |
| ∞   | 90 (cap) | 97 |

`/rob` no-skill row (base 55 / cap 90) for comparison: 0→55, 25→63, 50→68,
100→74, 200→80, 300→82, cap 90. House-rob (40/85): 0→40, 50→57, 100→65, 200→72,
300→76, cap 85.

### 1.3 Where each roll is replaced (exact hook points)

1. **`/rob` — `cmds/crime.inc` `Crime_RobSuccessFor()` (lines 120–128).** Replace
   the three hardcoded returns with per-skill base/cap into `Rob_SuccessChance`:
   ```pawn
   static Crime_RobSuccessFor(playerid)
   {
       new n = PlayerInfo[playerid][pSuccessfulRobberies];
       switch(PlayerInfo[playerid][pSkill])
       {
           case SKILL_PICKPOCKET: return Rob_SuccessChance(65, 95, n);
           case SKILL_CONARTIST:  return Rob_SuccessChance(45, 88, n);
       }
       return Rob_SuccessChance(55, 90, n);
   }
   ```
   The existing roll at crime.inc:184 (`random(100) >= Crime_RobSuccessFor(...)`)
   is unchanged. On success, increment (see §1.5) after the payout at ~line 204.

2. **Safe-crack — `systems/robbery.inc` `Robbery_CrackTick()` (line 335).** Replace:
   ```pawn
   new chance = ROB_CRACK_BASE_SUCCESS + (PlayerInfo[playerid][pCrowbarEquipped] ? CROWBAR_SUCCESS_BONUS : 0);
   ```
   with:
   ```pawn
   new chance = Rob_SuccessChance(45, 90, PlayerInfo[playerid][pSuccessfulRobberies]);
   if(PlayerInfo[playerid][pCrowbarEquipped]) chance += CROWBAR_SUCCESS_BONUS;
   if(chance > 97) chance = 97;
   ```
   `ROB_CRACK_BASE_SUCCESS` becomes the *base* argument — keep the define but
   repoint it to 45, or inline. Fail path (Robbery_Abort, line 338) unchanged.

3. **House rob — `House_RobSteal` (housing.inc), gated by `HOUSEROB_BASE_SUCCESS`
   (defines.inc:593).** Replace the base with `Rob_SuccessChance(40, 85, n)` before
   the SuperLock/pet penalties are subtracted, so penalties still apply on top.

4. **`/holdup` — `systems/holdup.inc`.** It has no binary success roll today
   (holdup.inc:151–156 always pays). Add an **entry roll** at the start of the
   holdup command (before the clerk-wait begins): roll
   `random(100) >= Rob_SuccessChance(60, 92, n)` → the clerk refuses / silent
   alarm, holdup aborts with a short cooldown + small wanted. If it passes, the
   register logic is untouched.

5. **`/shoplift` — `systems/robbery.inc`.** Same pattern: add an entry roll
   `Rob_SuccessChance(65, 95, n)` before the `$200–$900` payout; on fail, no cash,
   `SHOPLIFT_WANTED` still applies, short cooldown.

### 1.4 What a failed robbery costs

Uniform failure contract (`[OUR DESIGN]`, owner-tunable):

- **No loot** (already the case for `/rob` and safe-crack).
- **Small wanted** — the *attempt* wanted, not the full crime wanted:
  - `/rob` fail: keep the existing `GivePlayerWanted(ROB_WANTED, "Attempted robbery")`
    (crime.inc:188) — already correct.
  - `/holdup` / `/shoplift` fail: `+2` ("Attempted holdup"/"Attempted shoplift").
  - Safe-crack / house fail: keep the already-applied start wanted (WL6) — no
    additional penalty, the aborted crack is punishment enough.
- **Cooldown** — a *fail* cooldown shorter than a success one, so failure is a
  grind-tax not a lockout. New defines:
  ```pawn
  #define ROB_FAIL_CD        20   // secs before you can retry after a failed rob-type crime
  #define HOLDUP_FAIL_CD     30
  #define SHOPLIFT_FAIL_CD   45
  ```
- **`FailedRobberies` +1** and `Jail_SavePlayer` (or defer to disconnect save).

### 1.5 Incrementing the grind (success side)

Add a single shared incrementer so every success path is one line:

```pawn
FUNCTION Rob_RecordSuccess(playerid)
{
    PlayerInfo[playerid][pSuccessfulRobberies]++;
    Jail_SavePlayer(playerid);   // shared LSplayers UPDATE, no competing query
    return 1;
}
```

Call sites (all already the success/reward points named by the inventory):
- `/rob` success — `cmds/crime.inc` after the payout (~line 204, alongside `pScore += ROB_SCORE`).
- Safe delivery — `Robbery_Deliver()` (`robbery.inc` ~line 483, where `pScore += 1`).
- `/holdup` completion — `Holdup_End(playerid, true)` (`holdup.inc` ~line 101/166).
- `/shoplift` success — the payout block in `robbery.inc`.
- House rob success — `House_RobSteal` success branch (housing.inc).

> **Anti-farm note for the owner (§6):** because rep only climbs on *success* and
> failures are cheap, a player could grind `/shoplift` (lowest risk) to inflate
> rep that then boosts *bank* success. Options: (a) accept it (shared "robber
> skill" is thematically fine); (b) split into per-family counters
> (`SuccessfulRobberies` for safe/house/holdup, keep `/rob` and `/shoplift` on
> their own smaller counter). Recommend (a) for v1 simplicity.

### 1.6 Persistence summary

Follows the LSplayers pattern exactly: column add → enum var → `cache_get_value_int`
load → `Jail_SavePlayer` UPDATE extension → `Rob_RecordSuccess`/fail increment
flushes immediately. No new save path, no competing query.

---

## 2. IN-GAME INFO / HELP BROWSER

Built entirely on the existing textdraw menu (`players/menu.inc`) — **no new UI**.

### 2.1 Menu ids

Extend the enum in `server/defines.inc` (currently ends `MENU_GPS_LIST` at
defines.inc:136 — append, don't renumber):

```pawn
    MENU_GPS_LIST,
    MENU_HELP_CATS,   // NEW: the category list
    MENU_HELP_CMDS    // NEW: the command list for the selected category
```

### 2.2 Content data table (lives in `cmds/help.inc`, new file)

Two static arrays — the *only* place content lives, so adding a command is a
one-line data edit:

```pawn
enum e_HelpCat { HC_NAME[24], HC_BLURB[48] }
new const g_HelpCat[][e_HelpCat] = {
    // index = category id (matches HCMD_CAT below)
    {"Getting Started", "Spawn, skills, money basics"},
    {"Crime",           "Rob, rape, drugs, holdups"},
    {"Robberies",       "Bank, casino, house, special"},
    {"Cops",            "Arrest, ticket, dispatch, 10-codes"},
    {"Economy",         "Bank, ATM, stocks, lotto, jobs pay"},
    {"Property",        "Houses, vehicles, locks"},
    {"Jobs & Missions", "Deliveries, patrol, courier"},
    {"Skills",          "Pickpocket, con artist, medic..."},
    {"Fishing & Farming","Rods, coolers, plant, harvest"},
    {"Social & Chat",   "PM, whisper, ignore, radio"},
    {"Jail",            "Bail, appeal, bribe, escape"}
};

enum e_HelpCmd { HCMD_CAT, HCMD_NAME[24], HCMD_SHORT[40], HCMD_LONG[128] }
new const g_HelpCmd[][e_HelpCmd] = {
    // { category index, "/cmd", "one-line", "full description shown in MSGBOX" }
    { 1, "/rob (/rb)",   "Rob a nearby player's cash", "Rob the nearest on-foot player for 10-50% of their cash-in-hand. Fails sometimes; success improves as you complete more robberies. +2 wanted." },
    { 3, "/bankrob",     "Crack a bank safe",          "Start a 30s safe-crack at a bank checkpoint (WL6). Carry the safe to a hideout within 12 min. Crowbar raises success & speed." },
    { 4, "/arrest (/ar)","Arrest a warranted suspect", "On foot, within 4m of a WL6+ suspect: jail them, earn WL-scaled cash + score." },
    // ... one row per documented command
};
```

### 2.3 The commands

```pawn
CMD:help(playerid, params[])   // alias: /info, /hlp, /cmds
{
    Help_ShowCategories(playerid);
    return 1;
}
alias:help("info", "hlp")
```

**`Help_ShowCategories(playerid)`** — renders the category list:
```pawn
Help_ShowCategories(playerid)
{
    new nCats = sizeof(g_HelpCat);
    ShowTextDrawMenu(playerid, MENU_HELP_CATS, "~y~Feature Guide", nCats, nCats);
    ShowTextDrawMenuItems(playerid, 0, "~w~Pick a category number", "", "", 0);
    for(new i = 0; i < nCats; i++)
        ShowTextDrawMenuItems(playerid, i+1, "", g_HelpCat[i][HC_NAME], g_HelpCat[i][HC_BLURB], 0, i);
    return 1;
}
```

**Selection routing** — in `OnPlayerEnterTextDrawMenuOption` (`menu.inc`), add two
cases to the existing switch on `PlayerInfo[playerid][pMenu]`:

- `case MENU_HELP_CATS:` — `Option-1` is the category index (stored in
  `Menu_Item[playerid][Option]`). Hide the menu, then call
  `Help_ShowCommands(playerid, catIndex)`.
- `case MENU_HELP_CMDS:` — the selected row's `Menu_Item` is the `g_HelpCmd`
  row index. Hide the menu, then `ShowPlayerDialog(..., DIALOG_STYLE_MSGBOX, ...)`
  with `g_HelpCmd[row][HCMD_LONG]` as the body and a "Back" button that reopens
  the command list.

**`Help_ShowCommands(playerid, cat)`** — filters `g_HelpCmd` by `HCMD_CAT == cat`,
renders name in column1 + short blurb in column2, stores the global row index in
`item` so the detail dialog can look it up:
```pawn
Help_ShowCommands(playerid, cat)
{
    new row = 0;
    ShowTextDrawMenu(playerid, MENU_HELP_CMDS, g_HelpCat[cat][HC_NAME], MAX_TEXTDRAW_ROWS, 0 /*filled below*/);
    ShowTextDrawMenuItems(playerid, 0, "~w~Pick a command for details", "", "", 0);
    for(new i = 0; i < sizeof(g_HelpCmd); i++)
    {
        if(g_HelpCmd[i][HCMD_CAT] != cat) continue;
        row++;
        if(row > MAX_TEXTDRAW_ROWS) break;   // page cap (see below)
        ShowTextDrawMenuItems(playerid, row, "", g_HelpCmd[i][HCMD_NAME], g_HelpCmd[i][HCMD_SHORT], 0, i);
    }
    Menu_Options[playerid] = row;            // selectable rows = commands shown
    return 1;
}
```

### 2.4 Pagination

`MAX_TEXTDRAW_ROWS = 25` (menu.inc:22). Keep any single category ≤ ~22 commands
and no pagination is needed for v1. If a category overflows, add a per-player
`pHelpPage` var and reserve row 24/25 as "Next page →" (re-call
`Help_ShowCommands` with an offset) — flagged as a follow-up, not required for
the first ship.

### 2.5 Closure

Unchanged: the player fires (on foot) / horns (in vehicle) to close; the existing
`OnPlayerKeyStateChange` hook in `CnR.pwn` routes to `HideTextDrawMenu`. The
MSGBOX detail dialog closes via its own dialog response.

---

## 3. FULL COP SYSTEM

Built on the existing `Cop_Broadcast` (cop.inc:47) and `Robbery_AlertCops`
(robbery.inc:180). New shared code lands in a new **`systems/dispatch.inc`**
(included after wanted/jail/robbery, before `cmds/cop.inc`).

### 3.1 Cop chat channel (`/d` department radio)

CB has no persistent cop-channel toggle (`/copmsg` is one-shot). Add an always-on
**department channel**:

- **`/d [text]`** (alias `/dept`, `/copchat`) — send to the cop channel. Format:
  `[DEPT] {rank} Name: <text>` in `COLOR_COP_BLIP`, delivered via `Cop_Broadcast`.
  Runs the 10-code + quick-string expansion pass (§3.3) on `text` first.
  Gate: `IsCop(playerid)`.
- **`/copchatoff` / `/copchaton`** (toggle) — sets `pCopChatOn`; a cop with it off
  does not *receive* `/d` / dispatch chatter (still receives arrest-critical
  alerts). Persist `pCopChatOn` as a new LSplayers `TINYINT` (default 1), loaded
  and saved via the standard pattern. **[Owner: persist or session-only? §6]**
- `/copmsg` (`/cm`) is kept as an alias behaviour of `/d` (same channel).

### 3.2 Auto-dispatch / alerts

Central helper (replaces ad-hoc `Robbery_AlertCops`, which becomes a thin wrapper):

```pawn
// scope: DISPATCH_ALL (server-wide) or a radius around (x,y,z).
FUNCTION Dispatch_Alert(originPlayer, priority, const code[], const text[], Float:radius)
```

Emits `[DISPATCH] <10-code> <text> — <zone> (<distance if radius>)` in a priority
colour to every cop who has cop-chat on (critical priorities ignore the toggle).

**Events that fire dispatch** (message format + range):

| Event | Trigger site | Code | Range | Message |
|---|---|---|---|---|
| Store holdup begins | `holdup.inc` on `/holdup` start | 10-130 | server-wide | `[DISPATCH] 10-130 Robbery/Hold Up in progress — {zone}` |
| Safe-crack begins (bank) | `robbery.inc` `Robbery_StartCrack` (bank) | 10-132 | server-wide | `[DISPATCH] 10-132 Bank Robbery — {zone}` |
| Safe-crack begins (casino) | same, casino kind | 10-131 | server-wide | `[DISPATCH] 10-131 Casino Robbery — {zone}` |
| Special/house rob begins | `robbery.inc` / housing | 10-17 | server-wide | `[DISPATCH] 10-17 Robbery — {zone}` |
| Carrying the safe | `Robbery_BeginCarry` | 10-99 | server-wide | `[DISPATCH] 10-99 Suspect carrying stolen safe — {zone}` |
| WL9+ suspect proximity | `wanted.inc` on reaching WL9+ / periodic | 10-114 | 300m of each cop | `[DISPATCH] 10-114 Most Wanted nearby — {zone}` |
| Officer requests backup | `/backup` (cop.inc) | 10-78 | server-wide | `[DISPATCH] 10-78 Backup Needed at {zone} — {officer}` |
| Officer responds | `/respond` (new) | 10-23 | server-wide | `[DISPATCH] 10-23 {officer} responding` |

The existing `Wanted_AnnounceCrime()` proximity `[CRIME]` line (wanted.inc:91,
250m radius) stays as the *automatic* per-crime feed; `Dispatch_Alert` is the
higher-signal event feed layered on top. `Robbery_AlertCops` (robbery.inc:180)
is refactored to call `Dispatch_Alert(..., DISPATCH_ALL)`.

### 3.3 10-codes + quick strings

Ship a **curated subset** (the full 162 is overkill; owner picks additions §6).
Data table in `systems/dispatch.inc`:

```pawn
enum e_TenCode { TC_CODE[8], TC_MEANING[48] }
new const g_TenCodes[][e_TenCode] = {
    {"10-4",   "Affirmative"},        {"10-20",  "Location: $loc"},
    {"10-23",  "Arrived on scene"},   {"10-7",   "Murder"},
    {"10-17",  "Robbery"},            {"10-78",  "Backup needed at $loc"},
    {"10-80",  "In pursuit with $sus $loc"}, {"10-33", "Help immediately $loc"},
    {"10-97",  "Test comms"},         {"10-99",  "Wanted/Stolen"},
    {"10-114", "Most Wanted"},        {"10-130", "Robbery/Hold Up"},
    {"10-131", "Casino Robbery"},     {"10-132", "Bank Robbery"},
    {"10-51",  "Suspect on foot"},    {"10-138", "Suspect in a vehicle"},
    {"10-70",  "Lost visual $loc"},   {"10-85",  "Suspect in custody"},
    {"10-52",  "Ambulance needed $loc"}, {"10-32", "Units needed"},
    {"10-13",  "Follow me"},          {"10-12",  "Stand by"},
    {"10-22",  "Disregard"},          {"10-100", "5 min break"}
};
```

**`Cop_Expand10Code(dest[], const src[], playerid)`** — a single pass over any
cop-channel message that:
1. Replaces a leading/standalone `10-NN` token with `10-NN (Meaning)`.
2. Expands quick-strings: `$loc` → `PlayerInfo[playerid][pZoneName]`,
   `$sus` → last suspect the cop targeted (reuse `/vc`/`/su` last-target var),
   `$veh` → the cop's current vehicle model name, `$time` → game clock.

Applied by `/d`, `/copmsg`, `/backup`, `/report`, `/respond`. This matches CB's
"you can use 10-Codes and/or Quick Strings in your message" contract.

### 3.4 Missing cop commands (spec each)

| Command | Aliases | Module | Behaviour |
|---|---|---|---|
| `/respond` | `/yes` | cmds/cop.inc | Respond to the last dispatch call: fires `Dispatch_Alert(...,10-23,"responding",DISPATCH_ALL)` and sets a red checkpoint at the last dispatch origin (store `g_LastDispatchPos`). Cop-only. |
| `/robberies` | `/robs` | cmds/cop.inc | Dialog (TABLIST) of in-progress robberies/holdups not yet stopped. Data source: iterate players with an active `pRobSite`/`pHoldupStage`/carry flag; show type + zone + suspect. Cop-only. **Needs a small `g_ActiveRobberies` view or an on-the-fly scan of player robbery state.** |
| `/freeze` | `/fr`, `/pullover`, `/pu` | cmds/cop.inc | Order nearest suspect within range to comply: freezes them (`TogglePlayerControllable`) for `FREEZE_SECS 5`, sends both a client message; if suspect moves/leaves it lifts. **[Owner: hard freeze vs message-only? §6]** Maps to CB `/freeze`. |
| `/crimes` | `/crime` | cmds/cop.inc | Show the last N crimes a suspect committed. **Requires a small per-player ring buffer** `pCrimeLog[5]` (crime reason + time) populated by `GivePlayerWanted`'s `reason[]`. Cop-only + self. |
| `/vehrepair` | `/vehfix` | cmds/cop.inc | Cop on foot next to their PD vehicle: repair it for a fee (`COP_REPAIR_COST 15000`). Uses `RepairVehicle`. Cop-only. |
| `/coprank` | — | cmds/cop.inc | Show own `pCopRank` (0–10 Recruit→Commissioner), arrests to next rank, and current `/refill` weapon tier. Read-only; no state change. |

> CB has **no** `/taser`, `/cuff`, `/spike`, `/roadblock`, `/dispatch`, `/duty`,
> or `/su` command (confirmed in the reference §"Notable absences"). We keep our
> existing `/su` (it predates this doc and is useful) but do **not** add the
> others — they'd diverge from CB and aren't requested.

### 3.5 Cop-rank inspection & reward hook

`pCopRank` already increments per arrest (inventory). Add `/coprank` (above) for
visibility and a milestone message in the arrest reward path ("Promoted to
{rankTitle}!") at ranks that unlock a `/refill` weapon tier — pure UX, no new
persistence (CopRank already saved).

---

## 4. MISSING PLAYER / SOCIAL COMMANDS

Grounded in `cb-commands-full.md`. **`/w` is CB's `/whisper` = LOCAL proximity
chat, NOT a private message** (the reference calls this out explicitly). PM is
already `/pm`.

| Command | Aliases | Module | Syntax + behaviour |
|---|---|---|---|
| `/w` | `/whisper` | cmds/messages.inc | `/w <text>` — send to all players within `WHISPER_RANGE 15.0` (`* Name whispers: text` in a dim colour). NOT a PM. |
| `/say` | `/s` | cmds/messages.inc | `/say <text>` — explicit global chat line (mirrors default chat; useful when other channels are the norm). Low priority. |
| `/getid` | `/id` | cmds/player.inc | `/getid <partial name>` — resolve a name fragment to id(s); dialog list on multiple matches. |
| `/info` | `/i` | cmds/player.inc | `/info [id]` — class/skill/level/zone/wanted for a player (blank = self). **Name collision:** if `/info` is also a `/help` alias (§2.3), keep `/help` for the browser and use **`/pinfo`** or reserve `/i` here — see §6. |
| `/stats` | `/sts` | cmds/player.inc | `/stats [id]` — current-life stats (money, score, wanted, kills, robberies success/fail). Reads `pSuccessfulRobberies`/`pFailedRobberies` from §1. |
| `/complain` | — | cmds/player.inc | `/complain <text>` — civilian cheater/rule report → alerts on-duty admins (reuse the admin-chat sink used by `/a`). Distinct from cop-only `/report`. |
| `/locate` | `/loc` | cmds/player.inc | `/locate [id]` — show a player's current zone (blank = self). Cheap; reuse `pZoneName`. |
| `/giverep` | — | cmds/player.inc | `/giverep <id>` — +1 reputation to a nearby player (social rep, separate from robbery rep). Needs `LSplayers.PlayerRep` **[Owner: include rep economy or skip for v1? §6]**. |
| `/tip` | `/givetip` | cmds/messages.inc | `/tip <id> <amount>` — send a cash tip to a nearby player (thin wrapper over the existing `/givecash` transfer). |
| `/time` | `/day` | cmds/player.inc | `/time` — show the current game day + clock (read the game-clock state in CnR.pwn). |

Chat-channel parity already covered elsewhere: `/pm`,`/reply`,`/nopm`,`/ignore`
exist; `/copmsg` → `/d` in §3. `/carwhisper` (`/cw`) is optional (low value
without heavy passenger play) — **skip for v1** unless owner wants it (§6).

---

## 5. STAGES (dependency-ordered, each compile-green & shippable)

### STAGE A — Robbery Grind  *(no dependencies; ship first)*
- **SQL:** `LSplayers` + `SuccessfulRobberies`, `FailedRobberies` (cnr.sql CREATE + ALTER).
- **Vars:** `pSuccessfulRobberies`, `pFailedRobberies` (`player_vars.inc`).
- **Load/Save:** two `cache_get_value_int` in `loading_data.inc`; extend the
  `Jail_SavePlayer` UPDATE (widen buffer to ~900).
- **New code:** `Rob_SuccessChance()` + `Rob_RecordSuccess()` (in `robbery.inc`);
  `#define ROB_GRIND_K 80`, `ROB_FAIL_CD/HOLDUP_FAIL_CD/SHOPLIFT_FAIL_CD`.
- **Hooks replaced:** `Crime_RobSuccessFor` (crime.inc:120), safe-crack `chance`
  (robbery.inc:335), `HOUSEROB_BASE_SUCCESS` in `House_RobSteal`, add entry rolls
  to `/holdup` and `/shoplift`.
- **Increment calls:** `/rob` (crime.inc ~204), `Robbery_Deliver` (~483),
  `Holdup_End` (~101), `/shoplift` payout, house-rob success.
- **Result:** robberies fail on a curve; grind persists. Recompile + commit CnR.amx.

### STAGE B — Cop Chat + Dispatch + 10-Codes + Missing Cop Cmds  *(depends on A only for the /stats robbery fields; otherwise independent)*
- **New file:** `systems/dispatch.inc` — `Dispatch_Alert()`, `g_TenCodes[]`,
  `Cop_Expand10Code()`, `g_LastDispatchPos`. Include it in `CnR.pwn` (wrapped).
- **Cop chat:** `/d` (`/dept`,`/copchat`), `/copchaton`/`/copchatoff`,
  `pCopChatOn` (LSplayers TINYINT default 1). `/copmsg` routes to `/d`.
- **Dispatch wiring:** call `Dispatch_Alert` from holdup start, `Robbery_StartCrack`,
  `Robbery_BeginCarry`, WL9+ transition in `wanted.inc`, `/backup`. Refactor
  `Robbery_AlertCops` to delegate.
- **Crime log:** `pCrimeLog[5]` ring buffer written by `GivePlayerWanted`.
- **New cop cmds (cmds/cop.inc):** `/respond`(`/yes`), `/robberies`(`/robs`),
  `/freeze`(`/fr`,`/pu`,`/pullover`), `/crimes`, `/vehrepair`(`/vehfix`),
  `/coprank`.
- **Result:** full cop comms + auto-dispatch + 10-codes + the CB cop cmds we lack.

### STAGE C — Info / Help Browser  *(depends on nothing; can run parallel to B)*
- **Enum:** `MENU_HELP_CATS`, `MENU_HELP_CMDS` (defines.inc, append after MENU_GPS_LIST).
- **New file:** `cmds/help.inc` — `g_HelpCat[]`, `g_HelpCmd[]`,
  `Help_ShowCategories`, `Help_ShowCommands`, `CMD:help`+`alias`.
- **Menu routing:** two new cases in `OnPlayerEnterTextDrawMenuOption` (menu.inc)
  → command list, then MSGBOX detail dialog + Back.
- **Content:** populate `g_HelpCmd` rows for every documented command (pull the
  one-liners from the inventory's command list + the new cmds from Stages A/B/D).
- **Result:** `/help` (`/info`,`/hlp`) opens a categorised, drill-down guide.

### STAGE D — Remaining Player / Social Cmds  *(depends on A for /stats fields; independent of B/C)*
- **cmds/messages.inc:** `/w`(`/whisper`), `/say`(`/s`), `/tip`(`/givetip`).
  `#define WHISPER_RANGE 15.0`.
- **cmds/player.inc:** `/getid`(`/id`), `/info`(`/i`) [or `/pinfo` — §6],
  `/stats`(`/sts`), `/complain`, `/locate`(`/loc`), `/time`(`/day`),
  `/giverep` [if rep economy approved — §6].
- **Optional:** `/carwhisper`(`/cw`) if owner wants it.
- **Result:** CB-parity social/info command layer; `/complain` feeds admin sink.

> **Shared rule for every stage:** end at **0 errors / 0 warnings**, recompile
> and commit `gamemodes/CnR.amx`. Each stage is independently mergeable.

---

## 6. OWNER DECISIONS NEEDED

1. **Grind curve tuning** — approve K=80 and the per-type base/cap table (§1.2),
   or retune. Do you want the practical cap reached nearer ~200 (lower K) or a
   harsher ~400+ grind (higher K)?
2. **Column naming** — `SuccessfulRobberies`/`FailedRobberies` (recommended) vs
   the shorter `RobberyRep` from the inventory.
3. **Anti-farm** — accept a single shared robber-rep (recommended v1) or split
   per robbery family so `/shoplift` grind can't boost bank success (§1.5).
4. **10-code subset** — ratify the ~24-code list in §3.3 or name additions from
   the full 162 (`cb-commands-full.md` has the complete table).
5. **`/help` vs `/info` naming** — `/help` for the browser + `/pinfo` for player
   info, OR `/info` for the browser and `/i`/`/pinfo` for player info. They
   currently collide (§2.3 vs §4).
6. **Cop-chat toggle persistence** — persist `pCopChatOn` in LSplayers or keep it
   session-only?
7. **`/freeze`** — hard freeze (`TogglePlayerControllable` 5s) or CB-style
   message-only "comply" order?
8. **Reputation economy** — ship `/giverep` + `LSplayers.PlayerRep` now, or defer?
9. **Optional low-value cmds** — include `/say`, `/carwhisper` for CB parity, or
   skip to keep the surface small?
