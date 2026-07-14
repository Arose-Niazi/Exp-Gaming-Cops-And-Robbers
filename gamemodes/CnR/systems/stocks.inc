/*

	Stock market MACHINERY for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 7 - the stock market + full economy (THE finale). Design §9.4 (the
	market: 21 stocks, trading hours game 07:00-19:00, /markets //shares
	//sharessell, own ≤100k shares/company, 1% fee, price formula + activity-fed
	pool, daily tick) + §10 (economy integration: prime rate + per-domain
	multiplier) + §11.3 (stocks/stock_holdings/stock_reports tables) + §11.4
	(server_data.PrimeRate) + docs/research/economy.md §7 (the sf-cnr POOL/PRICE
	model, StockMarket_UpdateEarnings, dividends, bankruptcy).

	The CORE (the E_STOCK_* roster, config, the g_Stock market state, and the two
	price/pool functions StockMarket_UpdateEarnings / StockMarket_PriceMult that
	fishing/farming/etc. call) lives in systems/economy_stub.inc, which is included
	FIRST so those call sites resolve. THIS file holds everything else:

	  - Stocks_Init()          : load the stocks table async; seed the 21-stock
	                             roster on first boot if the table is empty.
	  - Stocks_OnPlayerLogin() : load a player's holdings async (race-guarded).
	  - Stocks_Tick()          : the daily market tick (recompute prices from
	                             pool+drift, bleed the pool, pay dividends, write a
	                             report row, move the prime rate, persist). Fired by
	                             GameModeClock at the game-day rollover (§10.3),
	                             guarded once-per-game-day. /astocktick forces it.
	  - Trading commands       : /markets (list), /shares (/stocks) [buy] <id> <qty>,
	                             /sharessell <id> <qty>, /markethistory. Trading-hours
	                             gate (07:00-19:00), 100k share cap, 1% fee. All money
	                             via Bank_* (buys paid for, sells credited). Holdings
	                             can't go negative / exceed the cap / oversell.

	NO MINT: money moves player<->market. The market is not a player, so a buy is a
	sink and a sell is a source (a gov-style sink/source by design, §9.4) - but every
	share bought is PAID FOR (Bank_TakeCash cost+fee) and every sell is CREDITED
	(Bank_GiveCash proceeds-fee), with the holding tracked in stock_holdings.

	Public API:
		Stocks_Init()                    : load/seed the market (async) - OnGameModeInit
		Stocks_OnPlayerLogin(playerid)   : load this player's holdings (async, race-guarded)
		Stocks_Tick()                    : run the daily market tick (§10.3 / /astocktick)
	Async handlers (public, via mysql_pquery):
		Stocks_OnLoad, Stocks_OnHoldingsLoad

	Functions Being Called:
		bank:        Bank_TakeCash, Bank_GiveCash, Bank_GetCash
		server_vars: SavePrimeRate (prime rate persisted on the tick)
		core:        StockMarket_Price, StockMarket_PriceMult (economy_stub.inc)
*/

//==============================================================================
//	Seed roster - the 21 CB stocks (economy.md §7.1). Order MUST match E_STOCK_*.
//	Each: name, IPO price, IPO float. MaxShares = IPOShares (available = IPOShares
//	at seed). Seeded on first boot only (Stocks_Init when the table is empty).
//==============================================================================

