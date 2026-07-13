# CB:CNR Research — The Crime / Cop Core Loop

**Scope:** How crime, wanted levels, cops, arrests, jail, drugs/rape (STDs), deathmatch restrictions, and anti-cheat worked on CrazyBob's Cops And Robbers (CB:CNR / CnR), the classic SA-MP server (crazybobs.net, closed 31 Dec 2024 after 18 years). This document is the reference for reviving these systems in our own gamemode.

**Legend:**
- **[VERIFIED]** — sourced from a citation below. URL noted inline.
- **[INFERRED]** — our reconstruction / design proposal where sources were thin or the feature was never in CB:CNR.

**Primary sources used (all consulted for this doc):**
- Official site (still serving cached help pages): `crazybobs.net/website/` — pages: `how-play`, `cnr-commands`, `crimes-jail-sentence`, `robberies`, `drug-planting-and-hunting`, `rules`.
- Fandom wiki (`crazybobs.fandom.com`) — pages read in full: `Wanted_Levels`, `Police_Commands`, `Police_guide`, `How_to_play`, `Deathmatch`, `Drugs`, `Drug_Commands`, `Jail`, `Kidnapper`, `Car_Jacker`, `ToRro's_Guide_To_Everything_CnR`.
- Search-surfaced wiki text: `Rapist`, `Prostitute`, `Drug_Dealer`, `Score_Guide`, `Rules`, `Untrue_Hacks`, `FAQ`.

> **Note on the CB:CNR economy scale:** money values in guides are large (hundreds of thousands to millions). Our numbers should be re-tuned to our own economy — treat CB:CNR cash figures as *ratios/intent*, not literal targets.

---

## 1. Roles: how you become a Cop vs a Criminal

CB:CNR is class/skill-based, exactly like our model (skin/class choice). [VERIFIED — `how-play`, `ToRro's Guide`, `Police_guide`]

**Player icon / marker colors** (this is the whole visual language of the loop) [VERIFIED — `How_to_play` wiki]:
| Color | Meaning |
|---|---|
| **Blue** | Law Enforcement (shade lightens with rank) |
| **Purple** | Cop requesting backup |
| **White** | Innocent civilian |
| **Yellow** | Suspect (wanted 1–5, ticketable) |
| **Orange** | Warrant-Issued Suspect (wanted 6–8, arrestable) |
| **Dark Orange** | Most Wanted (wanted 9–10) |
| **Green** | Driver On Duty (innocent) |

### Law Enforcement skills
All LE skills can arrest. [VERIFIED — `ToRro's Guide`]
- **Police Officer** — ticket + arrest, sees wanted list, can /report innocents. "Good starting skill."
- **Police Arms Dealer** — sells weapons to police only; can still arrest.
- **Public Medic** — can **force-cure** STDs on anyone without consent, heal, sell cures/condoms; can arrest.
- **Temporary Cop** — a civilian who steps into the temp-cop checkpoint inside the PD. Needs score >10 and no recent serious crimes. Keeps own skin, limited abilities, **cannot /refill in PD, cannot receive bribes, earns no daily pay**, loses rank on leaving force or city change. [VERIFIED — `Police_guide`, `ToRro's Guide`]

### Civilian skills relevant to the crime loop
[VERIFIED — `ToRro's Guide`, individual skill pages]
- **Rapist** — high rape success; better chance to pass an STD they don't have. (Advantage becomes marginal because any class carrying several STDs already rapes at ~100%.)
- **Drug Dealer** — sells drugs, holds up to **5000 g** (vs 500 g for other civ skills), buys cheaper. "Best class for a rapist with STDs" (carry huge drug stock to survive STDs).
- **Pickpocket** — robs up to **$100,000**, very high success rate. Has a mission: rob a total of **$10,000** from different players within **6.5 game hours**.
- **Con Artist** — robs up to **$500,000**, lower success than pickpocket; small stacking chance to dodge taxes.
- **Car Jacker** — can steal **locked** vehicles (except admin cars); crane-sell cooldown reduced by 3 min (5 min vs 8 min). +1 score per car sold.
- **Kidnapper** — see §2.6.
- **Hitman** — kills players who have hit contracts for the placed bounty.
- **Prostitute / Private Medic** — can pass/sell STDs and small amounts of drugs; can attempt to infect.
- **Arms Dealer, Street Vendor (Items Dealer), Driver, Food Delivery** — support/legal skills.

