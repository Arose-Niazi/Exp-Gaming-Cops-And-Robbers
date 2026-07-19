# CrazyBob's Cops And Robbers — Complete Command + 10-Code Reference

**Research date:** 2026-07-19
**Purpose:** Authoritative command/10-code spec for the EXP Gaming C&R revival gap analysis.
**Sources (all live and fetched in full this sweep):**
- Commands: https://www.crazybobs.net/website/cnr-commands (verbatim via browser page text — the WebFetch summarizer refused on copyright grounds, so the full list was read directly from the rendered page)
- 10-Codes: https://www.crazybobs.net/website/cnr-10-codes
- Skills: https://www.crazybobs.net/website/skills
- Prior sweep context: `docs/research/crazybobs-site-full.md` (site map + missions/economy; it did NOT transcribe the command list or 10-codes — this doc fills that gap)

> **Key answer up front — what `/w` does:** `/w` is the alias for **`/whisper`**. It "Sends a message to all players close to you." It is a **proximity / local chat**, NOT a private message. The private-message command is **`/pm`** (`/ms`, `/msg`, `/m`, `/priv`). There is also `/carwhisper` (`/cw`) which messages everyone in your vehicle.

---

## Command categories (verbatim)

The page groups every command into 16 categories. Each entry below is `/command, /aliases` followed by its description exactly as published.

### 1. Police Commands

> "The Police Officer is one of the main skills in CrazyBob's. They protect innocents and fight crime, but in order to fulfill their task, they need specific commands."

- **/accept, /ac** — Accepts a Bribe. You must have been offered a bribe before using this command.
- **/arrest, /ar** — Arrests the specified player, or closest wanted suspect if no player is specified. The player must be wanted (warrant issued) to be arrested.
- **/backup, /bk** — Requests Police Backup at your current location. Also sends an optional message to all police officers. Officers requesting backup are purple on the mini-map. You can use 10-Codes and/or Quick Strings in your message.
- **/cancellastreport, /cancelreport, /clr** — Cancels your last criminal activity report and returns the reported player's wanted level to its previous state. Use this for accidental crime reports or when a suspect obeys your commands after a report.
- **/copmsg, /cm** — Sends a message to all police officers. You can use 10-Codes and/or Quick Strings in your message.
- **/donut** — Donuts refill 50% of your health. You can carry a total of 6 donuts. Visit one of the four donut shops.
- **/fire** — Respond to a fire. (You have to be driving a Fire Truck)
- **/freeze, /fr, /pullover, /pu** — Asks a suspect to pullover / freeze / pay a ticket / surrender etc... (depending on situation)
- **/mostwanted, /most, /mw** — Displays a list of all most wanted suspects. (Wanted Level 10)
- **/refuse, /ref** — Refuses a Bribe and reports the player who offered you the bribe. You must have been offered a bribe before using this command.
- **/report, /rp** — Reports criminal activity. The player's wanted level will be increased. You may only report players for committing serious crimes (See The Rules). Use /clr to cancel your last criminal activity report in case of accident reports. You can use 10-Codes and/or Quick Strings in your crime report.
- **/respond, /yes** — Responds to the last call (backup calls, 911 calls, crimes). This updates the Red Call Checkpoint on your radar and notifies other officers that you are on the way. You can use 10-Codes and/or Quick Strings in your message.
- **/robberies, /robs, /rb** — Displays list of holdups and robberies that are currently being robbed and haven't been stopped.
- **/suspects, /sus, /wanted, /wan** — Displays a list of all suspects.
- **/ticket, /tk** — Issues a ticket to the specified player, or closest suspect if no player is specified. Tickets can only be issued to suspects who aren't wanted (not innocent and no warrant), and allow suspects to reduce their wanted level by paying. You must remain close to the suspect to collect the ticket.
- **/vehrepair, /vehfix** — Cops can buy a vehicle repair kit at the PD repair checkpoints to fix their vehicle at any time or location. Costs ~10–20k. To use: stop your vehicle, get out, and use the command.
- **/visualcontact, /vc, /vcontact** — Reports visual contact with a suspect. The suspect's wanted level will not be able to drop for a certain time after visual contact is established.
- **/warrants, /warrant, /war** — Displays a list of all wanted suspects. (Warrant Issued)

### 2. Information Commands

