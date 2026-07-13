# CB:CNR Primary Sources Sweep

**Purpose:** Citation backbone for our CnR revival. This document captures verbatim / near-verbatim mechanics from CrazyBob's Cops And Robbers (CB:CNR) primary and community sources, so the other three researchers (economy, crime/police, activities) can cite exact numbers.

**Legend:**
- **[V]** = VERIFIED — sourced from a cited URL below, transcribed as close to verbatim as the source allowed.
- **[I]** = INFERRED — our reconstruction where the source was thin or silent; must be treated as a design choice, not fact.

**Research date:** 2026-07-13. Server officially CLOSED end of 2024 after 18 years, but the website (`crazybobs.net/website/`), the Fandom wiki (`crazybobs.fandom.com`), and the forum (`forums.crazybobs.net`) are **still live and readable** as of this sweep.

**Access notes for future researchers:**
- `WebFetch` reaches `crazybobs.net/website/*` fine.
- `WebFetch` gets **HTTP 402** on `crazybobs.fandom.com` and **cannot** reach `web.archive.org` (blocked) or `forums.crazybobs.net` (ECONNREFUSED). The Fandom wiki and 10-codes were pulled via the **Chrome browser tool** (`get_page_text`), which works. Use the browser for Fandom/forum, not WebFetch.

---

## 1. Sources Index (URL → contents → reliability)

### Official website — `crazybobs.net/website/` (HIGH reliability: this is the developer's own doc)

| URL | Contents | Reliability |
|---|---|---|
| `https://www.crazybobs.net/website/cnr-commands` | **Complete command reference**, ~400 commands grouped by category, with aliases + one-line descriptions. Our single best command source. | HIGH |
| `https://www.crazybobs.net/website/cnr-10-codes` | **Full 10-code list (10-0 … 10-161)** verbatim. | HIGH |
| `https://www.crazybobs.net/website/how-play` | Overview of wanted levels, cops-vs-robbers, points/money. Light on numbers. | HIGH but shallow |
| `https://www.crazybobs.net/website/robberies` | Holdups, bank rob, casino rob, house rob, special robberies (v23 rework). | HIGH |
| `https://www.crazybobs.net/website/fishing` | Fishing, rods, permits, tournaments, records. | HIGH |
| `https://www.crazybobs.net/website/gambling` | Casino games, horse betting (2:1–10:1), scratch tickets, dice. | HIGH |
| `https://www.crazybobs.net/website/houses` | Houses: ownership limits, rent, tax, storage capacities. | HIGH |
| `https://www.crazybobs.net/website/drug-planting-and-hunting` | Drug growing: seeds, plant limits, grow time, yield, fertilizer, deer/hunting. | HIGH |
| `https://www.crazybobs.net/website/game-items` | Item catalogue (insurance, permits, crowbar, rod, adrenaline, etc.) — **no prices shown**. | HIGH (list), gap (prices) |
| `https://www.crazybobs.net/website/missions` | List of 19 missions (titles only). | HIGH (list only) |
| `https://www.crazybobs.net/website/skills` | 16 skill names. | HIGH (list only) |
| `https://www.crazybobs.net/website/about-cnr` | History + feature blurb. | HIGH (background) |
| `https://www.crazybobs.net/website/faq` | Registration, score/money basics, data-per-city. | HIGH but shallow |

### Fandom wiki — `crazybobs.fandom.com` (MEDIUM–HIGH: community-written, some pages "accurate for C2 BETA 11.1", so version-drift risk)

| URL | Contents | Reliability |
|---|---|---|
| `https://crazybobs.fandom.com/wiki/Wanted_Levels` | **The 1–10 wanted tier table** and colour coding. | HIGH |
| `https://crazybobs.fandom.com/wiki/Jail` | Jail mechanics: bail, appeal (12-jury), bribe ($1k–$15k), escape, breakout, parole, item loss. | HIGH |
| `https://crazybobs.fandom.com/wiki/Score_Guide` | **Best numeric source**: +/- score per activity, payouts, wanted-level side-effects. Marked "Accurate for C2 BETA 11.1". | HIGH (age caveat) |
| `https://crazybobs.fandom.com/wiki/ToRro's_Guide_To_Everything_CnR` | Huge player guide: economy, taxes, bank interest, money rush, drugs, horse bets, houses, insurance. | MEDIUM (player estimates, "someone correct me if wrong") |
| `https://crazybobs.fandom.com/wiki/Police_guide` | Cop commands, suspect tiers, PD interior, refill points, patrol payout, jail cell counts. | HIGH |
| `https://crazybobs.fandom.com/wiki/Police_Officer` | **Cop rank ladder (Recruit → Commissioner)**. | HIGH |
| `https://crazybobs.fandom.com/wiki/Robbery` | Holdup: +7 wanted, ~10s clerk, $2000–$6000/sec. | HIGH |
| `https://crazybobs.fandom.com/wiki/Con_Artist` | /rob up to $500,000, tax evasion 9/10, robs 1/10–1/2 of victim cash. | HIGH |
| `https://crazybobs.fandom.com/wiki/Pickpocket` | /rob up to $100,000, high success; pickpocket mission = rob $10,000 in 6.5 game hrs. | HIGH |
| `https://crazybobs.fandom.com/wiki/Kidnapper` | Ransom up to $50,000; steals 20–80% on-hand cash at hideout. | HIGH |
| `https://crazybobs.fandom.com/wiki/Drug_Dealer` | Carries 2500g (starts 500g); faster plant growth; cheaper buys. | HIGH |
| `https://crazybobs.fandom.com/wiki/Car_Jacker` | Steals locked cars; crane resell wait reduced by 3 min (to 5 min). | HIGH |
| `https://crazybobs.fandom.com/wiki/Prostitute` | /sex; legal except in PD; passes STDs/drugs. | HIGH |
| `https://crazybobs.fandom.com/wiki/Rapist` | /rape mechanics; no-STD raping; faster re-rape. | HIGH |
| `https://crazybobs.fandom.com/wiki/All_Commands_List` | Index of command categories (mirrors official). | HIGH (index) |
| `https://crazybobs.fandom.com/wiki/Untrue_Hacks` | Not mechanics — SA-MP lag/de-sync explainer (Lucky Victim bug, invisible driver). Useful for netcode expectations. | HIGH (context) |

**Empty/deleted Fandom pages (do not rely on):** `/wiki/Rape`, `/wiki/Lotto`, `/wiki/Hitman` (redirects to Untrue Hacks), `/wiki/Skill_Guide` (redirects to Rapist), `/wiki/Robbery` is thin.

### Forum — `forums.crazybobs.net` (community; unreachable via our tools this sweep, listed for future manual pull)

