/*

	Clothes & accessories system for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 6 (stage 1) - design §9.7 (clothes - crowbar slot goes live) + §5.7
	(the crowbar) + crazybobs-site-full §Clothes & Accessories.

	/clotheswear (/clothes /cw) is the full clothes menu. It replaces the M5 partial
	/clotheswear (which only offered the crowbar buy - that command lived in
	systems/robbery.inc and is superseded here). The menu offers:

	  1. Skins   - buy+wear a civilian skin (writes pSkinsSelected[] = owned, sets the
	               worn skin via pClassID/pSkin; owned skins re-worn free). Ties into
	               the 240-skin pSkinsSelected system (players/peds.inc). Restricted to
	               skins on the player's own TEAM so the cop/civ balance is untouched.
	  2. Crowbar - the $40,000 clothing accessory (§5.7). UNIFIED with the M5 crowbar:
	               buys/removes via systems/robbery.inc's Robbery_BuyCrowbar /
	               Robbery_RemoveCrowbar publics (one implementation, one price, one
	               pCrowbarEquipped flag). /buycrowbar and /crowbar are also served here.
	  3. Drug Bag- a $15,000 accessory that doubles the Drug Dealer carry cap
	               (pHasDrugBag -> 5000g vs 2500g, §4.3/§16 #2).

	CB's six clothing chains (ZIP/Binco/ProLaps/Sub Urban/Victim/Didier Sachs) are
	represented as clothes-shop MARKERS (proximity gate) rather than 240 individual
	branded garments - the skin buy IS the outfit change (design §9.7: "buying an
	outfit writes pSkinsSelected"). The crowbar vendor proximity that M5 deferred goes
	live here too (Truck Stop / Zero's RC Shop are folded into the shop markers).

	Public API:
		Clothes_Init()                                   : create clothes-shop markers/icons (OnGameModeInit)
		Clothes_Cleanup()                                : destroy them (OnGameModeExit)
		Clothes_OnDialogResponse(playerid, dialogid, response, listitem) : menu resolution

	Commands:
		/clotheswear (/clothes /cw)  (on-foot at a clothes shop)  the clothes + accessories menu
		/buycrowbar                  (civ, at a shop)             direct crowbar buy ($40,000, §5.7)
		/crowbar                     (civ)                        put the crowbar away (§5.7)

	Functions Being Called:
		bank:     Bank_TakeCash
		robbery:  Robbery_BuyCrowbar, Robbery_RemoveCrowbar (crowbar unification, §5.7)
		drugs:    Drug_SavePlayer (persists HasDrugBag)
		peds:     PedsInfo / pSkinsSelected (the 240-skin owned/worn system)
*/

//==============================================================================
//	Clothes-shop markers (the six CB chains + crowbar vendors) - §9.7/§5.7
//==============================================================================

// Clothes shop / crowbar-vendor locations. The six CB chains (ZIP/Binco/ProLaps/
// Sub Urban/Victim/Didier Sachs) plus the crowbar vendors (Truck Stop / Zero's RC
// Shop, §5.7) all resolve to a single "you're at a shop" proximity gate. Only LS
// coords are reachable while CITY_ == LS; SF/LV rows are kept for later.
static const Float:g_ClothesShops[][3] =
{
	{ 207.741,   -110.240, 1005.132 },	// Binco (Los Santos) - interior handled by proximity, world-0 doorway
	{ 161.393,   -93.087,  1001.804 },	// ProLaps (Los Santos)
	{ 498.897,   -76.514,  998.755 },	// Victim (Los Santos)
	{ 466.360,   -80.129,  998.777 },	// Sub Urban (Los Santos)
	{ 5.311,     -30.578,  1003.549 },	// ZIP (Los Santos)
	{ 2543.000,  2385.000, 10.500 },	// Truck Stop crowbar vendor (§5.7)
	{ -2427.000, 1021.000, 50.400 }		// Zero's RC Shop, Garcia SF crowbar vendor (§5.7)
};

static STREAMER_TAG_MAP_ICON:g_ClothesIcon[sizeof(g_ClothesShops)];

//==============================================================================
//	Setup / teardown
//==============================================================================