static const g_StockSeed[MAX_STOCKS][e_Stock] = {
	//  name                       price   pool  avail    IPOprice IPOsh    max
	{ !"Government Bonds",           100.0,  0.0, 500000,  100.0, 500000,  500000 },
	{ !"House Market",               250.0,  0.0, 300000,  250.0, 300000,  300000 },
	{ !"Fish Market",                 50.0,  0.0, 400000,   50.0, 400000,  400000 },
	{ !"Drug",                        80.0,  0.0, 400000,   80.0, 400000,  400000 },
	{ !"Ammunation",                 120.0,  0.0, 300000,  120.0, 300000,  300000 },
	{ !"24/7 Corporation",            40.0,  0.0, 500000,   40.0, 500000,  500000 },
	{ !"Vehicle Dealership",         160.0,  0.0, 250000,  160.0, 250000,  250000 },
	{ !"Police Department",           90.0,  0.0, 300000,   90.0, 300000,  300000 },
	{ !"Hospital",                    70.0,  0.0, 300000,   70.0, 300000,  300000 },
	{ !"Cluckin' Bell",               55.0,  0.0, 350000,   55.0, 350000,  350000 },
	{ !"Well Stacked Pizza",          60.0,  0.0, 350000,   60.0, 350000,  350000 },
	{ !"Burger Shot",                 58.0,  0.0, 350000,   58.0, 350000,  350000 },
	{ !"Sex Shops",                   45.0,  0.0, 300000,   45.0, 300000,  300000 },
	{ !"Bars & Dance Clubs",          65.0,  0.0, 300000,   65.0, 300000,  300000 },
	{ !"Diners",                      50.0,  0.0, 350000,   50.0, 350000,  350000 },
	{ !"Strip Clubs",                 75.0,  0.0, 250000,   75.0, 250000,  250000 },
	{ !"Bait Shops",                  48.0,  0.0, 300000,   48.0, 300000,  300000 },
	{ !"Off Track Betting",           85.0,  0.0, 250000,   85.0, 250000,  250000 },
	{ !"Gambling",                   140.0,  0.0, 200000,  140.0, 200000,  200000 },
	{ !"Transport",                  110.0,  0.0, 300000,  110.0, 300000,  300000 },
	{ !"Clothing",                    52.0,  0.0, 350000,   52.0, 350000,  350000 }
};

//==============================================================================
//	Init - load the stocks table (async). Seed on first boot if empty.
//==============================================================================

FUNCTION Stocks_Init()
{
	g_StocksLoaded = false;
	mysql_pquery(g_SQL, "SELECT * FROM `stocks` ORDER BY ID ASC", "Stocks_OnLoad", "");
	return 1;
}

// Seed the 21-stock roster into g_Stock[] + INSERT the rows (first boot only).
static Stocks_SeedRoster()
{
	new query[256];
	for(new i = 0; i < MAX_STOCKS; i++)
	{
		g_Stock[i] = g_StockSeed[i];
		mysql_format(g_SQL, query, sizeof(query),
			"INSERT INTO `stocks` (ID, Name, Price, Pool, AvailableShares, IPOPrice, IPOShares, MaxShares) VALUES (%d, '%e', %f, %f, %d, %f, %d, %d)",
			i, g_Stock[i][stName], g_Stock[i][stPrice], g_Stock[i][stPool], g_Stock[i][stAvailable],
			g_Stock[i][stIPOPrice], g_Stock[i][stIPOShares], g_Stock[i][stMaxShares]);
		mysql_pquery(g_SQL, query);
	}
	g_StocksLoaded = true;
	printf("[STOCKS] Seeded the %d-stock market (first boot).", MAX_STOCKS);
	return 1;
}

// Async result handler for Stocks_Init - read the rows, or seed if the table is empty.
FUNCTION Stocks_OnLoad()
{
	new rows;
	cache_get_row_count(rows);
	if(rows == 0)
	{
		Stocks_SeedRoster();
		return 1;
	}
	for(new i = 0; i < rows; i++)
	{
		new id;
		cache_get_value_int(i, "ID", id);
		if(id < 0 || id >= MAX_STOCKS) continue;
		cache_get_value_name(i, "Name", g_Stock[id][stName], 40);
		cache_get_value_float(i, "Price", g_Stock[id][stPrice]);
		cache_get_value_float(i, "Pool", g_Stock[id][stPool]);
		cache_get_value_int(i, "AvailableShares", g_Stock[id][stAvailable]);
		cache_get_value_float(i, "IPOPrice", g_Stock[id][stIPOPrice]);
		cache_get_value_int(i, "IPOShares", g_Stock[id][stIPOShares]);
		cache_get_value_int(i, "MaxShares", g_Stock[id][stMaxShares]);
		if(g_Stock[id][stPrice] < STOCK_PRICE_FLOOR) g_Stock[id][stPrice] = STOCK_PRICE_FLOOR;
		if(g_Stock[id][stIPOPrice] <= 0.0) g_Stock[id][stIPOPrice] = STOCK_PRICE_FLOOR;
	}
	g_StocksLoaded = true;
	printf("[STOCKS] Loaded %d stock(s).", rows);
	// Load holdings for anyone already logged in (hot reload).
	for(new pid = 0; pid < MAX_PLAYERS; pid++)
		if(IsPlayerConnected(pid) && !IsPlayerNPC(pid) && PlayerInfo[pid][pLoggedIn])
			Stocks_OnPlayerLogin(pid);
	return 1;
}