**Design note for us:** our class/skin choice maps 1:1. Keep the color legend above verbatim — it is the single most important readability contract of the whole mode.

---

## 2. Crime System — which actions are crimes and how each raises Wanted

Core principle [VERIFIED — `crimes-jail-sentence`]: *"Any crime committed will increase your Wanted Level based on its severity."* Crimes **stack** on top of existing wanted level. [VERIFIED — `Wanted_Levels`]

> **Important gap:** CB:CNR published its per-crime wanted values only as an **image** on `crimes-jail-sentence` (not machine-readable), and the exact point values were never in plain text on any source I could reach. Values below marked [INFERRED] are our reconstruction, consistent with the tier thresholds (1–5 suspect, 6–8 warrant, 9–10 most wanted) and with the specific verified anchors noted.

### 2.1 Crimes and their commands
[VERIFIED command syntax — `cnr-commands`, `Police_Commands`, `Drug_Commands`, skill pages]

| Crime action | Command(s) | Notes |
|---|---|---|
| Rob a player (steal cash) | `/rob` `/rb` | All civilians. Amount depends on skill (pickpocket ≤100k, con artist ≤500k). |
| Rape a player | `/rape` `/ra` | Leave nick/id blank to target closest player. Spreads STDs (§4). |
| Store holdup | `/holdup` `/hup` `/hold` `/storerob` `/robstore` | Must be **looking at a store clerk**. Alarm alerts cops. |
| Bank robbery | `/bankrob` | ~30 s wait at checkpoint; drains random players' bank accounts (max $100k each). |
| Casino / Federal Mint / special robberies | `/robbery` (+ `/robberies` `/robs` `/rb` for cops to list active) | Deliver the safe to a random hideout for payout. |
| Shoplift | `/shoplift` | At shop checkpoints. |
| Car theft / jacking | (enter vehicle; Car Jacker can take **locked** cars) | Selling stolen cars at the crane. |
| Give car keys (facilitation) | `/givecar` `/givekeys` `/gkeys` `/gk` | |
| Drug dealing / possession | `/drugs` `/takedrugs` `/givedrugs` (§4) | Growing, cooking, selling all illegal without permit. |
| Kidnapping | `/kidnap` `/kd` (§2.6) | **Always gives a warrant.** |
| Drive-by | (shooting from vehicle) | *Not documented as a distinct scripted crime; treated as attacking → self-defense/DM rules apply.* [INFERRED] |
| Escape jail | `/escape` `/esc` | Instantly sets **Wanted Level 10** (§3, §5). [VERIFIED — `Jail`] |
| Drunk driving | (drink too much alcohol, then drive) | Auto **Wanted Level 5**. [VERIFIED — `ToRro's Guide`] |
| Kill deer without hunting permit | — | Raises wanted level. [VERIFIED — `drug-planting-and-hunting`] |

### 2.2 Verified wanted-level anchors (exact values that ARE sourced)
- **Escaping jail → Wanted Level 10 (Most Wanted).** [VERIFIED — `Jail`]
- **Drunk driving → automatic Wanted Level 5.** [VERIFIED — `ToRro's Guide`]
- **Car-sell mission: getting into the final car on your list → automatic Wanted Level 10.** [VERIFIED — `ToRro's Guide`]
- **Courier mission: picking up the object → Wanted Level 6 (warrant).** [VERIFIED — `ToRro's Guide`]
- **Drug delivery mission: +2 wanted per delivery, but ONLY if a cop is nearby** (no warrant unless a cop is present 3 times). [VERIFIED — `ToRro's Guide`]
- **Kidnapping always issues a warrant** (i.e. jumps you to ≥6). [VERIFIED — `Kidnapper`]

### 2.3 Proposed per-crime wanted values [INFERRED — for our script]
Built to respect the verified anchors and the 1–5 / 6–8 / 9–10 tiers:
| Crime | Wanted added (proposal) |
|---|---|
| Petty rob / pickpocket | +2 |
| Rape | +2 (+1 if it infects) |
| Store holdup | +4 (lands you as a warrant if repeated) |
| Shoplift | +2 |
| Car theft (locked/jacked) | +3 |
| Drug possession caught / dealing near cop | +2 |
| Bank / casino / mint robbery | set to **6** on start (instant warrant + alarm), climb to 9–10 while carrying safe |
| Kidnapping | set to **6** (instant warrant, verified) |
| Murder of an innocent | +4 to +6 (flag as "serious"/violent → blocks `/appeal`, blocks bribe once at 10) |
| Drunk driving | set to **5** (verified) |
| Jail escape | set to **10** (verified) |

