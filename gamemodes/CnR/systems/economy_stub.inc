/*

	Stock market CORE (config + state + price/pool functions) for Cops And Robbers
	Scripted by Arose Niazi.
	Milestone 7 — the stock market + full economy (design §9.4/§10, economy.md §7).

	This file was the M6 economy_stub — a NO-OP placeholder so the fishing/farming
	sell paths could already wire the "activity feeds the market pool" calls. M7
	REPLACES the two no-op bodies with the REAL implementation. It stays included
	FIRST (before fishing/farming, which call StockMarket_UpdateEarnings /
	StockMarket_PriceMult) so those call sites resolve against the real functions —
	no duplicate definition (the heavy machinery — init, async CRUD, the daily tick,
	dividends, reports and the /markets//shares//sharessell trading commands — lives
	in systems/stocks.inc, included later; it operates on the state declared HERE).

	THE MODEL (economy.md §7.2, sf-cnr pattern):
	  - Each stock has a running POOL of net economic activity. Every relevant
	    transaction calls StockMarket_UpdateEarnings(stockid, cash, factor) which
	    does POOL += cash*factor (clamped POOL >= 0). Buy activity pushes the price
	    up; selling/dumping pushes it down (a negative factor is allowed).
	  - On the daily tick (Stocks_Tick, systems/stocks.inc) each price is recomputed
	    from its POOL + a small drift, a fraction of the pool is bled off, dividends
	    are paid, a report row is written and the prime rate is nudged.
	  - StockMarket_PriceMult(stockid) = clamp(Price/IPOPrice, 0.5, 2.0) — the
	    per-domain multiplier every price site reads (§10.1).

	Public API:
		StockMarket_UpdateEarnings(stockid, amount, factor) : feed the market pool (real)
		StockMarket_PriceMult(stockid)                      : per-domain price multiplier (real)
		StockMarket_Price(stockid)                          : raw current price (DOUBLE)
	State (defined here, driven by systems/stocks.inc):
		g_Stock[MAX_STOCKS][e_Stock]                        : the loaded market
*/

//==============================================================================
//	Stock ids (index into the stocks table). APPEND, never renumber — the DB and
//	the report history key off these. The first 7 were named in M6 (the sell
//	paths reference them); M7 fills out the full CB roster of 21 (economy.md §7.1).
//==============================================================================

enum
{
	E_STOCK_GOVERNMENT,			// 0 — Government Bonds (arrests/bail/tickets feed this, §10.2)
	E_STOCK_HOUSE_MARKET,		// 1 — House Market (house buys × this × prime, §10.1)
	E_STOCK_FISH_MARKET,		// 2 — Fish Market (fish sells feed this, §10.2)
	E_STOCK_DRUG,				// 3 — Drug (drug/crop sells feed this, §10.2)
	E_STOCK_AMMUNATION,			// 4 — Ammunation (weapon buys feed this, §10.2)
	E_STOCK_SUPA_SAVE,			// 5 — 24/7 Corporation (24/7 buys feed this, §10.2)
	E_STOCK_VEHICLE_DEALERSHIP,	// 6 — Vehicle dealership (buys/mods/jack-sell feed this)
	// M7 — the remaining 14 CB stocks (economy.md §7.1). Named companies + generic.
	E_STOCK_POLICE_DEPARTMENT,	// 7  — Police Department
	E_STOCK_HOSPITAL,			// 8  — Hospital
	E_STOCK_CLUCKIN_BELL,		// 9  — Cluckin' Bell
	E_STOCK_WELL_STACKED_PIZZA,	// 10 — Well Stacked Pizza
	E_STOCK_BURGER_SHOT,		// 11 — Burger Shot
	E_STOCK_SEX_SHOPS,			// 12 — Sex Shops
	E_STOCK_BARS_DANCE_CLUBS,	// 13 — Bars & Dance Clubs
	E_STOCK_DINERS,				// 14 — Diners
	E_STOCK_STRIP_CLUBS,		// 15 — Strip Clubs
	E_STOCK_BAIT_SHOPS,			// 16 — Bait Shops
	E_STOCK_OFF_TRACK_BETTING,	// 17 — Off Track Betting
	E_STOCK_GAMBLING,			// 18 — Gambling
	E_STOCK_TRANSPORT,			// 19 — Transport
	E_STOCK_CLOTHING,			// 20 — Clothing
	MAX_STOCKS					// 21 — array bound (must stay last)
}

//==============================================================================
//	Config (design §9.4/§10, economy.md §7.2)
//==============================================================================

#define STOCK_TRADING_OPEN_HOUR		7			// trading window opens (game 07:00, §9.4)
#define STOCK_TRADING_CLOSE_HOUR	19			// trading window closes (game 19:00 — frozen ≥19:00)
#define STOCK_MAX_OWN				100000		// max shares held per company (§9.4)
#define STOCK_TRADING_FEE_PCT		1			// 1% trading fee on buys AND sells (§9.4)
#define STOCK_HISTORY_DAYS			30			// report rows kept per stock (§7.2 "last 30 periods")

// Price model constants (economy.md §7.2 — POOL drives the price).
#define STOCK_PRICE_FLOOR			1.0			// price never drops below this (share > $0)
#define STOCK_POOL_FACTOR			200000.0	// pool→price divisor (bigger = slower price moves)
#define STOCK_PRICE_FACTOR			1.0			// pool contribution scale
#define STOCK_DRIFT_MAX				0.04		// ±4% random drift each tick (market noise)
#define STOCK_POOL_BLEED_PCT		25			// % of the pool bled off after each tick (mean reversion)
#define STOCK_DIVIDEND_PCT			2			// % of the day's pool paid to shareholders as a dividend
#define STOCK_PRICE_CEIL_MULT		4.0			// price hard-capped at IPO*this (guards runaway)

