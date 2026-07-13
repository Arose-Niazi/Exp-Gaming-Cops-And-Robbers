# Planned Features (source list)

The recovered feature plan for EXP Gaming Cops And Robbers. Most gameplay
features are modelled on **CrazyBob's Cops And Robbers** (crazybobs.net, now
closed) — this project is a revival of our take on it for our players.
Mechanics research lives in `docs/research/`; the implementable design in
`docs/GAMEPLAY-DESIGN.md`.

## Player commands

| Command | Alias | Notes |
|---------|-------|-------|
| /weapons | /ws | weapon dealer / buy menu |
| /moneybag | | find/hunt the hidden money bag |
| /moneyrush | | join the money-rush event |
| /radio | | radio channels/music |
| /rape | /ra | crime action vs nearby player |
| /rob | /rb | rob nearby player/store |
| /lock | /lk | lock vehicle/house |
| /unlock | /ulk | unlock |
| /takedrugs | | consume held drugs |
| /ad | | paid advertisement |
| /jumpkick | | melee action |
| /shop | | shop menu (24/7 etc.) |
| /drylake | /d | teleport: Dry Lake |
| /sfairport | /sfa | teleport: SF Airport |
| /bayside | /bs | teleport: Bayside |
| /lossantosdm | /lsadm | teleport: LS DM zone |
| /palominocreek | /pc | teleport: Palomino Creek |
| /pm | | private message |
| /reply | /r | reply to last PM |
| /goto | | teleport to player (restricted?) |

## DJ commands
- /djradio

## Admin commands

/showcommands, /healthhack, /ajail (**legacy note: not working — fix**),
/ajailed, /aimbotters, /clearaimbotters, /setarmour, /sethealth, /aflip,
/setvirtualworld, /setinterior, /move, /disarm, /arm, /a (admin chat),
/count, /areacount, /aduty, /aslap, /osearch, /rangearm, /rangedisarm,
/aka, /ips, /offlineaka, /offlineips,
/ban, /unban, /unbannick, /unbanid (legacy: mirrored to IRC → now in-game log
+ future Discord bridge).

## Owner commands
/atime, /startmoneyrush, /makeadmin, /makeregular, /makedj, /makedonator.

## Scripter commands
/addmoneybag, /alotto, /addvehicle, /addinterior, /lgoto, /makeowner,
/makescripter, /debug.

> The legacy plan had a name-bound `/getscripter` for one specific player.
> Per the project's recorded security decisions, rank grants are DB/config
> driven only — no name-based backdoors.

## Systems

- Vehicles (+ vehicle callbacks)
- Commands framework
- Housing system
- Skills (weapon skills) & Fighting styles
- Textdraws (HUD)
- Login system & Auto-login (exists; polish)
- Death Match Stadium (DMS)
- Sniper (arena)
- Duel
- Anti-parachute (DM zones)
- Clothes
- GPS (destinations — engine exists, handlers stubbed)
- Interiors (builder exists; gameplay hooks)
- Zones (exists; gameplay hooks)
- Fix: fire death reason ("grilled")
- Fix: gamemodeclock lottery timer bug (legacy note)

## CrazyBob's-style systems to research & adapt

Fishing, lotto, housing (buy/rent), **stock market** (prices affecting items,
weapons, houses, fish, …), farming, plus the classic CB:CNR loop (crimes,
wanted levels, cops/arrests, jail, jobs, bank, businesses).
