# 🚔 EXP Gaming — Cops And Robbers (SA-MP 0.3.7 Legacy Snapshot)

> A preserved snapshot of the **EXP GAMING Cops And Robbers** gamemode for
> **SA-MP 0.3.7** — scripted by **Arose Niazi**, started 11 October 2017 —
> locked here before the rebuild on **[open.mp](https://open.mp/)**
> (still targeting SA-MP **0.3.7** clients).

![Platform](https://img.shields.io/badge/platform-SA--MP%200.3.7--R2-lightgrey)
![Language](https://img.shields.io/badge/language-Pawn-orange)
![Compiler](https://img.shields.io/badge/compiler-pawno%20(Pawn%203.2)-blue)
![Status](https://img.shields.io/badge/status-legacy%20snapshot%20(WIP%20v0.01)-red)

---

## 📖 About

A cops-vs-robbers gamemode for Grand Theft Auto: San Andreas multiplayer,
built around a single active city (**Los Santos**; San Fierro / Las Venturas are
scaffolded via a `#define CITY_` switch but not populated). Persistence is
**MySQL** (maddinat0r R41-2 plugin) with **Whirlpool**-hashed passwords, and the
server is deeply integrated with a private **IRC** network for admin chat and
echo bots.

This snapshot is an **early work-in-progress (internal version 0.01)**: the
infrastructure is largely in place and neatly modularised, but the actual
cops-and-robbers gameplay loop (robberies, arrests, wanted levels, jail,
economy) was never built. It is preserved as-is — bugs, dev backdoors and all —
as the starting point for the open.mp rebuild.

---

## ✅ What's implemented

| System | Notes |
|--------|-------|
| **Accounts** | MySQL-backed register/login dialogs, Whirlpool password hashing, optional email, auto-login via stored IP + `gpci` serial match |
| **Ban system** | `Bans` filterscript (shared MG/MM ban DB): nick / IP / serial / range bans checked on connect |
| **Classes & teams** | ~240 selectable skins across CIVIL, POLICE, SHERIFF, FBI and UC_COP teams, with per-team class-selection cameras and textdraw UI |
| **Spawns** | 28 civilian spawn points + police/sheriff/FBI stations (Los Santos) |
| **Zone HUD** | 466 named map zones with enter/leave detection and a zone-name textdraw |
| **Interiors & teleports** | Admin builder (`/addinterior`, `/editinteriors`): streamed enter/exit pickups, per-interior weapon flags and access types stored in MySQL |
| **Actors (shop NPCs)** | Admin builder (`/addactor`, `/editactors`): 26 shop/service actor types (bank, 24-7, ammunation, …) with animations and 3D labels |
| **GPS** | RouteConnector-based pathfinding with on-road arrow objects and a textdraw route HUD — map-marker routing works (`/gps`, `/gpsoff`) |
| **Vehicles** | Admin spawner (`/addvehicle`, `/addvehicle2`, `/removevehicle`) persisting to MySQL; civilian/public/cop vehicle types |
| **Elevators** | Two full object elevator systems (SA-MP office building + Golden Brown apartments) with call buttons and floor queues |
| **Menu framework** | Reusable textdraw menu-box engine (chat-driven option selection) |
| **Game clock** | Accelerated in-game week (day/hour/minute), weather cycle, weekly stats persisted to MySQL |
| **IRC bridge** | 5 echo/admin bots, rank-scoped admin/management channels, `!s`/`!say` and `!players` IRC commands, connect/disconnect/action echoes |
| **Moderation plumbing** | Rank hierarchy (Player → Owner), rank-scoped message helpers, kick with delayed-kick fix, chat lock, proxy/VPN detection on connect |

### ⌨️ Command reference (all 17 — admin/builder/dev only)

`/addvehicle` `/addvehicle2` `/removevehicle` `/addactor` `/stopaddingactor`
`/editactors` `/addinterior` `/stopadding` `/editinteriors` `/gps` `/gpsoff`
`/saveloc` `/teleback` `/resetsamp` `/resetgrin` `/skin` — plus IRC `!s`/`!say`,
`!players`.

There are **no player-facing gameplay commands yet** — that work was planned
for after the infrastructure phase.

---

## ❌ What was planned but never built

The code contains clear scaffolding for systems that don't exist yet:

- **Robbery / hideout system** — `ROBBERY` interior type and GPS "Robbery
  Hideout" entries exist; nothing sets or uses them.
- **Mission system** — GPS "Mission Destination" entry, never assigned.
- **Cop gameplay** — no `/arrest`, `/cuff`, wanted levels, jail or bust logic
  despite four law-enforcement teams being fully defined.
- **Shop interactions** — actors advertise "Press Y for menu"; no handler or
  shop menus exist.
- **GPS destinations** — 20 GPS categories are listed in the menu; every
  handler except *Map Marker* is commented out.
- **Player settings** (`/settings` is referenced at registration but absent),
  **warnings system** (loads commented out), **activity/GPS statistics**
  (columns read, never written), **gender changes**, **vehicle ownership /
  economy** (columns and flags defined, unused), **animations module**
  (`anims.inc` is an empty header, not even included).

---

## 🧰 Requirements & bundled software

- **SA-MP 0.3.7-R2 server** — bundled (`samp-server.exe`, Windows).
- **Pawn compiler** — bundled (`pawno/`).
- **MySQL / MariaDB server** — *not* bundled; two databases expected
  (`cnr` for the gamemode, `mini_missions` for the Bans filterscript).
- **Plugins** (bundled in `plugins/`, loaded via `server.cfg`): crashdetect,
  log-plugin, streamer 2.9.1, sscanf 2.8.2, irc 1.4.8, mysql R41-2
  (+ `libmariadb.dll`), whirlpool, Pawn.CMD 3.1.4, RouteConnector
  (needs `scriptfiles/GPS.dat` — bundled), nativechecker.
- **Filterscripts**: `Bans`, `vspawner`, `int`.

> ⚠️ The gamemode `#include`s **geolocation** (Whitetiger) and calls
> `GetPlayerCountry`/`GetPlayerProxy`, but the plugin binary is **not** in
> `plugins/` and not in `server.cfg` — you must source it yourself (a
> `scriptfiles/geoip.db` is bundled) or stub those calls.

---

## 🚀 Quick start

1. Compile: open `pawno/pawno.exe` → `gamemodes/CnR.pwn` → **F5**
   (output `gamemodes/CnR.amx`).
2. Create the MySQL databases. **No `.sql` schema ships with this snapshot** —
   the tables (`players`, `LSplayers`, `LSvehicles`, `server_data`,
   `Interiors`, `Actors`, and the Bans FS `player_bans`) must be
   reverse-engineered from the queries. The open.mp rebuild ships a proper
   schema.
3. Edit credentials: `SQL_*` defines in `gamemodes/CnR.pwn` (defaults:
   `127.0.0.1` / `admin` / empty password / db `cnr`) and the Bans
   filterscript's own connection.
4. Edit `server.cfg` (RCON password, hostname, port — default **3333**).
5. Run `samp-server.exe` and connect with a SA-MP **0.3.7** client.

---

## 📁 Project structure

```
Exp-Gaming-Cops-And-Robbers/
├── gamemodes/
│   ├── CnR.pwn                 # entry point: includes, callbacks, game clock
│   ├── CnR/players/            # player modules (auth, spawns, peds, menus,
│   │                           #   vehicles, elevators, GPS state, messages…)
│   ├── CnR/server/             # server modules (defines, colors, zones,
│   │                           #   interiors, actors, GPS engine, IRC, misc)
│   └── Backups/                # superseded zones.inc variant
├── filterscripts/              # Bans, vspawner, int + stock SA-MP scripts
├── pawno/                      # legacy Pawn 3.2 compiler + includes
├── plugins/                    # prebuilt .dll/.so plugins (see list above)
├── scriptfiles/                # GPS.dat, geoip.db, properties/, vehicles/
├── npcmodes/                   # stock NPC recordings
└── server.cfg                  # SA-MP server config
```

Modules follow a `FUNCTION` macro convention (`forward public` + `public`) so
every cross-module call works via `CallLocalFunction` — handy to know when
reading the code.

---

## ⚠️ Important caveats & known issues (read before deploying)

This is an honest snapshot of a **shelved work-in-progress** — do **not** run
it publicly without addressing these:

**Security**
- `login_register.inc:156` contains a **hardcoded master-password backdoor**:
  a fixed Whirlpool hash is accepted as the password for *any* account.
- `OnPlayerClickPlayer` teleports **any** player to any clicked player — no
  rank check (`CnR.pwn:612`).
- Debug commands (`/saveloc`, `/teleback`, `/skin`, `/resetsamp`,
  `/resetgrin`) have **no permission guards**.
- `filterscripts/Bans.pwn` contains **hardcoded remote MySQL credentials**
  (including a public IP). Treat those credentials as burned; never reuse them.
- The IRC module hardcodes a private network (`irc.mg-s.us`) and a NickServ
  password; the bots will not connect anywhere useful today.

**Bugs**
- `loading_data.inc:32` loads the `ClassMusic` column into the *AutoLogin*
  field (copy-paste bug) — `pClassMusic` is never actually loaded.
- `CnR.pwn:202` passes the string `"d"` as an integer timer argument.
- Interior saves write column `CopWeapons` while loads read `CopsWeapons`;
  the `Actors` table has a `Loaction` (sic) column.
- `DeleteVehicle` hardcodes the `LSvehicles` table, ignoring the city macro.
- Class-selection/zone/menu textdraw handles are single globals rather than
  per-player arrays (works by accident with player-textdraw ID reuse).

**Operational**
- Only **Los Santos** is playable; SF/LV spawn/camera data is empty.
- The geolocation plugin is missing (see Requirements).
- `nativechecker` will flag unresolved natives if plugins are missing.

---

## 🗺️ What's next

The next major version moves to **[open.mp](https://open.mp/)** — modern
server, QAWNO compiler, maintained plugin stack, Dockerised deployment — while
continuing to target **SA-MP 0.3.7 clients** (no 0.3.DL required). The IRC
bridge will be retired, the security issues above fixed, and the planned
cops-and-robbers gameplay finally implemented.

---

## 🙏 Credits

**Arose Niazi** (script) · the **SA-MP team** · **Zeex** (crashdetect) ·
**Incognito** (streamer, irc) · **maddinat0r** (mysql) · **Whitetiger**
(geolocation) · **Y_Less** (whirlpool, sscanf2) · **emmet_** (sscanf2) ·
**YourShadow** (Pawn.CMD) · **Gammer_Z** (RouteConnector) · elevator systems
adapted from SA-MP team filterscript examples. Server binaries are covered by
`samp-license.txt` (SA-MP EULA).