// Persist a single stock's mutable fields (price/pool/available). Called after a
// trade moves AvailableShares and on the daily tick after prices recompute.
static Stocks_SaveOne(stockid)
{
	if(stockid < 0 || stockid >= MAX_STOCKS) return 0;
	new query[160];
	mysql_format(g_SQL, query, sizeof(query),
		"UPDATE `stocks` SET Price=%f, Pool=%f, AvailableShares=%d WHERE ID=%d LIMIT 1",
		g_Stock[stockid][stPrice], g_Stock[stockid][stPool], g_Stock[stockid][stAvailable], stockid);
	mysql_pquery(g_SQL, query);
	return 1;
}

//==============================================================================
//	Per-player holdings - load on login (race-guarded), upsert on trade
//==============================================================================

// Load a player's holdings from stock_holdings (async). Race-guarded so a slot
// re-used before the query resolves does not get another player's holdings.
FUNCTION Stocks_OnPlayerLogin(playerid)
{
	for(new s = 0; s < MAX_STOCKS; s++) PlayerInfo[playerid][pStocks][s] = 0;
	PlayerInfo[playerid][pStocksLoaded] = false;
	new query[96];
	mysql_format(g_SQL, query, sizeof(query), "SELECT StockID, Shares FROM `stock_holdings` WHERE aID=%d", PlayerInfo[playerid][pID]);
	mysql_pquery(g_SQL, query, "Stocks_OnHoldingsLoad", "dd", playerid, g_MysqlRaceCheck[playerid]);
	return 1;
}

// Async result handler - fill the cached holdings if the slot is still the same login.
FUNCTION Stocks_OnHoldingsLoad(playerid, race_check)
{
	if(!IsPlayerConnected(playerid) || race_check != g_MysqlRaceCheck[playerid]) return 0;
	new rows;
	cache_get_row_count(rows);
	for(new i = 0; i < rows; i++)
	{
		new sid, sh;
		cache_get_value_int(i, "StockID", sid);
		cache_get_value_int(i, "Shares", sh);
		if(sid < 0 || sid >= MAX_STOCKS) continue;
		if(sh < 0) sh = 0;
		if(sh > STOCK_MAX_OWN) sh = STOCK_MAX_OWN;
		PlayerInfo[playerid][pStocks][sid] = sh;
	}
	PlayerInfo[playerid][pStocksLoaded] = true;
	return 1;
}

// Upsert / delete a holding row after a trade. `shares` is the NEW absolute holding
// for this (account, stock); a 0 holding deletes the row.
static Stocks_SaveHolding(aID, stockid, shares)
{
	new query[160];
	if(shares <= 0)
		mysql_format(g_SQL, query, sizeof(query), "DELETE FROM `stock_holdings` WHERE aID=%d AND StockID=%d LIMIT 1", aID, stockid);
	else
		mysql_format(g_SQL, query, sizeof(query),
			"INSERT INTO `stock_holdings` (aID, StockID, Shares) VALUES (%d, %d, %d) ON DUPLICATE KEY UPDATE Shares=%d",
			aID, stockid, shares, shares);
	mysql_pquery(g_SQL, query);
	return 1;
}

//==============================================================================
//	Helpers
//==============================================================================

// Is the market open for trading right now? (game 07:00 <= hour < 19:00, §9.4).
static bool:Stocks_TradingOpen()
	return (GameHour >= STOCK_TRADING_OPEN_HOUR && GameHour < STOCK_TRADING_CLOSE_HOUR);

// The integer unit price a share trades at (rounded from the DOUBLE price, floored 1).
static Stocks_UnitPrice(stockid)
{
	new unit = floatround(StockMarket_Price(stockid));
	if(unit < 1) unit = 1;
	return unit;
}

// The 1% fee on a gross value (floored at 0; at least 1 on any non-trivial trade).
static Stocks_Fee(gross)
{
	if(gross <= 0) return 0;
	new fee = (gross / 100) * STOCK_TRADING_FEE_PCT;
	return fee;
}

//==============================================================================
//	The daily market tick (economy.md §7.2) - recompute prices, dividends, report
//==============================================================================

