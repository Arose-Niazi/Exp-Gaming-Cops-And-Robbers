/*

	Vehicle ownership (CB-simple model) for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 5 (stage 1) — design §9.12 (Vehicle ownership — CB-simple model).

	Deliberately minimal, exactly as CB:CNR did it — transient, per-vehicle runtime
	state, NO database persistence. Supersedes the M3 /lock stub that used to live
	in cmds/crime.inc (gVehicleLocked + CMD:lock/CMD:unlock were deleted there).

	Model — "your last-driven vehicle":
		- Every vehicle remembers its last driver (gVehLastDriver) and a lock flag
		  (gVehLocked); every player remembers the vehicle they last drove as driver
		  (pLastVehicle). Ownership follows the wheel: whoever takes the driver seat
		  becomes the last driver, and the previous owner loses /lock rights.
		- /lock (/lk) and /unlock (/ulk) act on the caller's pLastVehicle and MUST be
		  run on foot. A locked vehicle blocks entry for everyone but the owner
		  (SetVehicleParamsForPlayer door-lock + eject on a sneaked entry). The owner
		  driving into their own locked car auto-unlocks it.

	Theft = crime (GTA, §9.12):
		- Taking the DRIVER seat of a vehicle that already has a last driver who is
		  not you (and you are outside the re-steal grace) is grand theft auto:
		  GivePlayerWanted(WANTED_GTA, "Grand Theft Auto").
		- Pristine exception: a vehicle nobody has ever driven (last driver INVALID)
		  is free to take — you just become its owner, no crime.
		- Re-steal grace: retaking your own last-driven vehicle (or one you drove
		  within VEH_RESTEAL_GRACE) is not a crime.
		- Cops commandeering vehicles raise no GTA wanted.
		- This GTA path is for UNOCCUPIED (parked/owned) vehicles only. Jacking an
		  OCCUPIED vehicle out from under another player is the M3 car-jack path
		  (Wanted_OnPlayerStateChange, WANTED_CARJACK +3) — the two are mutually
		  exclusive (see Veh_IsTheft) so a single act never double-punishes.

	Reset: OnVehicleSpawn / OnVehicleDeath clear a vehicle's last driver + lock (a
	respawned car is pristine again); OnPlayerDisconnect clears the leaver's
	ownership marks (their last-driver entries → INVALID, their locked cars unlock)
	and pLastVehicle.

	Public API:
		Veh_IsTheft(playerid, vehicleid)          : bool — would taking this vehicle's
		                                            wheel be grand theft auto right now?
		                                            (reused by the Vehicle Theft mission, §6.19)
		Veh_IsOwnedBy(playerid, vehicleid)        : bool — is this player the vehicle's last driver?
		Veh_ClearVehicle(vehicleid)               : reset a vehicle to pristine (no owner, unlocked)
		Veh_OnPlayerStateChange(playerid, ns, os) : hook from OnPlayerStateChange (CnR.pwn)
		Veh_OnVehicleReset(vehicleid)             : hook from OnVehicleSpawn/OnVehicleDeath
		Veh_OnDisconnect(playerid)                : hook from OnPlayerDisconnect
		Veh_RefreshLockForPlayer(vehicleid, forid): push door-lock params for one player

	Functions Being Called:
		misc:    IsCop
		wanted:  GivePlayerWanted
*/

//==============================================================================
//	Runtime state (session only — no DB, per design §9.12)
//==============================================================================

// Per-vehicle: the last player to sit in the driver seat (INVALID_PLAYER_ID = a
// pristine / never-driven vehicle), whether it is locked, and the gettime() stamp
// of when that last driver took the wheel (drives the re-steal grace window).
static gVehLastDriver[MAX_VEHICLES + 1];
static bool:gVehLocked[MAX_VEHICLES + 1];
static gVehLastDriveTime[MAX_VEHICLES + 1];

// Called once at gamemode start so a fresh session has no stale ownership from a
// previous run reusing the same static memory image.
FUNCTION Veh_Init()
{
	for(new v = 0; v <= MAX_VEHICLES; v++)
	{
		gVehLastDriver[v] = INVALID_PLAYER_ID;
		gVehLocked[v] = false;
		gVehLastDriveTime[v] = 0;
	}
	return 1;
}

//==============================================================================
//	Internal helpers
//==============================================================================

// A usable vehicle id for our arrays: a real (non-zero) id inside MAX_VEHICLES.
static bool:Veh_ValidId(vehicleid)
	return (vehicleid >= 1 && vehicleid <= MAX_VEHICLES && vehicleid != INVALID_VEHICLE_ID);

