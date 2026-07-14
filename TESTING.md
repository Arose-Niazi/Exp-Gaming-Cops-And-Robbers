# 🧪 CnR — Testing Guide

Everything you need to test the EXP Gaming Cops And Robbers server locally.
Work top-to-bottom the first time; after that, jump to whichever system you
want to exercise.

---

## ✅ Current status (already done for you)

- **Server is RUNNING** on `config.test.json` — port **7778**, LAN mode, max 20 players.
- **Database is set up** in your WSL MySQL (8.0): database `cnr`, user `admin`
  (empty password, native-auth), all 13 tables loaded from `scriptfiles/cnr.sql`.
- Boot verified clean: mysql R41-4 connected, 21 stocks seeded, no query errors.

If the server isn't running (PC rebooted, etc.), see **§ Start / stop the server** below.

---

## 🎮 Connect

1. Open your **SA-MP 0.3.7** client (the `samplauncher`/`samp.exe`).
2. Add server → **`127.0.0.1:7778`** (LAN). If your client hides LAN servers,
   type the IP directly and press connect.
3. Pick any nickname (this becomes your account name — remember it).

> Client must be **0.3.7** (0.3.DL also connects, but this is a 0.3.7 build).

---

## �first-time account + become Scripter (to test everything)

Ranks are **database-driven** — a fresh account is a normal player. To reach
the admin/owner/scripter commands you promote your account **once** in the DB.

1. Connect, and at the dialog **Register** a password (6–32 chars), skip/enter email.
   You're now a registered player (aID 1 if you're the first).
2. Open a terminal and promote yourself to **Scripter (rank 7 = everything)**:

   ```bash
   wsl -e bash -lc "sudo mysql cnr -e \"UPDATE players SET Rank=7 WHERE UserName='YOUR_NICK';\""
   ```

   Replace `YOUR_NICK` with your exact in-game name. Also handy:

   ```bash
   # give yourself VIP + DJ + a pile of cash to test the economy
   wsl -e bash -lc "sudo mysql cnr -e \"UPDATE players SET Vip=1,Dj=1 WHERE UserName='YOUR_NICK'; UPDATE LSplayers SET Money=100000000 WHERE aID=(SELECT aID FROM players WHERE UserName='YOUR_NICK');\""
   ```
3. **Reconnect** (F8 disconnect → reconnect) so the new rank/flags load.
   You should log in as **Server Scripter**.

> A second test identity: connect with a different nickname to test PMs, robbing,
> dueling, arrests, kidnapping, etc. between two players. Two SA-MP clients on
> one PC works, or use a friend on your LAN.

---

## 🕹️ In-game reference commands

- **`/commands`** — paginated list of everything you can run.
- **`/showcommands`** — toggle seeing which commands other players use.
- Most systems have a launcher dialog: `/missions`, `/markets`, `/bank`,
  `/clotheswear`, `/skill`, `/fightstyle`, `/gps`.

---

## 📋 Test checklist by system

Tick each as you go. "Expect" = what should happen.

### M2 — Moderation & accounts
- [ ] `/pm <id> <text>` + `/reply (/r) <text>` — Expect: private msg to target only; `/reply` answers last sender.
- [ ] `/nopm`, `/ignore <id>` — Expect: blocks PMs / that player's chat.
- [ ] `/a <text>` (staff) — Expect: message to staff only.
- [ ] `/ajail <id> [mins] [reason]`, `/ajailed`, `/unajail <id>` — Expect: target frozen in admin jail, auto-released, survives their reconnect.
- [ ] `/aka <id>`, `/ips <id>`, `/offlineaka <name>`, `/offlineips <name>` — Expect: shows accounts sharing IP/serial.
- [ ] `/sethealth /setarmour /setvirtualworld /setinterior /aflip /move /goto /disarm /arm /aslap /count /areacount /osearch` — Expect: each does what it says on the target; refuses targets ranked ≥ you.
- [ ] **Bans**: `/ban <id> [days] [reason]`, then reconnect on that account → Expect: ban dialog + kick. `/unban <name>` → can rejoin. Also `/offlineban`, `/unbannick`, `/unbanid`.
- [ ] Teleports: `/drylake (/d)`, `/sfairport (/sfa)`, `/bayside (/bs)`, `/lossantosdm (/lsadm)`, `/palominocreek (/pc)` — Expect: teleport; DM zones strip parachute + suppress wanted.