// Recompute one stock's price from its pool + drift, cap/floor it, then bleed the
// pool. Returns the fraction of the day's pool set aside for dividends (pre-bleed).
static Float:Stocks_TickPrice(stockid)
{
	new Float:pool = g_Stock[stockid][stPool];
	new Float:ipo  = g_Stock[stockid][stIPOPrice];

	// Base price from the accumulated pool (economy.md §7.2 "pool sets the price"),
	// anchored at the IPO price so an idle stock reverts toward IPO rather than 0.
	new Float:newPrice = ipo + (pool / STOCK_POOL_FACTOR) * STOCK_PRICE_FACTOR;

	// Random drift ±STOCK_DRIFT_MAX (market noise).
	new Float:drift = (float(random(2001)) / 1000.0 - 1.0) * STOCK_DRIFT_MAX;	// -DRIFT..+DRIFT
	newPrice *= (1.0 + drift);

	// Floor + ceiling (guards a runaway or a $0 share).
	if(newPrice < STOCK_PRICE_FLOOR) newPrice = STOCK_PRICE_FLOOR;
	if(newPrice > ipo * STOCK_PRICE_CEIL_MULT) newPrice = ipo * STOCK_PRICE_CEIL_MULT;
	g_Stock[stockid][stPrice] = newPrice;

	// Dividend allocation = a small % of the day's pool (paid before the bleed).
	new Float:dividendPot = (pool * float(STOCK_DIVIDEND_PCT)) / 100.0;

	// Bleed a fraction of the pool (mean reversion - activity fades over time).
	pool -= (pool * float(STOCK_POOL_BLEED_PCT)) / 100.0;
	if(pool < 0.0) pool = 0.0;
	g_Stock[stockid][stPool] = pool;

	return dividendPot;
}

// Pay a dividend from `pot` to every shareholder pro-rata (shares / sold shares).
// Shares sold = MaxShares - AvailableShares. Online holders get cash immediately;
// the cash is a source by design (like the lotto payout) - no mint from holdings.
static Stocks_PayDividends(stockid, Float:pot)
{
	if(pot < 1.0) return 0;
	new sold = g_Stock[stockid][stMaxShares] - g_Stock[stockid][stAvailable];
	if(sold <= 0) return 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || !PlayerInfo[i][pLoggedIn]) continue;
		new held = PlayerInfo[i][pStocks][stockid];
		if(held <= 0) continue;
		new payout = floatround(pot * (float(held) / float(sold)));
		if(payout < 1) continue;
		Bank_GiveCash(i, payout);
		new msg[144];
		format(msg, sizeof(msg), ""COL_GREEN"* Dividend: "COL_WHITE"$%d"COL_GREEN" from "COL_WHITE"%s"COL_GREEN" (%d shares)."COL_WHITE, payout, g_Stock[stockid][stName], held);
		SendClientMessage(i, COLOR_SERVER_INFO, msg);
	}
	return 1;
}

// Bankruptcy check (economy.md §7.2): a stock below IPO*20% relists at IPO and
// holders lose 20% of their shares. Minimal implementation (online holders + a
// bulk haircut UPDATE for offline holders). Returns 1 if the stock went bankrupt.
static Stocks_CheckBankruptcy(stockid)
{
	new Float:ipo = g_Stock[stockid][stIPOPrice];
	if(g_Stock[stockid][stPrice] >= ipo * (float(STOCK_BANKRUPTCY_FLOOR_PCT) / 100.0)) return 0;

	// Relist at IPO, reset the pool, restore only the ~20% haircut removed from holders.
	// `sold` is computed here (before the haircut loop below) so it reflects current holdings.
	new sold = g_Stock[stockid][stMaxShares] - g_Stock[stockid][stAvailable];
	g_Stock[stockid][stPrice] = ipo;
	g_Stock[stockid][stPool] = 0.0;
	g_Stock[stockid][stAvailable] += (sold * STOCK_BANKRUPTCY_HAIRCUT_PCT) / 100;

	// Haircut every holder by STOCK_BANKRUPTCY_HAIRCUT_PCT% (online cache + a bulk
	// UPDATE for the rest; the reduced offline rows are re-read on next login).
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || !PlayerInfo[i][pLoggedIn]) continue;
		new held = PlayerInfo[i][pStocks][stockid];
		if(held <= 0) continue;
		new keep = held - (held * STOCK_BANKRUPTCY_HAIRCUT_PCT) / 100;
		if(keep < 0) keep = 0;
		PlayerInfo[i][pStocks][stockid] = keep;
		Stocks_SaveHolding(PlayerInfo[i][pID], stockid, keep);
		new msg[160];
		format(msg, sizeof(msg), ""COL_RED"* %s went BANKRUPT and relisted "COL_WHITE"- you lost %d%% of your shares (%d left).", g_Stock[stockid][stName], STOCK_BANKRUPTCY_HAIRCUT_PCT, keep);
		SendClientMessage(i, COLOR_ERROR, msg);
	}
	// Offline holders: haircut their rows in one query (delete any that round to 0).
	new query[192];
	mysql_format(g_SQL, query, sizeof(query),
		"UPDATE `stock_holdings` SET Shares=Shares-FLOOR(Shares*%d/100) WHERE StockID=%d", STOCK_BANKRUPTCY_HAIRCUT_PCT, stockid);
	mysql_pquery(g_SQL, query);
	mysql_format(g_SQL, query, sizeof(query), "DELETE FROM `stock_holdings` WHERE StockID=%d AND Shares<=0", stockid);
	mysql_pquery(g_SQL, query);

	new bmsg[144];
	format(bmsg, sizeof(bmsg), ""COL_RED"[MARKET] "COL_WHITE"%s "COL_RED"crashed and went bankrupt - relisted at IPO ($%d).", g_Stock[stockid][stName], floatround(ipo));
	SendClientMessageToAll(COLOR_ERROR, bmsg);
	return 1;
}