// Push the door-lock state of `vehicleid` to a single player: the owner (its last
// driver) always keeps access; everyone else is locked out when gVehLocked is set.
// SetVehicleParamsForPlayer(veh, forid, objective, doors) — doors=1 locks.
FUNCTION Veh_RefreshLockForPlayer(vehicleid, forid)
{
	if(!Veh_ValidId(vehicleid)) return 0;
	if(forid == INVALID_PLAYER_ID || !IsPlayerConnected(forid) || IsPlayerNPC(forid)) return 0;
	new bool:lockOut = (gVehLocked[vehicleid] && gVehLastDriver[vehicleid] != forid);
	SetVehicleParamsForPlayer(vehicleid, forid, 0, lockOut ? 1 : 0);
	return 1;
}

// Push the lock state to every connected human (used when a lock toggles or the
// owner changes so the client-side doors reflect reality for everyone at once).
static Veh_RefreshLockForAll(vehicleid)
{
	if(!Veh_ValidId(vehicleid)) return 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		Veh_RefreshLockForPlayer(vehicleid, i);
	}
	return 1;
}

//==============================================================================
//	Public API
//==============================================================================

// Is `playerid` the current owner (last driver) of `vehicleid`?
FUNCTION Veh_IsOwnedBy(playerid, vehicleid)
{
	if(!Veh_ValidId(vehicleid) || playerid == INVALID_PLAYER_ID) return 0;
	return (gVehLastDriver[vehicleid] == playerid);
}

// Would `playerid` taking the driver seat of `vehicleid` be grand theft auto right
// now? True only for an UNOCCUPIED, already-owned, not-yours vehicle outside the
// re-steal grace, when the taker is not a cop. False for a pristine (never-driven)
// vehicle, your own vehicle, a recent re-steal, or a cop commandeering.
// Reused by the Vehicle Theft mission (§6.19) to decide when a lift counts as a
// crime. Note: an OCCUPIED vehicle is the M3 car-jack path, not this — this
// returns false when another player is still aboard so the two never double-fire.
FUNCTION Veh_IsTheft(playerid, vehicleid)
{
	if(playerid == INVALID_PLAYER_ID || !Veh_ValidId(vehicleid)) return 0;
	if(IsCop(playerid)) return 0;								// cops commandeer, no GTA

	new owner = gVehLastDriver[vehicleid];
	if(owner == INVALID_PLAYER_ID) return 0;					// pristine — free to take
	if(owner == playerid) return 0;								// it is already yours

	// Re-steal grace: you drove it within the window → not a crime to retake it.
	if(gettime() - gVehLastDriveTime[vehicleid] <= VEH_RESTEAL_GRACE) return 0;

	// Occupied vehicles are the M3 car-jack path (WANTED_CARJACK), not GTA — keep
	// the two mutually exclusive so a single act is never punished twice.
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(i == playerid || !IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(GetPlayerVehicleID(i) == vehicleid) return 0;		// someone else aboard → jack path
	}
	return 1;
}

// Reset a vehicle to pristine: no owner, unlocked, grace stamp cleared, and the
// (now cleared) lock pushed to everyone so any lingering client-side lock lifts.
FUNCTION Veh_ClearVehicle(vehicleid)
{
	if(!Veh_ValidId(vehicleid)) return 0;
	new wasLocked = gVehLocked[vehicleid];
	gVehLastDriver[vehicleid] = INVALID_PLAYER_ID;
	gVehLocked[vehicleid] = false;
	gVehLastDriveTime[vehicleid] = 0;
	if(wasLocked) Veh_RefreshLockForAll(vehicleid);
	return 1;
}

//==============================================================================
//	OnPlayerStateChange — the core of the ownership / theft / lock model
//==============================================================================

// Called from OnPlayerStateChange (CnR.pwn) AFTER the M3 car-jack detector so an
// occupied-vehicle jack is scored by wanted.inc and this handler stays on the
// UNOCCUPIED / ownership path only.
FUNCTION Veh_OnPlayerStateChange(playerid, newstate, oldstate)
{
	// --- Entering a seat: enforce a lock against non-owners (any seat, §9.12). ---
	if(newstate == PLAYER_STATE_DRIVER || newstate == PLAYER_STATE_PASSENGER)
	{
		new vehicleid = GetPlayerVehicleID(playerid);
		if(Veh_ValidId(vehicleid) && gVehLocked[vehicleid] && gVehLastDriver[vehicleid] != playerid)
		{
			// The owner entering the DRIVER seat auto-unlocks (handled below); any
			// other entry into a locked car by a non-owner is ejected.
			if(!(newstate == PLAYER_STATE_DRIVER && gVehLastDriver[vehicleid] == playerid))
			{
				RemovePlayerFromVehicle(playerid);
				SendClientMessage(playerid, COLOR_ERROR, "This vehicle is locked.");
				return 1;
			}
		}
	}

	// The rest of the model only reacts to taking the DRIVER seat.
	if(newstate != PLAYER_STATE_DRIVER)
	{
		#pragma unused oldstate
		return 0;
	}

	new vehicleid = GetPlayerVehicleID(playerid);
	if(!Veh_ValidId(vehicleid)) return 0;

	// --- Owner into driver seat of their locked vehicle → auto-unlock (§9.12). ---
	if(gVehLastDriver[vehicleid] == playerid && gVehLocked[vehicleid])
	{
		gVehLocked[vehicleid] = false;
		Veh_RefreshLockForAll(vehicleid);
		SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_GREEN"* Your vehicle unlocked "COL_WHITE"as you got in.");
	}

	// --- Theft = crime: evaluate BEFORE reassigning ownership (§9.12). ---
	// Veh_IsTheft already excludes cops, pristine cars, your own car, re-steals
	// within grace, and occupied cars (the M3 jack path), so it fires only for an
	// unoccupied, owned, not-yours vehicle — the pure GTA case.
	if(Veh_IsTheft(playerid, vehicleid))
		GivePlayerWanted(playerid, WANTED_GTA, "Grand Theft Auto");

	// --- Ownership follows the wheel: this player is now the last driver. The old
	// owner thereby loses /lock rights (they are no longer the last driver). ---
	gVehLastDriver[vehicleid] = playerid;
	gVehLastDriveTime[vehicleid] = gettime();
	PlayerInfo[playerid][pLastVehicle] = vehicleid;

	// Refresh the doors so the new owner's access (and everyone else's lockout, if
	// still locked) is correct after the ownership change.
	Veh_RefreshLockForAll(vehicleid);
	return 1;
}

