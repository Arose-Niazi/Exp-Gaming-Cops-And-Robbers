# EXP Gaming Cops And Robbers — open.mp

Cops-vs-robbers gamemode for GTA:SA multiplayer on **open.mp**, targeting
**SA-MP 0.3.7 clients** (no 0.3.DL). The original SA-MP 0.3.7 tree is preserved
under `legacy/` and tagged/released as `v0.3.7-legacy`.

## Build

Compiler: **QAWNO** (`qawno/pawncc.exe` Windows, `qawno/pawncc` Linux,
`qawno/mac/pawncc` macOS). Do **not** use pawno — the legacy compiler lives
under `legacy/pawno/` only.

Compile the gamemode (Windows):

```
qawno/pawncc.exe "-;+" "-(+" "-\\" "-Z-" "-igamemodes" "-iqawno/include" -d3 -t4 "-ogamemodes/CnR" "gamemodes/CnR.pwn" WINDOWS_COMPILER=1
```

Linux/CI: use `./qawno/pawncc` and **drop** `WINDOWS_COMPILER=1` (the define
switches module include paths to backslashes; every module `#include` is
wrapped in `#if defined WINDOWS_COMPILER`). In VS Code: **Ctrl+Shift+B**.
Batch build (gamemode + any filterscripts): `python compile.py`.

**Always recompile and commit `gamemodes/CnR.amx` after changing `CnR.pwn` or
any `gamemodes/CnR/*.inc` — production runs the committed binary.**

The build must stay at **0 errors / 0 warnings**.

## Run

```
omp-server.exe                                   # Windows, uses config.json
./omp-server                                     # Linux
omp-server.exe --config-path config.test.json    # local test (port 7778, LAN mode)
```

`network.allow_037_clients` is `true` in **both** configs — SA-MP 0.3.7 clients
are the target audience. No artwork/CustomModels pipeline (that is 0.3.DL-only).

## Architecture

- `gamemodes/CnR.pwn` — entry point: includes, SA-MP callbacks, game clock.
- `gamemodes/CnR/players/` — player modules (auth, loading, spawns, peds,
  menus, messages, vehicles, elevators, cmds).
- `gamemodes/CnR/server/` — server modules (defines, colors, db_config, zones,
  interiors, actors, GPS, misc).
- Cross-module calls use the `FUNCTION` macro (`forward public` + `public`) and
  `CallLocalFunction` — module include order matters for direct calls.
- City selection via `#define CITY_` in `CnR.pwn` (LS active; SF/LV stubs).

open.mp **components** (`components/`) auto-load — do NOT list them in the
config. Only true legacy plugins go in `config.json` → `pawn.legacy_plugins`:
crashdetect, mysql (R41-4), streamer, whirlpool, RouteConnectorPlugin.
The mysql plugin needs `log-core*.dll/.so` + `libmariadb.dll` at the repo root,
and RouteConnector needs `scriptfiles/GPS.dat`.

## Database

MySQL via maddinat0r R41 (async `mysql_pquery` + `cache_*`). Credentials live
in `gamemodes/CnR/server/db_config.inc` behind a `LOCAL_DB` toggle (defined in
`CnR.pwn`). Schema: `scriptfiles/cnr.sql` (tables: players, LSplayers,
LSvehicles, server_data, Interiors, Actors). Never commit real credentials.

## Recorded decisions

- **0.3.7 target, not DL** — `allow_037_clients: true`; no artwork/CustomModels.
- **IRC bridge retired** — the `IRC_Send*` wrappers in
  `gamemodes/CnR/players/messages.inc` are kept as no-ops so call sites
  survive; wire a Discord (or other) bridge through them later.
- **Bans filterscript not yet ported** — `CallForChecking` short-circuits to
  `BanCheckDone(playerid, 0)`; reintroduce a native ban lookup there. The
  legacy `Bans.pwn` (under `legacy/filterscripts/`) contains burned credentials
  — never reuse them.
- **Backdoors removed at the port** — the hardcoded master-password hash and
  the unguarded click-teleport are gone; never reintroduce name/hash-based
  auth bypasses.
- The rcon password in `config.json` is a rotate-before-deploy placeholder.
- `.pdb` files are gitignored; `qawno/`, `components/`, `plugins/`, `legacy/`
  are linguist-vendored.
- The Mini-Missions reference project's **weapon-config** include is
  deliberately NOT used here (per project decision) — do not add
  `weapon-config.inc` or its death/damage rewiring.
