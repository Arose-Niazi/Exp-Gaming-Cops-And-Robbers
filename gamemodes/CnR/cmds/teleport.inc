/*

	Teleport commands (+ DM-zone flags) for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 2 — design §11.4 / §5 / §7.6
	Last Updated: -----

	Credits:

	Functions:
		DoTeleport(playerid, tpid)         : shared teleport routine
		StripParachute(playerid)           : rebuild weapons without id 46
		ClearDMZone(playerid)              : leave a DM zone (called on death/spawn/other tp)

	Commands:
		/drylake (/d)         — Dry Lake (DM zone)
		/sfairport (/sfa)     — SF Airport
		/bayside (/bs)        — Bayside
		/lossantosdm (/lsadm) — LS DM zone (DM zone, anti-parachute)
		/palominocreek (/pc)  — Palomino Creek

	Functions Being Called:

*/

enum e_Teleport
{
	Float:tpX,
	Float:tpY,
	Float:tpZ,
	Float:tpA,
	tpInt,
	tpVW,
	bool:tpDMZone,
	tpName[32]
};

// Fixed teleport table — sensible real SA coordinates.
new const TeleportData[][e_Teleport] =
{
	// X            Y            Z          A       Int VW  DMzone  Name
	{ -746.6155,   1591.0000,   26.7326,  0.0,  0,  0, true,  "Dry Lake" },
	{ -1236.2380, -22.9873,    14.1484,  0.0,  0,  0, false, "SF Airport" },
	{ -175.7648,   2400.2451,  33.1339,  0.0,  0,  0, false, "Bayside" },
	{  1544.8907, -1353.6543,  329.4738, 0.0,  0,  0, true,  "Los Santos DM" },
	{  2312.5334,  60.4179,    26.4766,  0.0,  0,  0, false, "Palomino Creek" }
};

// Anti-parachute: strip weapon 46 by rebuilding the player's arsenal without it.
StripParachute(playerid)
{
	new weapons[13][2], hadOther = 0;
	for(new slot = 0; slot < 13; slot++)
		GetPlayerWeaponData(playerid, slot, weapons[slot][0], weapons[slot][1]);
	for(new slot = 0; slot < 13; slot++)
		if(weapons[slot][0] != 0 && weapons[slot][0] != PARACHUTE_WEAPON && weapons[slot][1] != 0) hadOther = 1;
	#pragma unused hadOther
	ResetPlayerWeapons(playerid);
	for(new slot = 0; slot < 13; slot++)
	{
		if(weapons[slot][0] == 0 || weapons[slot][0] == PARACHUTE_WEAPON) continue;
		GivePlayerWeapon(playerid, weapons[slot][0], weapons[slot][1]);
	}
	return 1;
}

ClearDMZone(playerid)
{
	PlayerInfo[playerid][pInDMZone] = false;
	return 1;
}

DoTeleport(playerid, tpid)
{
	if(!PlayerInfo[playerid][pSpawned])
		return SendClientMessage(playerid, COLOR_ERROR, "You must be spawned to teleport.");
	if(PlayerInfo[playerid][pAdminJailed])
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot teleport while admin-jailed.");
	// Blocked while a cop holds visual contact (pursuit) — set by /visualcontact in M3.
	if(PlayerInfo[playerid][pVCFreezeUntil] != 0 && gettime() < PlayerInfo[playerid][pVCFreezeUntil])
		return SendClientMessage(playerid, COLOR_ERROR, "You cannot teleport while a cop has visual contact on you.");

	// Leaving any previous DM zone.
	ClearDMZone(playerid);

	SetPlayerPos(playerid, TeleportData[tpid][tpX], TeleportData[tpid][tpY], TeleportData[tpid][tpZ]);
	SetPlayerFacingAngle(playerid, TeleportData[tpid][tpA]);
	SetPlayerInterior(playerid, TeleportData[tpid][tpInt]);
	SetPlayerVirtualWorld(playerid, TeleportData[tpid][tpVW]);
	SetCameraBehindPlayer(playerid);

	new string[96];
	if(TeleportData[tpid][tpDMZone])
	{
		PlayerInfo[playerid][pInDMZone] = true;
		StripParachute(playerid);
		format(string, sizeof(string), "You teleported to %s. "COL_RED"(DM zone — no wanted/arrest, parachute disabled)", TeleportData[tpid][tpName]);
	}
	else format(string, sizeof(string), "You teleported to %s.", TeleportData[tpid][tpName]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	return 1;
}

CMD:drylake(playerid)      return DoTeleport(playerid, 0);
alias:drylake("d")

CMD:sfairport(playerid)    return DoTeleport(playerid, 1);
alias:sfairport("sfa")

CMD:bayside(playerid)      return DoTeleport(playerid, 2);
alias:bayside("bs")

CMD:lossantosdm(playerid)  return DoTeleport(playerid, 3);
alias:lossantosdm("lsadm")

CMD:palominocreek(playerid) return DoTeleport(playerid, 4);
alias:palominocreek("pc")