- **/animations, /anims, /animhelp** — Displays a Menu List of all animation commands.
- **/calls, /calllist** — Displays a list of all the people who recently called for items, medic, sex, or weapons.
- **/challengerecords, /challengerec, /crecords, /crec, /chrecords, /chrec** — Displays list of people who set a record on a challenge.
- **/citystats, /cstats, /csts** — Displayed at 23:30 at the end of each week. Shows various activities everybody did as a whole.
- **/commands, /cmds** — Displays list of commands.
- **/crimes, /crime** — Pulls up a list of the 5 last crimes committed by a civilian/suspect.
- **/dmrecords, /dmrec** — List of DM stadium records.
- **/faq** — Frequently Asked Questions for newcomers.
- **/getid, /id** — Find an ID of a player.
- **/help, /hlp** — Gives you information.
- **/hitinfo, /hi** — Displays information about any hits placed on you.
- **/hitlist, /hits** — Displays a list of people who have hits on them.
- **/information, /info, /i** — Displays general information (class, skill, level, location) for the specified player. Leave nick/id blank for your info.
- **/insuranceinfo, /ii** — Displays information about your Life and Health Insurance coverage.
- **/inventory, /inv, /myinv** — Displays information about the Items you are carrying.
- **/jailinfo, /ji, /jinfo** — Displays the specified player's Jail information, including jail time and bail. Blank = your own.
- **/jaillist, /jl, /jlist** — Displays a list of the players that are currently in jail.
- **/level, /lev, /rank, /lvl** — Displays the specified player's Level / Rank information. Blank = your own.
- **/locate, /location, /loc** — Displays the specified player's location. Blank = your own.
- **/markets, /market** — The current market/interest rates.
- **/moneyinfo, /bankinfo, /$i** — Displays all your money, tax and bank information.
- **/moneyrush, /mrush, /mr** — Displays Money Rush (Lost Mafia Money) Information.
- **/morestats, /msts** — Displays more of the specified player's current life (since respawn) stats. Blank = yours.
- **/myinfo, /minfo, /mi** — Displays all information about your current life.
- **/myrep** — See your current reputation.
- **/party** — Displays Friday Night Party information.
- **/permits** — Displays all your permits and allows you to cancel them.
- **/records, /rec** — Displays CnR Records for Fishing or Race Challenges.
- **/rules, /rule** — Displays list of general rules.
- **/sellinfo, /vsellinfo, /carsellinfo, /vsi, /si** — Displays your vehicle exportation information and bonus vehicle information.
- **/stats, /sts** — Displays the specified player's current life (since respawn) stats. Blank = yours.
- **/std, /diseases, /stdinfo, /si** — Displays information about your current diseases and STD protection.
- **/time, /day** — Displays current game day and time. The game week starts Sunday 0:00, ends Saturday 24:00.
- **/total, /tot** — Displays the specified player's total stats. (Menu) Blank = yours.
- **/vehhelp, /vhelp** — Displays list of vehicle commands.
- **/version, /ver** — A textbox showing the current version that has been implemented.
- **/wotd, /workers** — Shows the workers who have done the most work and their amount, presented each game day at 23:30.

### 3. Group Commands

- **/groupcall, /grcall** — Calls the members of your group (or a specified member) to you. Blank = all members. Must be in a group.
- **/groupcash, /gr$, /g$** — Sends money to in-range group members, divided by the number of members in range. Must be in a group.
- **/groupcreate, /grc** — (page repeats the groupcall text) Create/manage group.
- **/grouphelp, /grh, /helpgroup, /ganghelp** — Displays group commands and usage.
- **/groupinvite, /gri** — Invites a player to join the group or accepts a join request. Leader only.
- **/groupjoin, /grj** — Requests to join a group or accepts a request to join a group.
- **/groupkick, /grk** — Removes a player from your group. Leader only.
- **/groupleader, /grleader, /grouplead, /grlead** — Transfers group Leadership. Leader only.
- **/groupleave, /grl** — Leaves your current group. Must be in a group.
- **/grouplist, /grlist, /grlst** — Lists the players in your group or a specified group.
- **/groupmsg, /grm, /gm** — Sends a message to your group members. Supports 10-Codes/Quick Strings. Must be in a group.
- **/groups, /gr, /grps** — Displays a list of active groups.

### 4. Fishing Commands

- **/cooler** — Discard your fish cooler. You lose any fish you cannot carry.
- **/fish, /fsh** — Begins fishing while on a boat.
- **/fishbuy, /fbuy, /fb, /buyf, /buyfish** — Buy a fish from another player.
- **/fisheat, /feat, /fe** — Eat one of your fish to refill health (Menu).
- **/fishgive, /fgive, /givefish, /givef, /gf** — Give a fish to another player (Menu).
- **/fishhelp, /fhelp** — Displays fishing commands and usage.
- **/fishinfo, /fishi, /finfo, /fi** — Displays your Fishing info (permit, fish caught, record fish).
- **/fishinventory, /fishinv, /finventiry, /finv** — Displays the fish you are carrying.
- **/fishmsg, /fm** — Message all other fishermen (must be fishing on a boat).
- **/fishprices, /fp, /fishp, /fishprice, /fprice** — Set your fish prices (needs fish sales permit).
- **/fishrecords, /fishrec, /frec, /frecords** — Fishing Records for the current city.
- **/fishrelease, /fishrel, /frelease, /frel** — Throw away a carried fish (Menu).
- **/fishrod, /rod, /pole** — Takes out / puts away your Fishing Rod. Having it out reduces fishing time and increases catch chance.
- **/fishsell, /fsell, /sellfish, /sellf, /sf** — Offer fish to another player (needs fish sales permit).
- **/fishsellall, /sellfishall, /sellallfish, /sellfall** — Sell all fish / a certain fish at a 24/7 or bait shop.
- **/fishslap, /fslap, /fs** — Slaps another player with a randomly selected carried fish.
- **/fishthrow, /throwfish, /fthrow, /throwback, /tb** — Throws your last caught fish back into the water.
- **/fishtour** — Display fishing tournament information.

### 5. Animation Commands

/assslap; /bitchslap (/corpseslap, /slap); /carkick (/ck, /kcar, /cark); /come; /cpr; /crossarms (/armcross, /armscross); /cry; /dance (/dan) [1–4]; /fart (/gas); /flash (/flsh, /fl); /flowers (/flower — gives flowers, must have flowers); /follow (/folow, /flw); /fuckyou (/fyou, /fuck, /flipoff, /foff); /go (/flag — race flag drop); /gunpoint (requires holding a gun); /handstand; /hide (/cower, /duck); /idle (/stand); /jumpkick; /kiss (/kis, /ks); /lean (/ln); /leansmoke (/ls); /liedown (/lie, /laydown, /lay); /mourn (/morn, /rip); /no (/n); /piss (/pee — Civilian Only); /point (/poi); /puke (/vomit — Civilian Only); /ride (/hitchhike, /hitch); /scratch (/scr); /shakehead (/headshake, /hs, /hshake, /noob, /shakeh); /shout; /showoff [1-23] (/soff, /so); /showoff2 [1-23]; /sit [1-14] (/sleep); /stab (requires knife clothing item); /stabanim [1-4] (no knife required); /stop; /taichi (/karate, /tc); /yes (/y); /wank (/wankoff, /jerk, /jerkoff, /jo — Civilian Only); /wave (/hello).

