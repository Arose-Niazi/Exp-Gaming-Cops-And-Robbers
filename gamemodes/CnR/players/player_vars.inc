/*

	Players variables for Cops And Robbers script
	Scripted by Arose Niazi.
	Scripting started on 11th October 2017
	Last Updated: -----
	
	Credits:
		
	
	Functions:
		ClearPlayerVariables(playerid);
		
	Functions Being Called:
		
*/


enum pInfo
{
	pID,
	pUserName[MAX_PLAYER_NAME],
	pPassword[MAX_HASH_SIZE],
	pRegisteredDate[50],
	pEmail[MAX_EMAIL_SIZE],
	pCurrent_country[56],
	bool:pRegistered,
	bool:pLoggedIn,
	bool:pAutoLogin,
	pLoginAttempts,
	
	pRank,
	bool:pDj,
	bool:pVip,
	bool:pRegularPlayer,
	
	pMoney,
	
	pZone,
	pZoneName[MAX_ZONE_NAME],
	pExteriorName[MAX_ZONE_NAME],
	bool:pInPoliceProperty,
	pInterior,
	
	bool:pClassSelection,
	
	pClassID,
	pSkin,
	pTeam,
	pSkinsSelected[MAX_PEDS],
	pGender,
	pGenderChanges[3],
	
	bool:pSpawned,
	pSpawns,
	
	bool:pHourlyTimeUpdate,
	bool:pShowJoinMessages,
	bool:pClassMusic,
	
	pSerial[MAX_SERIAL_SIZE],
	pIP[MAX_IP_SIZE],
	p_AdminNick[MAX_PLAYER_NAME],
	
	pMonthlyActivity,
	
	pMenu,
	
	bool:pAddingInterior,
	pAddInteriorType,
	Float:pAddInteriorOSX,
	Float:pAddInteriorOSY,
	Float:pAddInteriorOSZ,
	Float:pAddInteriorOSA,
	Float:pAddInteriorISX,
	Float:pAddInteriorISY,
	Float:pAddInteriorISZ,
	Float:pAddInteriorISA,
	Float:pAddInteriorOPX,
	Float:pAddInteriorOPY,
	Float:pAddInteriorOPZ,
	Float:pAddInteriorIPX,
	Float:pAddInteriorIPY,
	Float:pAddInteriorIPZ,
	pAddInteriorInt,
	pAddInteriorOutZone[MAX_ZONE_NAME],
	pAddInteriorZone[MAX_ZONE_NAME],
	pAddInteriorVW,
	bool:pAddInteriorCivWeapons,
	bool:pAddInteriorCopWeapons,
	bool:pAddInteriorWeapons,
	pEditingIntID,
	pEditingInterior,
	bool:pAddInteriorLabels,
	
	pMenuTD_BG,
	
	bool:pAddingActor,
	bool:pEditingActor,
	pEditingActorID,
	pAddActorType,
	pAddActorInt,
	pAddActorVW,
	Float:pAddActorSpawnX,
	Float:pAddActorSpawnY,
	Float:pAddActorSpawnZ,
	Float:pAddActorSpawnA,
	pAddActorSkin,
	pAddActorLocation[MAX_ZONE_NAME],
	pAddActorAnimationType,
	bool:pAddActorLabelEnable,
	
	GPS_RouteID,
	GPS_Destination,
	GPS_CreatedObjects[MAX_NODS],
	GPS_CreatedMarkers[100],
	GPS_MakerLength,
	bool:GPS_Calculating,
	GPS_Lenght,
	GPS_LostCount,
	GPS_Nods[MAX_NODS],
	Float:GPS_NodsDistance[MAX_NODS],
	Float:GPS_Distance,
	pTimes_GPSUsed,
	bool:pDistanceInMiles,
	bool:pShowGPSDistance,
	
	pHideOutLocation,
	pMissionLocation,

	Float:pCustomMapMarker[3],

	// ---- M2 : command / moderation infra ----
	pLastPMFrom,				// last PM sender id for /reply (-1 = none)
	bool:pNoPM,					// /nopm toggle (session-only)
	pPMFloodTime,				// GetTickCount() of last PM (flood throttle)
	bool:pShowCommands,			// staff command-echo toggle
	bool:pAdminDuty,			// /aduty on-duty flag
	bool:pAdminJailed,			// currently admin-jailed
	pAdminJailUntil,			// unix time the admin-jail ends (0 = free) [persisted]
	pAdminJailTimer,			// per-player release timer handle (-1 = none)
	bool:pInDMZone,				// inside a DM-flagged teleport zone (anti-parachute)
	bool:pDebugMode,			// scripter /debug toggle
	pVCFreezeUntil,				// cop /visualcontact hold expiry (set in M3; blocks teleport)
	bool:pHealthHackFlags,		// anti-cheat HP-hack suspicion (detection lands M3)
	bool:pAimbotFlags,			// anti-cheat aimbot suspicion (detection lands M3)

	// ---- M3 : wanted-level system (design §2) ----
	pWanted,					// 0..MAX_WANTED (LSplayers.Wanted)
	pWantedDecayCounter,		// seconds accumulated toward the next -1 decay tick
	pVCByCop,					// playerid of the cop currently holding visual contact (-1 = none)
	pSpawnProtectUntil,			// gettime() while spawn protection is active (0 = off)

