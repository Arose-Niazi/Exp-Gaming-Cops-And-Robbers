/*

	Lotto (pick-a-number lottery) for Cops And Robbers script
	Scripted by Arose Niazi.
	Milestone 4 (stage 2) - design §9.5 (lotto) + §10.3 (the clock-tick draw
	fix) + §11.4 (server_data.LottoJackpot) + docs/research/economy.md §4.

	Pick-a-number lotto: /lotto <1..LOTTO_MAX> buys one ticket for the current
	game-day (LOTTO_TICKET_PRICE via Bank_TakeCash; the number is stored in
	LSplayers.LottoNumber). Ticket sales feed the accumulating jackpot held in
	server_data.LottoJackpot (ServerInfo[sLottoJackpot]).

	THE CLOCK-TICK FIX (§10.3): the draw is bound to the IN-GAME clock, not a
	real-time timer - this is the legacy "lottery timer bug" fix. Lotto_Draw()
	fires from GameModeClock()'s hour transition when GameHour hits
	LOTTO_DRAW_HOUR (18), guarded by a static "last drawn game-day" so it fires
	exactly once per game-day and never double-fires on a skipped/relogged hour.
	The guard resets on the game-week rollover (handled in CnR.pwn).

	Lotto_Draw() picks a winning number 1..LOTTO_MAX, pays every holder of that
	number an equal split of the jackpot (via Bank_GiveCash) + score, announces
	the result, and clears everyone's ticket for the next day. If nobody holds
	the number the jackpot ROLLS OVER (kept + topped to the base) for next draw.

	Credits:
		economy.md §4 for the lotto mechanic grounding.

	Public API:
		Lotto_Draw()                 : run a draw (clock hour 18 / /alotto force)
		Lotto_ResetDayGuard()        : (none - the guard is static in CnR.pwn)

	Commands:
		/lotto <1..125>   (player)  buy a ticket for the game-day (§9.5)
		/alotto           (scripter) force-run Lotto_Draw() for testing (§8.3/§10.3)

	Functions Being Called:
		bank:        Bank_TakeCash, Bank_GiveCash
		jail:        Jail_SavePlayer (shared LSplayers persistence)
		server_vars: SaveLottoJackpot
*/

//==============================================================================
//	Internal helpers
//==============================================================================

// Add a ticket sale to the jackpot (overflow-guarded) and persist it.
static Lotto_FeedJackpot(amount)
{
	if(amount <= 0) return 0;
	if(ServerInfo[sLottoJackpot] > MAX_MONEY - amount) return 0;	// overflow guard
	ServerInfo[sLottoJackpot] += amount;
	SaveLottoJackpot();
	return 1;
}

//==============================================================================
//	The draw (design §9.5 / §10.3) - called from the game-clock hour transition
//==============================================================================

// Run one lotto draw. Picks a winning number, pays every holder of it an equal
// split of the jackpot, +score, announces, and clears all tickets. No winner =>
// the jackpot rolls over (kept and floored at the base). This is the function
// GameModeClock() calls at game-hour 18 (the clock-tick fix) and /alotto forces.
FUNCTION Lotto_Draw()
{
	new winningNumber = 1 + random(LOTTO_MAX);	// 1..LOTTO_MAX

	// Count winners (holders of the drawn number).
	new winners = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || !PlayerInfo[i][pLoggedIn]) continue;
		if(PlayerInfo[i][pLottoNumber] == winningNumber) winners++;
	}

	new jackpot = ServerInfo[sLottoJackpot];
	new string[160];

	if(winners == 0)
	{
		// Nobody won - roll the jackpot over to the next draw (kept, floored).
		if(ServerInfo[sLottoJackpot] < LOTTO_JACKPOT_BASE) ServerInfo[sLottoJackpot] = LOTTO_JACKPOT_BASE;
		SaveLottoJackpot();
		format(string, sizeof(string), ""COL_YELLOW"[LOTTO] "COL_WHITE"The number was "COL_GREEN"%d"COL_WHITE" - no winner! The "COL_GREEN"$%d"COL_WHITE" jackpot rolls over.", winningNumber, jackpot);
		SendClientMessageToAll(COLOR_SERVER_INFO, string);
	}
	else
	{
		// Split the jackpot equally between winners (integer division floors it).
		new share = jackpot / winners;
		if(share < 1) share = 1;
		format(string, sizeof(string), ""COL_YELLOW"[LOTTO] "COL_WHITE"The number was "COL_GREEN"%d"COL_WHITE"! "COL_GREEN"%d"COL_WHITE" winner(s) split "COL_GREEN"$%d"COL_WHITE" (%d each).", winningNumber, winners, jackpot, share);
		SendClientMessageToAll(COLOR_SERVER_INFO, string);

		for(new i = 0; i < MAX_PLAYERS; i++)
		{
			if(!IsPlayerConnected(i) || IsPlayerNPC(i) || !PlayerInfo[i][pLoggedIn]) continue;
			if(PlayerInfo[i][pLottoNumber] != winningNumber) continue;
			Bank_GiveCash(i, share);
			PlayerInfo[i][pScore] += LOTTO_WIN_SCORE + LOTTO_PLAY_SCORE;
			new msg[144];
			format(msg, sizeof(msg), ""COL_GREEN"* You WON the lotto! "COL_WHITE"+"COL_GREEN"$%d"COL_WHITE" and +%d score.", share, LOTTO_WIN_SCORE + LOTTO_PLAY_SCORE);
			SendClientMessage(i, COLOR_SERVER_INFO, msg);
		}

		// Jackpot paid out - reset to the base for the next round.
		ServerInfo[sLottoJackpot] = LOTTO_JACKPOT_BASE;
		SaveLottoJackpot();
	}

	// Clear every ticket. Online players are zeroed in memory so the next
	// Jail_SavePlayer persists it; then one bulk UPDATE clears ALL rows
	// (offline holders included) so a paid-once ticket can never re-enter a
	// future draw without re-buying.
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		PlayerInfo[i][pLottoNumber] = 0;
	}
	new clearQuery[80];
	format(clearQuery, sizeof(clearQuery), "UPDATE "TABLE_PLAYERS_C" SET LottoNumber=0 WHERE LottoNumber<>0");
	mysql_pquery(g_SQL, clearQuery);
	return 1;
}

