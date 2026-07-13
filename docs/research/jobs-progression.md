# CB:CNR Research — Jobs, Progression & Social

Research for the EXP Gaming Cops And Robbers revival, modelled on **CrazyBob's Cops
And Robbers** (CB:CNR / crazybobs.net, closed after 18 years). This document
covers **jobs/classes, skills & weapon-skill progression, score/level
progression, social systems (PM/adverts/radio/DJ/report), Regular/Donator perks,
events & arenas (DM Stadium, duels, teleport zones), and misc actions**
(/jumpkick, vehicle & house locks, /goto).

**Legend:** `[VERIFIED]` = sourced from CB:CNR official/archived/community
material (URL cited). `[INFERRED]` = reconstruction from CB behaviour + SA-MP
norms. `[DESIGN]` = CB never had this (or it differed from our repo); closest
classic SA-MP design proposed.

**Primary sources used**
- Official site (live 2026): https://www.crazybobs.net/website/cnr-commands ,
  `/skills`, `/how-play`, `/faq`, `/rules`, `/donate`, `/cnrradio`
- Wayback raw captures (`id_`): skills page 2011
  (`web.archive.org/web/20110505032940id_/http://crazybobs.net:80/website/skills`),
  commands page 2011 (`.../20111110101301id_/.../cnr-commands`), location pages
  (`/gym`, `/regular-players-club`, `/dm-stadium`, `/about-cnr`).
- Fandom wiki via Wayback raw: **Score Guide**
  (`web.archive.org/web/20191018011018id_/https://crazybobs.fandom.com/wiki/Score_Guide`),
  **Wanted Levels** (`.../20190818084347id_/.../Wanted_Levels`),
  **Ticketing and Arresting** (`.../20191018020832id_/.../Ticketing_And_Arresting`).
- Community: forums.crazybobs.net threads t=16734 (donator/regular benefits),
  t=83492 (donator status); SA-MP wiki `SetPlayerSkillLevel`/Weapon Skills;
  Argonath RPG `/ad` reference; PatrickGTR/sf-cnr (SA-MP CnR clone).

---

## 1. Jobs / Classes / Skills

### 1.1 The CB model: Skin → Skill, not hard classes `[VERIFIED]`

CB:CNR splits the population into **two families**:

- **Law Enforcement** (cops/medics/technicians) — chosen by picking a **cop
  skin**. Restricted: cops cannot do civilian crime skills.
- **Civilians** — pick a **civilian skin**, then pick a **Skill**. The skill
  *enhances* one activity but **does not restrict** what else you can do
  ("Skills don't limit what you can do, only enhance particular activities").
  Source: https://www.crazybobs.net/website/how-play

Changing things (all cost money) `[VERIFIED — /faq]`:
- **Change skill** → at **City Hall**, for a price. Command in later builds:
  `/skill` (`/skl`) — "Change skill with skin".
- **Change skin** → at the **Hospital**, for a price.
- `/fakeskill` (`/fskill`) — set a *displayed* skill different from your real
  one (used by Kidnappers to hide as a Driver).

> Source: FAQ verbatim — "you will be able to choose from a list of skills. If
> at any point you would like to change skills, you can visit City Hall and have
> it changed for a price. If you wish to change your skin, you can go to the
> Hospital and have that changed for a price."

### 1.2 Full skill list (16 skills) with exact commands `[VERIFIED — 2011 skills page + official skills page]`

**Law Enforcement family**

| Skill | Role | Skill commands | Earns from |
|---|---|---|---|
| **Police Officer** | Arrest warrants (orange), ticket suspects (yellow) | `/arrest` (`/ar`), `/report`, `/cancellastreport`, `/ticket` (`/tk`), `/backup` (`/bk`), `/respond`, `/copmsg` (`/cm`), `/refill`, `/accept`, `/refuse`, `/jaillist`, `/suspects`, `/warrants`, `/mostwanted`, `/mission` | arrest bonus (scaled by wanted level), $500/ticket, patrol-mission $, COTD |
| **Public Medic** | Heal/cure anyone; can force-cure to stop epidemics | `/medic`, `/cure`, `/healme`, `/cureme`, `/prices`, `/calls`; refill at Hospital | selling heals/items |
| **Private Medic** | Heal/cure + sell items **+ can `/infect`** | `/medic`, `/healme`, `/cureme`, `/infect`, `/calls`; refill at Hospital | selling heals/items |
| **Police Technician / Law-Enforcement Arms Dealer** | Refill cops' weapons/ammo (LE only) + mechanic services | `/weapons`, `/prices`, `/calls`, `/vehrepair`; refill at Ammunation | weapon/repair sales to cops |