	// ---- M3 (stage 2) : cop + jail system (design §3) ----
	pScore,						// player score (LSplayers.Score) — cop rewards / arrests
	pCopRank,					// cop rank ladder 0..10 Recruit->Commissioner (LSplayers.CopRank)
	pRefillPoints,				// cop /refill points earned by cop work, max REFILL_POINTS_MAX
	pRefillCooldown,			// gettime() the next /refill is allowed (0 = ready)
	pDonutCarry,				// /donut heals carried (reset at donut shop), max DONUT_CARRY_MAX
	pSeriousCrimes,				// count of serious (murder/violent) crimes — blocks appeal (LSplayers.SeriousCrimes)

	// ticket (issued by a cop, WL1-5); escalates to a warrant if unpaid in time
	pTicketAmount,				// outstanding fine ($) — 0 = no ticket
	pTicketByCop,				// cop id that issued the ticket (-1 = none)
	pTicketExpiry,				// gettime() the ticket escalates to a warrant (§2.4)

	// jail (cop-jail, distinct from the M2 admin-jail; admin-jail takes precedence)
	bool:pJailed,				// currently cop-jailed (LSplayers.Jailed)
	pJailUntil,					// gettime() the sentence ends (0 = free) [persisted as remaining]
	pJailStart,					// gettime() the sentence began (for the served-minimum check)
	pJailBail,					// bail cost to buy out (LSplayers.Bail)
	pJailReason[64],			// charge text (LSplayers.JailReason)
	pJailTimer,					// per-player release/tick timer handle (-1 = none)
	bool:pEscapeChained,		// re-caught escapee, cannot /escape again (LSplayers.EscapeChained)

	// bribe (jailed player offers cash to a nearby cop; cop /accept or /refuse)
	pBribeTo,					// cop id the bribe was offered to (-1 = none)
	pBribeAmount,				// bribe amount offered
	pBribeFrom,					// (on the cop) jailed player id offering the bribe (-1 = none)

	// appeal (jury of online players votes /guilty or /innocent)
	bool:pOnJury,				// currently sitting on an appeal jury
	pJuryFor,					// the jailed player this juror is voting on (-1 = none)
	pAppealYes,					// (on the appellant) innocent votes tallied
	pAppealNo,					// (on the appellant) guilty votes tallied
	pAppealJurors,				// (on the appellant) jurors summoned for this appeal

	// ---- M3 (stage 3) : player crime commands (design §4) ----
	pSkill,						// civilian/cop skill (pickpocket/con-artist/rapist/…) — set at City Hall in M6 (LSplayers.Skill)

	// /rob (§4.1)
	pRobCooldown,				// gettime() the player may /rob again (per-robber cooldown)
	pRobbedUntil,				// gettime() this player can be robbed again (per-victim cooldown)

	// /rape + STD (§4.2)
	pRapeCooldown,				// gettime() the player may /rape again
	pSTD,						// STD bitmask (LSplayers.STDs) — STD_CHLAMYDIA..STD_MARYLOU
	pSTDDrainCounter,			// seconds accumulated toward the next STD HP-drain tick
	pCondoms,					// condoms carried (reduce infection chance) (LSplayers.Condoms)
	bool:pChastityBelt,			// prevents being raped, breakable (LSplayers.ChastityBelt)

	// /takedrugs + drug system (§4.3)
	pDrugs,						// grams of drugs carried (LSplayers.Drugs)
	bool:pHasDrugBag,			// §16 #2: doubles the Drug Dealer carry cap (acquired via M6 refill points)
	bool:pDrugClickMode,		// /td with no amount → click-mode (5g/click, F exits)
	pDrugBuzzStage,				// BUZZ_NONE..BUZZ_VERYHIGH (heal speed / buzz length)
	pDrugEffectUntil,			// gettime() the current buzz ends (0 = not buzzed)
	pDrugHealCounter,			// seconds accumulated toward the next small heal tick
	bool:pDrugOverdose,			// currently overdosing (slow death until adrenaline)

	// /jumpkick (§5)
	pJumpkickCooldown,			// gettime() the player may /jumpkick again

	// ---- M4 (stage 1) : bank, taxes & insurance (design §10/§11.2) ----------
	pBankMoney,					// banked cash (LSplayers.BankMoney) — safe from robbery/medical fees
	pTaxOwed,					// unpaid tax carried forward (LSplayers.TaxOwed) — chased next tax tick
	pLifeInsurance,				// active life-insurance policy count (LSplayers.LifeInsurance), max LIFE_INS_MAX
	pHealthInsExpiry,			// gettime() health insurance lapses (LSplayers.HealthInsExpiry, 0 = none)
	pGiveCashCooldown,			// gettime() the player may /givecash again (anti-abuse)