### 2.4 Crime reporting — by cops, not victims
This is a subtle but crucial CB:CNR detail: **most crimes auto-raise wanted; the `/report` command is a COP tool, not a victim tool.** [VERIFIED — `Police_Commands`, `cnr-commands`]
- `/report` `/rp` `<nick/id> <reason>` — **police** report criminal activity; raises the player's wanted level. *"You may only report players for committing serious crimes (see The Rules)."* Abuse → kick/ban.
- `/cancellastreport` `/cancelreport` `/clr` — cancels your last report, restoring the player's previous wanted level. Used for accidental reports or when the suspect complies after being reported. [VERIFIED — `Police_Commands`]
- `/visualcontact` `/vc` `/vcontact` — cop reports visual contact; **freezes the suspect's wanted-level decay for a time** so it can't tick down while being chased. [VERIFIED — `Police_Commands`]
- There is **no victim `/report`**; scripted crimes (rob, rape, holdup, etc.) raise wanted automatically. Witnesses don't file reports — cops do, for serious/observed behavior. [INFERRED from absence + verified cop-only report]

**Design note:** implement automatic wanted increases inside each crime handler; expose `/report` + `/clr` only to LE; add `/vc` to pause decay during active pursuit. This prevents victim-side report abuse while letting cops escalate genuine bad actors.

### 2.5 Robbery payout mechanics
[VERIFIED — `robberies`, `ToRro's Guide`]
- **Payouts scale with server population** (number of cops + civilians online). Higher when busy.
- **Alarm/notification:** cops are alerted **only when the alarm goes off**; a warrant is issued on the robber.
- **Losing the cops before reaching your hideout reduces your wanted level** (a get-away reward).
- Bank robbery pulls from **random** players' bank accounts, max **$100,000 per victim**; typical full-server take ~$500k–$1M.
- **Crowbar** (bought at Truck Stops / RC shops) lowers robbery time and raises success chance.

### 2.6 Kidnapping (a distinctive crime worth cloning)
[VERIFIED — `Kidnapper`]
- Kidnapper disguises as a Driver (green). Victim enters thinking it's a taxi → `/kidnap (ransom)` locks them in.
- **Always gives a warrant.**
- Commands: `/kidnap` `/kd (nick id) (amount)`, `/kidnapall` `/kda`, `/release` `/re`, `/releaseall` `/relall`, `/fakeskill` `/fskill`, `/ransom` `/ran`.
- Ransom up to **$50,000**. On delivering victim to hideout, steal **20%–80% of victim's on-hand cash**. Small bonus on kidnap and on delivery.
- If victim `/quit`s while kidnapped (not yet in hideout), kidnapper is still paid and victim still loses the money.
- Victim in a hideout has a **time-to-live**; if not released, they die (and pay medical fees on rejoin). Kidnapper gets a small bonus on that death.

---

## 3. Wanted Levels — the spine of the loop

[VERIFIED — `Wanted_Levels`, `Police_guide`, `How_to_play`]

**Ten wanted levels, three tiers, mapped to marker + chat-name color:**

| WL | Tier | Radar/name color | Cop can… |
|---|---|---|---|
| 1–5 | **Suspect** | Yellow | **Ticket only** (cannot arrest) |
| 6–8 | **Warrant-Issued Suspect** | Orange | **Arrest** (must be on foot) |
| 9–10 | **Most Wanted** (also warranted) | Dark Orange | Arrest OR **take down with lethal force for a reward** |

Mechanics:
- **Crimes stack** on existing WL. [VERIFIED]
- **Decay over time:** WL slowly decreases; a suspect can degrade from a higher tier back down (e.g. WL6 warrant → WL5 suspect). [VERIFIED — `Wanted_Levels`]
- **Decay is paused** while a cop has `/visualcontact` on the suspect. [VERIFIED — `Police_Commands`]
- **Losing all pursuing cops during a robbery reduces WL.** [VERIFIED — `robberies`]
- **Confessing at churches** lowers WL; **bribing** lowers WL; **avoiding cops for a long time** lowers WL. [VERIFIED — `crimes-jail-sentence`]
- **Ticket → warrant escalation:** a ticketed Suspect who **fails to pay the ticket in time becomes a Warrant-Issued Suspect** (arrestable). [VERIFIED — `Wanted_Levels`]

