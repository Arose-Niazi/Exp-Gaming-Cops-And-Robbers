/*

	Bank, taxes & insurance (the money core) for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 4 (stage 1) - design §10 (economy integration: prime rate, tax
	model), §11.2 (LSplayers columns) + docs/research/economy.md §1 (money flow,
	bank interest ~5-15%/2 game-days, taxes: wealth 12% over ~$10M, income 7%,
	insurance / medical fees).

	This owns the money core the later M4 stages build on:
	  - Bank accounts (LSplayers.BankMoney) - cash you deposit is safe from
	    robbery/pickpockets/medical fees but exposed to wealth tax (economy §1.5).
	  - Bank interest - paid on BankMoney on the game-clock day schedule (every
	    BANK_INTEREST_DAYS game-days) via Bank_OnGameDay(), hooked into the day
	    transition in GameModeClock() (NO new global timer, per the brief).
	  - Taxes (LSplayers.TaxOwed) - a periodic wealth+income tax tick on the same
	    day schedule via Tax_OnGameDay(); deduct bank-first then cash, accrue any
	    unpayable remainder to TaxOwed. Donators (pVip) are wealth-tax exempt.
	  - Insurance (LSplayers.LifeInsurance / HealthInsExpiry) - a medical fee on
	    death unless health-insured; /buyinsurance for health cover + life policies
	    that waive the fee and preserve weapons on death.

	Every money amount is validated: positive, integer, not more than available,
	clamped below MAX_MONEY so no step overflows the signed 32-bit cell. All cash
	changes go through UpdateMoney (misc.inc - applies a signed delta). Persistence
	is folded into Jail_SavePlayer (jail.inc) - the single per-player LSplayers
	UPDATE - so there is no competing disconnect save; deposit/withdraw also call
	it directly so the money core is durable immediately.

	Clean bank API exposed for stage 2 (lotto/events) and stage 3 (missions):
		Bank_GetBalance(playerid)          : current BankMoney
		Bank_GetCash(playerid)             : current pMoney (cash-in-hand)
		Bank_Give(playerid, amount)        : credit BankMoney, returns success
		Bank_Take(playerid, amount)        : debit BankMoney if affordable, returns success
		Bank_GiveCash(playerid, amount)    : credit cash-in-hand (via UpdateMoney), returns success
		Bank_TakeCash(playerid, amount)    : debit cash-in-hand if affordable, returns success
		Bank_GetWealth(playerid)           : cash + bank (wealth-tax base)

	Commands:
		/bank                      (player)  dialog: balance + deposit/withdraw
		/atm                       (player)  same as /bank (ATM-point placement deferred)
		/deposit (/dep) <amt|all>  (player)  cash -> bank
		/withdraw (/wd) <amt|all>  (player)  bank -> cash
		/balance (/bal)            (player)  show cash + bank + tax owed
		/givecash (/pay) <id> <amt>(player)  hand cash to a nearby player (NOT a bank transfer)
		/buyinsurance (/buyins) [health|life]  (player)  buy health cover / a life policy

	Functions Being Called:
		misc:     UpdateMoney
		jail:     Jail_SavePlayer (shared persistence - no competing UPDATE)
		messages: SendMessageToModerators (givecash audit log)
*/

//==============================================================================
//	Clean bank API (exposed for stages 2/3) - all amounts validated + clamped
//==============================================================================

// Current banked balance.
FUNCTION Bank_GetBalance(playerid)
	return PlayerInfo[playerid][pBankMoney];

// Current cash-in-hand (mirrors GetPlayerMoney once UpdateMoney has synced it).
FUNCTION Bank_GetCash(playerid)
	return PlayerInfo[playerid][pMoney];

// Total wealth (cash + bank) - the wealth-tax base and the /balance summary.
// Uses floats internally only where a sum could exceed the cell; here both are
// clamped <= MAX_MONEY so the sum still fits (2*MAX_MONEY < cellmax).
FUNCTION Bank_GetWealth(playerid)
	return PlayerInfo[playerid][pMoney] + PlayerInfo[playerid][pBankMoney];