**Civilian skills** (each is a "specialization" of a broader activity)

| Skill | Advantage | Skill commands | Earns from |
|---|---|---|---|
| **Arms Dealer** | Sell weapons to civilians (crime; +wanted if cop nearby) | `/weapons [id]`, `/prices`, `/calls` | +1 pt per unique buyer; high cash |
| **Drug Dealer** | Sell drugs (crime) **+ plants grow faster** | `/drugs [id]`, `/prices`, `/calls` | +1 pt/unique buyer; +wanted if seen |
| **Car Jacker** | **Can steal locked cars**; shorter cooldown selling at crane | `/sell` (at crane) | crane sale price + faster throughput |
| **Con Artist** | Robs bigger amounts, moderate success | `/rob [id]` | +1 pt + cash per rob |
| **Pickpocket** | **Very high** rob success, smaller amounts | `/rob [id]` | +1 pt + cash per rob |
| **Rapist** | Higher `/rape` success | `/rape [id]` | +1 pt per success |
| **Hitman** | Fulfil hit contracts | `/hits`, `/hit`, `/cancelhit` | +1 pt/contract; big cash |
| **Kidnapper** | Lock passengers in Driver vehicles, ransom; appears as Driver | `/kidnap [id] [ransom]`, `/kidnapall`, `/release`, `/releaseall`, `/fakeskill` | +1 pt/kidnap; ransom cash |
| **Driver** (Taxi) | On-duty fare collection; can use Taxi/Limo/Bus/Air; Driver Upgrade → almost any vehicle | `/driver [price]`, `/fare [price]`, `/calls` (fee is **$ / 10 game-minutes**) | +1 pt per *new* passenger; fares |
| **Food Delivery** | Sell food/drinks; can use Pizzaboys | `/food [id]`, `/prices`, `/calls` | +1 pt/unique buyer; +$500/item to next-day pay |
| **Street Vendor** | Sell items (condoms, permits) — **can sell to cops too** | `/items [id]`, `/prices`, `/calls`; refill at 24/7 | +1 pt/unique buyer |
| **Prostitute** | Sell "hot coffee"; strip for fee (club) or tips (public) | `/sex [id]`, `/strip`, `/calls` (see Pimping) | +1 pt/buyer; tips |

Cross-cutting service calls: customers use `/heal /medic`, `/weapons`,
`/drugs`, `/food`, `/items`, `/driver /taxi`, `/mechanic`, `/sex`; providers see
requests via `/calls` (`/calllist`). Providers set prices via `/prices` and can
block buyers via the **No-Sell** list (`/nosell`, `/noselladd`, `/nsaddall`,
`/noselllist`). Source: cnr-commands page.

> **Note — some clone gamemodes add jobs CB never had** (Fireman, Lumberjack,
> Trucker as a *skill*, Mining). CB kept these as **missions** (see §3) rather
> than skills. Ref: PatrickGTR/sf-cnr README. Keep them as missions/activities,
> not skins, to stay faithful.

### 1.3 Mapping CB skills onto our 240-skin / 5-team model `[INFERRED / DESIGN]`

Our engine (`gamemodes/CnR.pwn`, `players/peds.inc`) already models this the
CB-native way: a `PedsInfo[MAX_PEDS]` table where every playable class row has a
`PedSkinID`, a `PedTeam` (one of **CIVIL / POLICE / SHERIFF / FBI / UC_COP**),
and a `PedName`. `PlayerInfo[pClassID]` selects the row; `pTeam` is derived from
it. This is exactly CB's "skin picks your family" design — reuse it.