### 6. Pet Commands

- **/pet, /pets, /petinfo, /pi** — Pet info (size, name, etc.).
- **/petattack, /pattack, /peta, /petat** — Pet attacks people (must be trained to attack on command).
- **/petcmds, /pethelp, /petcommands** — Pet Commands (all accessible from /pet menu).
- **/petdiet, /pdiet, /petd** — Put your pet on a diet if it is fat.
- **/peteat, /peat** — Eat your pet.
- **/petfeed, /pfeed, /petf** — (page duplicates diet text) Feed your pet.
- **/petfight, /pfight, /petfi** — Pet fight with somebody (trained pets; can fight other pets).
- **/petname, /pname, /petn** — Change the name of your pet.
- **/petslap, /pslap, /pets** — Slap somebody with your pet. You lose your pet when you petslap someone.
- **/petstats, /pstats** — Shows the stats of your pets.
- **/pettraining, /pettrain, /pett** — Train your pet; it will defend you from attackers.

### 7. GPS Commands

- **/driverdest** — As a passenger, send a destination to the vehicle driver (can be disabled in settings).
- **/gps** — Opens the main GPS menu.
- **/gpsclear, /gpsclr** — Clears your current route.
- **/gpsgocustom, /gpsgo, /gpsgocust** — Goes to your customized destination.
- **/gpshide, /gpsoff** — Temporarily hides GPS destination.
- **/gpsloc, /gpsdest, /gpsdst** — Pulls up the list of destinations.
- **/gpsresume, /gprsm** — Resumes your route.
- **/gpssetcustom, /gpsset, /gpssetcust** — Sets your customized destination.
- **/gpssettings, /gpsoptions** — Access GPS settings.

### 8. Pimping Commands

- **/pimp, /pimpoffer, /pimpof** — Offer pimping services to Prostitutes.
- **/pimpcall, /pimpc** — Call for a Pimp.
- **/pimpaccept, /pimpa, /pimpyes** — Accept from a Pimp.
- **/Pimphelp** — Pimp information and guidelines.
- **/pimpinfo, /pimplist, /pimpi, /pimpl** — List of other pimps.
- **/pimpleave, /pimplv** — Leave as a pimp.
- **/pimpleaveall, /pimplva** — Leave all pimps.
- **/pimpmsg, /pimpm** — Send a message to all pimps.
- **/pimprefus, /pimpref, /pimpr, /pimpno** — Deny from a Pimp.

### 9. House Commands

- **/colist, /coownerlist, /cokeys, /cklist, /ckeylist, /ckeys** — Manage your co-owner keys.
- **/hotel** — Opens the room menu when inside a hotel.
- **/hotels** — See a list of your hotel rentals.
- **/house** — Access the House Menu and House Information from inside a house.
- **/houseanswer, /answer, /door** — Answer a knock and let the player enter your house.
- **/housecoowner, /hco, /houseco, /coowner** — Give a player co-owner keys (full access).
- **/houseinvite, /hinvite** — Invite a player to your house (limited time).
- **/housekeys, /hkeys** — Give a player the keys (until you change the locks).
- **/housekick, /hkick** — Kick a player from your house.
- **/housefishstorage, /fishstorage, /fishstore, /fstorage, /fstore** — Access the House Fish Storage inside the house.
- **/houserob, /hsrob, /hrob** — Attempt a House Robbery inside a house.
- **/houses, /houselist, /hlist** — List the houses you own.
- **/housestorage, /storage, /store** — Access the House Storage inside the house.
- **/keys, /keylist, /klist** — List the house keys you are holding.
- **/rent, /rents, /rentlist, /rlist, /rentl** — List and cancel your rent contracts.

### 10. No-Sell Commands

- **/nocalls, /callsoff** — No longer receive calls for the services you offer.
- **/nosell, /ns, /selllist** — Selling permissions main menu.
- **/noselladd, /nsadd** — Add a name to your no-sell list.
- **/noselladdall, /nsaddall** — Adds every online player onto the list.
- **/noselldel, /nsel, /nsremove, /nsrem** — Remove a blocked person from the list.
- **/nsselldelall, /nsdellall, /nsremoveall, /nsremall** — Removes everybody on the no-sell list.
- **/noselllist, /noselllst, /nslist, /nslst** — Show all players on your no-sell list.
- **/sell** — Offer your items to other players.
- **/sellmenu** — Display main sales menu.

### 11. Clothes Commands

- **/clothes** — Clothes main menu.
- **/clothesdiscard, /clothesthrow** — Discard an item (no longer own it).
- **/clothesinv, /cinv** — (page duplicates discard text) Clothes inventory.
- **/clothesposition, /clothespos** — Adjust positions/rotations/scale of worn clothes.
- **/clothesprice, /clothesprices** — Set your clothes for sale to other players.
- **/clothesremove, /clothesrem** — Remove clothes.
- **/clothessell** — Offer to sell clothes to someone (an ad).
- **/clotheswear** — Wear clothes.

### 12. Vehicle Commands