### M3 — Core Cops & Robbers loop
- [ ] Pick a **cop** class at spawn (Police/Sheriff/FBI) for one identity, **civilian** for the other.
- [ ] **Wanted**: as civ, `/rob (/rb) <id>` a player → Expect: cash stolen (skill-capped), you gain wanted; name/blip colour changes by tier (yellow 1-5 / orange 6-8 / dark 9-10).
- [ ] `/rape (/ra) <id>` → Expect: infects an STD (HP drains over time); `/ad` adrenaline cures it.
- [ ] `/takedrugs (/td)` — Expect: buzz heal; over 60g overdoses (need a drug bag/seeds first — see farming).
- [ ] **Cop**: `/vc` (visual contact) pauses a suspect's wanted decay; `/su <id> <crime>`; `/ticket (/tk) <id>` (WL1-5); **`/arrest (/ar) <id>`** (WL6+, on foot, near) → Expect: suspect disarmed + jailed, you get score + cash.
- [ ] **Jail** (as the jailed player): `/bail`, `/bribe <amount>` (a cop `/accept`s or `/refuse`s), `/appeal` → jury of online players votes, `/escape` (→ instant WL10), `/breakout <id>`.
- [ ] Die by fire (e.g. `/ad`… or Molotov) → Expect: death feed says "grilled".

### M4 — Economy & events
- [ ] **Bank**: `/bank` (or `/atm`), `/deposit (/dep) <amt|all>`, `/withdraw (/wd) <amt|all>`, `/balance (/bal)`, `/givecash (/pay) <id> <amt>` — Expect: menu options reachable; amounts validated; givecash moves cash 1:1 to a nearby player.
- [ ] **Lotto**: `/lotto <1-125>` — Expect: $5,000 charged, ticket stored. Force a draw as Scripter: `/alotto` → Expect: winner paid / jackpot rolls over.
- [ ] **Money events**: `/addmoneybag` (scripter) then `/moneybag` warmer/colder hunt → pickup reward. `/startmoneyrush` (owner) then `/moneyrush (/mr)` → cash-rain arena.
- [ ] `/ad` (adrenaline: full heal + cure), `/advert <text>` ($200, 90s cd), `/radio`.
- [ ] **Holdup**: stand at a store-clerk actor, `/holdup` → Expect: +7 wanted, register pays out over a few seconds. *(Needs actors placed — see note ⚠️ below.)*
- [ ] **Missions**: `/missions` → pick Drug Delivery / Truck / Trash / Food → follow the race checkpoints → Expect: per-checkpoint + completion payout, game-hour cooldown after.