	// ---- M4 (stage 2) : lotto, money events, /ad + /advert, holdup ----------
	pLottoNumber,				// lotto number picked for the current game-day (LSplayers.LottoNumber, 0 = no ticket)
	pAdrenaline,				// adrenaline pills carried (/ad — full heal + STD cure + OD clear), max ADRENALINE_MAX
	pAdvertCooldown,			// gettime() the player may /advert again (§7)
	pHoldupCooldown,			// gettime() the player may /holdup again (§5.2)
	bool:pHoldingUp,			// currently mid-holdup (clerk wait / register payout running)
	pHoldupTimer,				// per-player holdup sequence timer handle (-1 = none)
	pHoldupStage,				// holdup sequence step (0 = clerk wait, 1..HOLDUP_PAYOUT_SECS = paying)
	pHoldupTake,				// running total taken from the register this holdup
	Float:pHoldupX,				// store position locked at holdup start (leave-the-zone check)
	Float:pHoldupY,
	Float:pHoldupZ,
	bool:pInMoneyRush,			// joined the active money-rush event
	pMoneyRushEarned,			// cash grabbed during the current money-rush (per-event leaderboard)

	// ---- M5 (stage 2) : full robbery system + crowbar (design §5) -----------
	bool:pCrowbarEquipped,		// crowbar clothing item equipped (LSplayers.CrowbarEquipped) — shortens robbery/safe time + raises register take (§5.7)
	pRobberyHistory,			// bitmask of robbery types committed (LSplayers.RobberyHistory, ROB_*) — drives crowbar confiscation on arrest (§5.7)
	pRobStage,					// safe-type robbery stage (ROBSTAGE_NONE/CRACKING/CARRYING) — bank/casino/special (§5.3-5.6)
	pRobKind,					// which safe-type robbery is running (ROBKIND_BANK/CASINO/SPECIAL)
	pRobSite,					// index into the robbery-site table the current robbery belongs to
	pRobTimer,					// per-player safe-crack countdown timer handle (-1 = none)
	pRobCrackLeft,				// seconds left on the current safe-crack (counts down on pRobTimer)
	pRobDeadline,				// gettime() the carry-to-hideout window expires (0 = not carrying)
	pRobHideout,				// hideout-site index the safe must be delivered to while carrying
	pShopliftCooldown,			// gettime() the player may /shoplift again (§5.1)

	// ---- M4 (stage 3) : reusable mission framework (design §6) --------------
	pOnMission,					// active mission id (index into MissionReg[]), or -1 = none
	pMissionCheckpoint,			// current checkpoint index reached so far (0..route length)
	pMissionRouteLen,			// number of checkpoints in the active route (usually 5)
	pMissionVehicle,			// vehicle id the mission is bound to (vehicle missions), or -1
	pMissionData,				// generic per-mission scratch slot (running earnings this run)
	Float:pMissionRouteX[MISSION_MAX_CHECKPOINTS],	// active route checkpoint coords (X)
	Float:pMissionRouteY[MISSION_MAX_CHECKPOINTS],	// active route checkpoint coords (Y)
	Float:pMissionRouteZ[MISSION_MAX_CHECKPOINTS],	// active route checkpoint coords (Z)
	pMissionCooldown[MISSION_COUNT],	// last-completion game-hour stamp per mission (0 = never) [persisted, packed]

	// ---- M6 (stage 4) : bespoke mission state (Paperboy real-time / target / race / domestic) — session only ----
	pMissionDeadline,				// gettime() a real-time mission (Paperboy §6.11 / Domestic §6.3) expires (0 = none)
	pMissionStartHour,				// game-hour a windowed target mission (Pickpocket/Sexual/Flower) started (window expiry check)
	pMissionTargetCd,				// gettime() the next target action (pickpocket/encounter/flower) is allowed (anti-spam)
	pMissionRaceId,					// which g_RaceReg race is active (Race Challenge §6.13), or -1
	Float:pMissionDestX,			// single-destination coords (Domestic Disturbance §6.3 — the target house)
	Float:pMissionDestY,
	Float:pMissionDestZ,

	// ---- M5 (stage 1) : vehicle ownership — CB-simple model (design §9.12) --
	pLastVehicle,						// vehicleid this player last drove as DRIVER (INVALID_VEHICLE_ID = none) — /lock//unlock act on it (session only)

	// ---- M5 (stage 3) : housing system (design §9.1/§5.5) ------------------
	pInsideHouse,						// house index the player is currently inside (-1 = outside) — session only
	pOwnedHouseCount,					// how many houses this player owns right now (own-cap MAX_OWNED_HOUSES) — recomputed on house load/buy/sell
	pHouseSpawn,						// house index this player spawns at (/sethousespawn), or -1 = default city spawn (persisted: LSplayers.HouseSpawnID)
	pHouseRobCooldown,					// gettime() the player may /houserob again (§5.5 per-robber cooldown, session only)

	// ---- M5 (stage 4) : kidnap crime (design §4.4) — all session-only ------
	pKidnapVictim,						// playerid this player is currently holding kidnapped (-1 = none)
	pKidnappedBy,						// playerid currently kidnapping this player (-1 = free)
	pKidnapRansom,						// ransom set on the held victim (≤ KIDNAP_RANSOM_MAX)
	pKidnapCooldown,					// gettime() the player may /kidnap again (per-kidnapper cooldown)
	bool:pFakeSkillDriver,				// /fakeskill — kidnapper disguised as a Driver (session flag, §4.4)