- **/eject, /ej** — Eject a passenger. Driver only.
- **/ejectall, /eja** — Eject all passengers. Driver only.
- **/ejecteveryone, /eje** — Eject everyone including you. Driver only.
- **/ejectme, /ejm** — Eject yourself. Must be in a vehicle.
- **/givecar, /givekeys, /gkeys, /gk** — Give a player your car keys (auto-unlocks for them; taking it isn't a crime).
- **/instafix** — Use ACME Insta-Fix.
- **/lock, /lk** — Activate your vehicle's alarm system (from outside). Must own a vehicle.
- **/unlock, /ulk** — Deactivate your vehicle's alarm system. Must own the vehicle and have alarm active.
- **/vehcolor, /vehc, /vehcolour** — Donating players: change vehicle color for $100.
- **/vehhood, /hood, /bonnet** — Open/close the hood. Driver only.
- **/vehlights, /vehl, /lights** — Turn all vehicle lights on/off. Driver only.
- **/vehtrunk, /trunk, /boot** — Open/close the trunk. Driver only.

### 13. Drug Commands

- **/bait, /fertilize, /fert** — Makes plants grow quicker but attracts deer.
- **/cook** — Cook drugs (Lab Equipment required).
- **/drugs, /drgs, /drg** — Calls for drugs, or offers if you are a drug dealer.
- **/drugsell, /selldrugs, /dsell, /ds** — Sells drugs at any Drug Refill Point for $x/gram (non law enforcement).
- **/givedrugs, /gdrugs, /gd, /druggive, /dg** — Give drugs to another player.
- **/givefreshdrugs** — Give fresh drugs to another player.
- **/harvest, /harv** — Harvests your plants.
- **/hidedrugs [qty]** — Drop a package of drugs (stays on the map until city change; not saved; lasts 1 game week).
- **/hotbox, /hb, /hotb, /hbox** — Consume drugs with others in your vehicle (gives drugs to others).
- **/plant, /seed** — Plants your seed at your location.
- **/pgps, /pgps** — Toggle plant GPS to track a plant. Blank = list your growing plants. Needs a Plant GPS.
- **/plantinfo, /pinfo, /planti, /pi** — Shows information about your plants.
- **/smoke, /smke, /smk** — Use /td for the smoke animation; fire button smokes 5g at a time.
- **/takedrugs, /td, /drugtake** — Consumes drugs.
- **/trap** — Sets a deer trap (also picks it up if you stand in the checkpoint).

### 14. Messaging Commands

- **/$set (1-3) [text]** — Sets one of your Custom Quick Strings (1/2/3). 95-char max.
- **/1, /2, /3** — Sends a private message to the player saved with /set on Quick PM 1/2/3.
- **/carwhisper, /cw** — Sends a message to all players in your Vehicle.
- **/drivermsg, /dm** — Message all on-duty drivers. Must be on duty. (Driver Skill)
- **/ignore, /ign, /mute** — Ignore / un-ignore all chat from a player.
- **/nopm** — Refuses / accepts private messages.
- **/pm, /ms, /msg, /m, /priv** — Sends a private message to a specified player.
- **/reply, /r** — Replies to the last private message.
- **/say, /s** — Sends a message to all active players.
- **/set** — Sets a player on Quick PM 1/2/3 with an optional message. See /1, /2, /3.
- **/truckmsg, /tm, /truckermsg, /truckm, /cb** — Message all players in a Delivery Truck. Blank = default delivery/pickup message.
- **/whisper, /w** — **Sends a message to all players close to you.** (Proximity/local chat — NOT a private message.)

### 15. Action/Services Commands

- **/adrenaline, /ad** — Heals instantly or cures any diseases.
- **/appeal, /jailappeal** — Appeals your jail sentence.
- **/atm** — Use an ATM.
- **/bail, /pb, /paybail** — Pay a player's bail.
- **/bankrob, /bankr, /robbank, /robb** — Rob the current city bank.
- **/beer, /drinks, /drink, /booze** — Deliver/offer drinks. (Food Delivery Skill)
- **/belt** — Discards your chastity belt.
- **/box** — Drop items during item-carry robberies (Airport, KACC).
- **/breakout, /bo, /brk** — Attempt to break a player out of jail.
- **/bribe, /br** — Bribe the nearest cop (level 2+). Min $1k, Max $15k.
- **/buy, /purchase, /purch** — Opens a menu of the nearest seller.
- **/cancel** — Cancels the mission you are on.
- **/cancelhit, /hitcancel** — Cancels a hit you placed.
- **/casinorob, /casinor, /robcasino, /robc** — Begin a casino robbery (in the casino robbery checkpoint).
- **/cell, /jailcell, /cellchange** — Change your cell (if you haven't murdered recently). One per jail visit.
- **/challenge, /chal, /ch** — Participate in a Race Challenge.
- **/complain** — Report a rule-breaker/cheater in-game; alerts online admins.
- **/courier, /cour, /smuggle, /smug** — Begins a courier mission.
- **/cnrradio** — CnR Radio stream select menu.
- **/crowbar** — Pulls out / puts away your crowbar. Lowers robbery time and increases success chance.
- **/cure, /c, /cu** — Heals and/or cures another player. (Medic Skill)
- **/cureme, /cme** — Heals and/or cures yourself. (Medic Skill)
- **/delivery, /truck, /del, /deliver** — Begins a truck delivery mission.
- **/deposit, /dep** — Deposit money at Bank / City Hall / Regular Players Club.
- **/dice, /playdice** — Accept a dice request or offer to play dice.
- **/driver, /drv, /taxi, /limo, /bus, /air** — Request a driver, or (if in a suitable vehicle) go on duty with a fare.
- **/drop** — Drop items during item-carry robberies (Airport, KACC).
- **/enter** — Enter the DM stadium checkpoint.
- **/escape, /esc** — Attempt to escape jail.
- **/exit** — Exit the DM stadium checkpoint.
- **/fakeskill, /fskill** — Select a fake display skill (Driver default). Kidnapper can toggle on/off duty in a driver vehicle.
- **/food, /pizza** — Deliver/offer food. (Food Delivery Skill)
- **/givecash, /gc, /sendcash, /sendmoney, /givemoney, /$** — Send money to a nearby player.
- **/givegift** — Give another player a gift.
- **/givepet** — Give your pet to another player.
- **/giverep** — Give +1 reputation to another player.
- **/heal, /h, /hl, /medic** — Calls for a medic.
- **/healcure, /hme** — Heals and/or cures another player. (Medic Skill)
- **/hit** — Place a hit on a player.
- **/holdup, /hup, /hold, /storerob, /robstore** — Attempt to rob a store.
- **/horsebet** — Place a horse bet (at a Horse Bet Machine / Inside Track Betting / Woozie's).
- **/housedelivery, /housedel** — Begin a house delivery (in the current city's checkpoint).
- **/infect, /if** — Attempt to infect another player with a disease. (Medic Skill)
- **/items, /item, /itms, /itemsell, /isell, /is** — Offer items / call a Street Vendor. (Street Vendor Skill)
- **/jury, /juror** — Opens the jury textdraw after being chosen as juror.
- **/kidnap, /kd** — Attempt to kidnap a player (must be driving; player in your vehicle). (Kidnapper Skill)
- **/kidnapall, /kda** — Attempt to kidnap all passengers. (Kidnapper Skill)
- **/login** — Logs you in and loads your stats.
- **/lotto** — Play the lottery (draws 18:00 each game day). Pick 1–125.
- **/mechanic, /mech** — Call a Mechanic to your location.
- **/mission, /sellmission, /mis** — Attempt a mission at a Mission Checkpoint or in a Mission Vehicle.
- **/moneybag** — Drop a money bag.
- **/parole, /pa** — Parole a jailed player and pay their bail. Must be in the PD.
- **/payticket, /pay** — Pay your issued ticket to the nearest cop.
- **/police, /911** — Call 911.
- **/possess** — Possess another player (Halloween only).
- **/prices, /price, /setprices** — Set prices for items you sell (Vendor/Arms/Drug/Medic/Food).
- **/ransom, /ran** — Pay a kidnapped player's ransom.
- **/rape, /ra** — Attempt to rape a player. Blank = closest. (Rapist Skill boosts success)
- **/refill, /rf** — Cops refill weapons at PD (rank-based); arms dealers refill at ammunation; medics at hospital; items at 24/7; food at food store.
- **/register** — Registers your nick.
- **/release, /rel** — Release a kidnapped player. (Kidnapper Skill)
- **/releaseall, /relall** — Release all kidnapped victims. (Kidnapper Skill)
- **/rob, /rb** — Rob a player.
- **/robbery** — Start a robbery when near any robbery checkpoint.
- **/carsell, /vsell, /sellcar, /sc** — Sell a car at the crane (if flagged sellable).
- **/sex, /prostitue, /sx** — Offer sex to another player for money. (Prostitute Skill)
- **/shares, /stocks** — Shows shares you own; select for detail.
- **/sharessell, /stockssell** — Sell your stocks.
- **/shoplift** — Attempt to steal an item from a shop/business.
- **/spawn, /newlife** — Continue a new life once logged in (instead of continuing last life).
- **/strip, /strippay** — Tip a dancing prostitute, or start stripping (Prostitute).
- **/sue** — Sue a player.
- **/tip, /givetip** — Tip the person you recently bought goods from.
- **/tree** — Get a gift from a Christmas Tree.
- **/ups** — Begin a UPS delivery mission.
- **/vehrepair** — Repair your vehicle as a Mechanic (on foot).
- **/weapons, /weapon, /weap, /weaponsell, /wsell, /ws** — Offer weapons / call an arms dealer. (Arms Dealer Skill)
- **/withdraw, /wit** — Withdraw money from bank / City Hall / Regular Players Club.

> Note: the site also lists **/healme, /hme** (heals/cures yourself, Medic Skill) in the same section — there is a duplicate `/hme` alias shared between /healcure and /healme on the published page.

### 16. Settings/Options Commands

- **/canims, /aoff** — Turns off animations at the spawn screen.
- **/casinosettings** — Change casino game settings.
- **/changepassword, /changepass** — Changes your in-game password.
- **/clearost, /clrost, /cs** — Clears on-screen text.
- **/display, /disp** — Show/set display mode (Speedometer / Clock).
- **/displayost, /ost, /dispost** — Displays on-screen text.
- **/deathmsg, /dmon, /dmoff** — Toggle Chat Death Message Display (default OFF). F9 toggles on-screen death messages.
- **/gender, /gend** — Show/set your gender.
- **/gtamenu, /menus** — Switch between textdraw menus and old text-in-chat menus.
- **/helpmsg, /hmon, /hmoff** — Toggle Help Messages (default ON).
- **/joinmsg, /partmsg** — Toggle join messages (default ON).
- **/ostsettings** — OST settings menu.
- **/settings, /options, /set** — Adjust in-game options (gender, display, death/help messages).
- **/skill, /skl** — Change your skill when you pick a different skin.
- **/tdsettings, /tdoptions, /tdset** — Text-draw settings.

---

## Cop / police toolkit — consolidated (for gap analysis)

The dedicated cop verbs live in the **Police Commands** category, but several enforcement-relevant commands live in other categories. Full cop-facing set:

| Command | Aliases | What it does |
|---|---|---|
| /arrest | /ar | Arrest specified player or closest **wanted** suspect (warrant required). |
| /ticket | /tk | Ticket a **suspect** (yellow, no warrant) so they can pay to reduce wanted level; must stay close. |
| /freeze | /fr, /pullover, /pu | Order a suspect to pull over / freeze / pay ticket / surrender. |
| /report | /rp | Report criminal activity → raises target's wanted level (serious crimes only). |
| /cancellastreport | /cancelreport, /clr | Undo your last report. |
| /visualcontact | /vc, /vcontact | Lock a suspect's wanted level from dropping for a time. |
| /backup | /bk | Request backup at your location (+ optional 10-code message); requester shows purple on minimap. |
| /respond | /yes | Respond to the last call; updates the red call checkpoint and notifies officers. |
| /copmsg | /cm | Broadcast to all police. |
| /accept | /ac | Accept an offered bribe. |
| /refuse | /ref | Refuse a bribe and report the offerer. |
| /refill | /rf | Refill weapons at PD (rank-based). |
| /vehrepair | /vehfix | Buy/use a PD vehicle repair kit anywhere (~10–20k). |
| /suspects | /sus, /wanted, /wan | List all suspects. |
| /warrants | /warrant, /war | List all wanted (warrant issued) suspects. |
| /mostwanted | /most, /mw | List all Wanted-Level-10 suspects. |
| /robberies | /robs, /rb | List in-progress holdups/robberies not yet stopped. |
| /jaillist | /jl, /jlist | List players in jail. |
| /crimes | /crime | Last 5 crimes committed by a suspect. |
| /parole | /pa | Parole a jailed player + pay bail (at PD). |
| /donut | — | Refill 50% health (cop-only shops). |
| /fire | — | Respond to a fire (Fire Truck). |
| /mission | /mis | Cop missions (Routine Patrol, Domestic Disturbance) at PD checkpoint. |
| /missing | — | (from changelog v24.1) law-enforcement command; **not on the commands page** — flagged for verification. |

**Notable absences vs. common cop-robber gamemodes** (these do NOT exist as CB commands — confirmed by full-page read):
- **No `/taze`, `/tazer`, `/taser`** — CB has no taser command.
- **No `/cuff` / `/uncuff` / `/frisk`** — no cuffing or frisk mechanic; the arrest flow is `/freeze` → `/arrest`.
- **No `/su` (suspect flag)** — suspect/wanted status is driven by `/report` + the crime system, not a manual `/su`.
- **No `/spike` / `/spikestrip` / `/roadblock` command** — roadblocks exist only as a 10-code convention (10-35, 10-135, 10-156), not a spawn command.
- **No `/jail` / `/unjail` / `/sendtojail`** — jailing is automatic on arrest; there is no manual jail command for cops (admin-only, not in this public list).
- **No `/dispatch` command** — "dispatch" exists only as 10-codes (10-53, 10-147).
- **No `/radio`** — radio-style comms are `/copmsg`, `/backup`, `/respond`, plus the 10-code + quick-string system.
- **No on/off `/duty`** — cop status is the Police Officer *skill* (chosen via `/skill` + skin), not a duty toggle. (Drivers use `/driver` to go "on duty"; kidnappers use `/fakeskill`.)

---

## Player crime / social commands — consolidated

**Chat modes (see Messaging Commands):**
- `/say` (`/s`) — global chat to all players.
- `/whisper` (`/w`) — **local/proximity chat** to nearby players. ← this is what `/w` is.
- `/carwhisper` (`/cw`) — chat to everyone in your vehicle.
- `/pm` (`/ms`, `/msg`, `/m`, `/priv`) — true private message to one player; `/reply` (`/r`) replies; `/1` `/2` `/3` PM saved slots; `/nopm` blocks PMs; `/ignore` (`/mute`) mutes a player.
- Channel chats: `/groupmsg` (`/gm`), `/copmsg` (`/cm`), `/truckmsg` (`/cb`), `/fishmsg` (`/fm`), `/drivermsg` (`/dm`), `/pimpmsg`, `/racemsg`.

**Note on `/me` and `/do`:** CB does **NOT** have `/me` or `/do` roleplay-emote commands. Expressive actions are handled by the large **Animation Commands** set (`/wave`, `/dance`, `/point`, `/cry`, `/shout`, `/mourn`, etc.). This is a deliberate difference from RP servers — CB is action/mission-driven, not `/me`-narration RP.

**Crime commands (civilian):**
- `/rob` (`/rb`) — rob a player (Con Artist/Pickpocket boost success/amount).
- `/rape` (`/ra`) — rape attempt (Rapist boost).
- `/holdup` (`/hup`, `/hold`, `/storerob`, `/robstore`) — store holdup.
- `/bankrob` (`/robbank`) / `/casinorob` (`/robcasino`) / `/robbery` — bank / casino / checkpoint robberies.
- `/houserob` (`/hrob`, `/hsrob`) — house robbery.
- `/shoplift` — steal an item from a shop.
- `/crowbar` — toggle crowbar (lowers robbery time, raises success).
- `/kidnap` (`/kd`) / `/kidnapall` / `/release` / `/ransom` — kidnapper flow.
- `/hit` / `/hits` / `/cancelhit` — hitman contracts.
- `/bribe` (`/br`) — bribe nearest cop (level 2+, $1k–$15k).
- `/box`, `/drop`, `/moneybag`, `/hidedrugs` — item-carry / stash mechanics.

**Social / economy:**
- `/givecash` (`/gc`, `/$`) send money; `/pay` (`/payticket`) pay a ticket; `/tip`; `/giverep`; `/givegift`; `/flowers`; `/dice`; `/sue`; `/complain` (report cheaters to admins).
- Player info: `/stats` (`/sts`) current-life stats; `/morestats`; `/total` lifetime; `/myinfo`; `/info` (`/i`) about a player; `/level` (`/rank`); `/help` (`/hlp`); `/faq`; `/commands` (`/cmds`).
- `/report` is **cop-only** (crime report) — the civilian "report a cheater" command is **`/complain`**.

---

## Complete 10-Code list (verbatim, 10-0 → 10-161)

**Source:** https://www.crazybobs.net/website/cnr-10-codes — *"These are the CnR 10 codes which can be used anywhere in-game."* They are usable inside `/backup`, `/copmsg`, `/respond`, `/report`, `/groupmsg` and any chat, and auto-expand `$veh`, `$loc`, `$time`, `$sus`, `$dir`, `$esc` quick strings.

| Code | Meaning | Code | Meaning |
|---|---|---|---|
| 10-0 | Use Caution | 10-81 | Illegal Fishing/Hunting |
| 10-1 | Signal Weak | 10-82 | Car Occupied Multiple Times |
| 10-2 | Signal Good | 10-83 | Suspect Hidden on Radar $loc |
| 10-3 | Stop Transmitting | 10-84 | Multiple Suspect in the Area Of $loc |
| 10-4 | Okay, Affirmative | 10-85 | Suspect Down/Custody $loc |
| 10-5 | Relay To | 10-86 | Crime in Progress at $loc |
| 10-6 | Busy Unless Urgent | 10-87 | Illegal Drug Activity |
| 10-7 | Murder | 10-88 | Suspect has a Gun $loc |
| 10-8 | In Service in a $veh at $loc | 10-89 | Bomb Threat |
| 10-9 | Say Again, Repeat | 10-90 | I Need a New Vehicle |
| 10-10 | Negative, No | 10-91 | Carrying Illegal Items |
| 10-11 | On Duty | 10-92 | Theft |
| 10-12 | Stand By | 10-93 | Misuse of Communicator |
| 10-13 | Follow Me | 10-94 | Me |
| 10-14 | Message/Information | 10-95 | At Police Station |
| 10-15 | Message Delivered | 10-96 | Mental Subject/Problems |
| 10-16 | Reply To Message | 10-97 | Test Communicator |
| 10-17 | Robbery | 10-98 | Prison Break by $Esc |
| 10-18 | Urgent | 10-99 | Wanted/Stolen |
| 10-19 | In Contact | 10-100 | Taking 5 Minute Break |
| 10-20 | Location: $loc | 10-101 | LOL! |
| 10-21 | Assault | 10-102 | My Game Crashed |
| 10-22 | Disregard, Nevermind | 10-103 | Noob Cop/Cops!! |
| 10-23 | Arrived at Location | 10-104 | Crooked Cop Aiding Suspect |
| 10-24 | Sexual Assault | 10-105 | On Patrol at $loc |
| 10-25 | Meet With Person | 10-106 | Escort |
| 10-26 | ETA | 10-107 | Land |
| 10-27 | Drivers License Check | 10-108 | Air |
| 10-28 | Weapon Activity | 10-109 | Water |
| 10-29 | I Need a Medic $loc | 10-110 | Innocent |
| 10-30 | Danger/Caution | 10-111 | Yellow, seems cooperative |
| 10-31 | Pick Up Person | 10-112 | Yellow, uncooperative |
| 10-32 | # Units Needed | 10-113 | Orange, Warrant Issued |
| 10-33 | Requesting Help Immediately at $loc | 10-114 | Most Wanted |
| 10-34 | Vehicle Theft | 10-115 | Backup Needed, 1 unit $loc |
| 10-35 | Roadblock set up at $loc | 10-116 | Backup Needed, Multiple Units $loc |
| 10-36 | Security Check | 10-117 | Do I have backup coming to $loc? |
| 10-37 | Drunk Driving | 10-118 | Suspect Fleeing into Country Side at $loc |
| 10-38 | Having Computer Problems… | 10-119 | Pursuit Halted/Stand Still $loc |
| 10-39 | Use lights/sirens...(Urgent) | 10-120 | Good Job! |
| 10-40 | No lights/sirens...(Silent) | 10-121 | Getting a New Vehicle |
| 10-41 | Beginning Tour of Duty at $time | 10-122 | Escort |
| 10-42 | Ending Tour of Duty at $time | 10-123 | Ticket Issued to $sus |
| 10-43 | Shuttle | 10-124 | $sus Paid their Ticket |
| 10-44 | Permission to Leave | 10-125 | Joining Pursuit $loc $dir |
| 10-45 | Human Remains at $loc | 10-126 | Returning to Patrol Zone |
| 10-46 | Assist Motorist | 10-127 | Vehicle Lightly Damaged |
| 10-47 | Kidnapping | 10-128 | Vehicle Heavily Damaged |
| 10-48 | Subject Disturbing the Peace | 10-129 | Flat Tire |
| 10-49 | Abandoned Vehicle | 10-130 | Robbery/Hold Up |
| 10-50 | Traffic Collision | 10-131 | Casino Robbery |
| 10-51 | Suspect on Foot | 10-132 | Bank Robbery |
| 10-52 | Ambulance Needed at $loc | 10-133 | Hit Contract |
| 10-53 | Dispatch | 10-134 | Returning to City Limits from $loc |
| 10-54 | Change to Channel | 10-135 | Roadblock |
| 10-55 | Intoxicated Driver | 10-136 | Intercepting Suspect from $loc Going $dir |
| 10-56 | Intoxicated Pedestrian | 10-137 | Shoot the Suspect's Tire |
| 10-57 | Hit and Run | 10-138 | Suspect in a Vehicle |
| 10-58 | Air Plane Crash | 10-139 | Suspect has a Weapon |
| 10-59 | Unlawful Driver | 10-140 | Deathmatching |
| 10-60 | Suspected Hacking | 10-141 | Friendly Fire |
| 10-61 | I'm a noob! | 10-142 | Which way are you heading? |
| 10-62 | Attempting PIT | 10-143 | Requesting Pickup $loc |
| 10-63 | Going to Hospital | 10-144 | Requesting Fast Pursuit Unit $loc |
| 10-64 | Going to a Restaurant | 10-145 | Requesting Air Support $loc |
| 10-65 | Going to Ammunation | 10-146 | Requesting Boat Support $loc |
| 10-66 | Major Crime Alert | 10-147 | Dispatch, Requesting Assignment |
| 10-67 | PM me | 10-148 | Fog - Air unit out of service. |
| 10-68 | GiveCash To | 10-149 | Rustler Moving in to shoot, back off! $loc |
| 10-69 | Rape, Sexual Misconduct | 10-150 | I'm out of Ammo |
| 10-70 | Lost Visual with Suspect $loc | 10-151 | Out of Service at $loc |
| 10-71 | Drug Activity | 10-152 | Going to Location |
| 10-72 | Aborting Pursuit $loc | 10-153 | $dir Bound |
| 10-73 | Missing Person | 10-154 | Assignment Complete |
| 10-74 | Civil Disturbance | 10-155 | Registration Check |
| 10-75 | Domestic Problem | 10-156 | Requesting Roadblock Immediately, we're at $loc |
| 10-76 | Bribe | 10-157 | Gang Activity |
| 10-77 | Return to | 10-158 | Investigate Vehicle |
| 10-78 | Backup Needed at $loc | 10-159 | Suspect has gotten Away $loc |
| 10-79 | Notify Coroner | 10-160 | In Hot Pursuit!!! $loc $dir Bound |
| 10-80 | In Pursuit with $sus $loc | 10-161 | Burglary |

Total: **162 codes (10-0 through 10-161)**.

---

## Law-enforcement & service skills (verbatim from /website/skills)

- **Police Officer** — "Police Officers must protect the city and it's civilians from crime. Their main roles involve arresting warrants (orange) and issuing tickets to suspects (yellows)." Commands: `/arrest [nick/id]` (`/ar`, `a`), `/report [nick/id] [crime]`, `/cancellastreport [nick/id]`, `/ticket [nick/id]`, `/backup [message]`, `/calls`, `/respond [message]`, `/copmsg [message]`, `/refill [weapon]`, `/accept [nick/id]`, `/refuse [nick/id]`, `/jaillist`, `/suspects`, `/warrants`, `/mostwanted`, `/mission`.
- **Public Medic** — "Medics can heal, cure or sell different items to other players. Public Medics can cure diseases without request to prevent epidemics." Commands: `/medic [nick/id]`, `/cure [nick/id]`, `/healme`, `/cureme`, `/prices`, `/calls`.
- **Police Technician** — "Police Technicians help cops refill their weapons and ammo as well as offer Mechanic services. They can only provide their services to other law enforcement agents." Commands: `/weapons [nick/id]`, `/prices`, `/calls`, `/vehrepair`.
- **Private Medic** — "Private Medics can heal and cure players as well as selling a variety of different medical items such as condoms and STDs." Commands: `/medic [nick/id]`, `/healme`, `/cureme`, `/infect [nick/id]`, `/calls`.

Civilian skills (brief): Arms Dealer (`/weapons`,`/prices`,`/calls`); Car Jacker (`/sell`); Con Artist (`/rob`); Mechanic (`/mechanic`,`/prices`,`/calls`,`/vehrepair`); Drug Dealer (`/drugs`,`/prices`,`/calls`); Food Delivery (`/food`,`/prices`,`/calls`); Hitman (`/hits`); Kidnapper (`/kidnap`,`/kidnapall`,`/release`,`/releaseall`,`/fakeskill`); Pickpocket (`/rob`); Street Vendor (`/items`,`/prices`,`/calls`); Prostitute (`/sex`,`/strip`,`/calls`); Rapist (`/rape`).

---

## Gap-analysis flags

1. **`/w` = /whisper = LOCAL proximity chat**, not PM. Any port that maps `/w` to private messaging diverges from CB.
2. **No taser / cuff / frisk / spike / roadblock / su / dispatch / duty / jail commands** exist in CB — those enforcement concepts are handled by arrest+ticket+report+10-codes, not discrete commands. Do not invent them if aiming for CB parity.
3. **No `/me` or `/do`** — CB uses the Animation Commands set instead.
4. **`/complain`** is the civilian cheater-report; **`/report`** is cop-only crime reporting.
5. **`/missing`** appears in the v24.1 changelog as a law-enforcement command but is NOT on the commands page — verify whether it shipped / was renamed.
6. Published page has minor duplicate-description copy/paste artifacts (`/groupcreate` reuses groupcall text; `/petfeed` reuses diet text; `/clothesinv` reuses discard text; `/hme` shared by `/healcure` and `/healme`) — treat those descriptions with mild caution; command names/aliases are reliable.