FUNCTION Clothes_Init()
{
	for(new i = 0; i < sizeof(g_ClothesShops); i++)
	{
		// T-shirt map icon (model 45) so players can find a shop (§9.7).
		g_ClothesIcon[i] = CreateDynamicMapIcon(g_ClothesShops[i][0], g_ClothesShops[i][1], g_ClothesShops[i][2], 45, 0, -1, -1, -1, 200.0, MAPICON_LOCAL);
	}
	return 1;
}

FUNCTION Clothes_Cleanup()
{
	for(new i = 0; i < sizeof(g_ClothesShops); i++)
		if(DestroyDynamicMapIcon(g_ClothesIcon[i])) {}
	return 1;
}

// Is the player standing at a clothes shop / crowbar vendor? Returns 1/0. If NO shop
// is close (interiors are entered via the enex system so the door coords may be an
// interior), we still allow the menu from anywhere with a note - clothes shops are
// cosmetic, and the crowbar/drug-bag purchases are gated by price, not location.
static Clothes_AtShop(playerid)
{
	for(new i = 0; i < sizeof(g_ClothesShops); i++)
		if(IsPlayerInRangeOfPoint(playerid, CLOTHES_SHOP_RANGE, g_ClothesShops[i][0], g_ClothesShops[i][1], g_ClothesShops[i][2]))
			return 1;
	return 0;
}

//==============================================================================
//	Skins submenu - buy + wear from the 240-skin pSkinsSelected system (§9.7)
//==============================================================================

// Build + show the skins list for the player's TEAM. Owned skins (pSkinsSelected)
// are free to re-wear and flagged green; unowned skins cost CLOTHES_SKIN_PRICE to
// buy+wear. Listing is same-team only so a purchase never crosses the cop/civ line.
static Clothes_ShowSkins(playerid)
{
	new list[3072], row[64], count = 0;
	new team = PlayerInfo[playerid][pTeam];
	for(new i = 0; i < MAX_PEDS; i++)
	{
		if(PedsInfo[i][PedTeam] != team) continue;
		new bool:owned = (PlayerInfo[playerid][pSkinsSelected][i] != 0);
		new bool:worn  = (PlayerInfo[playerid][pClassID] == i);
		if(worn)       format(row, sizeof(row), ""COL_GREEN"%s (worn)\n", PedsInfo[i][PedName]);
		else if(owned) format(row, sizeof(row), ""COL_WHITE"%s "COL_GREEN"(owned)\n", PedsInfo[i][PedName]);
		else           format(row, sizeof(row), ""COL_WHITE"%s "COL_YELLOW"$%d\n", PedsInfo[i][PedName], CLOTHES_SKIN_PRICE);
		if(strlen(list) + strlen(row) >= sizeof(list) - 1) break;	// keep the dialog buffer safe
		strcat(list, row, sizeof(list));
		count++;
	}
	if(count == 0) return SendClientMessage(playerid, COLOR_ERROR, "There are no outfits available for your team.");
	ShowPlayerDialog(playerid, CLOTHES_SKINS_DIALOG, DIALOG_STYLE_LIST, ""COL_WHITE"Clothes - Outfits (your team)", list, "Wear", "Back");
	return 1;
}

// Resolve which PedsInfo index the Nth same-team row maps to (the skins dialog only
// lists same-team peds, so listitem is a position within that filtered set).
static Clothes_SkinIndexForRow(playerid, listitem)
{
	new team = PlayerInfo[playerid][pTeam], seen = 0;
	for(new i = 0; i < MAX_PEDS; i++)
	{
		if(PedsInfo[i][PedTeam] != team) continue;
		if(seen == listitem) return i;
		seen++;
	}
	return -1;
}