	// ---- M5 (stage 4) : Airport Robbery box mechanic (§6.1) — session-only --
	pAirportPhase,						// AIRPHASE_NONE/COLLECT/DELIVER — the bespoke box-collection state
	pAirportBoxes,						// boxes collected this run (drives the delivery payout)
	pAirportDeadline,					// gettime() the current airport phase (collect / deliver) expires
	pAirportVehicle,					// the truck/van/garbage-truck the airport run is bound to (-1 = none)
	pAirportSite,						// which airport site index this run belongs to
	pBoxCollectCooldown,				// gettime() the next /box grab is allowed (anti-spam) [OUR DESIGN]

	// ---- M6 (stage 1) : skills, fighting styles & clothes (design §9.8/§9.7) ----
	pFightStyle,						// chosen GTA fighting style (SetPlayerFightingStyle) — persisted (LSplayers.FightStyle, default 4), re-applied on spawn (§9.8)
	pSkillChangeCd,						// gettime() the player may /skill (change skill) again — re-pick cooldown (session; §9.8) [OUR DESIGN]

	// ---- M6 (stage 2) : fishing & farming (design §9.2/§9.3) ---------------
	// Fishing (§9.2) — persisted permits/rod/cooler + a compact cooler aggregate.
	pFishPermits,						// fishing permits held (LSplayers.FishPermits, ≤50, 1 fish/permit)
	pHuntPermits,						// hunting permits held (LSplayers.HuntPermits, ≤20, deer)
	pDrugSeeds,							// drug plant seeds held (LSplayers.DrugSeeds, ≤10)
	bool:pHasFishingRod,				// owns a fishing rod (LSplayers.HasFishingRod — faster/higher catch)
	bool:pHasFishCooler,				// owns a fish cooler (LSplayers.HasFishCooler — bigger catch capacity)
	bool:pHasFishSalesPermit,			// owns a fish sales permit (LSplayers.FishSalesPermit — sell to players)
	pCoolerCount,						// fish in the cooler (LSplayers.CoolerCount, 0..cap)
	pCoolerWeight,						// total pounds of fish in the cooler (LSplayers.CoolerWeight)
	pCoolerValue,						// accumulated base $ value of the cooler contents (LSplayers.CoolerValue, pre-market-mult)
	bool:pFishing,						// currently reeling in a catch (session — the /fish wait)
	pFishTimer,							// per-player /fish reel-in timer handle (-1 = none)
	Float:pFishX,						// fishing-spot position locked at /fish start (moved-away check)
	Float:pFishY,
	Float:pFishZ,
	// Farming (§9.3) — plants are DB rows; these are session refs / cooldowns.
	pPlantCount,						// how many active plants this player owns right now (own-cap MAX_PLANTS_PER_PLAYER) — recounted on load/plant/harvest
	pFarmJobCooldown,					// gettime() the player may start a farm job again (§9.3 legal crop, session)
	pHuntCooldown,						// gettime() the player may /hunt again (§9.3, session)

	// ---- M6 (stage 3) : DM/sniper/duel arenas (design §9.6) — all session-only --
	pArena,								// current arena kind (ARENA_NONE/DMS/SNIPER/DUEL) — suppresses wanted/arrest, respawns inside
	// Pre-arena snapshot restored on /leave or duel end (clean loadout restore, §6.4).
	bool:pArenaSaved,					// true while a snapshot is held (guards a double save/restore)
	Float:pArenaBackX,					// prior world position (X/Y/Z/angle)
	Float:pArenaBackY,
	Float:pArenaBackZ,
	Float:pArenaBackA,
	pArenaBackInt,						// prior interior + virtual world
	pArenaBackVW,
	Float:pArenaBackHP,					// prior health + armour
	Float:pArenaBackArmour,
	pArenaBackWeap[13],					// prior 13 weapon-slot weapon ids
	pArenaBackAmmo[13],					// prior 13 weapon-slot ammo counts
	// Duel state (§6.4) — a challenge/accept escrow pair.
	pDuelOpponent,						// the other duellist's playerid (-1 = none)
	pDuelChallenger,					// the id that OWNS the duel VW (lower/initiating id) — for VW derivation
	pDuelStake,							// escrowed stake this player anted (winner takes the sum)
	pDuelOfferTo,						// (on challenger) id the pending challenge was sent to (-1 = none)
	pDuelOfferFrom,						// (on target) id that challenged this player (-1 = none)
	pDuelOfferStake,					// stake proposed in the pending challenge
	pDuelOfferExpiry,					// gettime() the pending challenge lapses
	bool:pInDuel,						// true once both are teleported into a live duel (mid-duel quit = forfeit)

	// ---- M6 (stage 3) : safe / no-crime zones (design §2.5 / §9.10) ------------
	bool:pInSafeZone,					// inside a hospital/RPC safe zone (no-crime; extends pInPoliceProperty)