| URL | Contents | Reliability |
|---|---|---|
| `https://forums.crazybobs.net/viewtopic.php?t=78368` | "CnR Commands [v24 UPDATE]" — newest command dump. | HIGH (if reachable) |
| `https://forums.crazybobs.net/viewtopic.php?f=11&t=25623` | Score Guide (forum original of the wiki page). | HIGH |
| `https://forums.crazybobs.net/viewtopic.php?t=237` | "Captainjohns CnR guide for noobs". | MEDIUM |
| `https://forums.crazybobs.net/viewtopic.php?f=11&t=631` | Drug planting guide. | MEDIUM |
| `https://forums.crazybobs.net/viewtopic.php?f=11&t=2508` | Police Guide (forum original). | HIGH |
| `https://forums.crazybobs.net/viewtopic.php?t=3496` | "BEST money making ideas!" | MEDIUM |

### GitHub clones / reference gamemodes (executable spec — see §9)

| Repo | Notes |
|---|---|
| `https://github.com/PatrickGTR/sf-cnr` | San Fierro C&R — most complete, 1043 commits, 100% Pawn, MIT. Closest large open reference. |
| `https://github.com/Sorenkai/Kai-s-Cops-and-Robbers` | Early SA-MP 0.3.7 C&R, SQLite auth, robbing system. Small. |
| `https://github.com/UnholyBeast/OSG-CNR` | WIP CNR in Pawn. Thin README. |
| `https://github.com/bharel/GCnR` | "Game's Cops and Robbers", gcnr.net. |
| `https://github.com/Arose-Niazi/Exp-Gaming-Cops-And-Robbers` | **This project's own repo.** |

---

## 2. Wanted Levels [V]

Source: Fandom `Wanted_Levels`, `Police_guide`.

Ten wanted levels grouped into three tiers, each with a radar/name colour:

| Wanted Level | Tier | Radar colour | Cop action |
|---|---|---|---|
| 0 | Innocent / civilian | White (name) | none |
| 1 | Suspect | Yellow | Ticket only (cannot arrest) |
| 2 | Suspect | Yellow | Ticket only |
| 3 | Suspect | Yellow | Ticket only |
| 4 | Suspect | Yellow | Ticket only |
| 5 | Suspect | Yellow | Ticket only |
| 6 | Warrant Issued Suspect | Orange | Arrest (on foot) |
| 7 | Warrant Issued Suspect | Orange | Arrest |
| 8 | Warrant Issued Suspect | Orange | Arrest |
| 9 | Most Wanted Warrant Issued Suspect | Dark Orange | Arrest OR take-down with force for reward |
| 10 | Most Wanted Warrant Issued Suspect | Dark Orange | Arrest OR take-down (kill) for +2 cop score |

Verbatim rules:
- "Suspects are ticket-able and cannot be arrested. When a ticket is issued, the Suspect will have a limited amount of time to pay the Ticket... If the Suspect fails to pay the ticket in time, they will become a Warrant Issued Suspect and can be arrested." [V]
- "Wanted Levels will slowly decrease over time and the Criminal's notoriety may degrade into the previous tier." (i.e. WL decays passively.) [V]
- "Multiple and repeated Crimes will stack on top of the Suspect's existing Wanted Level." [V]
- A cop's `/visualcontact` (`/vc`) "will not allow the suspects wanted level to drop" — i.e. maintaining visual freezes decay. [V]
- Arrest is only possible while the warrant is **on foot** (not in a vehicle). [V]

### Wanted-level side effects of specific crimes [V] (from Score Guide / skill pages / ToRro)
- **Holdup / store robbery:** instantly raises wanted level by **+7** → Felon. (Robbery wiki) [V]
- **Selling drugs where a cop can see you:** +1 wanted. (Score Guide) [V]
- **Drug delivery mission:** +2 wanted **each** delivery if a cop is in sight (cop-sight radius is smaller at night). (Score Guide) [V]
- **Selling weapons near cops (Arms Dealer):** +1 wanted. [V]
- **Selling a car at the crane with a cop around:** +1 wanted (rare). [V]
- **Killing a deer without a hunting permit:** +6 wanted (even far from city). (Score Guide) [V]
- **Killing a Hippy:** instant warrant. [V]
- **Hitman kill (contract):** at least wanted level 6. [V]
- **Kidnapping:** +1 wanted per civilian kidnapped, **+6 per cop** kidnapped; kidnapping "will always give you a warrant." [V]
- **Car sell mission, final car:** automatic **Level 10 Most Wanted**. [V]
- **Courier mission:** wanted level 6 on pickup of goods. [V]
- **Drunk driving:** automatic level 5, car spins randomly. (ToRro) [V]
- **Fishing too close to shore without a permit while a cop sees you:** a wanted level. [V]
- **/escape from jail:** wanted jumps to **Level 10 Most Wanted** + "Jail Escape" charge. [V]

---

## 3. Jail, Bail, Appeal, Escape [V]

Source: Fandom `Jail`, `Score_Guide`, `Police_guide`.

**Jails (by active city):**
- Los Santos: LSPD HQ, Pershing Square.
- Las Venturas: LVPD HQ, Roca Escalante.
- San Fierro: SFPD HQ, Downtown.
- Jail cell count: **3 cells** in San Fierro and Los Santos, **6 cells** in Las Venturas. [V]

