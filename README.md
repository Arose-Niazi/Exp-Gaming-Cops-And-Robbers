# 🚔 EXP Gaming — Cops And Robbers (open.mp / SA-MP 0.3.7)

> The revival of the **EXP GAMING Cops And Robbers** gamemode — ported from the
> 2017 SA-MP 0.3.7 codebase to the modern **[open.mp](https://open.mp/)**
> framework, still fully playable with standard **SA-MP 0.3.7** clients
> (no 0.3.DL required).

![Platform](https://img.shields.io/badge/platform-open.mp%20%7C%20SA--MP%200.3.7-brightgreen)
![Language](https://img.shields.io/badge/language-Pawn-orange)
![Compiler](https://img.shields.io/badge/compiler-QAWNO%203.10-blue)
![Status](https://img.shields.io/badge/status-gameplay%20complete%20(M2–M7)%20%E2%9C%94-brightgreen)

> 🗃️ The original SA-MP 0.3.7 tree is preserved under [`legacy/`](./legacy)
> and released as
> [**v0.3.7-legacy**](https://github.com/Arose-Niazi/Exp-Gaming-Cops-And-Robbers/releases/tag/v0.3.7-legacy).

---

## 📖 About

A cops-vs-robbers gamemode for Grand Theft Auto: San Andreas multiplayer set in
**Los Santos** (San Fierro / Las Venturas scaffolded for later). MySQL-backed
accounts, four law-enforcement teams plus civilians across ~240 skins, streamed
interiors and shop NPCs, a road-following GPS, object elevators, and a
textdraw-heavy UI. Scripted by **Arose Niazi** (started October 2017; ported to
open.mp July 2026).

---

## ✨ What's new vs. the legacy snapshot

| Area | Legacy (SA-MP 0.3.7) | open.mp port |
|------|----------------------|--------------|
| Server | `samp-server` 0.3.7-R2 | **open.mp** (`omp-server`) + components |
| Compiler | pawno (Pawn 3.2) | **QAWNO** (Pawn 3.10), 0 errors / 0 warnings |
| Config | `server.cfg` | `config.json` (+ `config.test.json`) |
| Plugins | 10 legacy plugins | components + 5 legacy plugins (crashdetect, mysql, streamer, whirlpool, RouteConnector) |
| IRC bridge | 5 bots on a private network | retired (no-op wrappers ready for a Discord bridge) |
| Geolocation | plugin binary missing | include-only SQLite GeoIP (bundled `geoip.db`) |
| Security | master-password backdoor, unguarded teleports/commands | **removed / rank-gated** |
| Database | no schema shipped | `scriptfiles/cnr.sql` + `db_config.inc` credential toggle |
| Known bugs | timer arg, settings-load, column typos | fixed at the port |
| Deployment | manual | **Docker** (Ubuntu 24.04, healthcheck, compose files) |

---

## 🧰 Requirements

- **open.mp server** (`omp-server` / `omp-server.exe`) — bundled.
- **QAWNO** Pawn compiler — bundled under `qawno/`.
- **open.mp components** (`components/`) — bundled; auto-load.
- **Legacy plugins** (bundled under `plugins/`): crashdetect, mysql R41-4
  (+ `log-core*`, `libmariadb.dll` at the root), streamer, whirlpool,
  RouteConnector (needs `scriptfiles/GPS.dat` — bundled).
- **MySQL / MariaDB** server — schema in `scriptfiles/cnr.sql`.
- A **SA-MP 0.3.7** client to play.

Everything needed to build and run is committed to the repo (self-contained).

---

## 🚀 Build & Run

```bash
# 1. Clone
git clone https://github.com/Arose-Niazi/Exp-Gaming-Cops-And-Robbers.git
cd Exp-Gaming-Cops-And-Robbers

# 2. Compile the gamemode with QAWNO (Windows)
qawno/pawncc.exe "-;+" "-(+" "-\\" "-Z-" "-igamemodes" "-iqawno/include" -d3 -t4 "-ogamemodes/CnR" "gamemodes/CnR.pwn" WINDOWS_COMPILER=1
#    (Linux/CI: use ./qawno/pawncc and drop WINDOWS_COMPILER=1)
#    In VS Code: press Ctrl+Shift+B.   Batch: python compile.py

# 3. Database
mysql -u root -p -e "CREATE DATABASE cnr CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci"
mysql -u root -p cnr < scriptfiles/cnr.sql
#    Credentials: gamemodes/CnR/server/db_config.inc (LOCAL_DB toggle in CnR.pwn)

# 4. Run
omp-server.exe                                   # Windows
./omp-server                                     # Linux
omp-server.exe --config-path config.test.json    # local test config (port 7778, LAN)
```

Then connect with a **SA-MP 0.3.7** client on port **7777**.

> ⚠️ **Before deploying:** change the placeholder RCON password in
> `config.json` (`CHANGE_ME_cnr_rcon_2026`). Never commit real secrets.

### 🐳 Docker

```bash
docker compose up                          # dev: server + MariaDB
docker compose -f docker-compose.prod.yml up -d   # prod: build + ro .amx mounts
```

`.amx` files are volume-mounted read-only, so routine updates are
`git pull` + container restart — no image rebuild.

---

## 📁 Project structure

```
Exp-Gaming-Cops-And-Robbers/
├── gamemodes/
│   ├── CnR.pwn              # entry point (compile → CnR.amx)
│   └── CnR/
│       ├── players/         # auth, loading, spawns, peds, menus, messages,
│       │                    #   vehicles, elevators, base commands
│       ├── systems/         # gameplay systems (wanted, jail, bank, housing,
│       │                    #   robberies, missions, stocks, fishing, farming …)
│       ├── cmds/            # command modules (admin, arrest, crime, economy,
│       │                    #   robbery, property, job, arena, DJ, owner …)
│       └── server/          # defines, colors, db_config, zones, interiors,
│                            #   actors, GPS engine, misc
├── components/              # open.mp components (auto-load)
├── plugins/                 # legacy plugins (crashdetect, mysql, streamer,
│                            #   whirlpool, RouteConnector)
├── qawno/                   # QAWNO compiler + modern includes
├── scriptfiles/             # GPS.dat, geoip.db, cnr.sql, map data
├── legacy/                  # preserved SA-MP 0.3.7 tree (see v0.3.7-legacy)
├── config.json              # open.mp server config (0.3.7 clients allowed)
├── config.test.json         # local test profile
├── compile.py               # batch compiler (gamemode + filterscripts)
└── Dockerfile / docker-compose*.yml
```

---

## ✅ Full feature set (M1–M7 shipped, 0 errors / 0 warnings)

### 🏗️ Foundation (M1 — scaffold)
Accounts (MySQL + Whirlpool, auto-login), classes/teams (~240 skins; CIVIL,
POLICE, SHERIFF, FBI, UC_COP), 466-zone name HUD, streamed interiors with
teleport pickups (admin builder), shop/service NPC actors (admin builder),
RouteConnector GPS with on-road arrows (`/gps`), persistent admin-spawned
vehicles, two object-elevator systems, textdraw menu framework, accelerated
game clock + weather, rank hierarchy with scoped messaging, proxy/VPN
detection.

### 🛡️ Moderation & accounts (M2)
- Rank tiers: `SERVER_PLAYER` → `SERVER_OWNER` + `SERVER_SCRIPTER` (Scripter)
- PM / reply system (`/pm` `/reply` `/r` `/nopm` `/ignore`), flood throttle
- Full admin suite: `/warn` `/kick` `/ban` `/unban` `/mute` `/ajail` (fixed),
  `/aduty`, `/aka` `/ips` (offline-capable), `/showcommands`, `/akick`
- Native ban system (IP, serial, name) — replaces legacy Bans filterscript
- Teleport commands + DM-zone flags; owner/scripter grant commands

### 🔫 Core CnR loop (M3)
- **10-level wanted system** — star HUD, cop visual-contact decay (wanted drops
  if no cop has line-of-sight for the decay window)
- **Cop commands:** `/arrest` `/ticket` `/vc` (visual contact) `/su` (suspect
  upgrade) `/cuff` `/backup`; rank gates on special cop abilities
- **Jail:** bail, bribe, jury appeal (`JURY_SIZE 15`), escape, breakout;
  spectator lock while incarcerated
- **Crimes:** `/rob` (skill-capped pickpocket), `/rape` + STD infection system,
  `/takedrugs` + overdose mechanic; fire "grilled" death fix
- Spawn protection, safe zones, DM zones; Routine Patrol + Domestic
  Disturbance missions

### 💰 Economy & events (M4)
- **Bank:** `/deposit` `/withdraw` `/atm` `/givecash`; interest, tax, insurance
- **Lotto:** jackpot draw fires on the game clock (`clock-tick` fix); buy
  tickets in-game
- **Moneybag / Moneyrush** pickups; `/ad` adrenaline boost + `/advert`
- Store holdup base; Drug Delivery, Truck Delivery, Trash Pickup, Food
  Delivery missions

### 🏠 Property & big crime (M5)
- **CB-simple vehicle ownership:** `/lock` / `/unlock` on last-driven car;
  GTA-theft detection; dealership flow
- **Full robberies system:** shoplifting, store holdup, bank robbery (per-branch
  cooldowns), casino robbery, house robbery, special robbery (v23-style),
  crowbar for forced entry
- **Housing:** buy / sell / rent / storage / spawn point / superlock; house
  actor and pickup set
- **Kidnap:** bind + ransom flow
- Vehicle Theft, Holdup Mission, Airport Robbery missions

### 🎣 Jobs, activities & arenas (M6)
- Criminal / civilian **skill trees**; **fighting styles** (learned)
- **Clothes** system (outfit slots, gender toggle)
- **Fishing** (`/fish`, rod item, catch table)
- **Drug farming** (plant → harvest cycle) + **legal crop job**
- **DM Stadium**, **sniper arena**, **duel arena**
- **GPS destination categories** fully wired (previously engine-only stubs)
- **DJ radio** (stream control commands)
- All remaining missions — **19 missions total** across all categories

### 📈 Stock market & full economy (M7)
- **21 tradable stocks** — buy / sell / portfolio HUD
- Stock prices multiply weapon shop / 24-7 / house / fish / crop prices
  (economy integration table)
- **Prime rate** wiring; **dividends** + bankruptcy events
- **Daily market tick** fires on the game clock; `stock_reports` table keeps
  last 30 periods

## 🗺️ Status / what's next

The full CB:CNR feature set is implemented and committed (M2–M7, 0/0
build, booting). Remaining work is tuning and polish:

- **Not yet populated:** San Fierro and Las Venturas city areas (scaffolded,
  no gameplay content placed)
- **Deferred niceties:** hunting AI, fishing tournaments, 24/28 race
  challenges, cash weapon/vehicle shops that would light up the AMMUNATION /
  `VEHICLE_DEALERSHIP` stock feeds, ATM / shop proximity gates, co-owned houses

See **`docs/GAMEPLAY-DESIGN.md` §14** for owner tuning questions (prices,
cooldowns, balance) and **§16** for code follow-ups from site research
(constants / behaviours to adjust in already-built stages).

---

## 🙏 Credits

**Arose Niazi** (script) · the **SA-MP team** and **open.mp team** · **Zeex**
(crashdetect) · **Incognito** (streamer) · **maddinat0r** (mysql) ·
**Whitetiger** (geolocation) · **Y_Less** (whirlpool, sscanf2) · **emmet_**
(sscanf2) · **YourShadow** (Pawn.CMD) · **Gammer_Z** (RouteConnector).