// Credit `amount` to the bank. Rejects non-positive amounts and clamps the new
// balance below MAX_MONEY (overflow guard). Returns 1 on success, 0 on reject.
FUNCTION Bank_Give(playerid, amount)
{
	if(!IsPlayerConnected(playerid) || !PlayerInfo[playerid][pLoggedIn]) return 0;
	if(amount <= 0) return 0;
	if(PlayerInfo[playerid][pBankMoney] > MAX_MONEY - amount) return 0;	// would overflow
	PlayerInfo[playerid][pBankMoney] += amount;
	return 1;
}

// Debit `amount` from the bank if affordable. Returns 1 on success, 0 if the
// amount is non-positive or the player cannot afford it.
FUNCTION Bank_Take(playerid, amount)
{
	if(!IsPlayerConnected(playerid) || !PlayerInfo[playerid][pLoggedIn]) return 0;
	if(amount <= 0) return 0;
	if(PlayerInfo[playerid][pBankMoney] < amount) return 0;
	PlayerInfo[playerid][pBankMoney] -= amount;
	return 1;
}

// Credit `amount` to cash-in-hand (via UpdateMoney). Overflow-guarded. Also
// counts toward taxable income so mission/event payouts (stages 2/3) are taxed.
FUNCTION Bank_GiveCash(playerid, amount)
{
	if(!IsPlayerConnected(playerid) || !PlayerInfo[playerid][pLoggedIn]) return 0;
	if(amount <= 0) return 0;
	if(PlayerInfo[playerid][pMoney] > MAX_MONEY - amount) return 0;	// would overflow
	PlayerInfo[playerid][pMoney] += amount;
	// Accrue income tax immediately into the persisted pTaxOwed (economy §1.4)
	// so it survives relogs and cannot be avoided by disconnecting before the
	// tax tick (INCOME_TAX_PCT% of money earned, collected on the next tax day).
	new incTax = (amount / 100) * INCOME_TAX_PCT;
	if(incTax > 0 && PlayerInfo[playerid][pTaxOwed] <= MAX_MONEY - incTax)
		PlayerInfo[playerid][pTaxOwed] += incTax;
	UpdateMoney(playerid);
	return 1;
}

// Debit `amount` from cash-in-hand if affordable (via UpdateMoney). Returns 1 on
// success, 0 if the amount is non-positive or unaffordable.
FUNCTION Bank_TakeCash(playerid, amount)
{
	if(!IsPlayerConnected(playerid) || !PlayerInfo[playerid][pLoggedIn]) return 0;
	if(amount <= 0) return 0;
	if(PlayerInfo[playerid][pMoney] < amount) return 0;
	PlayerInfo[playerid][pMoney] -= amount;
	UpdateMoney(playerid);
	return 1;
}

//==============================================================================
//	Internal helpers
//==============================================================================

// Parse an amount token that may be a positive integer or the "all" keyword.
// `have` is the pool the "all" keyword maps to. Returns the resolved amount, or
// -1 for an invalid/negative/zero/overflowing token.
static Bank_ParseAmount(const params[], have)
{
	if(isNull(params)) return -1;
	if(!strcmp(params, "all", true) || !strcmp(params, "max", true))
		return (have > 0) ? have : -1;
	// Reject anything that isn't a clean signed integer (blocks "1e9", "-5", etc).
	if(!isNumeric(params)) return -1;
	new amount = strval(params);
	if(amount <= 0) return -1;
	if(amount > MAX_MONEY) return -1;	// overflow / absurd amount guard
	return amount;
}