**Suspect marking / "/su":** CB:CNR did **not** use a distinct `/su` "mark suspect" command the way some servers do. The equivalent is `/freeze` `/fr` `/pullover` `/pu` (aliased around the "su" idea) — an order to the suspect to comply, plus `/report` to raise WL. [VERIFIED — `Police_Commands`, `Police_guide`] Cops don't manually "flag" a clean player as a suspect; committing a scripted crime is what marks you.

**Spawn / self-defense protection:** the rules protect *innocents*, not "spawn timers":
- Cops **may not** attack innocent civilians (white) or other cops (blue); **only shoot in self-defense or to stop a fleeing suspect.** [VERIFIED — `rules`, `Deathmatch`]
- A **police officer may not shoot/attack anyone in jail, even if they're committing crimes** — this is explicitly deathmatching. [VERIFIED — `Deathmatch`]
- Innocent (white) players are effectively DM-protected: attacking them for no reason is bannable. [VERIFIED — `Deathmatch`, `rules`]
- **[INFERRED design]** implement a short spawn-protection timer (e.g. 5–10 s) during which a freshly-spawned player can't be damaged and can't damage others; CB:CNR leaned on rules + admins rather than a hard script timer, but a script timer is the safer classic-feel choice for us.

---

## 4. Cops — commands, arrest mechanics, rewards, rules

### 4.1 Full police command set
[VERIFIED — `Police_Commands`, `Police_guide`, `cnr-commands`]

| Command | Aliases | Effect |
|---|---|---|
| `/arrest` | `/ar` | Arrest specified player, or **closest wanted suspect** if none given. Target must be **warranted (WL6+) and on foot**. Cuffs them (gun pointed, hands up), disarms, sends to jail. |
| `/ticket` | `/tk` | Ticket specified player or closest suspect. **Only issuable to Suspects (yellow, WL1–5) who aren't warranted.** Cop must **stay close to collect** the ticket. |
| `/freeze` `/pullover` | `/fr` `/pu` | Order suspect to pull over / freeze / surrender / pay ticket (context-dependent). |
| `/report` | `/rp` | Report a suspect for a serious crime → raises their WL. |
| `/cancellastreport` | `/cancelreport` `/clr` | Undo last report, restore previous WL. |
| `/visualcontact` | `/vc` `/vcontact` | Report visual contact → freezes suspect's WL decay for a time. |
| `/backup` | `/bk` | Request backup at your location (+optional msg). Requesting cops show **purple** on minimap. |
| `/copmsg` | `/cm` | Private message to all cops. |
| `/respond` | `/yes` | Respond to last call (backup/911/crime); updates the red call checkpoint, notifies others. |
| `/suspects` | `/sus` `/wanted` `/wan` | List all suspects. |
| `/warrants` | `/war` `/warrant` | List all warranted suspects (with WL, location, vehicle). |
| `/mostwanted` | `/most` `/mw` | List all Most Wanted (WL10). |
| `/robberies` | `/robs` `/rb` | List active holdups/robberies not yet stopped. |
| `/accept` | `/ac` | Accept an offered bribe. |
| `/refuse` | `/ref` | Refuse a bribe and report the briber. |
| `/jaillist` | `/jl` | List jailed players + remaining time + bail. |
| `/parole` | `/pl` | Release a jailed player **for free** once they've served their minimum. |
| `/donut` | — | Heal 50% HP; carry up to 6; at donut shops (SF, LS). |
| `/refill` | — | At PD: spend Refill Points on health/armor/weapons/ammo. |
| `/vehrepair` | `/vehfix` | Buy/use a vehicle repair kit (car must be stopped, on foot). |
| `/mission` | `/mis` | Patrol or Domestic Disturbance mission (see rewards below). |
| `/training` | `/train` `/policetrain` | New-cop tutorial. |

