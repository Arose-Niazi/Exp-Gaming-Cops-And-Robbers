/*

	Economy stub for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 6 (stage 2) — a NO-OP placeholder for the M7 stock market so the
	fishing/farming (and any earlier) sell paths can already wire the "activity
	feeds the market pool" call today (design §10.1/§10.2, economy.md §7.2).

	M7 (systems/stocks.inc) will REPLACE this stub with the real implementation:
	    StockMarket_UpdateEarnings(stockid, amount, factor):
	        POOL[stockid] += amount * factor      // clamp POOL >= 0
	which drives the per-stock price formula (economy.md §7.2). Until then every
	sell just calls this no-op so the call sites compile and are correctly placed.

	Stock ids (E_STOCK_*) mirror the 21 CB stocks (economy.md §7.1). Only the ones
	referenced by the shipped sell paths are enumerated here; M7 fills out the rest.
	Fish sells use FISH_MARKET with a base mult of 1.0 (factor tuned per §10.2);
	drug sells use DRUG. The base market MULTIPLIER on the sale price is 1.0 for now
	(the stub returns 1.0) — M7 replaces StockMarket_PriceMult with a real lookup.

	Public API (called by fishing.inc / farming.inc now; other sell paths later):
		StockMarket_UpdateEarnings(stockid, amount, factor)  : feed the market pool (no-op stub)
		StockMarket_PriceMult(stockid)                       : per-domain price multiplier (returns 1.0 stub)
*/

// Stock ids (index into the M7 stocks table). APPEND, never renumber — the DB and
// M7 report history key off these. Only the M6 sell-path stocks are named here.
enum
{
	E_STOCK_GOVERNMENT,		// 0 — Government Bonds (arrests/bail/tickets feed this, M7)
	E_STOCK_HOUSE_MARKET,	// 1 — House Market
	E_STOCK_FISH_MARKET,	// 2 — Fish Market (fish sells feed this, §10.2)
	E_STOCK_DRUG,			// 3 — Drug (drug/crop sells feed this, §10.2)
	E_STOCK_AMMUNATION,		// 4 — Ammunation (weapon buys feed this, M7)
	E_STOCK_SUPA_SAVE,		// 5 — 24/7 Corporation (24/7 buys feed this, M7)
	E_STOCK_VEHICLE_DEALERSHIP	// 6 — Vehicle dealership
	// … the remaining 14 CB stocks are added in M7 (systems/stocks.inc).
}

// Fish/drug sale market factors (economy.md §7.2/§8.3/§10.2 — supply pushes price):
// selling fish/drugs feeds the pool so heavy dumping depresses the price.
#define STOCK_FACTOR_FISH		0.15		// fish sale → Fish Market pool (§10.2 / §8.3)
#define STOCK_FACTOR_DRUG		0.25		// drug/crop sale → Drug pool (§10.2 dumping lowers) [OUR DESIGN factor]

// NO-OP: feed the market pool for `stockid` with `amount * factor`. M7 replaces the
// body with the real running-pool accumulation (economy.md §7.2). Kept a FUNCTION
// (forward+public) so M7 can override it in place without touching the call sites.
FUNCTION StockMarket_UpdateEarnings(stockid, amount, Float:factor)
{
	#pragma unused stockid
	#pragma unused amount
	#pragma unused factor
	// no-op until M7 (systems/stocks.inc) implements the pool accumulation.
	return 1;
}

// Per-domain price multiplier used by the sell/price formulas (design §10.1). The
// stub returns 1.0 (FISH_MARKET base mult 1.0 / DRUG mult 1.0, per the brief); M7
// replaces this with clamp(stockPrice/IPO_PRICE, 0.5, 2.0).
stock Float:StockMarket_PriceMult(stockid)
{
	#pragma unused stockid
	return 1.0;
}