// Persist the owned-skins bitfield back to LSplayers.SkinsSelected as the same
// '|'-delimited packed column loading_data reads (one int per ped, 0/1 owned). Called
// on a skin PURCHASE so a bought outfit survives relogs (§9.7 "purchase = unlock").
static Clothes_SaveSkins(playerid)
{
	if(!PlayerInfo[playerid][pLoggedIn]) return 0;
	new packed[1555], part[8];	// MAX_PEDS ints, up to "1|" each - mirrors the 1555-char load buffer
	for(new i = 0; i < MAX_PEDS; i++)
	{
		format(part, sizeof(part), (i == 0) ? "%d" : "|%d", PlayerInfo[playerid][pSkinsSelected][i] ? 1 : 0);
		strcat(packed, part, sizeof(packed));
	}
	new query[1700];
	mysql_format(g_SQL, query, sizeof(query), "UPDATE "TABLE_PLAYERS_C" SET SkinsSelected='%e' WHERE aID=%d LIMIT 1", packed, PlayerInfo[playerid][pID]);
	mysql_pquery(g_SQL, query);
	return 1;
}

// Wear a skin the player already owns (or just bought): set the worn class/skin,
// keep the team, apply it live, and persist Skin+ClassID (mirrors the spawn UPDATE).
static Clothes_WearSkin(playerid, ped)
{
	PlayerInfo[playerid][pClassID] = ped;
	PlayerInfo[playerid][pSkin] = PedsInfo[ped][PedSkinID];
	PlayerInfo[playerid][pTeam] = PedsInfo[ped][PedTeam];
	SetPlayerSkin(playerid, PedsInfo[ped][PedSkinID]);

	new string[128];
	mysql_format(g_SQL, string, sizeof(string), "UPDATE "TABLE_PLAYERS_C" SET Skin=%d, ClassID=%d WHERE aID=%d LIMIT 1",
		PlayerInfo[playerid][pSkin], PlayerInfo[playerid][pClassID], PlayerInfo[playerid][pID]);
	mysql_pquery(g_SQL, string);

	new msg[128];
	format(msg, sizeof(msg), ""COL_GREEN"* You are now wearing "COL_WHITE"%s"COL_GREEN"."COL_WHITE, PedsInfo[ped][PedName]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}

//==============================================================================
//	Top menu - /clotheswear (/clothes /cw) (§9.7)
//==============================================================================

// Row order MUST match the switch in Clothes_OnDialogResponse (top menu case).
static Clothes_ShowMenu(playerid)
{
	new list[512];
	format(list, sizeof(list),
		""COL_WHITE"Outfits\tbuy / wear a skin\n"\
		""COL_WHITE"Crowbar\t%s\n"\
		""COL_WHITE"Drug Bag\t%s",
		PlayerInfo[playerid][pCrowbarEquipped] ? (""COL_GREEN"equipped - remove") : (""COL_YELLOW"$40,000"),
		PlayerInfo[playerid][pHasDrugBag] ? (""COL_GREEN"owned") : (""COL_YELLOW"$15,000"));
	ShowPlayerDialog(playerid, CLOTHES_MENU_DIALOG, DIALOG_STYLE_TABLIST, ""COL_WHITE"Clothes & Accessories\t ", list, "Select", "Cancel");
	return 1;
}

CMD:clotheswear(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(IsPlayerInAnyVehicle(playerid)) return SendClientMessage(playerid, COLOR_ERROR, "Get out of your vehicle to browse the clothes shop.");
	// Proximity is a soft gate - if the player is not at a shop marker we still open
	// the menu but note it (clothes are cosmetic; the priced items self-gate) (§9.7).
	if(!Clothes_AtShop(playerid))
		SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_YELLOW"* Tip: "COL_WHITE"visit a clothes shop or crowbar vendor (see the map icons) to try things on.");
	return Clothes_ShowMenu(playerid);
}
alias:clotheswear("clothes", "cw")

// /buycrowbar - direct crowbar buy ($40,000, §5.7). Now gated to a shop/vendor (the
// M5 vendor-proximity that was deferred goes live here). Unified with the clothes
// menu + robbery.inc's Robbery_BuyCrowbar (one buy path).
CMD:buycrowbar(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(!Clothes_AtShop(playerid))
		return SendClientMessage(playerid, COLOR_ERROR, "Crowbars are sold at clothes shops, Truck Stops and Zero's RC Shop - stand at a vendor (map icons).");
	return Robbery_BuyCrowbar(playerid);
}