**No dedicated `/taze`, `/cuff`, `/frisk`, `/spikestrips` or `/roadblock` command existed as separate verbs in CB:CNR's documented set.** Cuffing is folded into `/arrest` (gun-point + hands-up animation). "Frisk" and tasing were not scripted commands. [VERIFIED absence — not present in `Police_Commands`/`cnr-commands`/`Police_guide`] Vehicle takedowns were done by ramming / the community "Lag PIT" maneuver, not a spike-strip command. [VERIFIED — `Police_guide`]

**[INFERRED — features to add for a modern-classic feel]** If we want `/tackle` (taze/cuff), `/frisk`, spike strips, and roadblocks, these were common on *other* SA-MP CnR-style servers. Closest classic designs:
- **`/tackle`/`/taze`** — on-foot, short range, stuns/drops the suspect for a few seconds so `/arrest` lands. Mirror the arrest proximity rule.
- **`/frisk`** — reveals a suspect's carried weapons/drugs/cash to the cop (info only); low WL requirement so it works on Suspects too. Keep it non-lethal and non-punitive to avoid abuse.
- **Spike strips / roadblocks** — deployable object at cop's position with a cooldown and a max count; bursts tires / blocks lane. Model as a placeable pickup like CB:CNR's deer traps.

### 4.2 Arrest mechanics (proximity, on-foot rule, sub-mission key)
[VERIFIED — `Police_guide`, `Police_Commands`]
- Arrest requires the suspect be **warranted (WL6+) AND on foot** — you cannot arrest someone in a vehicle; you must stop/disable the car first.
- `/arrest` with no ID targets the **nearest** warranted suspect (proximity-based, anywhere in the world — **not** only at a jail).
- Pressing the **sub-mission key (2)** in a cop car auto-picks the right action: `/ar` on nearest criminal on foot, `/pu` (pullover) on nearest criminal in a vehicle.
- On arrest: suspect is **disarmed**, gun-pointed, hands go up, and they teleport to the city's jail. [VERIFIED — `Jail`]

### 4.3 Arrest / cop rewards
[VERIFIED — `Score_Guide` snippets, `Police_guide`]
- **Ticket collected:** **+1 score** and a **$500 ticket-collection bonus** (to the cop). Suspect pays to reduce their WL.
- **Arrest (warrant):** **+1 score**; suspect disarmed + jailed; **cash bonus that scales with the suspect's wanted level.**
- **Killing a Most Wanted (WL10):** **+2 score** (the "takedown" reward — the only case where lethal force on a suspect is rewarded).
- **Daily pay** by rank + a **"Cop Of The Day"** bonus to the best cop each day.
- **Patrol mission:** reach all 5 random checkpoints in time — **$2,500 each + $25,000 all-checkpoint bonus = $37,500 total.** Domestic Disturbance: reach a random house fast; less time = more money. [VERIFIED — `Police_guide`]
- **Refill Points:** earned by doing cop work, max **10**; spent at PD to heal/armor/rearm; weapon availability gated by rank. [VERIFIED — `Police_guide`]

### 4.4 Cop rules (hard constraints for our script + admin policy)
[VERIFIED — `rules`, `Deathmatch`, `Police_guide`]
- **Cannot arrest a 0-wanted / innocent player** (script enforces: target must be WL6+). 
- **Cannot ticket a warranted player** (tickets only WL1–5) and **cannot ticket an innocent** (WL0).
- Cops may **only use force in self-defense or to stop a fleeing suspect.**
- Cops **may not attack innocents (white) or other cops (blue).**
- Cops **may not shoot anyone in jail**, ever — bannable DM.
- Cops **should not team up / drive around with criminals.**
- A wanted criminal **fighting back against a chasing cop is NOT DM** *if the criminal is genuinely trying to escape* (leave the city to lose WL). [VERIFIED — `Deathmatch`]

---

## 5. Jail — sentences, bail, bribe, appeal, escape

[VERIFIED — `Jail`, `crimes-jail-sentence`, `ToRro's Guide`]