Recommended mapping:

- **Team → CB family:**
  - `POLICE`, `SHERIFF`, `FBI` = CB **Law Enforcement**. Give each a
    LE-only *skill* on top of the team: e.g. Officer (default), Public Medic,
    Private Medic, Police Technician. (CB had one cop skin family; we have three
    agencies — treat SHERIFF/FBI as flavour + optional perk tiers, not separate
    balance.)
  - `UC_COP` (undercover) = **cop who appears CIVIL** — the engine already
    groups `CIVIL,UC_COP` together for spawn/appearance. This is the perfect
    home for CB's `/fakeskill` concept in reverse: a cop that shows a civilian
    skin. Give UC_COP the ability to arrest but no on-map blip / white name.
  - `CIVIL` = CB **Civilian** family → carries one of the 13 civilian skills.
- **Skill is a separate field from skin.** Do **not** hardcode one skill per
  skin — CB lets any civilian skin pick any civilian skill at City Hall. Add a
  `pSkill` enum to `PlayerInfo` (values: DRUGDEALER, ARMSDEALER, PICKPOCKET,
  CONARTIST, CARJACKER, HITMAN, KIDNAPPER, DRIVER, FOODDELIVERY, STREETVENDOR,
  PROSTITUTE, RAPIST, plus LE skills). `/skill` at City Hall changes `pSkill`;
  `/skin` (or Hospital) changes `pSkin`/`pClassID` within the allowed team.
- **240 skins**: expose them as cosmetic choices *within* a team (many CIVIL
  skins, a curated set of cop skins per agency). The skin does not change
  balance — only the skill does. This mirrors CB, where dozens of civilian
  skins all shared the same civilian skill pool.

---

## 2. Skills — weapon skill levels & fighting styles

### 2.1 SA-MP weapon skill (`SetPlayerSkillLevel`) `[VERIFIED — SA-MP wiki]`

SA-MP has a native per-weapon skill system: `SetPlayerSkillLevel(playerid,
skill, level)` with `level` 0–999. Three tiers matter: **Poor (0–199),
Gangster (200–999), Hitman (999)**. Skill types include
`WEAPONSKILL_PISTOL`, `_PISTOL_SILENCED`, `_DESERT_EAGLE`, `_SHOTGUN`,
`_SAWNOFF_SHOTGUN`, `_SPAS12_SHOTGUN`, `_MICRO_UZI`, `_MP5`, `_AK47`, `_M4`,
`_SNIPERRIFLE`. At **999 ("Hitman")** the player gets faster reload, dual-wield
where applicable, and can move/strafe while firing certain guns.
Source: https://wiki.sa-mp.com/wiki/SetPlayerSkillLevel , open.mp Weapon Skills.

**Did CB grant weapon-skill progression by use?** `[INFERRED]` No public CB
document describes a "shoot to level up your pistol" mechanic, and CB's own
"Skills" are the *job* system above (not GTA weapon skills). CB's Hitman skin is
canonically associated with max weapon proficiency, and most SA-MP CnR clones
simply **set all weapon skills to max (999) on spawn** so gunplay feels
consistent, gating power through *weapons you can buy/refill* rather than a
grind. Recommendation:

- **`[DESIGN]`** Default: set all `SetPlayerSkillLevel(...,999)` on spawn for
  responsive combat (classic SA-MP CnR feel), OR
- **`[DESIGN]`** Optional progression flavour: start civilians at Poor and raise
  the relevant weapon skill as their *player level* (§3) rises, capping Hitman
  skin/skill at 999 immediately (a soft perk of choosing Hitman). Keep it simple
  — CB players expect tight aim, not an RPG grind.

### 2.2 Fighting styles at the Gym `[VERIFIED — 2011 /gym page]`

> "Tired of that same, standard fighting style? Visit any of the three gyms and
> change your fighting style! There are three options to choose from. Found in
> **Ganton, Garcia, and Redsands East**."