// Move the prime rate a little toward where the aggregate market sits vs its IPO
// baseline (economy.md §1.4 "prime governs the whole economy"). Above-IPO market
// -> prime drifts up (goods cheaper, houses pricier); below -> down. Clamped + stepped.
static Stocks_MovePrimeRate()
{
	new Float:sumRatio = 0.0;
	for(new i = 0; i < MAX_STOCKS; i++)
	{
		new Float:ipo = g_Stock[i][stIPOPrice];
		if(ipo <= 0.0) { sumRatio += 1.0; continue; }
		sumRatio += g_Stock[i][stPrice] / ipo;
	}
	new Float:avgRatio = sumRatio / float(MAX_STOCKS);	// ~1.0 at baseline
	new Float:target = avgRatio;
	if(target < PRIME_RATE_MIN) target = PRIME_RATE_MIN;
	if(target > PRIME_RATE_MAX) target = PRIME_RATE_MAX;

	new Float:cur = ServerInfo[sPrimeRate];
	if(target > cur) cur += PRIME_RATE_STEP; else if(target < cur) cur -= PRIME_RATE_STEP;
	if(cur < PRIME_RATE_MIN) cur = PRIME_RATE_MIN;
	if(cur > PRIME_RATE_MAX) cur = PRIME_RATE_MAX;
	ServerInfo[sPrimeRate] = cur;
	SavePrimeRate();
	return 1;
}

// Write one report row per stock for this game-day, then trim to the last N days.
static Stocks_WriteReports(day)
{
	new query[160];
	for(new i = 0; i < MAX_STOCKS; i++)
	{
		mysql_format(g_SQL, query, sizeof(query),
			"REPLACE INTO `stock_reports` (StockID, Day, Price) VALUES (%d, %d, %f)", i, day, g_Stock[i][stPrice]);
		mysql_pquery(g_SQL, query);
	}
	// Trim history to the last STOCK_HISTORY_DAYS periods (drop older rows).
	mysql_format(g_SQL, query, sizeof(query), "DELETE FROM `stock_reports` WHERE Day <= %d", day - STOCK_HISTORY_DAYS);
	mysql_pquery(g_SQL, query);
	return 1;
}

// THE DAILY TICK - recompute every price from pool+drift, pay dividends, check
// bankruptcy, move the prime rate, write a report row, persist. Fired from
// GameModeClock at the game-day rollover (§10.3, guarded once-per-day) and by
// /astocktick. Uses the monotonic g_GameDayCounter as the report day.
FUNCTION Stocks_Tick()
{
	if(!g_StocksLoaded) return 0;
	new day = g_GameDayCounter;

	for(new i = 0; i < MAX_STOCKS; i++)
	{
		new Float:dividendPot = Stocks_TickPrice(i);
		Stocks_PayDividends(i, dividendPot);
		Stocks_CheckBankruptcy(i);	// may relist + haircut
		Stocks_SaveOne(i);			// persist price/pool/available
	}
	Stocks_MovePrimeRate();
	Stocks_WriteReports(day);

	new msg[144];
	format(msg, sizeof(msg), ""COL_YELLOW"[MARKET] "COL_WHITE"The stock market has settled for the day. Prime rate: "COL_GREEN"%.2f"COL_WHITE". Check "COL_GREEN"/markets"COL_WHITE".", ServerInfo[sPrimeRate]);
	SendClientMessageToAll(COLOR_SERVER_INFO, msg);
	return 1;
}