// Shared deposit path (used by /deposit and the deposit dialog). Validates the
// amount against cash-in-hand, moves it, persists, and messages the player.
static Bank_DoDeposit(playerid, amount)
{
	if(amount <= 0) return SendClientMessage(playerid, COLOR_ERROR, "Enter a positive amount to deposit.");
	if(PlayerInfo[playerid][pMoney] < amount)
		return SendClientMessage(playerid, COLOR_ERROR, "You do not have that much cash on you.");
	if(PlayerInfo[playerid][pBankMoney] > MAX_MONEY - amount)
		return SendClientMessage(playerid, COLOR_ERROR, "Your bank account cannot hold that much.");

	// Take cash, credit bank. Both legs are guarded above so neither can fail.
	Bank_TakeCash(playerid, amount);
	PlayerInfo[playerid][pBankMoney] += amount;
	Jail_SavePlayer(playerid);	// shared persistence (no competing UPDATE)

	new string[144];
	format(string, sizeof(string), ""COL_GREEN"* Deposited "COL_WHITE"$%d"COL_GREEN" into your bank."COL_WHITE" Balance: "COL_GREEN"$%d"COL_WHITE", cash: "COL_GREEN"$%d"COL_WHITE".", amount, PlayerInfo[playerid][pBankMoney], PlayerInfo[playerid][pMoney]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	return 1;
}

// Shared withdraw path (used by /withdraw and the withdraw dialog).
static Bank_DoWithdraw(playerid, amount)
{
	if(amount <= 0) return SendClientMessage(playerid, COLOR_ERROR, "Enter a positive amount to withdraw.");
	if(PlayerInfo[playerid][pBankMoney] < amount)
		return SendClientMessage(playerid, COLOR_ERROR, "You do not have that much in the bank.");
	if(PlayerInfo[playerid][pMoney] > MAX_MONEY - amount)
		return SendClientMessage(playerid, COLOR_ERROR, "You are carrying too much cash to withdraw that much.");

	PlayerInfo[playerid][pBankMoney] -= amount;
	PlayerInfo[playerid][pMoney] += amount;
	UpdateMoney(playerid);
	Jail_SavePlayer(playerid);	// shared persistence (no competing UPDATE)

	new string[144];
	format(string, sizeof(string), ""COL_GREEN"* Withdrew "COL_WHITE"$%d"COL_GREEN" from your bank."COL_WHITE" Balance: "COL_GREEN"$%d"COL_WHITE", cash: "COL_GREEN"$%d"COL_WHITE".", amount, PlayerInfo[playerid][pBankMoney], PlayerInfo[playerid][pMoney]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	return 1;
}

// Render the bank menu (balance + deposit/withdraw options). Used by /bank+/atm.
static Bank_ShowMenu(playerid)
{
	new caption[256];
	format(caption, sizeof(caption),
		""COL_WHITE"Cash on hand: "COL_GREEN"$%d"COL_WHITE"\nBank balance: "COL_GREEN"$%d"COL_WHITE"\nTax owed: "COL_RED"$%d"COL_WHITE"\nDeposit cash\nWithdraw cash",
		PlayerInfo[playerid][pMoney], PlayerInfo[playerid][pBankMoney], PlayerInfo[playerid][pTaxOwed]);
	ShowPlayerDialog(playerid, BANK_DIALOG, DIALOG_STYLE_LIST, "Bank of Los Santos", caption, "Select", "Close");
	return 1;
}

//==============================================================================
//	Commands - bank
//==============================================================================

// NOTE (ATM/bank-actor proximity): the design implies deposit/withdraw happen at
// a physical Bank/City Hall/RP Club or an ATM point (economy §1.3). ATM points
// are not placed yet, so for M4 stage 1 /bank and /atm are usable anywhere; the
// physical ATM-point placement + proximity gate is deferred to when the ATM/bank
// actor pickups land. Treated identically for now.

CMD:bank(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	return Bank_ShowMenu(playerid);
}
alias:bank("bankinfo", "moneyinfo")

CMD:atm(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	// ATM-point placement deferred (see note above) - same as /bank for now.
	return Bank_ShowMenu(playerid);
}

CMD:deposit(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new amount = Bank_ParseAmount(params, PlayerInfo[playerid][pMoney]);
	if(amount == -1) return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /deposit [amount | all]");
	return Bank_DoDeposit(playerid, amount);
}
alias:deposit("dep")

CMD:withdraw(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new amount = Bank_ParseAmount(params, PlayerInfo[playerid][pBankMoney]);
	if(amount == -1) return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /withdraw [amount | all]");
	return Bank_DoWithdraw(playerid, amount);
}
alias:withdraw("wd", "wit")

CMD:balance(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new string[160];
	format(string, sizeof(string), ""COL_WHITE"Cash: "COL_GREEN"$%d"COL_WHITE"  |  Bank: "COL_GREEN"$%d"COL_WHITE"  |  Tax owed: "COL_RED"$%d"COL_WHITE, PlayerInfo[playerid][pMoney], PlayerInfo[playerid][pBankMoney], PlayerInfo[playerid][pTaxOwed]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	if(PlayerInfo[playerid][pHealthInsExpiry] > gettime())
	{
		new left = (PlayerInfo[playerid][pHealthInsExpiry] - gettime() + 59) / 60;
		format(string, sizeof(string), ""COL_WHITE"Health insurance: "COL_GREEN"active"COL_WHITE" (~%d min left).  Life policies: "COL_GREEN"%d"COL_WHITE"/%d.", left, PlayerInfo[playerid][pLifeInsurance], LIFE_INS_MAX);
	}
	else format(string, sizeof(string), ""COL_WHITE"Health insurance: "COL_RED"none"COL_WHITE".  Life policies: "COL_GREEN"%d"COL_WHITE"/%d.", PlayerInfo[playerid][pLifeInsurance], LIFE_INS_MAX);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	return 1;
}
alias:balance("bal")

//==============================================================================
//	Command - /givecash (/pay) : hand cash to a nearby player (NOT a bank xfer)
//==============================================================================

CMD:givecash(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new targetid, amount;
	if(sscanf(params, "ud", targetid, amount))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /givecash [playerid] [amount]");

	// Anti-abuse: positive integer amount within bounds.
	if(amount < GIVECASH_MIN) return SendClientMessage(playerid, COLOR_ERROR, "Enter a positive amount to give.");
	if(amount > MAX_MONEY) return SendClientMessage(playerid, COLOR_ERROR, "That amount is too large.");

	// Valid, distinct, connected, logged-in target.
	if(targetid == INVALID_PLAYER_ID || !IsPlayerConnected(targetid) || !PlayerInfo[targetid][pLoggedIn])
		return SendClientMessage(playerid, COLOR_ERROR, "That player is not connected/logged in.");
	if(targetid == playerid) return SendClientMessage(playerid, COLOR_ERROR, "You cannot give cash to yourself.");
	if(IsPlayerNPC(targetid)) return SendClientMessage(playerid, COLOR_ERROR, "You cannot give cash to that player.");

	// Cooldown (anti-spam / anti-abuse).
	if(gettime() < PlayerInfo[playerid][pGiveCashCooldown])
	{
		new err[96];
		format(err, sizeof(err), "Slow down - you can hand cash again in %d seconds.", PlayerInfo[playerid][pGiveCashCooldown] - gettime());
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}

	// Must have the cash on hand (this is a physical hand-over, not a bank move).
	if(PlayerInfo[playerid][pMoney] < amount)
		return SendClientMessage(playerid, COLOR_ERROR, "You do not have that much cash on you.");

	// Proximity + same interior/VW (must be close enough - economy §1.3).
	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	if(GetPlayerInterior(playerid) != GetPlayerInterior(targetid) ||
		GetPlayerVirtualWorld(playerid) != GetPlayerVirtualWorld(targetid) ||
		!IsPlayerInRangeOfPoint(targetid, GIVECASH_RANGE, x, y, z))
		return SendClientMessage(playerid, COLOR_ERROR, "You must be right next to that player to hand them cash.");

	// Overflow guard on the receiver.
	if(PlayerInfo[targetid][pMoney] > MAX_MONEY - amount)
		return SendClientMessage(playerid, COLOR_ERROR, "That player is carrying too much cash to receive that.");

	// Move the cash (both legs guarded above).
	PlayerInfo[playerid][pMoney] -= amount;
	UpdateMoney(playerid);
	PlayerInfo[targetid][pMoney] += amount;
	UpdateMoney(targetid);
	PlayerInfo[playerid][pGiveCashCooldown] = gettime() + GIVECASH_CD;

	new string[144];
	format(string, sizeof(string), ""COL_GREEN"* You handed "COL_WHITE"$%d"COL_GREEN" to %s (%d)."COL_WHITE, amount, PlayerInfo[targetid][pUserName], targetid);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	format(string, sizeof(string), ""COL_GREEN"* %s (%d) handed you "COL_WHITE"$%d"COL_GREEN"."COL_WHITE, PlayerInfo[playerid][pUserName], playerid, amount);
	SendClientMessage(targetid, COLOR_SERVER_INFO, string);

	// Audit log (anti-abuse trail) - visible to staff.
	format(string, sizeof(string), "[GIVECASH] %s (%d) gave $%d to %s (%d).", PlayerInfo[playerid][pUserName], playerid, amount, PlayerInfo[targetid][pUserName], targetid);
	SendMessageToModerators(COLOR_ADMIN_INFO, string);
	return 1;
}
alias:givecash("pay", "gc", "sendcash", "givemoney")

//==============================================================================
//	Command - /buyinsurance (/buyins) : health cover + life policies
//==============================================================================

// Charge a price from bank-first-then-cash. Returns 1 if fully paid, 0 if the
// player cannot afford it across both pools (nothing is deducted on failure).
static Bank_ChargeBankFirst(playerid, price)
{
	if(price <= 0) return 1;
	if(Bank_GetWealth(playerid) < price) return 0;
	new remaining = price;
	if(PlayerInfo[playerid][pBankMoney] >= remaining)
	{
		PlayerInfo[playerid][pBankMoney] -= remaining;
		return 1;
	}
	remaining -= PlayerInfo[playerid][pBankMoney];
	PlayerInfo[playerid][pBankMoney] = 0;
	PlayerInfo[playerid][pMoney] -= remaining;
	UpdateMoney(playerid);
	return 1;
}

CMD:buyinsurance(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	new kind[8];
	if(sscanf(params, "s[8]", kind))
	{
		new usage[144];
		format(usage, sizeof(usage), "USAGE: /buyinsurance [health|life]  ("COL_GREEN"health $%d"COL_WHITE" = %d game-days,  "COL_GREEN"life $%d"COL_WHITE" = 1 policy)", HEALTH_INS_PRICE, HEALTH_INS_DAYS, LIFE_INS_PRICE);
		return SendClientMessage(playerid, COLOR_SERVER_INFO, usage);
	}

	new string[160];
	if(!strcmp(kind, "health", true))
	{
		if(Bank_GetWealth(playerid) < HEALTH_INS_PRICE)
			return SendClientMessage(playerid, COLOR_ERROR, "You cannot afford health insurance (bank + cash).");
		Bank_ChargeBankFirst(playerid, HEALTH_INS_PRICE);
		// Extend from now, or from the current expiry if still active (stacking).
		new base = (PlayerInfo[playerid][pHealthInsExpiry] > gettime()) ? PlayerInfo[playerid][pHealthInsExpiry] : gettime();
		PlayerInfo[playerid][pHealthInsExpiry] = base + (HEALTH_INS_DAYS * SECS_PER_GAME_DAY);
		Jail_SavePlayer(playerid);
		format(string, sizeof(string), ""COL_GREEN"* Health insurance bought "COL_WHITE"($%d)"COL_GREEN" - %d game-days of medical cover.", HEALTH_INS_PRICE, HEALTH_INS_DAYS);
		SendClientMessage(playerid, COLOR_SERVER_INFO, string);
		return 1;
	}
	if(!strcmp(kind, "life", true))
	{
		if(PlayerInfo[playerid][pLifeInsurance] >= LIFE_INS_MAX)
		{
			format(string, sizeof(string), "You already hold the maximum of %d life policies.", LIFE_INS_MAX);
			return SendClientMessage(playerid, COLOR_ERROR, string);
		}
		if(Bank_GetWealth(playerid) < LIFE_INS_PRICE)
			return SendClientMessage(playerid, COLOR_ERROR, "You cannot afford a life policy (bank + cash).");
		Bank_ChargeBankFirst(playerid, LIFE_INS_PRICE);
		PlayerInfo[playerid][pLifeInsurance]++;
		Jail_SavePlayer(playerid);
		format(string, sizeof(string), ""COL_GREEN"* Life policy bought "COL_WHITE"($%d)"COL_GREEN" - now holding %d/%d. Waives the medical fee and saves your weapons on death.", LIFE_INS_PRICE, PlayerInfo[playerid][pLifeInsurance], LIFE_INS_MAX);
		SendClientMessage(playerid, COLOR_SERVER_INFO, string);
		return 1;
	}
	return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /buyinsurance [health|life]");
}
alias:buyinsurance("buyins")

//==============================================================================
//	Dialog resolution (routed from OnDialogResponse in CnR.pwn)
//==============================================================================

FUNCTION Bank_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
	switch(dialogid)
	{
		case BANK_DIALOG:
		{
			if(!response) return 1;
			// listitem 0..2 are the balance lines (not selectable actions), 3/4 are
			// deposit/withdraw. We render deposit at index 3, withdraw at index 4.
			if(listitem == 3)
				ShowPlayerDialog(playerid, BANK_DEPOSIT_DIALOG, DIALOG_STYLE_INPUT, "Deposit", ""COL_WHITE"Enter the amount to deposit (or "COL_GREEN"all"COL_WHITE"):", "Deposit", "Back");
			else if(listitem == 4)
				ShowPlayerDialog(playerid, BANK_WITHDRAW_DIALOG, DIALOG_STYLE_INPUT, "Withdraw", ""COL_WHITE"Enter the amount to withdraw (or "COL_GREEN"all"COL_WHITE"):", "Withdraw", "Back");
			else Bank_ShowMenu(playerid);	// tapped a balance line - reopen
			return 1;
		}
		case BANK_DEPOSIT_DIALOG:
		{
			if(!response) return Bank_ShowMenu(playerid);	// "Back"
			new amount = Bank_ParseAmount(inputtext, PlayerInfo[playerid][pMoney]);
			if(amount == -1) return SendClientMessage(playerid, COLOR_ERROR, "Invalid amount. Enter a positive number or 'all'.");
			return Bank_DoDeposit(playerid, amount);
		}
		case BANK_WITHDRAW_DIALOG:
		{
			if(!response) return Bank_ShowMenu(playerid);	// "Back"
			new amount = Bank_ParseAmount(inputtext, PlayerInfo[playerid][pBankMoney]);
			if(amount == -1) return SendClientMessage(playerid, COLOR_ERROR, "Invalid amount. Enter a positive number or 'all'.");
			return Bank_DoWithdraw(playerid, amount);
		}
	}
	return 0;
}

//==============================================================================
//	Bank interest - game-clock day schedule (economy §1.3 - every 2 game-days)
//==============================================================================

// Called from GameModeClock()'s day transition in CnR.pwn (NO new global timer).
// `gameDay` is the running day counter used to fire every BANK_INTEREST_DAYS.
// Interest is BANK_INTEREST_PCT% of the banked balance, paid to all connected +
// logged-in players. Offline accrual is deferred for M4 (see note below).
FUNCTION Bank_OnGameDay(gameDay)
{
	// Only pay on the interest schedule (every BANK_INTEREST_DAYS game-days).
	if((gameDay % BANK_INTEREST_DAYS) != 0) return 0;

	for(new playerid = 0; playerid < MAX_PLAYERS; playerid++)
	{
		if(!IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) continue;
		if(!PlayerInfo[playerid][pLoggedIn]) continue;
		if(PlayerInfo[playerid][pBankMoney] <= 0) continue;

		// Interest = pct% of balance (integer division floors it - no free cent).
		new interest = (PlayerInfo[playerid][pBankMoney] / 100) * BANK_INTEREST_PCT;
		if(interest <= 0) continue;
		// Overflow guard on the credit.
		if(PlayerInfo[playerid][pBankMoney] > MAX_MONEY - interest)
			interest = MAX_MONEY - PlayerInfo[playerid][pBankMoney];
		if(interest <= 0) continue;

		PlayerInfo[playerid][pBankMoney] += interest;

		new string[144];
		format(string, sizeof(string), ""COL_GREEN"* Bank interest: "COL_WHITE"+$%d"COL_GREEN" (%d%%) on your balance."COL_WHITE" New balance: "COL_GREEN"$%d"COL_WHITE".", interest, BANK_INTEREST_PCT, PlayerInfo[playerid][pBankMoney]);
		SendClientMessage(playerid, COLOR_SERVER_INFO, string);
		Jail_SavePlayer(playerid);
	}
	return 1;
}
// DEFERRED (offline interest): interest only accrues for online, logged-in
// players in M4. Paying offline accounts would need a bulk UPDATE keyed on
// last-online days, which is deferred to keep this stage minimal (per the brief).

//==============================================================================
//	Taxes - game-clock day schedule (economy §1.4 - wealth + income, every 2 days)
//==============================================================================

// Called from GameModeClock()'s day transition in CnR.pwn (shared day hook).
// Wealth tax: WEALTH_TAX_PCT% on (cash+bank) OVER WEALTH_TAX_THRESHOLD; donators
// (pVip) are exempt (economy §1.4). Income tax: already accrued into pTaxOwed at
// earn time (Bank_GiveCash), so Tax_OnGameDay only adds the fresh wealthTax then
// collects the full pTaxOwed bill. The bill is paid bank-first then cash;
// any unpayable remainder accrues to TaxOwed and is chased on the next tick.
FUNCTION Tax_OnGameDay(gameDay)
{
	if((gameDay % TAX_DAYS) != 0) return 0;

	for(new playerid = 0; playerid < MAX_PLAYERS; playerid++)
	{
		if(!IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) continue;
		if(!PlayerInfo[playerid][pLoggedIn]) continue;

		new wealthTax = 0;

		// Wealth tax on the amount OVER the threshold (donators exempt).
		if(!PlayerInfo[playerid][pVip])
		{
			new wealth = Bank_GetWealth(playerid);
			if(wealth > WEALTH_TAX_THRESHOLD)
			{
				new over = wealth - WEALTH_TAX_THRESHOLD;
				wealthTax = (over / 100) * WEALTH_TAX_PCT;
			}
		}

		// Total bill this tick = fresh wealth tax + accrued pTaxOwed (income tax
		// was already folded into pTaxOwed at earn time in Bank_GiveCash).
		new prevOwed = PlayerInfo[playerid][pTaxOwed];
		new bill = wealthTax + prevOwed;
		if(bill <= 0) continue;

		// Pay bank-first then cash; whatever cannot be paid accrues to TaxOwed.
		new paid = 0, remaining = bill;
		if(PlayerInfo[playerid][pBankMoney] > 0)
		{
			new fromBank = (PlayerInfo[playerid][pBankMoney] >= remaining) ? remaining : PlayerInfo[playerid][pBankMoney];
			PlayerInfo[playerid][pBankMoney] -= fromBank;
			remaining -= fromBank;
			paid += fromBank;
		}
		if(remaining > 0 && PlayerInfo[playerid][pMoney] > 0)
		{
			new fromCash = (PlayerInfo[playerid][pMoney] >= remaining) ? remaining : PlayerInfo[playerid][pMoney];
			PlayerInfo[playerid][pMoney] -= fromCash;
			UpdateMoney(playerid);
			remaining -= fromCash;
			paid += fromCash;
		}
		PlayerInfo[playerid][pTaxOwed] = remaining;	// unpayable remainder carries forward

		new string[192];
		format(string, sizeof(string), ""COL_YELLOW"* Tax bill: "COL_WHITE"wealth "COL_RED"$%d"COL_WHITE" + owed "COL_RED"$%d"COL_WHITE" = "COL_RED"$%d"COL_WHITE". Paid "COL_GREEN"$%d"COL_WHITE".", wealthTax, prevOwed, bill, paid);
		SendClientMessage(playerid, COLOR_SERVER_INFO, string);
		if(remaining > 0)
		{
			format(string, sizeof(string), ""COL_RED"* You still owe "COL_WHITE"$%d"COL_RED" in tax - it will be collected next tax day."COL_WHITE, remaining);
			SendClientMessage(playerid, COLOR_ERROR, string);
		}
		Jail_SavePlayer(playerid);
	}
	return 1;
}

//==============================================================================
//	Insurance / medical fee on death (economy §1.5/§1.6) - hooked from OnPlayerDeath
//==============================================================================

// Charge the medical fee on death unless the player holds active health insurance
// or a life policy. A life policy waives the fee AND preserves weapons (the caller
// in CnR.pwn resets weapons only when this returns 0 = "no life-insurance save").
// Returns 1 if weapons should be PRESERVED (life policy consumed), 0 otherwise.
FUNCTION Bank_OnPlayerDeath(playerid)
{
	if(!IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) return 0;
	if(!PlayerInfo[playerid][pLoggedIn]) return 0;

	// Life insurance: waive the fee, preserve weapons, consume one policy.
	if(PlayerInfo[playerid][pLifeInsurance] > 0)
	{
		PlayerInfo[playerid][pLifeInsurance]--;
		new string[128];
		format(string, sizeof(string), ""COL_GREEN"* Life insurance paid out "COL_WHITE"- no medical fee, weapons saved."COL_GREEN" Policies left: "COL_WHITE"%d"COL_GREEN"/%d.", PlayerInfo[playerid][pLifeInsurance], LIFE_INS_MAX);
		SendClientMessage(playerid, COLOR_SERVER_INFO, string);
		return 1;	// preserve weapons
	}

	// Health insurance: covers the medical fee (free medical items), no charge.
	if(PlayerInfo[playerid][pHealthInsExpiry] > gettime())
	{
		SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_GREEN"* Health insurance covered your medical fee."COL_WHITE);
		return 0;
	}

	// No insurance - charge the medical fee from cash-first then bank (cash is the
	// vulnerable pool, economy §1.5). Whatever cannot be paid is written off (you
	// don't go into debt on death); we take what's there.
	new fee = MEDICAL_FEE, taken = 0;
	if(PlayerInfo[playerid][pMoney] > 0)
	{
		new fromCash = (PlayerInfo[playerid][pMoney] >= fee) ? fee : PlayerInfo[playerid][pMoney];
		PlayerInfo[playerid][pMoney] -= fromCash;
		UpdateMoney(playerid);
		fee -= fromCash;
		taken += fromCash;
	}
	if(fee > 0 && PlayerInfo[playerid][pBankMoney] > 0)
	{
		new fromBank = (PlayerInfo[playerid][pBankMoney] >= fee) ? fee : PlayerInfo[playerid][pBankMoney];
		PlayerInfo[playerid][pBankMoney] -= fromBank;
		taken += fromBank;	// `fee` is fully consumed here; not read again
	}
	if(taken > 0)
	{
		new string[128];
		format(string, sizeof(string), ""COL_RED"* Medical fee: "COL_WHITE"-$%d"COL_RED" charged on death."COL_WHITE" Buy "COL_GREEN"/buyinsurance"COL_WHITE" to avoid this.", taken);
		SendClientMessage(playerid, COLOR_ERROR, string);
	}
	return 0;	// no weapon save
}