// Bankruptcy (economy.md §7.2): a stock that craters to <20% of IPO relists at IPO
// and holders lose STOCK_BANKRUPTCY_HAIRCUT_PCT% of their shares.
#define STOCK_BANKRUPTCY_FLOOR_PCT	20			// price < IPO*20% → bankrupt/relist
#define STOCK_BANKRUPTCY_HAIRCUT_PCT 20			// holders lose 20% of shares on a relist

// Price-multiplier clamp (design §10.1: clamp(price/IPO, 0.5, 2.0)).
#define STOCK_MULT_MIN				0.5
#define STOCK_MULT_MAX				2.0

// Prime rate (server_data.PrimeRate, §10.1). Moves a little each tick with the
// aggregate market; clamped to this range. Higher prime = lower goods, higher houses.
#define PRIME_RATE_MIN				0.75
#define PRIME_RATE_MAX				1.50
#define PRIME_RATE_STEP				0.02		// max move per daily tick

// Fish/drug sale market factors (economy.md §7.2/§8.3/§10.2 — supply pushes price).
// Selling feeds the pool with a POSITIVE factor: heavy dumping raises supply which
// (via the price recompute + the sale competing for the pool) depresses the multiplier
// relative to a scarce market. Kept from M6 so the existing call sites are unchanged.
#define STOCK_FACTOR_FISH			0.15		// fish sale → Fish Market pool (§10.2 / §8.3)
#define STOCK_FACTOR_DRUG			0.25		// drug/crop sale → Drug pool (§10.2 dumping lowers)

// Buy-activity factors (economy.md §7.2 real sf-cnr call sites, §10.2).
#define STOCK_FACTOR_SHOP			0.25		// 24/7 buy → SUPA_SAVE pool
#define STOCK_FACTOR_AMMU			0.25		// weapon buy → AMMUNATION pool
#define STOCK_FACTOR_GOV			0.10		// arrest/bail/ticket → GOVERNMENT pool

//==============================================================================
//	Market state (the loaded stocks table). Declared HERE so the price/pool
//	functions below and the machinery in systems/stocks.inc share one array.
//==============================================================================

enum e_Stock
{
	stName[40],				// display name
	Float:stPrice,			// current share price
	Float:stPool,			// running activity reservoir (>=0)
	stAvailable,			// shares left to buy from the market
	Float:stIPOPrice,		// initial price (multiplier baseline)
	stIPOShares,			// initial float (dilution scaling)
	stMaxShares				// total shares in existence
}
new g_Stock[MAX_STOCKS][e_Stock];
new bool:g_StocksLoaded = false;	// true once the stocks table has loaded (Stocks_OnLoad)

//==============================================================================
//	Core price/pool functions (REAL — replace the M6 no-ops)
//==============================================================================

// Feed the market pool for `stockid` with `amount * factor` (economy.md §7.2).
// POOL is clamped >= 0 so a net-negative activity stream cannot drive it negative.
// A no-op if the market has not loaded yet (early boot) or the id is out of range.
FUNCTION StockMarket_UpdateEarnings(stockid, amount, Float:factor)
{
	if(!g_StocksLoaded) return 0;
	if(stockid < 0 || stockid >= MAX_STOCKS) return 0;
	new Float:delta = float(amount) * factor;
	new Float:pool = g_Stock[stockid][stPool] + delta;
	if(pool < 0.0) pool = 0.0;
	g_Stock[stockid][stPool] = pool;
	return 1;
}

// Raw current price of a stock (DOUBLE). Floored at STOCK_PRICE_FLOOR.
stock Float:StockMarket_Price(stockid)
{
	if(stockid < 0 || stockid >= MAX_STOCKS || !g_StocksLoaded) return STOCK_PRICE_FLOOR;
	if(g_Stock[stockid][stPrice] < STOCK_PRICE_FLOOR) return STOCK_PRICE_FLOOR;
	return g_Stock[stockid][stPrice];
}

// Per-domain price multiplier (design §10.1): clamp(price / IPOprice, 0.5, 2.0).
// Every price site (shop/weapon/house/fish/drug) multiplies its base by this so
// prices move with the market. Returns 1.0 if the market has not loaded yet.
stock Float:StockMarket_PriceMult(stockid)
{
	if(stockid < 0 || stockid >= MAX_STOCKS || !g_StocksLoaded) return 1.0;
	new Float:ipo = g_Stock[stockid][stIPOPrice];
	if(ipo <= 0.0) return 1.0;
	new Float:m = StockMarket_Price(stockid) / ipo;
	if(m < STOCK_MULT_MIN) m = STOCK_MULT_MIN;
	if(m > STOCK_MULT_MAX) m = STOCK_MULT_MAX;
	return m;
}

// Convenience: a base buy-price scaled by a domain's market multiplier (§10.1
// shopPrice = base * mult(stock)). Floored at $1. Used by shop/weapon buy sites.
stock StockMarket_BuyPrice(base, stockid)
{
	new scaled = floatround(float(base) * StockMarket_PriceMult(stockid));
	return (scaled < 1) ? 1 : scaled;
}