**On arrest / going to jail:** [V]
- You are disarmed of weapons.
- Crowbar removed if crime history has a robbery violation; fishing rod removed if it has a fishing violation. (Exception: if you died to a cop's use of force, crowbar/rod are NOT removed — intentional.)
- You get a **jail time** and a **bail** amount, both scaled by past crimes + current offenses.

**Four ways out:** [V]
1. **Serve full sentence**, then `/bail` (pay bail) or get a cop `/parole`.
2. **`/appeal`** — type a reason, a **jury of 12** players (cops/bots/non-jailed civilians) votes guilty/innocent. Innocent → jail time set to 0:00, then pay bail or be paroled. **Cannot appeal** if <1:00 remaining OR if you committed a serious offense (murder, etc.).
3. **`/bribe`** a cop — cop can `/accept` or `/refuse`. Bribe amount is **$1,000–$15,000**. Accept → jail time + bail reduced (scaled by time remaining, bail, and bribe amount). Refuse → jail time + bail INCREASED and you're charged with attempted bribery + you lose 1 score. **Most Wanted (WL10) players may NOT bribe.** [V]
4. **`/escape`** (solo) — sets you to WL10 Most Wanted + "Jail Escape" charge, broadcasts to all cops, spawns you in/near PD. Small chance a cop spots you before you even escape → you're "chained" to the cell and cannot escape again this sentence. If arrested after escaping → also chained. [V]

**Breakout (2-player):** `/breakout` (`/bo`, `/brk`) — needs a partner outside; both get wanted; you run out of the PD. Success → both +1 score; fail → only the outside player loses 1 score. [V]

**Parole:** a cop may `/parole` a jailed player for free **once they have served their minimum sentence**. [V]

**Jail is a rape hotspot** — cells are "popular spots for rapists." Bring drugs (heal) and condoms. A Public Medic can force-cure STDs. [V] (See §7.)

---

## 4. Police / Cops [V]

Source: Fandom `Police_Officer`, `Police_guide`, official `how-play`, `cnr-commands`.

**Rank ladder (0 → 10):** [V]
`0 Recruit · 1 Training · 2 Foot Patrol · 3 Officer · 4 Officer · 5 Sergeant · 6 Sergeant · 7 Lieutenant · 8 Lieutenant · 9 Captain · 10 Commissioner`
- Rank rises by arresting + ticketing; higher rank = access to higher-power weapon refills at PD + more Police Garage vehicles/resources. Radar dot gets a lighter shade of blue with rank. **Kill an innocent civilian or cop → lose rank.** [V]

**Refill points:** cops earn "Refill Points" by doing cop work, spent at the PD bot (MMB) to heal/armor/weapons/ammo. **Max 10 refill points.** Weapon availability gated by rank. [V]

**Cop scoring & payouts:** [V]
- `/arrest` a warrant → **+1 score** + cash bonus scaled by suspect's wanted level (arrest pays more than killing).
- Take down a **WL10** suspect (kill) → **+2 score** (use only when life endangered; arrest still preferred).
- `/ticket` a yellow suspect → **+1 score** + **$500** ticket-collection bonus. Can't repeatedly ticket the same person for score.
- **Cop Patrol Mission** (`/mission` at PD checkpoint, routine patrol): reach 5 random checkpoints before timer → **$2,500 each** + **$25,000** all-complete bonus = **$37,500** total, +1 score per checkpoint. [V]
- **Domestic Disturbance mission:** drive to one random house before timer; faster arrival = more money. [V]
- **Cop Of The Day (COTD):** +1 score + **$25,000**. [V]

**Suspect tiers to a cop:** yellow (WL1–5) = ticket only; orange (WL6–10) = arrest (on foot); dark orange (WL9–10, Most Wanted) = arrestable + take-down-for-reward. "No suspects or civilians should be shot unless in self defense." [V]

**Temp Cop:** a civilian with >10 score and no recent serious crimes can step into the PD signup checkpoint to become a volunteer cop (keeps their skin, limited abilities: cannot `/refill` in PD, cannot be bribed, no daily pay; gains ranks but loses them on leaving/ city change). [V]

**Key cop commands (aliases):** `/arrest` `/ar`; `/ticket` `/tk`; `/freeze` `/fr` `/pullover` `/pu`; `/suspects` `/sus`; `/warrants` `/war`; `/mostwanted` `/mw`; `/copmsg` `/cm`; `/backup` `/bk` (radar dot turns purple); `/visualcontact` `/vc`; `/report` `/rp`; `/accept` `/ac`; `/refuse` `/ref`; `/parole` `/pa`; `/vehrepair`; `/donut` (refills 50% HP, carry max 6). [V]

---

## 5. Robberies [V]

Source: official `robberies`, Fandom `Robbery`, `Score_Guide`, ToRro.

**General:** `/robbery` at location checkpoints; "A warrant for your Arrest will be issued when you commit a Robbery." Since **Version 23**, robberies have individual settings — variable timers + difficulty scaled by proximity to police activity. Each robbery = **+1 score**. [V]

**Store Holdups** — `/holdup` (`/hup`, `/storerob`): [V]
- Locations: 24/7, Ammunation, Bars, Restaurants/Diners, Dance Clubs, Sex Shops, Inside Track Betting, Clothes stores.
- On `/holdup`: **instantly +7 wanted** (Felon). Wait ~**10 seconds** for the clerk to open the register, then you receive **$2,000–$6,000 per second**.
- **Crowbar** increases the per-second amount AND reduces the register open time by a couple seconds.
- Robber MUST be on foot; a cop who enters the shop and presses MMB can arrest mid-holdup. Leave the cash zone and exit to finish. You can `/bribe` a cop to go innocent (keep crowbar + weapons). [V]

**Bank Robbery** — `/bankrob` (`/robbank`) at bank checkpoint: [V]
- All **3 city banks** (LV, SF, LS) robbable. Wait ~**30 seconds** at the checkpoint (cops alerted), then deliver the **safe to a random hideout** to get paid.
- "Bank Robberies take money away from other players accounts." Banks close during/after a robbery.
- **Max taken per victim account: $100,000** (ToRro says max $100k/person; a small % of 0–500,000 range is drawn from *random* accounts, not all). On a full server expect **~$500k–$1M** total. Bank-insured players are protected. Expect big revenge hits afterward. [V]

**Casino Robbery** — `/casinorob` (or `/robbery` at casino): all 5 casinos (3 LV, 1 SF, 1 LS). Rob interior, then transport safe to a random hideout to crack it for payment. [V]

**Special / major robberies (v23+):** SF Federal Mint, LS Observatory Mansion, SF Cargo Ship, SF Zombotech, SF Drug Factory, LV Methylamine at K.A.C.C. Fuels. [V]

**House Robbery** — `/houserob` (`/hrob`): break into houses; success triggers a police report if the owner is present or an alarm exists; can steal stored items (see §8). [V]

**Shoplifting** — `/shoplift` at shop checkpoints; steals items into inventory. [V]

**Crowbar:** "will lower your robbery time (robberies & holdups) and increase your chance of a successful robbery." From Truck Stops or Zero's RC Shop. [V]

**Player robbery** — `/rob`: see §6 (skill-gated caps). [V]

---

## 6. Crime skills & /rob economics [V]

Source: Fandom skill pages + `Score_Guide` + ToRro.

**/rob a player** (`/rob <id/nick>`): +1 score + cash. Victims in vehicles or in special buildings (Bank, City Hall, Ammunation) **cannot be robbed**. [V]

| Skill | /rob max per hit | Success | Notes |
|---|---|---|---|
| Non-robbing skill | **$50,000** | low | Score Guide: "the most you can rob from a player is $50000" |
| Pickpocket | **$100,000** | very high | robs smaller amounts than Con Artist but more reliably |
| Con Artist | **$500,000** | moderate (can fail, may tip off victim) | robs 1/10–1/2 of victim's cash; **tax evasion 9/10 of the time** (stackable) |
| Robbing skill (generic, per Score Guide) | **$500,000** | — | "As a robbing skill this increases to $500000." |

- **Pickpocket mission:** rob a total of **$10,000** from different players within **6.5 game hours**. Checkpoints in LS Willowfield, SF Financial, LV Redsands West. [V]
- **Kidnapper** — `/kidnap <id> <ransom>` (`/kd`), `/kidnapall` (`/kda`), `/release`, `/releaseall`, `/ransom`, `/fakeskill`. Ransom **up to $50,000/victim**; at hideout you steal **20%–80% of victim on-hand cash**; small bonus on kidnap; victim in hideout has a death timer; if victim quits while kidnapped, kidnapper still paid + victim still loses money; small bonus if victim dies in hideout after timer. Kidnappers disguise as Drivers (`/fakeskill`). [V]
- **Hitman** — `/hit` place, `/hits` list, `/cancelhit`. Complete contract = +1 score + the placed cash. Kills give ≥WL6. Placed hits can auto-cancel — check `/hits` before the kill. (ToRro: "most effective way of making money.") [V]
- **Car Jacker** — steals locked cars (not admin cars); crane resell cooldown reduced by **3 min** → **5 min** between sales (vs base 8 min implied). [V]
- **Arms Dealer** (civ + police variants) — `/weapons`; +1 score per unique buyer; +1 wanted if selling near cops. [V]
- **Drug Dealer** — see §7. **Street Vendor** — `/items`, sells 24/7 items to anyone incl. cops. **Medic (public/private)** — `/medic`, sells heals/cures/condoms; public medic can force-cure STDs without consent. [V]

---

## 7. Drugs, Growing, Hunting, STDs, Rape [V]

Source: official `drug-planting-and-hunting`, Fandom `Drug_Dealer` `Rapist` `Prostitute`, `Score_Guide`, ToRro.

**Growing (`/plant` / `/seed`):** [V]
- Carry up to **10 seeds** at a time (sold at 24/7, Drug Refill Point "R", Bait Shops, Street Vendors).
- Plant **max 5 plants** at once. Cannot plant near checkpoints, water, or other plants.
- Plant shows as a red checkpoint visible ~**50 m** away (cops detect from farther). Others can't see it from far until fully mature.
- **Full growth ≈ 20 minutes real time → up to 200 grams.** Drug Dealers grow faster.
- **Drug Plant Fertilizer** (`/bait`, `/fertilize`) → grows faster but attracts more deer.
- Plants/seeds saved on quit IF registered, but do **not** grow offline. Per ToRro, plants over ~150g will eventually die if not harvested.
- **Harvest** by re-entering the checkpoint; +1 score per plant (must be **>50g** on harvest for the point). "harvest them" even if not yours. [V]

**Hunting:** deer eat plants; killing a deer gives bonus + health; **no hunting permit → +6 wanted** for killing a deer. Killing a Hippy → instant warrant. Deer Traps (`/trap`) kill threats. Hunting permits: carry up to **20**. [V]

**Selling drugs:** [V]
- `/drugsell` / `/selldrugs` (`/ds`) at a Drug Refill Point ("R"): minimum **200g** sale gives +1 score; refill-point prices are lower than player prices.
- Drug Dealer `/drugs` (or `/sell`) to players: +1 score per **unique** buyer; +1 wanted if a cop sees.
- Drug Dealer carries **2500g** (Score Guide/ToRro also cite up to 5000g with a drug bag), starts with **500g**; other civ skills cap at 500g. Drug Bag adds +2500g (dealer) / +500g (others). [V]
- Set player prices with `/prices`. [V]

**Consuming drugs (`/td` / `/smoke` / `/takedrugs`):** [V]
- Drugs restore health and let you survive with STDs. ToRro: "last about 5 seconds per gram" and heal a small amount every ~1.5–2 s. `/smoke` = 5 grams per press.
- Taking too much → **Overdose** (-1 score, usually leads to death = another -1). "Bad drugs" have a chance to give a random STD and -1 score.
- `/cook` produces drugs (requires lab equipment); `/hotbox`, `/hidedrugs`, `/givedrugs`. [V]

**STDs / diseases:** [V]
- Range from Chlamydia (mild) up to **Mary Lou** (worst). Killed by disease over time if you don't have drugs.
- Sources: Sex Shop, Private Medic, hospital; free from bad drugs, random rape, Daisy the Cow (Mad Cow). Having any STD makes your `/rape` success ~100% even if not a Rapist.
- **Condoms** (from medics, hospital, 24/7): protect vs infection during rape (still take rape damage). House storage caps condoms at 25.
- **Chastity Belt** (hospital): prevents rape entirely, can break — keep condoms as backup. `/belt` discards it.
- **Adrenaline Pill** (`/ad`): instantly full-heal + cure all diseases; carry up to 3.
- **Health Insurance:** free heals/cures/condoms at hospitals, lasts **5 game days**. [V]

**Rape (`/rape` / `/ra`):** +1 score per successful rape. May pass on / generate diseases (can kill you too). **Rapist skill:** higher success even with 1–2 diseases, doesn't need diseases to infect, shorter re-rape cooldown, longer gap between periodic disease damage. General "can't attack the same person twice in a row" restriction is bypassed on a kidnapped victim. Prostitution/`/sex` is **legal everywhere except inside PDs**; cops may buy sex. [V]

---

## 8. Houses, Bank, Economy, Gambling, Fishing [V]

### Houses [V] (official `houses`, ToRro)
- **1500+ houses** across San Andreas. Bank sells at current value; **10% discount for full-time Police Officers.** Owners set their own resale prices.
- Ownership limits: **own 2**, co-own 8, rent 10 houses.
- Lose houses on losing current life, OR if not visited for **2 weeks real time**.
- **Rent** auto-paid by tenants each gameday; price fixed from contract date; a house can't be for-rent and for-sale simultaneously; owners can evict anytime.
- **Property + co-owner tax** auto-paid every **2 gamedays**; failing property tax **twice in a row** → one house seized.
- **Car save** houses: park near the driveway, go inside, `/quit` — saves the car's ID (not mods/NOS/condition).
- **Storage caps:** money; **condoms 25**, **flowers 25**, **drugs 5000 g**, **drug seeds 25**, **deer traps 25**, **fish 10**, **clothing 20 pieces** — all stealable in a house robbery.
- Commands: `/house`, `/houseinvite` `/hinvite`, `/housekeys` `/hkeys`, `/housecoowner`, `/houserob` `/hrob`, `/housestorage` `/store`, `/rent`, `/hotel`, `/hotels`. [V]

### Bank & Economy [V] (ToRro, FAQ, Score Guide)
- Deposit protects cash from `/rob` (in-pocket cash is robbable). `/deposit` `/withdraw` `/atm` `/moneyinfo`.
- **Bank interest ≈ 5–15% every 2 gamedays**, paid to account (works best if you don't pay taxes). ToRro example: 3M → ~$150k–$250k interest. (Note: his worked example is internally inconsistent — treat 5–15%/2days as the headline, verify in-game.)
- **Income tax ≈ 10% of daily income**; **wealth tax** applies if you hold **> $7 million** (on-hand + bank) — figure was $7M "from Beta 12.8, can change." Con Artists can dodge taxes 9/10. **Tax refunds** exist on income tax (not wealth tax).
- **Bank Insurance (Beta 12):** **2%/day + $25,000 down payment**, protects your money if the bank is robbed.
- **Bank robbery draw:** small % (**0–500,000** total range) pulled from *random* accounts; **max $100,000 per account.**
- **Money Rush** (Beta 12): every other day ~**13:00** the Mafia drops a money bag in a named area; search to grab it. `/moneyrush` `/mr`. [V]
- **Give cash:** `/givecash` `/gc` `/$`. **Shares/stocks:** `/shares`, `/sharessell`. [V]

### Gambling [V] (official `gambling`, ToRro)
- **Casino games:** Slot Machines, Blackjack, Video Poker — press Y looking at a machine. High Roller Casino allows higher bets. `/casinosettings`.
- **Horse betting:** bet on 1 of **5 horses**, odds **2:1 → 10:1**, races **every 2 game hours** (odd hours: 1:00, 3:00…). Bet **$100–$150,000**. Win → +1 score + payout = bet × odds (e.g. $150k on 10:1 = $1.5M). `/horsebet`. [V]
- **Scratch'n'Win:** from 24/7, gas stations, casinos, City Hall, RPC. Buy for a set price, chance to win up to ~2× (e.g. $25k ticket → up to $50k). [V]
- **Dice:** `/dice`, in casinos or player-to-player. [V]
- **Lotto** (`/lotto`): draws daily at **18:00**. Win → **+5 score** (+1 for participation bonus) + big cash. (Fandom Lotto page is empty; draw time from command list, score from Score Guide.) [V]

### Fishing [V] (official `fishing`, Score Guide)
- `/fish` from a boat. **Fishing Rod** (Bait Shops) reduces fishing time + raises catch chance; lost on jail unless you hold a fishing permit.
- **Fishing Permit** (24/7, Bait Shop, Street Vendors, City Hall): fish legally + reduces wait between casts; **carry up to 50.** A separate **fish sales permit** is needed to sell to players (`/fishsell`, `/fishprices`).
- Without a permit, fishing too near shore in a cop's sight → wanted level.
- Bulk sell at 24/7 / bait shops with `/fishsellall`; eat fish to heal (`/fisheat`).
- +1 score for a **record-breaking** fish; also points from selling.
- **Tournaments** randomly every couple days, run **4:00 → 20:00**; types: largest catch / most of a type / highest quantity; rewards scale with participant count. **Daily bonus fish** — first to catch it wins big cash. `/fishrecords`, `/fishtour`. [V]

---

## 9. Missions & Payouts summary [V]

Source: `Score_Guide`, `Police_guide`, `Pickpocket`.

| Mission / activity | Command | Payout | Score | Cooldown / notes |
|---|---|---|---|---|
| Truck Delivery | `/delivery` `/truck` | (cargo-based) | +1/delivery | some cargo illegal → wanted (not warrant) |
| Courier | `/courier` | high (> truck) | +1/delivery | WL6 on pickup; can't accept with a warrant |
| Drug Delivery | (marker) | **$5,000/checkpoint** + **$50,000** all-5 bonus | +1/checkpoint | **once every 5 game hours**; +2 wanted/delivery if cop in sight |
| Food Delivery mission | (Hotdog/Mr Whoopee) | **$2,500/checkpoint** + **$25,000** all bonus | +1/checkpoint | — |
| Car Sell Mission ("Gone in 15 Hours") | (marker) | **$15,000/car**; **$75,000** all-complete; last car **$75,000** on sell | +1/car | final car → auto **WL10**; damaged cars won't count |
| Trash Pickup | (Trashmaster) | **$2,500/checkpoint** + **$25,000** all | +1/checkpoint | civ equivalent of cop patrol |
| Cop Patrol | `/mission` | **$2,500/checkpoint** + **$25,000** all = **$37,500** | +1/checkpoint | cops only |
| Domestic Disturbance | `/mission` | faster = more $ | — | cops only |
| Pickpocket mission | `/mission` | (rob $10,000 total) | — | 6.5 game hrs; civ only |
| Crane car sale | `/carsell` `/vsell` | the stated price on entry (less if damaged) | +1 | timer between sales; Car Jacker faster (5 min) |
| Bonus car (daily) | `/vsi` to see | ~$50k+ (rises if unsold) | — | announced daily ~01:00 |
| Race Challenge | `/challenge` `/ch` | cash | +1 finish, +2 podium | — |
| DM Arena | `/enter`/`/exit` | cash | +1 if Top10 | kills without dying |

**Score losses [V]:** death **-1** (any cause; "unfair death" by a cop as innocent civilian = no loss, keeps cash/weapons at 1000 ammo, but still lose armor); arrest **-1**; refused bribe **-1** (no point for a successful bribe); overdose **-1** (+ likely death -1); bad drugs **-1**; failed jail escape **-1**; failed breakout (outside player) **-1**; failed kidnapper-escape **-1**.

**Regular Player status:** auto-granted via a mix of play time + score ("Auto-Regular"). Unlocks the RPC (Regular Player's Club) — safe zone (no crimes), food/gambling pickups. Temp-cop eligibility starts around **>10 score**; ToRro suggests farming to **50** as cop then **100** score before free-roaming skills. [V/I]

---

## 10. Items catalogue [V] (official `game-items`) — prices are a GAP

Items exist and are described but the site does **not** list prices. Prices must come from forum guides or the clone gamemodes (or be designed by us).

- **Life Insurance** (City Hall / Street Vendor) — price varies by market + death frequency.
- **Health Insurance** — free heals/cures/condoms at hospitals, **5 game days**.
- **Hunting Permits** — carry **20** (Street Vendor / Bait Shop / Supa Save / City Hall).
- **Fishing Permits** — carry **50** (24/7 / Bait Shop / Street Vendor / City Hall).
- **Gun Permit** — City Hall / PD / Church / Ammunation (required to buy Ammunation weapons).
- **Adrenaline Pill** — full heal + cure, carry **3**.
- **Chastity Belt** — anti-rape, breakable (hospital).
- **Fishing Rod** — Bait Shops.
- **Crowbar** — Truck Stops / Zero's RC Shop.
- **Ammo Bag** — carry more ammo per weapon.
- **Vehicle Repair Kit** — repair on foot (Gas Stations / Mechanics); `/vehrepair`.
- **ACME Insta-Fix** — instant repair while stopped, carry **1**; `/instafix`.
- **Public Transit Card** — free bus/airport, expires after **1 game week**.
- **Donut** (cop) — refills 50% HP, carry **6**; `/donut`.

---

## 11. Skills roster [V]

16 skills on the official page; the wiki Category:Skills lists 20 (includes variants). Grouped:

**Law Enforcement (blue, lighter = higher rank; all can arrest):** Police Officer · Public Medic · Police Technician · Law Enforcement Arms Dealer · (Temp Cop = civilian volunteer).

**Civilian:** Car Jacker · Con Artist · Pickpocket · Private Medic · Arms Dealer · Prostitute · Rapist · Drug Dealer · Hitman · Kidnapper · Street Vendor (Items Dealer) · Mechanic · Food Delivery (special — can't change out of without dying) · Driver (green when on-duty).

Skin change: once per gameday. Skill change: **first change free, then ~$33,000 average**. `/newlife` resets stats but lets you change freely. [V]

---

## 12. 10-Codes [V] (official `cnr-10-codes`, verbatim full list)

Usable anywhere in-game; `$veh $loc $sus $dir $time $Esc` are auto-filled placeholders.

```
10-0 Use Caution              10-41 Beginning Tour of Duty at $time   10-82 Car Occupied Multiple Times
10-1 Signal Weak              10-42 Ending Tour of Duty at $time      10-83 Suspect Hidden on Radar $loc
10-2 Signal Good              10-43 Shuttle                           10-84 Multiple Suspect in Area of $loc
10-3 Stop Transmitting        10-44 Permission to Leave               10-85 Suspect Down/Custody $loc
10-4 Okay, Affirmative        10-45 Human Remains at $loc             10-86 Crime in Progress at $loc
10-5 Relay To                 10-46 Assist Motorist                   10-87 Illegal Drug Activity
10-6 Busy Unless Urgent       10-47 Kidnapping                        10-88 Suspect has a Gun $loc
10-7 Murder                   10-48 Subject Disturbing the Peace      10-89 Bomb Threat
10-8 In Service in a $veh @$loc 10-49 Abandoned Vehicle               10-90 I Need a New Vehicle
10-9 Say Again, Repeat        10-50 Traffic Collision                 10-91 Carrying Illegal Items
10-10 Negative, No            10-51 Suspect on Foot                   10-92 Theft
10-11 On Duty                 10-52 Ambulance Needed at $loc          10-93 Misuse of Communicator
10-12 Stand By                10-53 Dispatch                          10-94 Me
10-13 Follow Me               10-54 Change to Channel                 10-95 At Police Station
10-14 Message/Information     10-55 Intoxicated Driver                10-96 Mental Subject/Problems
10-15 Message Delivered       10-56 Intoxicated Pedestrian            10-97 Test Communicator
10-16 Reply To Message        10-57 Hit and Run                       10-98 Prison Break by $Esc
10-17 Robbery                 10-58 Air Plane Crash                   10-99 Wanted/Stolen
10-18 Urgent                  10-59 Unlawful Driver                   10-100 Taking 5 Minute Break
10-19 In Contact              10-60 Suspected Hacking                 10-101 LOL!
10-20 Location: $loc          10-61 I'm a noob!                       10-102 My Game Crashed
10-21 Assault                 10-62 Attempting PIT                    10-103 Noob Cop/Cops!!
10-22 Disregard, Nevermind    10-63 Going to Hospital                 10-104 Crooked Cop Aiding Suspect
10-23 Arrived at Location     10-64 Going to a Restaurant             10-105 On Patrol at $loc
10-24 Sexual Assault          10-65 Going to Ammunation               10-106 Escort
10-25 Meet With Person        10-66 Major Crime Alert                 10-107 Land
10-26 ETA                     10-67 PM me                             10-108 Air
10-27 Drivers License Check   10-68 GiveCash To                       10-109 Water
10-28 Weapon Activity         10-69 Rape, Sexual Misconduct           10-110 Innocent
10-29 I Need a Medic $loc     10-70 Lost Visual with Suspect $loc     10-111 Yellow, seems cooperative
10-30 Danger/Caution          10-71 Drug Activity                     10-112 Yellow, uncooperative
10-31 Pick Up Person          10-72 Aborting Pursuit $loc             10-113 Orange, Warrant Issued
10-32 # Units Needed          10-73 Missing Person                    10-114 Most Wanted
10-33 Requesting Help Now $loc 10-74 Civil Disturbance                10-115 Backup Needed, 1 unit $loc
10-34 Vehicle Theft           10-75 Domestic Problem                  10-116 Backup Needed, Multiple Units $loc
10-35 Roadblock set up $loc   10-76 Bribe                             10-117 Do I have backup coming to $loc?
10-36 Security Check          10-77 Return to                        10-118 Suspect Fleeing into Country Side $loc
10-37 Drunk Driving           10-78 Backup Needed at $loc             10-119 Pursuit Halted/Stand Still $loc
10-38 Having Computer Problems 10-79 Notify Coroner                   10-120 Good Job!
10-39 Use lights/sirens(Urgent) 10-80 In Pursuit with $sus $loc      10-121 Getting a New Vehicle
10-40 No lights/sirens(Silent) 10-81 Illegal Fishing/Hunting         10-122 Escort
```
(Continues 10-123 Ticket Issued to $sus · 10-124 $sus Paid Ticket · 10-125 Joining Pursuit $loc $dir · 10-126 Returning to Patrol Zone · 10-127 Vehicle Lightly Damaged · 10-128 Vehicle Heavily Damaged · 10-129 Flat Tire · 10-130 Robbery/Hold Up · 10-131 Casino Robbery · 10-132 Bank Robbery · 10-133 Hit Contract · 10-134 Returning to City Limits from $loc · 10-135 Roadblock · 10-136 Intercepting Suspect from $loc Going $dir · 10-137 Shoot the Suspect's Tire · 10-138 Suspect in a Vehicle · 10-139 Suspect has a Weapon · 10-140 Deathmatching · 10-141 Friendly Fire · 10-142 Which way are you heading? · 10-143 Requesting Pickup $loc · 10-144 Requesting Fast Pursuit Unit $loc · 10-145 Requesting Air Support $loc · 10-146 Requesting Boat Support $loc · 10-147 Dispatch, Requesting Assignment · 10-148 Fog - Air unit out of service · 10-149 Rustler Moving in to shoot, back off! $loc · 10-150 I'm out of Ammo · 10-151 Out of Service at $loc · 10-152 Going to Location · 10-153 $dir Bound · 10-154 Assignment Complete · 10-155 Registration Check · 10-156 Requesting Roadblock Immediately, we're at $loc · 10-157 Gang Activity · 10-158 Investigate Vehicle · 10-159 Suspect has gotten Away $loc · 10-160 In Hot Pursuit!!! $loc $dir Bound · 10-161 Burglary)

---

## 13. Full Command Reference [V] (official `cnr-commands`, transcribed)

Grouped exactly as the official page. Primary command first, then aliases; description after `—`.

### Police
`/accept` `/ac` — accept a bribe · `/arrest` `/ar` — arrest closest/specified warrant · `/backup` `/bk` — request backup · `/cancellastreport` `/clr` · `/copmsg` `/cm` — message all police · `/donut` — refill 50% HP (max 6) · `/fire` — respond to fire (needs Fire Truck) · `/freeze` `/fr` `/pullover` `/pu` — order surrender · `/mostwanted` `/mw` — list WL10 · `/refuse` `/ref` — refuse bribe, report offerer · `/report` `/rp` — report criminal activity · `/respond` `/yes` — respond to last call · `/robberies` `/robs` `/rb` — list active robberies · `/suspects` `/sus` `/wanted` `/wan` — list suspects · `/ticket` `/tk` — ticket a non-warrant suspect · `/vehrepair` `/vehfix` — repair vehicle (10–20k) · `/visualcontact` `/vc` — report visual contact · `/warrants` `/war` — list warrants.

### Action / Services
`/adrenaline` `/ad` · `/appeal` `/jailappeal` · `/atm` · `/bail` `/pb` `/paybail` · `/bankrob` `/robbank` · `/beer` `/drinks` `/booze` · `/belt` · `/box` · `/breakout` `/bo` `/brk` · `/bribe` `/br` (1k–15k) · `/buy` `/purchase` · `/cancel` · `/cancelhit` · `/casinorob` `/robcasino` · `/cell` `/jailcell` · `/challenge` `/ch` · `/complain` · `/courier` `/smuggle` · `/cnrradio` · `/crowbar` · `/cure` `/c` · `/cureme` `/cme` · `/delivery` `/truck` `/del` · `/deposit` `/dep` · `/dice` · `/driver` `/taxi` `/limo` `/bus` `/air` · `/drop` · `/enter` · `/escape` `/esc` · `/exit` · `/fakeskill` `/fskill` · `/food` `/pizza` · `/givecash` `/gc` `/$` · `/givegift` · `/givepet` · `/giverep` · `/heal` `/h` `/medic` · `/healcure` · `/healme` · `/hit` · `/holdup` `/hup` `/storerob` · `/horsebet` · `/housedelivery` · `/infect` `/if` · `/items` `/isell` `/is` · `/jury` `/juror` · `/kidnap` `/kd` · `/kidnapall` `/kda` · `/login` · `/lotto` (draws 18:00) · `/mechanic` `/mech` · `/mission` `/sellmission` `/mis` · `/moneybag` · `/parole` `/pa` · `/payticket` `/pay` · `/police` `/911` · `/possess` (Halloween) · `/prices` `/setprices` · `/ransom` `/ran` · `/rape` `/ra` · `/refill` `/rf` · `/register` · `/release` `/rel` · `/releaseall` · `/rob` `/rb` · `/robbery` · `/carsell` `/vsell` `/sc` · `/sex` `/sx` · `/shares` `/stocks` · `/sharessell` · `/shoplift` · `/spawn` `/newlife` · `/strip` · `/sue` · `/tip` · `/tree` (Christmas) · `/ups` · `/weapons` `/ws` · `/withdraw` `/wit`.

### Information
`/animations` · `/calls` · `/challengerecords` · `/citystats` `/cstats` · `/commands` `/cmds` · `/crimes` · `/dmrecords` · `/faq` · `/getid` `/id` · `/help` · `/hitinfo` `/hi` · `/hitlist` `/hits` · `/information` `/info` `/i` · `/insuranceinfo` `/ii` · `/inventory` `/inv` · `/jailinfo` `/ji` · `/jaillist` `/jl` · `/level` `/rank` `/lvl` · `/locate` `/loc` · `/markets` · `/moneyinfo` `/bankinfo` `/$i` · `/moneyrush` `/mr` · `/morestats` · `/myinfo` `/mi` · `/myrep` · `/party` · `/permits` · `/records` · `/rules` · `/sellinfo` `/carsellinfo` · `/stats` `/sts` · `/std` `/diseases` `/si` · `/time` `/day` · `/total` `/tot` · `/vehhelp` · `/version` `/ver` · `/wotd` `/workers`.

### Messaging
`/$set (1-3) [text]` (95 char) · `/1` `/2` `/3` · `/carwhisper` `/cw` · `/drivermsg` `/dm` · `/ignore` `/mute` · `/nopm` · `/pm` `/msg` `/m` · `/reply` `/r` · `/say` `/s` · `/set` · `/truckmsg` `/tm` `/cb` · `/whisper` `/w`.

### Drugs
`/bait` `/fertilize` · `/cook` · `/drugs` `/drg` · `/drugsell` `/selldrugs` `/ds` · `/givedrugs` `/gd` · `/givefreshdrugs` · `/harvest` `/harv` · `/hidedrugs [qty]` · `/hotbox` `/hb` · `/plant` `/seed` · `/pgps` · `/plantinfo` `/pinfo` · `/smoke` `/smk` (5g/press) · `/takedrugs` `/td` · `/trap`.

### Fishing
`/cooler` · `/fish` `/fsh` · `/fishbuy` `/fb` · `/fisheat` `/fe` · `/fishgive` `/gf` · `/fishhelp` · `/fishinfo` `/fi` · `/fishinventory` `/finv` · `/fishmsg` `/fm` · `/fishprices` `/fp` · `/fishrecords` `/frec` · `/fishrelease` `/frel` · `/fishrod` `/rod` `/pole` · `/fishsell` `/sf` · `/fishsellall` · `/fishslap` `/fs` · `/fishthrow` `/tb` · `/fishtour`.

### Houses
`/colist` `/cokeys` · `/hotel` · `/hotels` · `/house` · `/houseanswer` `/door` · `/housecoowner` `/coowner` · `/houseinvite` `/hinvite` · `/housekeys` `/hkeys` · `/housekick` `/hkick` · `/housefishstorage` `/fstore` · `/houserob` `/hrob` · `/houses` `/hlist` · `/housestorage` `/store` · `/keys` `/klist` · `/rent` `/rlist`.

### Vehicle
`/eject` `/ej` · `/ejectall` `/eja` · `/ejecteveryone` `/eje` · `/ejectme` `/ejm` · `/givecar` `/gkeys` `/gk` · `/instafix` · `/lock` `/lk` · `/unlock` `/ulk` · `/vehcolor` `/vehc` ($100, donors only) · `/vehhood` `/hood` · `/vehlights` `/lights` · `/vehtrunk` `/trunk`.

### Group / Gang
`/groupcall` · `/groupcash` `/g$` · `/groupcreate` `/grc` · `/grouphelp` · `/groupinvite` `/gri` · `/groupjoin` `/grj` · `/groupkick` `/grk` · `/groupleader` · `/groupleave` `/grl` · `/grouplist` · `/groupmsg` `/gm` · `/groups` `/gr`.

### Pimping
`/pimp` `/pimpoffer` · `/pimpcall` · `/pimpaccept` `/pimpyes` · `/pimphelp` · `/pimpinfo` `/pimplist` · `/pimpleave` · `/pimpleaveall` · `/pimpmsg` · `/pimprefuse` `/pimpno`.

### Pets
`/pet` `/pi` · `/petattack` · `/petcmds` · `/petdiet` · `/peteat` · `/petfeed` · `/petfight` · `/petname` · `/petslap` · `/petstats` · `/pettraining`.

### GPS
`/driverdest` · `/gps` · `/gpsclear` · `/gpsgocustom` `/gpsgo` · `/gpshide` `/gpsoff` · `/gpsloc` `/gpsdest` · `/gpsresume` · `/gpssetcustom` `/gpsset` · `/gpssettings`.

### No-Sell / Selling
`/nocalls` `/callsoff` · `/nosell` `/ns` · `/noselladd` `/nsadd` · `/noselladdall` · `/noselldel` `/nsrem` · `/nsremoveall` · `/noselllist` `/nslist` · `/sell` · `/sellmenu`.

### Clothes
`/clothes` · `/clothesdiscard` · `/clothesinv` `/cinv` · `/clothesposition` · `/clothesprice` · `/clothesremove` · `/clothessell` · `/clotheswear`.

### Animations (selection)
`/dance` (1-4) · `/showoff [1-23]` · `/showoff2 [1-23]` · `/sit [1-14]` · `/stab` (needs knife) · `/stabanim [1-4]` · `/gunpoint` · `/kiss` · `/slap` · `/piss` (civ only) · `/wank` (civ only) · `/handstand` · `/taichi` · `/wave` · `/come` · `/follow` · `/cpr` · `/mourn` · plus dozens more (see official page for the full ~70-command list).

### Settings
`/settings` `/options` `/set` · `/skill` `/skl` · `/gender` · `/display` `/disp` · `/displayost` `/ost` · `/deathmsg` · `/helpmsg` · `/joinmsg` `/partmsg` · `/gtamenu` `/menus` · `/tdsettings` · `/ostsettings` · `/casinosettings` · `/changepassword` · `/canims` `/aoff` · `/clearost` `/cs`.

---

## 14. GitHub clone / reference gamemodes [V] — executable spec

These encode mechanics we can read directly (Pawn). Best used when the wiki/site is silent on exact constants.

| Repo | URL | Fidelity to CB:CNR | What it encodes |
|---|---|---|---|
| **sf-cnr** (PatrickGTR) | `github.com/PatrickGTR/sf-cnr` | High-scope but its **own** balance (San Fierro C&R, "first C&R to allow DM"). Not a 1:1 CB clone, but the systems overlap heavily. | store/player robbery, kidnap-for-ransom, ATM theft, security-truck heists, bank/drug-house/ship robberies, jail breakout via explosives, Alcatraz raid, jobs (firefighter, lumberjack, mining, meth, trucking), stocks, casino/poker/8-ball, houses/garages/gates/businesses, gang wars, hits, paintball, battle royale, racing. Dirs: `/gamemodes /filterscripts /plugins /npcmodes /scriptfiles`. MIT. **Best large open reference for reading exact Pawn implementations.** |
| **Kai's C&R** (Sorenkai) | `github.com/Sorenkai/Kai-s-Cops-and-Robbers` | Early / minimal. | login-register (SQLite), admin, class system, robbing system + first stores. Good for a *clean minimal* pattern, not for numbers. |
| **OSG-CNR** (UnholyBeast) | `github.com/UnholyBeast/OSG-CNR` | WIP "Cop's and Robbers" in Pawn; explicitly a CNR-style GM. | filterscripts + gamemodes; thin README — must read source. |
| **GCnR** (bharel) | `github.com/bharel/GCnR` | "Game's Cops and Robbers" (gcnr.net). | separate C&R lineage; another reference. |
| **This project** | `github.com/Arose-Niazi/Exp-Gaming-Cops-And-Robbers` | our own WIP. | — |

**Recommendation:** for any constant the official site/wiki doesn't give (e.g. exact item prices, refill costs, ammo prices, per-skill daily pay), read `sf-cnr` Pawn source first, then reconcile against the wiki's stated caps before we pick our own value.

---

## 15. Gaps → design-our-own [I]

Where CB:CNR sources are thin/silent. The activities/economy researchers should design these as classic-style values, cross-checked against sf-cnr:

- **Exact item prices** — site lists items, never prices. GAP. (Life/Health Insurance, permits, crowbar, rod, adrenaline, ammo bag, repair kits, condoms, seeds, donuts.)
- **Exact jail-time / bail formula** — only "scaled by past + current crimes" is stated; no numbers. GAP → design a per-wanted-level base + per-crime stack.
- **Wanted-level decay rate** — "slowly decreases over time"; no seconds-per-level. GAP.
- **Per-skill daily pay** — only Food Delivery's "+$500/item next day" is cited (unverified). GAP for all others.
- **Lotto exact prize / ticket cost** — draw 18:00 and +5 score known; jackpot math unknown (Fandom Lotto page empty). GAP.
- **Casino odds / house edge** for slots/blackjack/video-poker. GAP (only horse 2:1–10:1 and scratch ~2× given).
- **Fish species / prices / catch-weight table.** GAP.
- **Drug price curve** at refill point vs player. GAP (only min 200g sale + "lower than player" stated).
- **Rape / STD damage-over-time numbers** and STD tier damage values. GAP (only ordering Chlamydia→Mary Lou).
- **Pet system numbers** (training, protection %, diet). GAP — commands known, mechanics thin.
- **Group/gang mechanics beyond commands** (bonuses, wars). GAP — CB list is command-only; sf-cnr has full gang wars to model on.
- **Bank interest exact tiers** — 5–15%/2days headline is a player estimate (ToRro's own example is inconsistent). Verify/redesign.
- **Wealth-tax threshold** — $7M is Beta-12.8-era and explicitly "can change." Treat as a starting figure only.
- **Forum-only content** (v24 command update, drug planting guide, money-making thread) not pulled this sweep because `forums.crazybobs.net` was unreachable via our tooling — worth a manual pass in a browser for the freshest v24 numbers.