Implementation: `SetPlayerFightingStyle(playerid, style)` with SA-MP styles
`FIGHTING_STYLE_NORMAL(4)`, `_BOXING(5)`, `_KUNGFU(6)`, `_KNEEHEAD(7)`,
`_GRABKICK(15)`, `_ELBOW(16)`. CB exposed **3 options** at the gym — most
likely Boxing / Kung-Fu / Knee-head (the three visually distinct melee styles).

**Design for us `[DESIGN]`:** Gym pickups/checkpoints in Ganton, Garcia,
Redsands East. On enter, dialog offers the 3 styles. CB didn't state a price for
the fighting-style change (unlike skill/skin changes which cost money at City
Hall/Hospital); treat gym style change as **free or a small fee** (e.g. $500).
Persist chosen style and re-apply `SetPlayerFightingStyle` on every spawn.

---

## 3. Score / Level progression

### 3.1 Score is the universal currency of progression `[VERIFIED — Score Guide, C2 BETA 11.1]`

Almost every completed activity gives **+1 score** (a few give more/less). Score
+ playtime drives **Regular** status and general standing. Full point table:

**Civilian — non-class-restricted**

| Activity | Points | Cash | Notes |
|---|---|---|---|
| Truck delivery | +1 / delivery | — | may cause wanted level |
| Courier delivery | +1 / delivery | high | **warrant** on pickup |
| Race Challenge (`/challenge`) | +1 finish, **+2 podium** | — | |
| DM Arena Top10 | +1 + cash reward | yes | based on kill streak (no death) |
| Robbery (holdup/checkpoints) | +1 / robbery | big | **warrant** each |
| **Drug delivery** | **+1 + $5,000 per checkpoint**, **+$50,000 all** | — | **+2 wanted** if delivered in cop sight; **once / 5 game-hours** |
| Food delivery mission | +1 + $2,500/cp, **+$25,000 all** | — | |
| Car-sell mission ("Gone in 15 hours") | +1 + $15,000/car | — | **last car = WL10**, **+$75,000** if sold |
| Trash pickup | +1 + $2,500/cp, +$25,000 all | — | civilian alt to cop patrol |
| Fishing (`/fish`) | +1 for a **record** fish; points for selling | — | wanted if near shore w/o permit |
| Sell car at crane (`/sell`) | +1 + stated price | — | Car Jacker sells faster; damage lowers price |
| **Harvest drugs** | +1 / plant (**even others' plants**) | — | plant must have **>50g** |
| **Hunting** (deer near your plants) | +1 / deer | — | **+6 wanted** if no hunting permit; killing a Hippy = instant warrant |
| Sell drugs at refill (`/selldrugs`, min 200g) | +1 | lower $ | |
| **Win Lotto** | **+5** (win), **+1** (participation) | jackpot | |
| Win a horse bet | +1 | odds-based | |
| Escape jail (`/escape`) | +1 | — | fail = **−1** |
| Breakout (2 players, `/breakout`) | +1 both | — | fail = **−1** to the outside helper |
| Escape a kidnapper (`/escape`) | +1 | — | fail = **−1** |

**Civilian — semi/class-restricted** (per **unique** target)

| Activity | Points | Notes |
|---|---|---|
| `/rob` a player | +1 + cash | Pickpocket/Con Artist higher success; **non-rob skill max $50,000, rob skill max $500,000** |
| `/rape` a player | +1 | Rapist skill easier |
| `/food` sale | +1 / unique buyer | +$500/item to next-day pay |
| `/medic` sale (Private) | +1 / unique buyer | |
| `/items` (Street Vendor) | +1 / unique buyer | can sell to cops |
| `/weapons` (Arms Dealer) | +1 / unique buyer | +1 wanted if near cop |
| `/selldrugs` (Drug Dealer) | +1 / unique buyer | +1 wanted if near cop |
| `/sex` (Prostitute) | +1 / buyer | legal in LV/LS/SF; cops can buy |
| `/hits` (Hitman) | +1 / contract | kill ⇒ **≥ WL6** |
| `/kidnap` / `/kidnapall` | +1 / person | +1 wanted/civ, +6 wanted/cop; can `/rape` captive repeatedly for score |
| `/driver` | +1 / **new** passenger | no repeat points for same rider |

**Cop scoring**

| Activity | Points | Cash |
|---|---|---|
| `/arrest` a warrant | +1 | bonus scaled by suspect wanted level |
| Kill a **WL10** most-wanted | +2 | (arrest still pays more; only kill if endangered) |
| `/ticket` a suspect | +1 | +$500 collection bonus (no repeat-ticket farming) |
| Cop patrol mission | +1 + $2,500/cp | +$25,000 all |
| **Cop Of The Day (COTD)** | +1 | +$25,000 |

**How to LOSE score** `[VERIFIED]`

| Event | Loss |
|---|---|
| Death (any cause) | −1 (but **"unfair death"** by a cop while innocent civilian = no loss, keep cash/weapons) |
| Get `/arrest`ed (busted) | −1 |
| Cop **refuses** your `/bribe` | −1 (no point for a successful bribe) |
| Drug overdose | −1 (usually → death = another −1) |
| Take **bad** drugs | −1 |
| Failed jail escape | −1 |
| Failed breakout help | −1 |

> Full source (verbatim numbers above):
> `web.archive.org/web/20191018011018id_/https://crazybobs.fandom.com/wiki/Score_Guide`

### 3.2 Levels & level-gated perks `[VERIFIED + INFERRED]`

- CB exposes rank/level via `/level` (`/lev`, `/rank`). `[VERIFIED — commands]`
- **Cop rank gates weapon refills:** "You must be inside a Police Department and
  have reached a **certain rank** to refill. **More weapons are available as
  your rank increases.**" `[VERIFIED — 2011 skills page /refill]` This is the
  clearest level-gated perk in CB.
- **Regular** status is a soft level tier from **score + playtime** (§5).
- CB did not publish a numeric level→XP table. `[DESIGN]` Use a simple curve
  keyed off score (e.g. level = f(score) with rising thresholds), and gate: cop
  refill weapon tiers, Driver Upgrade (any vehicle on duty), and access to
  bigger missions behind level, exactly as CB gated cop refills by rank.

---

## 4. Social systems

### 4.1 Private messages & anti-spam `[VERIFIED — commands / messaging section]`

Exact CB command set:
- `/pm` (`/ms`, `/msg`, `/m`, `/priv`) `[id] [text]` — private message.
- `/reply` (`/r`) `[text]` — reply to last PM.
- `/whisper` (`/w`) — message nearby players only.
- `/say` (`/s`) — broadcast to all active players (**free**; this is CB's global
  chat channel, *not* a paid advert).
- `/nopm` — toggle refusing PMs.
- `/ignore` (`/ign`, `/mute`) `[id]` — mute a player's chat (toggle).
- **Quick PM:** `/set` saves a player to slot 1/2/3; `/1` `/2` `/3` PM that slot.
- **Quick Strings:** `/$set (1-3) [text]` — save a canned line (95-char max),
  fire it fast (great for vendors advertising prices without spamming manually).

**Anti-spam etiquette `[VERIFIED rule + INFERRED impl]`:** CB Rule 7 forbids
advertising/spam; Rule 2 forbids disturbing players minding their own business.
CB relied on `/ignore` + admin `/complain` reports. `[DESIGN]` Add a **PM/chat
flood throttle** (e.g. reject if >N messages in T seconds, or min 1–2s between
identical messages), auto-mute repeat offenders, and respect `/nopm`/`/ignore`.

### 4.2 Adverts / `/ad` `[DESIGN — CB had NO paid advert; our repo's /ad differs]`

**Important correction:** In CB:CNR, **`/ad` = `/adrenaline`** ("Heals instantly
or cures any diseases"), *not* an advertisement. `[VERIFIED — commands]` CB's
"advertising your goods" was done through free `/say`, Quick Strings, and the
service-call system — **there was no in-game paid advert command**, and Rule 7
actually *bans* real advertising.

Our `PLANNED-FEATURES.md` lists `/ad` as a **paid advertisement** — that's an
**our-own** feature borrowed from RP servers. Closest classic SA-MP design
(Argonath/LS-RP style):
- `/ad [text]` posts a coloured server-wide "Advertisement:" line. Reference
  cost ~**$200** (Argonath). Source:
  https://wiki.argonathrpg.eu/index.php/SA-MP_Script_Commands
- **Cooldown** ~**60–120s** per player; length cap (e.g. 64–100 chars); block
  URLs/other-server names to honour CB Rule 7.
- **`[DESIGN]` naming clash:** since CB's `/ad` means adrenaline, either name our
  paid advert **`/advert`** (keep `/ad` = adrenaline for authenticity) or accept
  the rename and document it. Recommend `/advert` to avoid confusing CB veterans.

### 4.3 Radio & DJ system `[VERIFIED — /cnrradio page]`

- **CnR Radio** = a **community-run 24/7 live stream** with rotating volunteer
  DJs (revived late 2012 by community members). Website http://cnr-radio.com/ .
- **In-game tune-in:** `/cnrradio` opens a menu of stream locations; the client
  plays the external stream (SA-MP `PlayAudioStreamForPlayer`).
- **DJ interaction / requests** happened via **IRC** channel `#cnr.radio` on
  `irc.tl` — you saw "who's the current DJ / what's playing" and requested songs
  there, **not** through an in-game `/djradio` command.
  Source: https://www.crazybobs.net/website/cnrradio

**Our `/radio` and `/djradio` `[DESIGN]`:** CB had no in-game `/djradio` host
command. Closest classic design:
- `/radio` (`/cnrradio`) — dialog listing preset stream URLs; selecting one calls
  `PlayAudioStreamForPlayer`; a "stop" option calls `StopAudioStreamForPlayer`.
- `/djradio` (for `/makedj` DJs) — let a live DJ **push a stream URL + now-playing
  text to all listeners** (broadcast the URL so everyone hears the DJ's set),
  plus a DJ announce line in chat. This modernises CB's IRC-based DJ hosting into
  an in-game command, which fits our `/makedj` owner grant.

### 4.4 Reporting & chat enforcement `[VERIFIED]`

- Players report rule-breakers with **`/complain [name/ID] [reason]`** — "alerts
  all admins that are online." (FAQ verbatim.) Note: CB's `/report` is the
  **cop** crime-report tool, *not* the player-report tool — keep them distinct.
- Cops use `/report` to raise a suspect's wanted level (serious crimes only) and
  `/cancellastreport` to undo.
- Server rules relevant to chat enforcement (https://www.crazybobs.net/website/rules):
  - R1 respect admins; R2 don't insult/disturb other players;
    R3 no deathmatching / random killing; R7 no advertising/spam;
    R9 don't quit to avoid a hit/attack/robbery/punishment (penalised).

---

## 5. Regular / VIP / Donator perks

Our `/makeregular` and `/makedonator` map cleanly onto CB's two tiers.

### 5.1 Regular Player `[VERIFIED — FAQ, RPC page, forum]`

- **Earned automatically** by a mix of **score + playtime**, tracked **per city**
  ("Auto-Regular"). Not bought. Source: FAQ; forum t=16734.
- **Perks:**
  - Access to the **Regular Players Club (RPC)** — exclusive lounge with music,
    located **Rodeo (LS), Financial (SF), Emerald Isle (LV)**; per-city unlock.
    `[VERIFIED — /regular-players-club page]`
  - Can **drive Rustlers** (jet). `[VERIFIED — forum t=16734]`
- **`/makeregular` design `[DESIGN]`:** keep the auto-award (score+playtime) as
  primary; `/makeregular` is the admin/owner override. Perk pack = RPC teleport
  access + special-vehicle permission (Rustler and similar).

### 5.2 Donating Player (VIP) `[VERIFIED — donate page + forum + wiki search]`

- Attained by **donating ($10, previously $5+)**. Officially "does not entitle
  you to special treatment" (donate page's public stance), but in practice CB
  granted quality-of-life perks. Source: https://www.crazybobs.net/website/donate ;
  forums t=16734 & t=83492.
- **Verified/consensus donator perks:**
  - **No wealth tax** — donators don't pay the tax non-donators pay on money in
    house/bank. `[VERIFIED]`
  - **`/vehcolor` (`/vehc`) vehicle colour change for $100** (donor-only).
    `[VERIFIED — commands page: "Change color ($100 for donors)"]`
  - Donor **name colour / tag** and small cosmetic flags. `[INFERRED]`
- **CB did NOT publicly document** spawn-armour, extra spawn weapons, or a
  dedicated VIP lounge for donators (the lounge is the *Regular* RPC, earned not
  bought). `[VERIFIED absence]`
- **`/makedonator` design `[DESIGN]`:** grant no-wealth-tax, `/vehcolor` at $100,
  donor name colour. If we want stronger VIP (spawn armour, extra ammo, a VIP
  lounge), those are **our additions** — keep them mild to preserve CB's
  "donating buys no advantage" spirit; the classic-faithful perks are the tax
  break + cosmetics.

---

## 6. Events & arenas

### 6.1 DM Stadium (DMS) `[VERIFIED — /dm-stadium page + Score Guide]`

- **Three stadiums:** Los Santos, San Fierro, Las Venturas.
- **Legal DM:** "you won't lose any of your lives or Life Insurances."
- **Entrance fee: $5,000.** **Each death costs $1,000.**
- **Enter/exit:** `/enter` and `/exit` (commands page). Inside, kills toward the
  **city Top10 kill-streak** give **+1 score + cash** (streak = kills without
  dying). `[VERIFIED — Score Guide "CnR DM Arena"]`
- **Spawn kit `[INFERRED]`:** DM arenas hand a fixed weapon set on entry
  (Score Guide references "shredding people with your **sawnoffs**" → sawn-off +
  a couple of guns). `[DESIGN]` give a fixed kit on `/enter`, strip on `/exit`,
  restore normal inventory after.

### 6.2 Teleport / DM-zone commands `[VERIFIED names + DESIGN for our set]`

CB's map location names match our repo's teleport list — Bayside, Palomino
Creek, Dillimore, etc. appear on the CB wiki location nav. Our repo teleports:

| Our cmd | Alias | Destination |
|---|---|---|
| `/drylake` | `/d` | Dry Lake |
| `/sfairport` | `/sfa` | SF Airport |
| `/bayside` | `/bs` | Bayside |
| `/lossantosdm` | `/lsadm` | LS DM zone |
| `/palominocreek` | `/pc` | Palomino Creek |

`[DESIGN]` These are our **DM/gathering teleports** (CB steered players to the
3 DM Stadiums via `/enter`; freeform DM zones like these are common on SA-MP
CnR servers). Design each as: warp to a fixed spot, optional weapon kit, and
apply the **anti-parachute** rule below.

### 6.3 Anti-parachute in DM zones `[DESIGN — CB no explicit rule text found]`

No CB doc states an anti-parachute rule, but it is the standard SA-MP DM-zone
mechanic (parachuting out to escape/avoid deathmatch is abuse). Closest classic
design: on entering a DM zone, **strip weapon 46 (parachute)** via
`ResetPlayerWeapons`/`RemovePlayerWeapon`, and in a per-tick or `OnPlayerUpdate`
check, if `GetPlayerWeapon == 46` inside the zone, remove it and warn. Also block
re-giving parachute while inside the zone.

### 6.4 Duels `[DESIGN — CB had no /duel; classic SA-MP pattern]`

CB provided legal PvP through the **DM Stadium**, not a 1v1 `/duel` system. Our
`/duel` is an addition. Closest classic SA-MP duel design:
- `/duel [id] [stake]` challenges a player; target `/accept`s.
- Escrow the **stake** from both; teleport both to a duel arena (own virtual
  world so they don't affect others), freeze, give a fixed **weapon set**, 3-2-1
  countdown, then unfreeze.
- **Winner takes the pot** (2× stake, optionally minus a small house cut).
- **Leave rules:** disconnecting or `/q`-ing mid-duel = **forfeit** (loser),
  stake goes to opponent — consistent with CB Rule 9 ("don't quit to avoid").
- Restore pre-duel position, weapons, health, world on finish.

### 6.5 Sniper arena `[DESIGN]`

Same framework as duels/DM but sniper-only weapon kit in an isolated virtual
world; entry fee optional; deaths don't affect real lives (like DMS). No CB
source; model on DMS economics ($5k entry / $1k per death) if a fee is wanted.

---

## 7. Misc actions

### 7.1 `/jumpkick` (melee) `[VERIFIED — animation commands]`

`/jumpkick` is one of CB's **animation commands** ("Jump and kick"). CB's
animation set is huge (`/animations` lists them). `[DESIGN]` Implement as an
`ApplyAnimation` jump-kick that deals small melee damage to a player directly in
front within short range (and/or is purely cosmetic like most CB anims). Related
melee/comedy anims to consider porting: `/slap` (`/bitchslap`), `/carkick`
(`/ck`), `/stab`/`/stabanim`, `/fishslap`, `/taichi`.

### 7.2 Vehicle & house locks `[VERIFIED — commands]`

- **Vehicle:** `/lock` (`/lk`) "Activate vehicle alarm" and `/unlock` (`/ulk`)
  "Deactivate alarm." Our repo already lists `/lock`/`/unlock` with these
  aliases. In SA-MP terms: set doors locked for everyone except the
  owner/co-owners via `SetVehicleParamsForPlayer`. Car Jackers can **defeat
  locks** (steal locked cars) — a skill perk. `[VERIFIED]`
- **House:** the same intent is served by the **house key** system —
  `/housekeys` (`/hkeys`) give keys, `/housecoowner` (`/hco`) co-owner keys,
  `/housekick`, `/houseinvite`, `/keys` list held keys, `/houserob` (`/hrob`).
  `[VERIFIED — house commands]` So "house lock" = key-gated door access, not a
  separate `/lock` toggle.

### 7.3 `/goto` — who may teleport `[DESIGN — not a CB player command]`

`/goto` (teleport to a player) is **not in CB's public command set** — CB
players moved via the GPS system, `/enter`, taxis, and location teleports, not a
free `/goto`. `/goto` is an **admin/staff** convenience on most SA-MP servers.
Our `PLANNED-FEATURES` marks it "restricted?". Closest classic design:
- **Admins/staff:** `/goto [id]` freely (spectate/moderation). Scripters get
  `/lgoto` (already in our owner/scripter list) for coordinate teleports.
- **Donators:** optionally a limited `/goto` to *friends/group members only*, or
  none — CB kept teleport-to-player as a staff tool, so the faithful default is
  **admins only**. Regular players should use GPS + vehicles.
- Never expose unrestricted `/goto` to all players (breaks the chase loop that
  is the heart of CnR).

---

## 8. Quick "faithful defaults" cheat-sheet for implementation

| Thing | CB-faithful value |
|---|---|
| Change skill | at City Hall, costs money; `/skill` |
| Change skin | at Hospital, costs money |
| Fighting style | 3 gyms (Ganton/Garcia/Redsands East), 3 styles, cheap/free |
| Weapon skills | default max (999) on spawn (classic feel) |
| Most activities | **+1 score**; death/arrest/bad-drug/failed-escape = **−1** |
| Drug delivery | +1 +$5k/cp, +$50k all, once / 5 game-hrs, +2 wanted if seen |
| Ticket | +1, +$500, no repeat farming; suspects = WL 1–5 |
| Arrest | +1, wanted-scaled cash; warrants = WL 6–10 |
| WL10 kill | +2 (arrest preferred) |
| COTD | +1, +$25k |
| DM Stadium | $5,000 entry, $1,000/death, Top10 streak = +1 + cash |
| Regular | auto by score+playtime/city; RPC (Rodeo/Financial/Emerald Isle) + Rustler |
| Donator | $10+ donate; no wealth tax; `/vehcolor` $100; name colour |
| PM spam | honour `/nopm` & `/ignore`; flood throttle; `/complain` to admins |
| `/ad` | = **adrenaline** in CB; name our paid advert `/advert` (~$200, ~60–120s cd) |
| Radio/DJ | `/cnrradio` stream menu; `/djradio` = broadcast stream URL to listeners |
| `/goto` | admins only (players use GPS/taxi/enter) |