**On being jailed:**
- Teleported to the current city's PD jail (LS Pershing Square, LV Roca Escalante, SF Downtown). Cell counts: **3 cells** in LS and SF, **6 cells** in LV. [VERIFIED — `Jail`, `Police_guide`]
- **Disarmed of all weapons.** Crowbar and fishing rod removed **if** your crime history includes robbery / fishing violations respectively. (Exception: if you died to a cop's lethal force, crowbar/rod are **not** removed — intentional.) [VERIFIED — `Jail`]
- A **jail time** and a **bail amount** are set based on past + current offenses. [VERIFIED]

**Four ways out of jail:**
1. **Serve the full sentence,** then either **pay bail** (`/bail` `/pb` `/paybail`) or have a cop **`/parole`** you (free, only after minimum served). [VERIFIED]
2. **`/appeal`** `/jailappeal` — type a reason; a **jury of 12** other players (cops, bots, or free civilians) votes innocent/guilty. *(One source says 12; the official page says "15 random jurors" — treat jury size as tunable; 12 is the wiki value.)* If innocent → jail time set to **0:00**. **Cannot appeal** with **< 1:00 remaining** or for a **serious offense (murder, etc.).** [VERIFIED — `Jail`; official page says 15]
3. **`/bribe [amount]`** a cop → cop chooses `/accept` or `/refuse`. Bribe range **$1,000–$15,000**. Accept → sentence + time reduced (scaled by remaining time, bail, and bribe). Refuse → jail time & bail **increased**, and you're charged with attempting to bribe. **Bribing happens near a cop (and historically at City Hall).** [VERIFIED — `Jail`]
4. **`/escape`** `/esc` — set to **Wanted Level 10 (Most Wanted)**, charged with **Jail Escape**, broadcast to all cops, spawn inside/outside PD. **Cannot bribe while Most Wanted.** If you escaped and get **re-arrested → chained to your cell, cannot attempt escape again** for the rest of the sentence. There's a **small chance a cop spots you mid-escape**, instantly chaining you. [VERIFIED — `Jail`, `crimes-jail-sentence`]
- **`/breakout` `/bo` `/brk`** — attempt to break **another** player out of jail. [VERIFIED — `cnr-commands`]

**Sentence-length model [INFERRED — exact formula was image-only]:** scale jail time with total wanted level at arrest + a per-crime history weight; bail ≈ a fraction of sentence value. Mark violent/murder crimes as "serious" to block `/appeal` and to block bribing once at WL10.

**Jail activities [VERIFIED]:** the cells are notorious for **raping** (both from inside and from outside the bars) — which is exactly why players carry drugs/condoms into jail; a **Public Medic** can force-cure STDs, making in-jail raping risky when good medics are around.

---

## 6. Rape / Drugs / STD system

### 6.1 Rape and STDs
[VERIFIED — `Drugs`, `ToRro's Guide`, `Rapist`/`Prostitute` snippets, `Police_guide`]
- `/rape` `/ra` (blank = closest player). Rape **can infect the victim with an STD** and **deals damage** even through a condom.
- **Rapist skill:** high rape success + better chance to pass an STD they don't personally have. Marginal advantage because **anyone carrying several STDs rapes at ~100% success.**
- **STD tiers:** from **Chlamydia** (mildest) up to **Mary Lou** (worst). STDs **drain health over time** and **kill you if untreated.** Some STDs are purchasable (hospital, Sex Shop, Private Medic) to intentionally load up for near-100% rape success.
- **Free/random STD sources:** bad drugs, random rapes, **Daisy the Cow** (Mad Cow Disease). [VERIFIED]
- **Protection & cures:**
  - **Condom** — bought at medics / hospital / 24-7; drastically reduces infection chance (you still take rape damage). 
  - **Chastity Belt** — *prevents* being raped at all, but **can break** (keep condoms as backup).
  - **Adrenaline Pill** (`/ad`) — **fully heals AND cures all diseases**; also saves you from a drug overdose.
  - **Public Medic** — can **force-cure** anyone (no consent needed).
  - **Drugs** — counteract STD health drain to buy time until cured (see below). [VERIFIED — `Drugs`, `Police_guide`]

### 6.2 Drugs (health / buzz / overdose)
[VERIFIED — `Drugs`, `Drug_Commands`, `drug-planting-and-hunting`, `ToRro's Guide`]
- **Drugs heal you over time.** Four buzz stages — **Low / Medium / High / Very High** (shown bottom-right) — determine heal speed and buzz duration. More drugs consumed = faster heal + longer buzz. Roughly **~5 s of effect per gram**, healing a small amount every ~1.5–2 s. 
- **Take drugs:** `/td` `/takedrugs [amount]` (press **F** to cancel animation). `/td` with **no amount** enters a mode where **each click = 5 g**; **F** exits.
- **Overdose:** taking **> 60 g at once** → overdose that **slowly kills** you. **Adrenaline pill** cancels the death. Max single intake is **60 g**. [VERIFIED]
- **Bad drugs:** random chance — forces a falling animation, **resets buzz to none**, and can **give you an STD.** [VERIFIED]
- **Buying drugs:** Drug Refill Points ("R" on map), Drug Dealer players (`/drugs`), bot dealers, prostitutes (via sex), Street Vendors ("white powder"), 24-7s/bars, cooking in a camper (`/cook`, needs lab equipment), or growing.
- **Carry limits:** Drug Dealer **5000 g**, other civilians **500 g** (drug bags extend these). [VERIFIED]
- **Growing:** buy seeds at 24-7/Refill Point/Bait Shop/Street Vendor (**carry up to 10 seeds**), `/plant` `/seed`, up to **5 plants** at once, a plant matures in **~20 min real time** yielding **up to ~200 g**, `/harvest` `/harv`. Plants don't grow while you're offline; **hippies/deer** can steal/eat them; plants **>~150 g start to die**. `/bait` `/fert` speeds growth but attracts deer; `/trap` sets deer/hippie traps; `/pgps` plant GPS; `/plantinfo`. **Cops see crops from farther away**; killing deer without a hunting permit raises WL. [VERIFIED]
- **Other drug commands:** `/drugsell` `/ds` (sell at Refill Points), `/givedrugs` `/gd`, `/givefreshdrugs`, `/hotbox` `/hb` (share buzz with vehicle passengers), `/smoke` (smoke animation, 5 g/click). [VERIFIED — `Drug_Commands`]

**Design note:** the drug/STD/rape triangle is the heart of CB:CNR's civilian-vs-civilian tension and its jail meta. Keep the loop: **rape → STD → victim must find drugs/adrenaline/medic → drug economy stays alive.** Overdose at >60 g and the adrenaline-pill escape are the two must-have safety valves.

---

## 7. Deathmatch (DM) restrictions & anti-random-DM enforcement

[VERIFIED — `Deathmatch`, `rules`]

CB:CNR is **not a deathmatch server**; there were **no "DM-allowed zones" as an official feature** (Dry Lake and Jefferson Motel were *player-culture* hotspots for DM groups, not sanctioned arenas). Random attacking/killing is **bannable**. Core rules:

- CrazyBob (verbatim intent): *"It's not a deathmatch server, so don't kill / attack people for no reason, or you will be kicked / banned. This includes random hit contracts."*
- **DM does not require a kill** — **attacking** for no reason (fists, melee, guns, vehicle weapons, parking on someone, damaging their occupied vehicle incl. tires/glass) is DM.
- Also DM: placing a hit **for no reason**; **cops shooting anyone in jail**; attacking over a **previous-city** incident; attacking for the **same incident multiple times** ("very, very bannable"); criminals **"hunting" cops or holing up to bait them** ("Jefferson Hotel Scenario"); placing **deer traps near buildings** to hurt people; **fishslapping to kill** for no reason; attacking someone **because they hit your friend** (you may only retaliate for something done to **you**).
- **NOT DM (legitimate combat):** a **wanted criminal fighting a chasing cop while genuinely trying to escape**; attacking the **hit placer** (killing them cancels the hit); attacking **hitmen who attack you first**; **self-defense** (cop↔civilian both directions).

**In-script enforcement CB:CNR relied on (mostly rules + admins, some script):** [VERIFIED — `Deathmatch`, `ToRro's Guide`]
- `/complain` to summon online admins; F8 screenshots + forum reports for offline evidence.
- Admins kick/ban (escalating with prior warnings); some admins theatrically minigun/cage/heli-blade DMers.
- Script-side: the wanted system itself channels aggression — a random shooter *"will eventually get a warrant and a cop can arrest him"*, turning DM into a cop-loop problem.

**Design proposals for us [INFERRED]:**
- **Damage gating:** script should suppress/track damage between two **white (innocent)** players — either block it outright in safe zones (PD, bank, RPC, hospital) or auto-flag the aggressor with wanted level so cops can respond.
- **Safe zones:** CB:CNR made the **Regular Player's Club (RPC)** a no-crime zone. [VERIFIED — `ToRro's Guide`] Replicate crime-disabled zones around PD/bank/spawn.
- If we *want* an optional consensual DM arena (Dry Lake style), gate it: teleport-in, no wanted, no arrests, no parachutes (see §8), separate world/interior — this was a *fan-server* pattern, not CB:CNR canon.

---

## 8. Anti-cheat bits relevant to us

[VERIFIED — `Untrue_Hacks` (via search), `FAQ`, `rules`, `ToRro's Guide`]

CB:CNR ran its own server-side **Anti-Cheat (AC)** that **auto-bans** on strong proof; third-party clients are blocked. Key detectable classes discussed:
- **Aimbot** — flagged by patterns like a player *"walking through gunfights constantly killing people"* with abnormal survivability.
- **Health hack** — player **maintaining constant/near-full health during sustained combat** (HP not dropping when it should).
- **Money hackers** — historically an issue; banned.
- Players report via **`/complain`** (online admin) and F8 screenshots → forums.

**Specific admin list-commands (`/aimbotters`, `/healthhack`):** these exact command names are **[INFERRED]** — plausible from admin tooling patterns but **not confirmed** in reachable CB:CNR docs (the AC internals were deliberately undocumented to avoid helping cheaters).

**Design proposals for our anti-cheat [INFERRED, modeled on classic SA-MP AC patterns]:**
- **Health-hack detection:** on each `OnPlayerTakeDamage`, log expected HP delta; if a player's HP fails to drop across N legitimate hits (or self-heals faster than any legit item allows), increment a suspicion counter → admin flag list (`/healthhack` = admin view of flagged players).
- **Aimbot flagging:** track hit-rate + snap-angle + headshot ratio over a rolling window; players exceeding thresholds land on an `/aimbotters` admin list for manual review (never auto-ban on this alone — false-positive risk, as CB:CNR itself occasionally mis-banned).
- **Anti-parachute / weapon restrictions in arenas:** on entering a DM arena or restricted interior, strip the parachute (weapon 46) and disallowed weapons each spawn/tick; CB:CNR-style servers commonly force-remove parachutes so players can't cheese vertical escapes in enclosed fight zones.
- Keep bans **evidence-gated** and reversible via an unban-request flow (CB:CNR's stated policy: *"bans are only issued after very good proof."*).

---

## 9. Quick reference — the core loop in one paragraph

A civilian picks a skill and commits scripted crimes (`/rob`, `/rape`, `/holdup`, car theft, drugs, kidnapping), each auto-raising a stacking **1–10 wanted level**. **WL1–5 = yellow Suspect** (cops `/ticket`, +1 score +$500), **WL6–8 = orange Warrant** (cops `/arrest` on-foot, +1 score + WL-scaled cash), **WL9–10 = dark-orange Most Wanted** (arrest or **kill for +2 score**). Wanted **decays over time** (paused by cop `/visualcontact`, reduced by escaping cops/confessing/bribing). Arrest disarms + jails the suspect at the city PD; they **serve time + bail**, `/appeal` to a 12-player jury, `/bribe` ($1k–$15k) a cop, or `/escape` (→ instant **WL10**, chained if re-caught). Rape spreads **STDs** that drain HP; **drugs** heal and stall STDs (**overdose >60 g**, cured by **adrenaline pill**), keeping a drug economy alive. **Random DM is banned** (no official DM zones — rules + admins + the wanted system enforce it), and a server-side **anti-cheat** flags aimbot/health/money hacks.

---

## 10. Open gaps to resolve during implementation
- **Exact per-crime wanted point values** — were image-only on `crimes-jail-sentence`; use the [INFERRED] table in §2.3 and tune in playtests.
- **Exact jail-time / bail formula** — image-only; §5 model is inferred.
- **Jury size** — wiki says 12, official page says 15; make it a config constant.
- **Drive-by** — never a distinct scripted crime; decide whether to add one (treat as attack + wanted, or ignore).
- **`/taze` `/cuff` `/frisk` / spike strips / roadblocks** — not in CB:CNR; §4.1 has classic-server proposals.
- **`/aimbotters` / `/healthhack` admin commands** — not confirmed; §8 proposes implementations.
- **Per-gram drug sell price, seed cost, exact heal-per-gram** — not published as numbers; tune to our economy.