//==============================================================================
//	Command - /lotto <1..LOTTO_MAX> : buy a ticket for the game-day (§9.5)
//==============================================================================

CMD:lotto(playerid, params[])
{
	if(!PlayerInfo[playerid][pLoggedIn]) return SendClientMessage(playerid, COLOR_ERROR, "You must be logged in.");

	new number;
	if(sscanf(params, "d", number))
	{
		new usage[176];
		format(usage, sizeof(usage), "USAGE: /lotto [number 1-%d]  ("COL_GREEN"$%d"COL_WHITE"/ticket, drawn at "COL_GREEN"%02d:00"COL_WHITE" game-time. Jackpot: "COL_GREEN"$%d"COL_WHITE")", LOTTO_MAX, LOTTO_TICKET_PRICE, LOTTO_DRAW_HOUR, ServerInfo[sLottoJackpot]);
		SendClientMessage(playerid, COLOR_SERVER_INFO, usage);
		if(PlayerInfo[playerid][pLottoNumber] != 0)
		{
			new held[96];
			format(held, sizeof(held), "You are holding lotto number "COL_GREEN"%d"COL_WHITE" for today's draw.", PlayerInfo[playerid][pLottoNumber]);
			SendClientMessage(playerid, COLOR_SERVER_INFO, held);
		}
		return 1;
	}

	if(number < 1 || number > LOTTO_MAX)
	{
		new err[80];
		format(err, sizeof(err), "Pick a number between 1 and %d.", LOTTO_MAX);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}
	if(PlayerInfo[playerid][pLottoNumber] != 0)
		return SendClientMessage(playerid, COLOR_ERROR, "You already bought a ticket for today's draw - one ticket per game-day.");

	// Charge the ticket from cash-in-hand (feeds the jackpot).
	if(!Bank_TakeCash(playerid, LOTTO_TICKET_PRICE))
	{
		new err[96];
		format(err, sizeof(err), "You cannot afford a ticket ($%d). Carry more cash.", LOTTO_TICKET_PRICE);
		return SendClientMessage(playerid, COLOR_ERROR, err);
	}

	PlayerInfo[playerid][pLottoNumber] = number;
	Lotto_FeedJackpot(LOTTO_TICKET_PRICE);
	Jail_SavePlayer(playerid);	// persist the ticket (shared LSplayers UPDATE)

	new string[176];
	format(string, sizeof(string), ""COL_GREEN"* Lotto ticket bought "COL_WHITE"($%d)"COL_GREEN" - your number is "COL_WHITE"%d"COL_GREEN". Draw at "COL_WHITE"%02d:00"COL_GREEN" game-time. Jackpot: "COL_WHITE"$%d"COL_GREEN".", LOTTO_TICKET_PRICE, number, LOTTO_DRAW_HOUR, ServerInfo[sLottoJackpot]);
	SendClientMessage(playerid, COLOR_SERVER_INFO, string);
	return 1;
}

//==============================================================================
//	Command - /alotto (scripter) : force a draw for testing (§8.3 / §10.3)
//==============================================================================

CMD:alotto(playerid, params[])
{
	#pragma unused params
	if(PlayerInfo[playerid][pRank] < SERVER_SCRIPTER) return SendClientMessage(playerid, COLOR_ERROR, ERROR_NOT_ADMIN);
	new string[128];
	format(string, sizeof(string), "%s (%d) forced a lotto draw (jackpot $%d).", PlayerInfo[playerid][pUserName], playerid, ServerInfo[sLottoJackpot]);
	SendMessageToAdmins(COLOR_ADMIN_INFO, string);
	Lotto_Draw();
	return 1;
}