	// ---- M7 : stock market (design §9.4/§10, §11.3) ---------------------------
	// Cached per-account share holdings (one cell per stock, index = E_STOCK_*).
	// Loaded from stock_holdings on login (Stocks_OnPlayerLogin, race-guarded);
	// a trade updates the cell + upserts/deletes the row. A holding is 0..STOCK_MAX_OWN.
	pStocks[MAX_STOCKS],
	bool:pStocksLoaded					// true once the holdings query has resolved for this slot
}

// Session-only ignore matrix (bit-packed to save memory): does A ignore B?
// pIgnored[a][b>>5] bit (b & 31). Reset per-connect in ClearPlayerVariables.
new pIgnored[MAX_PLAYERS][(MAX_PLAYERS + 31) / 32];

new PlayerInfo[MAX_PLAYERS][pInfo];


FUNCTION ClearPlayerVariables(playerid)
{
	PlayerInfo[playerid][pID]=0;
	strdel(PlayerInfo[playerid][pUserName],0,MAX_PLAYER_NAME);
	strdel(PlayerInfo[playerid][pPassword],0,MAX_HASH_SIZE);
	PlayerInfo[playerid][pRegistered]=PlayerInfo[playerid][pLoggedIn]=PlayerInfo[playerid][pDj]=PlayerInfo[playerid][pVip]=PlayerInfo[playerid][pRegularPlayer]=false;
	PlayerInfo[playerid][pLoginAttempts]=PlayerInfo[playerid][pRank]=PlayerInfo[playerid][pMoney]=0;
	strdel(PlayerInfo[playerid][pRegisteredDate],0,50);
	strdel(PlayerInfo[playerid][pEmail],0,MAX_EMAIL_SIZE);
	strdel(PlayerInfo[playerid][pCurrent_country],0,56);
	
	PlayerInfo[playerid][pZone]=-1;
	strdel(PlayerInfo[playerid][pZoneName],0,MAX_ZONE_NAME);
	strdel(PlayerInfo[playerid][pExteriorName],0,MAX_ZONE_NAME);
	PlayerInfo[playerid][pInPoliceProperty]=false;
	PlayerInfo[playerid][pInterior]=0;
	
	PlayerInfo[playerid][pClassSelection]=PlayerInfo[playerid][pDistanceInMiles]=false;
	
	PlayerInfo[playerid][pClassID]=-1;
	PlayerInfo[playerid][pSkin]=0;
	for(new i=0; i<MAX_PEDS; i++) PlayerInfo[playerid][pSkinsSelected][i]=0;
	PlayerInfo[playerid][pGender]=MALE;
	for(new i=0; i<3; i++) PlayerInfo[playerid][pGenderChanges][i]=0;
	
	PlayerInfo[playerid][pSpawns]=0;
	PlayerInfo[playerid][pAutoLogin]=PlayerInfo[playerid][pHourlyTimeUpdate]=PlayerInfo[playerid][pShowJoinMessages]=PlayerInfo[playerid][pClassMusic]=PlayerInfo[playerid][pShowGPSDistance]=true;
	
	strdel(PlayerInfo[playerid][pSerial],0,MAX_SERIAL_SIZE);
	strdel(PlayerInfo[playerid][p_AdminNick],0,MAX_PLAYER_NAME);
	strdel(PlayerInfo[playerid][pIP],0,MAX_IP_SIZE);
	PlayerInfo[playerid][pMonthlyActivity]=0;
	PlayerInfo[playerid][pTeam]=-1;
	PlayerInfo[playerid][pMenu]=NO_MENU;
	PlayerInfo[playerid][pAddInteriorType]=-1;
	PlayerInfo[playerid][pEditingInterior]=PlayerInfo[playerid][pAddInteriorCivWeapons]=PlayerInfo[playerid][pAddInteriorCopWeapons]=PlayerInfo[playerid][pAddInteriorWeapons]=PlayerInfo[playerid][pAddingInterior]=PlayerInfo[playerid][pAddInteriorLabels]=false;
	strdel(PlayerInfo[playerid][pAddInteriorZone],0,MAX_ZONE_NAME);
	strdel(PlayerInfo[playerid][pAddInteriorOutZone],0,MAX_ZONE_NAME);
	PlayerInfo[playerid][pAddInteriorOSX]=PlayerInfo[playerid][pAddInteriorOSY]=PlayerInfo[playerid][pAddInteriorOSZ]= PlayerInfo[playerid][pAddInteriorOSA]=PlayerInfo[playerid][pAddInteriorISX]=PlayerInfo[playerid][pAddInteriorISY]=
	PlayerInfo[playerid][pAddInteriorISZ]=PlayerInfo[playerid][pAddInteriorISA]=PlayerInfo[playerid][pAddInteriorOPX]=PlayerInfo[playerid][pAddInteriorOPY]=PlayerInfo[playerid][pAddInteriorOPZ]=PlayerInfo[playerid][pAddInteriorIPX]=PlayerInfo[playerid][pAddInteriorIPY]=PlayerInfo[playerid][pAddInteriorIPZ]=
	PlayerInfo[playerid][pCustomMapMarker][0]=PlayerInfo[playerid][pCustomMapMarker][1]=PlayerInfo[playerid][pCustomMapMarker][2]=0.0;
	PlayerInfo[playerid][pAddInteriorInt]=PlayerInfo[playerid][pAddInteriorVW]=PlayerInfo[playerid][pEditingIntID]=0;
	PlayerInfo[playerid][pMenuTD_BG]=4294967125;
	
	PlayerInfo[playerid][pAddingActor]= PlayerInfo[playerid][pEditingActor]=PlayerInfo[playerid][pAddActorLabelEnable]=false;
	PlayerInfo[playerid][pEditingActorID]=PlayerInfo[playerid][pAddActorType]=PlayerInfo[playerid][pAddActorInt]=PlayerInfo[playerid][pAddActorVW]=PlayerInfo[playerid][pAddActorSkin]=PlayerInfo[playerid][pAddActorAnimationType]=0;
	PlayerInfo[playerid][pAddActorSpawnX]=PlayerInfo[playerid][pAddActorSpawnY]=PlayerInfo[playerid][pAddActorSpawnZ]=PlayerInfo[playerid][pAddActorSpawnA]=0.0;
	strdel(PlayerInfo[playerid][pAddActorLocation],0,MAX_ZONE_NAME);
	
	PlayerInfo[playerid][pHideOutLocation]=PlayerInfo[playerid][pMissionLocation]=-1;
	
	PlayerInfo[playerid][GPS_RouteID]=PlayerInfo[playerid][GPS_Destination]=-1;
	for(new i=0; i<MAX_NODS; i++)
	{
		PlayerInfo[playerid][GPS_CreatedObjects][i]=INVALID_OBJECT_ID;
		PlayerInfo[playerid][GPS_Nods][i]=-1;
		PlayerInfo[playerid][GPS_NodsDistance][i]=0.0;
	}
	for(new i=0; i<100; i++)
	{
		PlayerInfo[playerid][GPS_CreatedMarkers][i]=-1;
	}
	PlayerInfo[playerid][GPS_Calculating]=false;
	PlayerInfo[playerid][GPS_Lenght]=PlayerInfo[playerid][GPS_LostCount]=PlayerInfo[playerid][pTimes_GPSUsed]=PlayerInfo[playerid][GPS_MakerLength]=0;
	PlayerInfo[playerid][GPS_Distance]=0.0;

	// ---- M2 : command / moderation infra ----
	PlayerInfo[playerid][pLastPMFrom]=-1;
	PlayerInfo[playerid][pAdminJailUntil]=PlayerInfo[playerid][pPMFloodTime]=PlayerInfo[playerid][pVCFreezeUntil]=0;
	if(PlayerInfo[playerid][pAdminJailTimer] != -1)
	{
		KillTimer(PlayerInfo[playerid][pAdminJailTimer]);
	}
	PlayerInfo[playerid][pAdminJailTimer]=-1;
	PlayerInfo[playerid][pNoPM]=PlayerInfo[playerid][pShowCommands]=PlayerInfo[playerid][pAdminDuty]=PlayerInfo[playerid][pAdminJailed]=false;
	PlayerInfo[playerid][pInDMZone]=PlayerInfo[playerid][pDebugMode]=PlayerInfo[playerid][pHealthHackFlags]=PlayerInfo[playerid][pAimbotFlags]=false;
	for(new i=0; i<sizeof(pIgnored[]); i++) pIgnored[playerid][i]=0;

	// ---- M3 : wanted-level system ----
	PlayerInfo[playerid][pWanted]=PlayerInfo[playerid][pWantedDecayCounter]=PlayerInfo[playerid][pSpawnProtectUntil]=0;
	PlayerInfo[playerid][pVCByCop]=-1;

	// ---- M3 (stage 2) : cop + jail system ----
	PlayerInfo[playerid][pScore]=PlayerInfo[playerid][pCopRank]=PlayerInfo[playerid][pRefillPoints]=0;
	PlayerInfo[playerid][pRefillCooldown]=PlayerInfo[playerid][pDonutCarry]=PlayerInfo[playerid][pSeriousCrimes]=0;
	PlayerInfo[playerid][pTicketAmount]=PlayerInfo[playerid][pTicketExpiry]=0;
	PlayerInfo[playerid][pTicketByCop]=-1;
	if(PlayerInfo[playerid][pJailTimer] != -1) KillTimer(PlayerInfo[playerid][pJailTimer]);
	PlayerInfo[playerid][pJailTimer]=-1;
	PlayerInfo[playerid][pJailed]=PlayerInfo[playerid][pEscapeChained]=false;
	PlayerInfo[playerid][pJailUntil]=PlayerInfo[playerid][pJailStart]=PlayerInfo[playerid][pJailBail]=0;
	strdel(PlayerInfo[playerid][pJailReason],0,64);
	PlayerInfo[playerid][pBribeTo]=PlayerInfo[playerid][pBribeFrom]=-1;
	PlayerInfo[playerid][pBribeAmount]=0;
	PlayerInfo[playerid][pOnJury]=false;
	PlayerInfo[playerid][pJuryFor]=-1;
	PlayerInfo[playerid][pAppealYes]=PlayerInfo[playerid][pAppealNo]=PlayerInfo[playerid][pAppealJurors]=0;

	// ---- M3 (stage 3) : player crime commands ----
	PlayerInfo[playerid][pSkill]=0;
	PlayerInfo[playerid][pRobCooldown]=PlayerInfo[playerid][pRobbedUntil]=PlayerInfo[playerid][pRapeCooldown]=0;
	PlayerInfo[playerid][pSTD]=PlayerInfo[playerid][pSTDDrainCounter]=PlayerInfo[playerid][pCondoms]=0;
	PlayerInfo[playerid][pChastityBelt]=false;
	PlayerInfo[playerid][pDrugs]=PlayerInfo[playerid][pDrugBuzzStage]=PlayerInfo[playerid][pDrugEffectUntil]=PlayerInfo[playerid][pDrugHealCounter]=0;
	PlayerInfo[playerid][pDrugClickMode]=PlayerInfo[playerid][pDrugOverdose]=PlayerInfo[playerid][pHasDrugBag]=false;
	PlayerInfo[playerid][pJumpkickCooldown]=0;

	// ---- M4 (stage 1) : bank, taxes & insurance ----------------------------
	PlayerInfo[playerid][pBankMoney]=PlayerInfo[playerid][pTaxOwed]=0;
	PlayerInfo[playerid][pLifeInsurance]=PlayerInfo[playerid][pHealthInsExpiry]=PlayerInfo[playerid][pGiveCashCooldown]=0;

	// ---- M4 (stage 2) : lotto, money events, /ad + /advert, holdup ---------
	PlayerInfo[playerid][pLottoNumber]=PlayerInfo[playerid][pAdrenaline]=0;
	PlayerInfo[playerid][pAdvertCooldown]=PlayerInfo[playerid][pHoldupCooldown]=0;
	if(PlayerInfo[playerid][pHoldupTimer] != -1) KillTimer(PlayerInfo[playerid][pHoldupTimer]);
	PlayerInfo[playerid][pHoldupTimer]=-1;
	PlayerInfo[playerid][pHoldingUp]=PlayerInfo[playerid][pInMoneyRush]=false;
	PlayerInfo[playerid][pHoldupStage]=PlayerInfo[playerid][pHoldupTake]=PlayerInfo[playerid][pMoneyRushEarned]=0;
	PlayerInfo[playerid][pHoldupX]=PlayerInfo[playerid][pHoldupY]=PlayerInfo[playerid][pHoldupZ]=0.0;

	// ---- M4 (stage 3) : reusable mission framework -------------------------
	PlayerInfo[playerid][pOnMission]=-1;
	PlayerInfo[playerid][pMissionVehicle]=-1;
	PlayerInfo[playerid][pMissionCheckpoint]=PlayerInfo[playerid][pMissionRouteLen]=PlayerInfo[playerid][pMissionData]=0;
	for(new i=0; i<MISSION_MAX_CHECKPOINTS; i++)
	{
		PlayerInfo[playerid][pMissionRouteX][i]=PlayerInfo[playerid][pMissionRouteY][i]=PlayerInfo[playerid][pMissionRouteZ][i]=0.0;
	}
	// Cooldown stamps are loaded from the DB in loading_data; clear the slots on a
	// fresh connect so a relog into a not-yet-loaded slot cannot read stale data.
	for(new i=0; i<MISSION_COUNT; i++) PlayerInfo[playerid][pMissionCooldown][i]=0;
	// M6 (stage 4) bespoke mission state (session only).
	PlayerInfo[playerid][pMissionDeadline]=PlayerInfo[playerid][pMissionStartHour]=PlayerInfo[playerid][pMissionTargetCd]=0;
	PlayerInfo[playerid][pMissionRaceId]=-1;
	PlayerInfo[playerid][pMissionDestX]=PlayerInfo[playerid][pMissionDestY]=PlayerInfo[playerid][pMissionDestZ]=0.0;

	// ---- M5 (stage 1) : vehicle ownership — CB-simple model ----------------
	PlayerInfo[playerid][pLastVehicle]=INVALID_VEHICLE_ID;

	// ---- M5 (stage 2) : full robbery system + crowbar (design §5) ----------
	// pCrowbarEquipped + pRobberyHistory are loaded from the DB in loading_data;
	// zero them on a fresh connect so a not-yet-loaded slot can't read stale data.
	PlayerInfo[playerid][pCrowbarEquipped]=false;
	PlayerInfo[playerid][pRobberyHistory]=0;
	if(PlayerInfo[playerid][pRobTimer] != -1) KillTimer(PlayerInfo[playerid][pRobTimer]);
	PlayerInfo[playerid][pRobTimer]=-1;
	PlayerInfo[playerid][pRobStage]=ROBSTAGE_NONE;
	PlayerInfo[playerid][pRobKind]=PlayerInfo[playerid][pRobSite]=PlayerInfo[playerid][pRobCrackLeft]=0;
	PlayerInfo[playerid][pRobDeadline]=PlayerInfo[playerid][pRobHideout]=PlayerInfo[playerid][pShopliftCooldown]=0;

	// ---- M5 (stage 3) : housing system (design §9.1/§5.5) ------------------
	// pHouseSpawn is loaded from the DB in loading_data; zero the session slots on
	// a fresh connect so a not-yet-loaded slot can't read stale data.
	PlayerInfo[playerid][pInsideHouse]=-1;
	PlayerInfo[playerid][pOwnedHouseCount]=0;
	PlayerInfo[playerid][pHouseSpawn]=-1;
	PlayerInfo[playerid][pHouseRobCooldown]=0;

	// ---- M5 (stage 4) : kidnap crime (design §4.4) — session-only ----------
	PlayerInfo[playerid][pKidnapVictim]=PlayerInfo[playerid][pKidnappedBy]=-1;
	PlayerInfo[playerid][pKidnapRansom]=PlayerInfo[playerid][pKidnapCooldown]=0;
	PlayerInfo[playerid][pFakeSkillDriver]=false;

	// ---- M5 (stage 4) : Airport Robbery box mechanic (§6.1) — session-only -
	PlayerInfo[playerid][pAirportPhase]=AIRPHASE_NONE;
	PlayerInfo[playerid][pAirportBoxes]=PlayerInfo[playerid][pAirportDeadline]=0;
	PlayerInfo[playerid][pAirportVehicle]=-1;
	PlayerInfo[playerid][pAirportSite]=PlayerInfo[playerid][pBoxCollectCooldown]=0;

	// ---- M6 (stage 1) : skills, fighting styles & clothes (design §9.8/§9.7) ----
	// pFightStyle is loaded from the DB in loading_data; default to NORMAL(4) on a
	// fresh connect so a not-yet-loaded slot can't read stale data. The change
	// cooldown is session-only.
	PlayerInfo[playerid][pFightStyle]=FIGHTSTYLE_DEFAULT;
	PlayerInfo[playerid][pSkillChangeCd]=0;

	// ---- M6 (stage 2) : fishing & farming (design §9.2/§9.3) ---------------
	// Permits/rod/cooler/cooler-aggregate + plant-count are loaded from the DB in
	// loading_data; zero them on a fresh connect so a not-yet-loaded slot can't read
	// stale data. The reel-in timer + session cooldowns are session-only.
	PlayerInfo[playerid][pFishPermits]=PlayerInfo[playerid][pHuntPermits]=PlayerInfo[playerid][pDrugSeeds]=0;
	PlayerInfo[playerid][pHasFishingRod]=PlayerInfo[playerid][pHasFishCooler]=PlayerInfo[playerid][pHasFishSalesPermit]=false;
	PlayerInfo[playerid][pCoolerCount]=PlayerInfo[playerid][pCoolerWeight]=PlayerInfo[playerid][pCoolerValue]=0;
	if(PlayerInfo[playerid][pFishTimer] != -1) KillTimer(PlayerInfo[playerid][pFishTimer]);
	PlayerInfo[playerid][pFishTimer]=-1;
	PlayerInfo[playerid][pFishing]=false;
	PlayerInfo[playerid][pFishX]=PlayerInfo[playerid][pFishY]=PlayerInfo[playerid][pFishZ]=0.0;
	PlayerInfo[playerid][pPlantCount]=PlayerInfo[playerid][pFarmJobCooldown]=PlayerInfo[playerid][pHuntCooldown]=0;

	// ---- M7 : stock market (design §9.4/§10) — holdings loaded async on login ---
	// Zero the cached holdings on a fresh connect so a not-yet-loaded slot cannot
	// read a previous player's holdings; Stocks_OnPlayerLogin fills them from the DB.
	for(new s = 0; s < MAX_STOCKS; s++) PlayerInfo[playerid][pStocks][s] = 0;
	PlayerInfo[playerid][pStocksLoaded] = false;

	// ---- M6 (stage 3) : DM/sniper/duel arenas (design §9.6) — session-only -----
	PlayerInfo[playerid][pArena]=ARENA_NONE;
	PlayerInfo[playerid][pArenaSaved]=false;
	PlayerInfo[playerid][pArenaBackX]=PlayerInfo[playerid][pArenaBackY]=PlayerInfo[playerid][pArenaBackZ]=PlayerInfo[playerid][pArenaBackA]=0.0;
	PlayerInfo[playerid][pArenaBackInt]=PlayerInfo[playerid][pArenaBackVW]=0;
	PlayerInfo[playerid][pArenaBackHP]=PlayerInfo[playerid][pArenaBackArmour]=0.0;
	for(new i=0; i<13; i++) { PlayerInfo[playerid][pArenaBackWeap][i]=0; PlayerInfo[playerid][pArenaBackAmmo][i]=0; }
	PlayerInfo[playerid][pDuelOpponent]=PlayerInfo[playerid][pDuelChallenger]=-1;
	PlayerInfo[playerid][pDuelStake]=0;
	PlayerInfo[playerid][pDuelOfferTo]=PlayerInfo[playerid][pDuelOfferFrom]=-1;
	PlayerInfo[playerid][pDuelOfferStake]=PlayerInfo[playerid][pDuelOfferExpiry]=0;
	PlayerInfo[playerid][pInDuel]=false;

	// ---- M6 (stage 3) : safe / no-crime zones (design §2.5 / §9.10) ------------
	PlayerInfo[playerid][pInSafeZone]=false;

	return 1;
}