// /crowbar - put the crowbar away (§5.7). Delegates to robbery.inc (the effect owner).
CMD:crowbar(playerid, params[])
{
	#pragma unused params
	return Robbery_RemoveCrowbar(playerid);
}

//==============================================================================
//	Drug bag accessory - doubles the Drug Dealer carry cap (§4.3/§16 #2)
//==============================================================================

static Clothes_BuyDrugBag(playerid)
{
	if(PlayerInfo[playerid][pHasDrugBag]) return SendClientMessage(playerid, COLOR_ERROR, "You already own a drug bag.");
	if(PlayerInfo[playerid][pMoney] < DRUGBAG_PRICE)
	{
		new err[128];
		format(err, sizeof(err), "A drug bag costs "COL_GREEN"$%d"COL_WHITE" cash - you cannot afford it.", DRUGBAG_PRICE);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	if(!Bank_TakeCash(playerid, DRUGBAG_PRICE)) return SendClientMessage(playerid, COLOR_ERROR, "Payment failed.");
	PlayerInfo[playerid][pHasDrugBag] = true;
	Drug_SavePlayer(playerid);	// persists HasDrugBag (+Skill/Drugs) - no competing query

	new msg[176];
	if(PlayerInfo[playerid][pSkill] == SKILL_DRUGDEALER)
		format(msg, sizeof(msg), ""COL_GREEN"* Drug bag bought "COL_WHITE"($%d)"COL_GREEN". "COL_WHITE"Your Drug Dealer carry cap is now "COL_GREEN"%dg"COL_WHITE".", DRUGBAG_PRICE, DRUG_DEALER_WITH_BAG);
	else
		format(msg, sizeof(msg), ""COL_GREEN"* Drug bag bought "COL_WHITE"($%d)"COL_GREEN". "COL_WHITE"It doubles your carry cap once you take the "COL_YELLOW"Drug Dealer"COL_WHITE" skill (/skill).", DRUGBAG_PRICE);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}

//==============================================================================
//	Dialog resolution (routed from OnDialogResponse in CnR.pwn)
//==============================================================================

FUNCTION Clothes_OnDialogResponse(playerid, dialogid, response, listitem)
{
	switch(dialogid)
	{
		case CLOTHES_MENU_DIALOG:
		{
			if(!response) return 1;	// Cancel
			switch(listitem)
			{
				case 0: Clothes_ShowSkins(playerid);					// Outfits
				case 1:												// Crowbar (buy or remove - unified §5.7)
				{
					if(PlayerInfo[playerid][pCrowbarEquipped]) Robbery_RemoveCrowbar(playerid);
					else Robbery_BuyCrowbar(playerid);
				}
				case 2: Clothes_BuyDrugBag(playerid);					// Drug Bag
			}
			return 1;
		}
		case CLOTHES_SKINS_DIALOG:
		{
			if(!response) { Clothes_ShowMenu(playerid); return 1; }	// Back -> top menu
			new ped = Clothes_SkinIndexForRow(playerid, listitem);
			if(ped == -1) return 1;

			// Already owned -> just wear it (free).
			if(PlayerInfo[playerid][pSkinsSelected][ped] != 0)
				return Clothes_WearSkin(playerid, ped);

			// Not owned -> buy it, mark it owned (pSkinsSelected), then wear it.
			if(PlayerInfo[playerid][pMoney] < CLOTHES_SKIN_PRICE)
			{
				new err[128];
				format(err, sizeof(err), "That outfit costs "COL_GREEN"$%d"COL_WHITE" cash - you cannot afford it.", CLOTHES_SKIN_PRICE);
				return SendClientMessage(playerid, COLOR_ERROR, err);
			}
			if(!Bank_TakeCash(playerid, CLOTHES_SKIN_PRICE)) return SendClientMessage(playerid, COLOR_ERROR, "Payment failed.");
			PlayerInfo[playerid][pSkinsSelected][ped] = 1;	// unlock (owned) - §9.7 "purchase = unlock+equip"
			Clothes_SaveSkins(playerid);					// persist the new ownership
			Clothes_WearSkin(playerid, ped);
			return 1;
		}
	}
	return 0;
}