//==============================================================================
//	Trading - the shared buy/sell paths (all money via Bank_*, guarded)
//==============================================================================

// Buy `qty` shares of `stockid`. Validates hours, the share cap, availability and
// affordability (price*qty + 1% fee via cash-in-hand), moves the money and shares.
static Stocks_DoBuy(playerid, stockid, qty)
{
	if(!Stocks_TradingOpen())
		return SendClientMessage(playerid, COLOR_ERROR, "The market is closed. Trading hours are game 07:00-19:00.");
	if(stockid < 0 || stockid >= MAX_STOCKS)
		return SendClientMessage(playerid, COLOR_ERROR, "Invalid stock id. See /markets.");
	if(!PlayerInfo[playerid][pStocksLoaded])
		return SendClientMessage(playerid, COLOR_ERROR, "Your holdings are still loading - try again in a moment.");
	if(qty <= 0) return SendClientMessage(playerid, COLOR_ERROR, "Enter a positive number of shares to buy.");

	// Share cap per company (§9.4).
	new held = PlayerInfo[playerid][pStocks][stockid];
	if(held + qty > STOCK_MAX_OWN)
	{
		new err[128];
		format(err, sizeof(err), "You may hold at most %d shares of a company (you hold %d).", STOCK_MAX_OWN, held);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	if(qty > g_Stock[stockid][stAvailable])
	{
		new err[112];
		format(err, sizeof(err), "Only %d shares are available on the market right now.", g_Stock[stockid][stAvailable]);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}

	// Cost = unit price * qty + 1% fee. Guard the multiply against overflow.
	new unit = Stocks_UnitPrice(stockid);
	if(qty > MAX_MONEY / unit)
		return SendClientMessage(playerid, COLOR_ERROR, "That order is too large.");
	new gross = unit * qty;
	new fee = Stocks_Fee(gross);
	if(gross > MAX_MONEY - fee)
		return SendClientMessage(playerid, COLOR_ERROR, "That order is too large.");
	new total = gross + fee;

	if(Bank_GetCash(playerid) < total)
	{
		new err[144];
		format(err, sizeof(err), "You need "COL_GREEN"$%d"COL_WHITE" cash-in-hand (%d @ $%d + 1%% fee $%d). Withdraw first.", total, qty, unit, fee);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	if(!Bank_TakeCash(playerid, total))
		return SendClientMessage(playerid, COLOR_ERROR, "Payment failed.");

	// Move the shares: out of the market float, into the holding.
	g_Stock[stockid][stAvailable] -= qty;
	PlayerInfo[playerid][pStocks][stockid] = held + qty;
	Stocks_SaveHolding(PlayerInfo[playerid][pID], stockid, held + qty);
	Stocks_SaveOne(stockid);

	// Buying is demand -> feed the pool (pushes the price up, §7.2). Factor 0.25 like
	// the shop/ammu buy activity - the gross spend flows into the company pool.
	StockMarket_UpdateEarnings(stockid, gross, STOCK_FACTOR_SHOP);

	new msg[176];
	format(msg, sizeof(msg), ""COL_GREEN"* Bought "COL_WHITE"%d"COL_GREEN" shares of "COL_WHITE"%s"COL_GREEN" for "COL_WHITE"$%d"COL_GREEN" (fee $%d). You now hold "COL_WHITE"%d"COL_GREEN"."COL_WHITE, qty, g_Stock[stockid][stName], total, fee, held + qty);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}

// Sell `qty` shares of `stockid`. Validates hours + that the player holds them,
// credits proceeds - 1% fee, returns the shares to the market float.
static Stocks_DoSell(playerid, stockid, qty)
{
	if(!Stocks_TradingOpen())
		return SendClientMessage(playerid, COLOR_ERROR, "The market is closed. Trading hours are game 07:00-19:00.");
	if(stockid < 0 || stockid >= MAX_STOCKS)
		return SendClientMessage(playerid, COLOR_ERROR, "Invalid stock id. See /markets.");
	if(!PlayerInfo[playerid][pStocksLoaded])
		return SendClientMessage(playerid, COLOR_ERROR, "Your holdings are still loading - try again in a moment.");
	if(qty <= 0) return SendClientMessage(playerid, COLOR_ERROR, "Enter a positive number of shares to sell.");

	new held = PlayerInfo[playerid][pStocks][stockid];
	if(held <= 0) return SendClientMessage(playerid, COLOR_ERROR, "You do not own any shares of that company.");
	if(qty > held) qty = held;	// clamp to holdings (can never oversell)

	new unit = Stocks_UnitPrice(stockid);
	if(qty > MAX_MONEY / unit)
		return SendClientMessage(playerid, COLOR_ERROR, "That order is too large.");
	new gross = unit * qty;
	new fee = Stocks_Fee(gross);
	new proceeds = gross - fee;
	if(proceeds < 0) proceeds = 0;

	// Move the shares back to the market float and credit the proceeds.
	PlayerInfo[playerid][pStocks][stockid] = held - qty;
	if(g_Stock[stockid][stAvailable] <= g_Stock[stockid][stMaxShares] - qty)
		g_Stock[stockid][stAvailable] += qty;
	else
		g_Stock[stockid][stAvailable] = g_Stock[stockid][stMaxShares];
	Stocks_SaveHolding(PlayerInfo[playerid][pID], stockid, held - qty);
	if(proceeds > 0) Bank_GiveCash(playerid, proceeds);

	// Selling is supply -> pull the pool down (dumping depresses the price, §7.2).
	StockMarket_UpdateEarnings(stockid, gross, -STOCK_FACTOR_SHOP);
	Stocks_SaveOne(stockid);

	new msg[176];
	format(msg, sizeof(msg), ""COL_GREEN"* Sold "COL_WHITE"%d"COL_GREEN" shares of "COL_WHITE"%s"COL_GREEN" for "COL_WHITE"$%d"COL_GREEN" (fee $%d). You now hold "COL_WHITE"%d"COL_GREEN"."COL_WHITE, qty, g_Stock[stockid][stName], proceeds, fee, held - qty);
	SendClientMessage(playerid, COLOR_SERVER_INFO, msg);
	return 1;
}

//==============================================================================
//	Commands
//==============================================================================

// /markets (/market) - list all stocks: id, name, price, %vs IPO, your holding.
CMD:markets(playerid, params[])
{
	#pragma unused params
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(!g_StocksLoaded) return SendClientMessage(playerid, COLOR_ERROR, "The market is still loading - try again in a moment.");

	new hdr[144];
	format(hdr, sizeof(hdr), ""COL_YELLOW"===== STOCK MARKET =====  "COL_WHITE"%s (prime "COL_GREEN"%.2f"COL_WHITE"). "COL_GREEN"/shares <id> <qty>"COL_WHITE" buy, "COL_GREEN"/sharessell <id> <qty>"COL_WHITE" sell.",
		Stocks_TradingOpen() ? "OPEN" : "CLOSED (07:00-19:00)", ServerInfo[sPrimeRate]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, hdr);

	for(new i = 0; i < MAX_STOCKS; i++)
	{
		new Float:ipo = g_Stock[i][stIPOPrice];
		new pct = (ipo > 0.0) ? floatround((g_Stock[i][stPrice] / ipo - 1.0) * 100.0) : 0;
		new line[144];
		format(line, sizeof(line), ""COL_WHITE"%2d. "COL_GREEN"%-22s"COL_WHITE" $%d  ("COL_YELLOW"%s%d%%"COL_WHITE")  held: "COL_GREEN"%d"COL_WHITE,
			i, g_Stock[i][stName], Stocks_UnitPrice(i), (pct >= 0) ? "+" : "", pct, PlayerInfo[playerid][pStocks][i]);
		SendClientMessage(playerid, COLOR_WHITE, line);
	}
	return 1;
}
alias:markets("market")

// /shares (/stocks) - with no/"list" args: show your holdings + values. With
// "<id> <qty>" or "buy <id> <qty>": buy that many shares (§9.4).
CMD:shares(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(!g_StocksLoaded) return SendClientMessage(playerid, COLOR_ERROR, "The market is still loading - try again in a moment.");

	new action[8], sid, qty;
	// Accept: "/shares", "/shares <id> <qty>", "/shares buy <id> <qty>".
	if(!sscanf(params, "s[8]dd", action, sid, qty) && !strcmp(action, "buy", true))
		return Stocks_DoBuy(playerid, sid, qty);
	if(!sscanf(params, "dd", sid, qty))
		return Stocks_DoBuy(playerid, sid, qty);

	// No buy args -> show holdings.
	if(!PlayerInfo[playerid][pStocksLoaded]) return SendClientMessage(playerid, COLOR_ERROR, "Your holdings are still loading - try again in a moment.");
	SendClientMessage(playerid, COLOR_SERVER_INFO, ""COL_YELLOW"===== YOUR SHARES =====  "COL_WHITE"("COL_GREEN"/shares <id> <qty>"COL_WHITE" buy, "COL_GREEN"/sharessell <id> <qty>"COL_WHITE" sell)");
	new any = 0;
	for(new i = 0; i < MAX_STOCKS; i++)
	{
		new held = PlayerInfo[playerid][pStocks][i];
		if(held <= 0) continue;
		any = 1;
		new value = 0, unit = Stocks_UnitPrice(i);
		if(held <= MAX_MONEY / unit) value = unit * held;	// overflow-safe value display
		new line[144];
		format(line, sizeof(line), ""COL_WHITE"%2d. "COL_GREEN"%-22s"COL_WHITE" %d shares @ $%d = "COL_GREEN"$%d"COL_WHITE, i, g_Stock[i][stName], held, unit, value);
		SendClientMessage(playerid, COLOR_WHITE, line);
	}
	if(!any) SendClientMessage(playerid, COLOR_WHITE, "You do not own any shares. Buy some with "COL_GREEN"/shares <id> <qty>"COL_WHITE" (see "COL_GREEN"/markets"COL_WHITE").");
	return 1;
}
alias:shares("stocks")

// /sharessell (/stockssell) <id> <qty> - sell shares you hold (§9.4).
CMD:sharessell(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(!g_StocksLoaded) return SendClientMessage(playerid, COLOR_ERROR, "The market is still loading - try again in a moment.");
	new sid, qty;
	if(sscanf(params, "dd", sid, qty))
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /sharessell <stock id> <qty>  (see /markets for ids)");
	return Stocks_DoSell(playerid, sid, qty);
}
alias:sharessell("stockssell")

// /markethistory <id> - show the last N daily closing prices for a stock (§7.2).
CMD:markethistory(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");
	if(!g_StocksLoaded) return SendClientMessage(playerid, COLOR_ERROR, "The market is still loading - try again in a moment.");
	new sid;
	if(sscanf(params, "d", sid) || sid < 0 || sid >= MAX_STOCKS)
		return SendClientMessage(playerid, COLOR_SERVER_INFO, "USAGE: /markethistory <stock id>  (see /markets for ids)");
	new query[128];
	mysql_format(g_SQL, query, sizeof(query), "SELECT Day, Price FROM `stock_reports` WHERE StockID=%d ORDER BY Day DESC LIMIT %d", sid, STOCK_HISTORY_DAYS);
	mysql_pquery(g_SQL, query, "Stocks_OnHistory", "dd", playerid, sid);
	return 1;
}

// Async handler for /markethistory - print the rows.
FUNCTION Stocks_OnHistory(playerid, stockid)
{
	if(!IsPlayerConnected(playerid) || !PlayerInfo[playerid][pLoggedIn]) return 0;
	new rows;
	cache_get_row_count(rows);
	new sName[40];
	if(stockid >= 0 && stockid < MAX_STOCKS) format(sName, sizeof(sName), "%s", g_Stock[stockid][stName]);
	else format(sName, sizeof(sName), "?");
	new hdr[96];
	format(hdr, sizeof(hdr), ""COL_YELLOW"===== %s - price history ====="COL_WHITE, sName);
	SendClientMessage(playerid, COLOR_SERVER_INFO, hdr);
	if(rows == 0) return SendClientMessage(playerid, COLOR_WHITE, "No history yet - the first report is written on the daily market tick.");
	for(new i = 0; i < rows; i++)
	{
		new day; new Float:price;
		cache_get_value_int(i, "Day", day);
		cache_get_value_float(i, "Price", price);
		new line[80];
		format(line, sizeof(line), ""COL_WHITE"Day %d: "COL_GREEN"$%d"COL_WHITE, day, floatround(price));
		SendClientMessage(playerid, COLOR_WHITE, line);
	}
	return 1;
}

// /astocktick (scripter) - force a market tick for testing (§10.3, like /alotto).
CMD:astocktick(playerid, params[])
{
	#pragma unused params
	if(PlayerInfo[playerid][pRank] < SERVER_SCRIPTER) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new string[128];
	format(string, sizeof(string), "%s (%d) forced a stock market tick.", PlayerInfo[playerid][pUserName], playerid);
	SendMessageToAdmins(COLOR_ADMIN_INFO, string);
	Stocks_Tick();
	return 1;
}