//==============================================================================
//	Reset hooks
//==============================================================================

// OnVehicleSpawn / OnVehicleDeath → the vehicle is respawned/destroyed, so it is
// pristine again (nobody owns it, no lock). Called from CnR.pwn.
FUNCTION Veh_OnVehicleReset(vehicleid)
{
	Veh_ClearVehicle(vehicleid);
	return 1;
}

// OnPlayerDisconnect → drop the leaver's ownership marks: any vehicle they last
// drove becomes pristine (and unlocks if it was locked, so it is not left sealed
// against everyone), and their pLastVehicle is cleared. Called from CnR.pwn.
FUNCTION Veh_OnDisconnect(playerid)
{
	for(new v = 1; v <= MAX_VEHICLES; v++)
	{
		if(gVehLastDriver[v] != playerid) continue;
		gVehLastDriver[v] = INVALID_PLAYER_ID;
		gVehLastDriveTime[v] = 0;
		if(gVehLocked[v])
		{
			gVehLocked[v] = false;
			Veh_RefreshLockForAll(v);
		}
	}
	PlayerInfo[playerid][pLastVehicle] = INVALID_VEHICLE_ID;
	return 1;
}

// OnPlayerConnect (or on login) → push every locked vehicle's door state to the
// freshly-connected player so a car locked before they joined reads as locked for
// them. Called from CnR.pwn OnPlayerConnect.
FUNCTION Veh_OnConnect(playerid)
{
	if(playerid == INVALID_PLAYER_ID || !IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) return 0;
	for(new v = 1; v <= MAX_VEHICLES; v++)
	{
		if(gVehLocked[v]) Veh_RefreshLockForPlayer(v, playerid);
	}
	return 1;
}

//==============================================================================
//	Commands — /lock (/lk) + /unlock (/ulk)  (design §9.12)
//==============================================================================

// Shared body for /lock and /unlock: act on the caller's pLastVehicle, on foot
// only, and only if they still own it (are still its last driver). Returns 1.
static Veh_SetLock(playerid, bool:locked)
{
	// Must be on foot — "can't run commands from inside a vehicle" (§9.12).
	if(IsPlayerInAnyVehicle(playerid))
		return SendClientMessage(playerid, COLOR_ERROR, "You must be on foot to lock or unlock your vehicle.");

	new vehicleid = PlayerInfo[playerid][pLastVehicle];
	if(!Veh_ValidId(vehicleid))
		return SendClientMessage(playerid, COLOR_ERROR, "You have not driven a vehicle to lock. Drive one first.");

	// Ownership follows the wheel — if someone else has since driven it, you have
	// lost the right to lock it (you are no longer its last driver).
	if(gVehLastDriver[vehicleid] != playerid)
		return SendClientMessage(playerid, COLOR_ERROR, "That is no longer your vehicle — someone else has driven it.");

	if(gVehLocked[vehicleid] == locked)
		return SendClientMessage(playerid, COLOR_SERVER_INFO, locked ?
			("Your vehicle is already locked.") : ("Your vehicle is already unlocked."));

	gVehLocked[vehicleid] = locked;
	Veh_RefreshLockForAll(vehicleid);

	SendClientMessage(playerid, COLOR_SERVER_INFO, locked ?
		(""COL_GREEN"* Vehicle locked."COL_WHITE" Nobody but you can enter it.") :
		(""COL_GREEN"* Vehicle unlocked."COL_WHITE" Others can enter it again."));
	return 1;
}

CMD:lock(playerid, params[])
{
	#pragma unused params
	return Veh_SetLock(playerid, true);
}
alias:lock("lk")

CMD:unlock(playerid, params[])
{
	#pragma unused params
	return Veh_SetLock(playerid, false);
}
alias:unlock("ulk")