### M5 — Property & big crime
- [ ] **Vehicle ownership**: drive any car, get out, `/lock (/lk)` → Expect: others can't enter; you entering auto-unlocks; `/unlock (/ulk)`. Steal a car someone else last drove → Expect: "Grand Theft Auto" wanted; a fresh/never-driven car is free.
- [ ] **Robberies**: `/shoplift`, `/holdup`, `/bankrob` (30s crack → deliver safe → drains victims' bank), `/casinorob`, `/houserob` (near an owned house, owner online). Buy the **crowbar** via `/clotheswear` ($40k) → robberies get faster/richer; get arrested → crowbar confiscated.
- [ ] **Housing**: near a for-sale house pickup, `/buyhouse`; then `/house` menu → store money, `/sethousespawn`, `/lockhouse`, `/superlock`, `/sellhouse`. *(Needs houses added — `/addhouse` as scripter, see note ⚠️.)*
- [ ] **Kidnap**: `/kidnap (/kd) <id>` → victim forced along; `/ransom`, victim `/payransom`, cop `/uncuff`.
- [ ] **Missions**: Vehicle Theft (deliver 5 different stolen cars, last = WL10), Holdup Mission, Airport Robbery (collect boxes with MMB or `/box`).

### M6 — Jobs, activities & arenas
- [ ] **Skills**: `/skill` → pick Pickpocket / Con Artist / Drug Dealer / Kidnapper / Car Jacker → Expect: your `/rob` cap, kidnap ransom, drug cap, or lock-defeat changes accordingly.
- [ ] **Fight style**: `/fightstyle` at a gym marker → Expect: melee style changes ($500).
- [ ] **Clothes**: `/clotheswear` → buy/wear a skin, buy crowbar / drug bag.
- [ ] **Fishing**: buy gear at bait shop (`/baitshop`), `/fish` at water, `/fishsellall` at market. Bonus fish at game-hour 05:00.
- [ ] **Farming**: `/buyseeds`, `/plant` (≤5), wait ~20 min (or watch grams tick), `/harvest` → grams into your drug stash; `/farmjob` legal crop loop.
- [ ] **Arenas**: `/dms` (2500-ammo deathmatch), `/sniper`, `/duel <id> <stake>` (opponent `/accept`) → Expect: teleport to arena, PvP, loadout restored on `/leave`, winner takes the pot.
- [ ] **GPS**: `/gps` → pick a category (PD/Bank/Hospital/24-7…) → Expect: a route draws to the nearest one.
- [ ] **DJ**: `/djradio <url> [now-playing]` (DJ flag) → Expect: stream plays for everyone; `/stopdjradio`.
- [ ] Remaining missions in `/missions`: Courier, House Delivery, Illegal Immigrant, Lawn Mowing, Paperboy (4-min real-time), Pickpocket, Sexual Encounter, Flower Delivery, Race Challenges, Tractor, + cop-only Routine Patrol & Domestic Disturbance.

### M7 — Stock market
- [ ] `/markets` — Expect: 21 stocks with prices; shows OPEN only during game **07:00–19:00**.
- [ ] `/shares <id> <qty>` to buy, `/sharessell <id> <qty>` to sell — Expect: cash charged + 1% fee; holdings tracked; can't exceed 100k shares/company.
- [ ] `/markethistory <id>` — last 30 daily closes.
- [ ] Force a daily tick as Scripter: `/astocktick` → Expect: prices move, dividends paid to holders.
- [ ] **Economy link**: note a house / fish / crop / 24-7 price, move the matching stock (buy a lot), tick, then re-check the price → Expect: it shifts (multiplier 0.5×–2×).

---

## ⚠️ Things that need world data placed first

Some systems key off admin-placed world objects that a fresh DB doesn't have.
As **Scripter**, seed a few so those systems are testable:

- **Shop/robbery actors** (for `/shop`, `/holdup`, `/shoplift`): `/addactor` — stand where you want a clerk, follow the prompts.
- **Houses** (for `/buyhouse`, `/houserob`): `/addhouse [price]` — stand at a door, run it; a for-sale house pickup appears.
- **Interiors / teleport pickups**: `/addinterior`.
- **Vehicles**: `/addvehicle`.

Without these, the related command will politely say "there's nothing here" —
that's expected on an empty world, not a bug.

---

## 🖥️ Start / stop the server

The server is already running in the background. To manage it yourself:

**Start (with a live console you can watch — recommended for testing):**
```powershell
cd "D:\Projects\samp\Exp-Gaming-Cops-And-Robbers"
.\omp-server.exe --config-path config.test.json
```
(Leave that window open; you'll see joins, chat, and errors live. Ctrl+C to stop.)

**Start hidden (background):**
```powershell
Start-Process .\omp-server.exe -ArgumentList "--config-path","config.test.json"
```

**Stop:**
```powershell
Get-Process omp-server | Stop-Process -Force
```

**Watch the log** (either mode writes `log.txt`):
```powershell
Get-Content log.txt -Wait -Tail 30
```

> `config.test.json` = local testing (port 7778, LAN, weaker rcon). `config.json`
> = the "real" profile (port 7777). Both use the same local `cnr` database.

---

## 🔑 RCON (in-game admin console)

Press **`~`** (or `/rcon login <password>`) in-game, password from `config.test.json`:
**`test_only_change_me`**. Then `/rcon <cmd>` (e.g. `gmx`, `kick`, `say`).
(This is separate from the gamemode's own rank system.)

---

## 🗄️ Verifying the database directly

Watch your actions persist. Examples:
```bash
# your account + money + rank
wsl -e bash -lc "sudo mysql cnr -e 'SELECT p.aID,p.UserName,p.Rank,l.Money,l.BankMoney,l.Wanted,l.Skill FROM players p JOIN LSplayers l USING(aID);'"

# stock prices after a tick
wsl -e bash -lc "sudo mysql cnr -e 'SELECT ID,Name,Price,IPOPrice FROM stocks ORDER BY ID;'"

# houses / plants / bans / vehicles
wsl -e bash -lc "sudo mysql cnr -e 'SELECT COUNT(*) FROM houses; SELECT COUNT(*) FROM plants; SELECT * FROM bans;'"
```

**Reset a test account** (start fresh):
```bash
wsl -e bash -lc "sudo mysql cnr -e \"DELETE FROM players WHERE UserName='YOUR_NICK';\""
```
**Wipe everything and reload the schema:**
```bash
wsl -e bash -lc "sudo mysql -e 'DROP DATABASE cnr; CREATE DATABASE cnr;' && sudo mysql cnr < /mnt/d/Projects/samp/Exp-Gaming-Cops-And-Robbers/scriptfiles/cnr.sql"
```

---

## 🩹 Troubleshooting

- **Can't see the server in the SA-MP list** → add `127.0.0.1:7778` manually; make sure LAN servers are shown.
- **"Server is not responding"** → it's not running; start it (§ above) and check `log.txt`.
- **Stuck at "connecting" / instant kick** → you may have failed login; the account exists — use the right password, or reset the account (§ above).
- **A command says "you can't do that"** → rank gate; make sure you promoted to Scripter and reconnected.
- **`/holdup` `/buyhouse` etc. say "nothing here"** → place the world data first (§ Things that need world data).
- **Server boots but gameplay doesn't save** → DB issue; confirm `log.txt` shows `plugin.mysql: R41-4 successfully loaded` and **no** `SQL: Failed to connect`. Re-run the DB setup if needed.
- **Time-gated features** (lotto draw 18:00, stock market 07:00–19:00, bonus fish 05:00) → use the scripter force commands (`/alotto`, `/astocktick`) or `/atime <hour>` (owner) to move the game clock.

---

Have fun. Anything that behaves wrong, note the command + what happened + check
`log.txt` for an `OnQueryError` or AMX backtrace, and I can dig in.
