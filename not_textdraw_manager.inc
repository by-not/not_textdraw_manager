/*
	=====================================================
	NOT_TEXTDRAW_MANAGER.INC - TextDraw & Window Manager
	=====================================================
	Author: not
	Version: 1.0
	Date: March 2026
	License: MIT

	DESCRIPTION:
	Centralized manager for TextDraw cursor selection, active UI window
	tracking, and safe TextDraw destruction for SA-MP and open.mp.
	Solves common TextDraw UI issues:
	- Cursor desynchronization and stuck selection mode (SelectTextDraw)
	- Centralized ESC cancellation (INVALID_TEXT_DRAW) without hook conflicts
	- Programmatic close vs. user ESC distinction (no false cancel callbacks)
	- Single active window registry per player (Inventory, Panels, Menus, etc.)
	- Safe TextDraw destruction with automatic handle reset
	- Clean ALS hooks compatible with YSI (y_hooks) and other libraries

	MAIN FEATURES:
	- Real-time cursor selection tracking (IsPlayerSelectingTD)
	- Active window tracking per player (ShowPlayerTDWindow / HidePlayerTDWindow)
	- Automatic ESC routing to OnPlayerCancelTDWindow / OnPlayerCancelTD
	- Configurable click debounce (never blocks ESC cancellation)
	- Automatic state cleanup on OnPlayerConnect and OnPlayerDisconnect
	- Safe PlayerTextDraw and global TextDraw destruction helpers

	BASIC CONFIGURATION:
	```pawn
	#define NTDM_DEBOUNCE_MS 150                // Min ms between TD clicks (0 = disabled)
	// #define NTDM_FORWARD_PLAYER_TD_CANCEL    // Optional: forward ESC to OnPlayerClickPlayerTextDraw
	#include <not_textdraw_manager>
	```

	USAGE EXAMPLE:
	```pawn
	enum {
		TD_WINDOW_NONE = 0,
		TD_WINDOW_INVENTORY,
		TD_WINDOW_FACTION_PANEL
	};

	// Open a UI window and enable cursor selection:
	ShowPlayerTDWindow(playerid, TD_WINDOW_INVENTORY, 0xFFFFFFFF);

	// Close the active window programmatically (will NOT trigger cancel callbacks):
	HidePlayerTDWindow(playerid);

	// Called when the player presses ESC while a registered window is open:
	public OnPlayerCancelTDWindow(playerid, windowid)
	{
		switch(windowid)
		{
			case TD_WINDOW_INVENTORY: HideInventoryUI(playerid);
			case TD_WINDOW_FACTION_PANEL: HideFactionPanel(playerid);
		}
		return 1;
	}
	```

	PUBLIC API:
	- ShowPlayerTDWindow(playerid, windowid, hovercolor = 0xFFFFFFFF)
	- HidePlayerTDWindow(playerid)
	- GetPlayerTDWindow(playerid)
	- SetPlayerTDWindow(playerid, windowid)
	- bool:IsPlayerInTDWindow(playerid, windowid = 0)
	- bool:IsPlayerSelectingTD(playerid)
	- SafeDestroyPlayerTD(playerid, &PlayerText:text)
	- SafeDestroyGlobalTD(&Text:text)

	CALLBACKS:
	- OnPlayerCancelTDWindow(playerid, windowid)
	- OnPlayerCancelTD(playerid)

	COMPATIBILITY:
	- SA-MP 0.3.7 and open.mp
	- Compatible with YSI (y_hooks) and standard ALS hooks
	- Zero external dependencies (requires only <a_samp>)

	=====================================================
*/

#if defined _not_textdraw_manager_included
	#endinput
#endif
#define _not_textdraw_manager_included

#include <a_samp>

//       _____ ____  _   _ ______ _____ _____            _____       _______
//      / ____/ __ \| \ | |  ____|_   _/ ____|   ___    |  __ \   /\|__   __|/\
//     | |   | |  | |  \| | |__    | || |  __   ( _ )   | |  | | /  \  | |  /  \
//     | |   | |  | | . ` |  __|   | || | |_ |  / _ \/\ | |  | |/ /\ \ | | / /\ \
//     | |___| |__| | |\  | |     _| || |__| | | (_>  < | |__| / ____ \| |/ ____ \
//      \_____\____/|_| \_|_|    |_____\_____|  \___/\/ |_____/_/    \_\_/_/    \_\
//
//

#if !defined NTDM_DEBOUNCE_MS
	#define NTDM_DEBOUNCE_MS (150)
#endif

static bool:NTDM_s_IsSelecting[MAX_PLAYERS];
static bool:NTDM_s_ProgrammaticCancel[MAX_PLAYERS];
static NTDM_s_ActiveWindow[MAX_PLAYERS];
static NTDM_s_LastClickTick[MAX_PLAYERS];

// User callbacks
forward OnPlayerCancelTDWindow(playerid, windowid);
forward OnPlayerCancelTD(playerid);

//      _____ _   _ _______ ______ _____  ______      _____ ______
//     |_   _| \ | |__   __|  ____|  __ \|  ____/\   / ____|  ____|
//       | | |  \| |  | |  | |__  | |__) | |__ /  \ | |    | |__
//       | | | . ` |  | |  |  __| |  _  /|  __/ /\ \| |    |  __|
//      _| |_| |\  |  | |  | |____| | \ \| | / ____ \ |____| |____
//     |_____|_| \_|  |_|  |______|_|  \_\_|/_/    \_\_____|______|
//
//

/**
 * Enables TextDraw cursor selection and registers the active window ID.
 *
 * @param playerid   Target player ID.
 * @param windowid   Non-zero window identifier (0 = none).
 * @param hovercolor Cursor hover highlight color (RGBA).
 * @return           1 on success, 0 if player is not connected.
 */
stock ShowPlayerTDWindow(playerid, windowid, hovercolor = 0xFFFFFFFF)
{
	if(!IsPlayerConnected(playerid)) return 0;

	NTDM_s_ActiveWindow[playerid] = windowid;
	NTDM_s_IsSelecting[playerid] = true;
	NTDM_s_ProgrammaticCancel[playerid] = false;
	NTDM_s_LastClickTick[playerid] = GetTickCount();

	return SelectTextDraw(playerid, hovercolor);
}

/**
 * Closes the active window and disables TextDraw cursor selection
 * without triggering OnPlayerCancelTDWindow / OnPlayerCancelTD.
 *
 * @param playerid Target player ID.
 * @return         1 on success, 0 if player is not connected.
 */
stock HidePlayerTDWindow(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;

	NTDM_s_ActiveWindow[playerid] = 0;

	if(NTDM_s_IsSelecting[playerid])
	{
		NTDM_s_IsSelecting[playerid] = false;
		NTDM_s_ProgrammaticCancel[playerid] = true;
		CancelSelectTextDraw(playerid);
	}
	return 1;
}

/**
 * Returns the player's current active window ID (0 if none).
 */
stock GetPlayerTDWindow(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	return NTDM_s_ActiveWindow[playerid];
}

/**
 * Updates the active window ID without altering cursor selection state.
 */
stock SetPlayerTDWindow(playerid, windowid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	NTDM_s_ActiveWindow[playerid] = windowid;
	return 1;
}

/**
 * Checks whether the player has a specific window open (or any window if windowid == 0).
 */
stock bool:IsPlayerInTDWindow(playerid, windowid = 0)
{
	if(!IsPlayerConnected(playerid)) return false;
	if(windowid == 0) return (NTDM_s_ActiveWindow[playerid] != 0);
	return (NTDM_s_ActiveWindow[playerid] == windowid);
}

/**
 * Checks whether the player currently has TextDraw cursor selection enabled.
 */
stock bool:IsPlayerSelectingTD(playerid)
{
	if(!IsPlayerConnected(playerid)) return false;
	return NTDM_s_IsSelecting[playerid];
}

/**
 * Safely destroys a PlayerTextDraw and resets its handle to INVALID_TEXT_DRAW.
 */
stock SafeDestroyPlayerTD(playerid, &PlayerText:text)
{
	if(text == PlayerText:INVALID_TEXT_DRAW) return 0;

	if(!IsPlayerConnected(playerid))
	{
		text = PlayerText:INVALID_TEXT_DRAW;
		return 0;
	}

	PlayerTextDrawDestroy(playerid, text);
	text = PlayerText:INVALID_TEXT_DRAW;
	return 1;
}

/**
 * Safely destroys a global TextDraw and resets its handle to INVALID_TEXT_DRAW.
 */
stock SafeDestroyGlobalTD(&Text:text)
{
	if(text == Text:INVALID_TEXT_DRAW) return 0;

	TextDrawDestroy(text);
	text = Text:INVALID_TEXT_DRAW;
	return 1;
}

//      _    _  ____   ____  _  __ _____
//     | |  | |/ __ \ / __ \| |/ // ____|
//     | |__| | |  | | |  | | ' /| (___
//     |  __  | |  | | |  | |  <  \___ \
//     | |  | | |__| | |__| | . \ ____) |
//     |_|  |_|\____/ \____/|_|\_\_____/
//
//

public OnPlayerConnect(playerid)
{
	NTDM_s_IsSelecting[playerid] = false;
	NTDM_s_ProgrammaticCancel[playerid] = false;
	NTDM_s_ActiveWindow[playerid] = 0;
	NTDM_s_LastClickTick[playerid] = 0;

	#if defined NTDM_OnPlayerConnect
		return NTDM_OnPlayerConnect(playerid);
	#else
		return 1;
	#endif
}
#if defined _ALS_OnPlayerConnect
	#undef OnPlayerConnect
#else
	#define _ALS_OnPlayerConnect
#endif
#define OnPlayerConnect NTDM_OnPlayerConnect
#if defined NTDM_OnPlayerConnect
	forward NTDM_OnPlayerConnect(playerid);
#endif

public OnPlayerDisconnect(playerid, reason)
{
	NTDM_s_IsSelecting[playerid] = false;
	NTDM_s_ProgrammaticCancel[playerid] = false;
	NTDM_s_ActiveWindow[playerid] = 0;
	NTDM_s_LastClickTick[playerid] = 0;

	#if defined NTDM_OnPlayerDisconnect
		return NTDM_OnPlayerDisconnect(playerid, reason);
	#else
		return 1;
	#endif
}
#if defined _ALS_OnPlayerDisconnect
	#undef OnPlayerDisconnect
#else
	#define _ALS_OnPlayerDisconnect
#endif
#define OnPlayerDisconnect NTDM_OnPlayerDisconnect
#if defined NTDM_OnPlayerDisconnect
	forward NTDM_OnPlayerDisconnect(playerid, reason);
#endif

public OnPlayerClickTextDraw(playerid, Text:clickedid)
{
	// Handle ESC / selection cancellation first (never blocked by click debounce)
	if(clickedid == Text:INVALID_TEXT_DRAW)
	{
		if(NTDM_s_ProgrammaticCancel[playerid])
		{
			NTDM_s_ProgrammaticCancel[playerid] = false;
			return 1;
		}

		new currentWindow = NTDM_s_ActiveWindow[playerid];
		NTDM_s_ActiveWindow[playerid] = 0;
		NTDM_s_IsSelecting[playerid] = false;

		#if defined NTDM_FORWARD_PLAYER_TD_CANCEL
			CallLocalFunction("OnPlayerClickPlayerTextDraw", "dd", playerid, INVALID_TEXT_DRAW);
		#endif

		if(currentWindow != 0)
		{
			if(CallLocalFunction("OnPlayerCancelTDWindow", "dd", playerid, currentWindow))
			{
				return 1;
			}
		}

		if(CallLocalFunction("OnPlayerCancelTD", "d", playerid))
		{
			return 1;
		}

		#if defined NTDM_OnPlayerClickTextDraw
			return NTDM_OnPlayerClickTextDraw(playerid, clickedid);
		#else
			return 0;
		#endif
	}

	// Click debounce for regular TextDraw clicks
	#if NTDM_DEBOUNCE_MS > 0
	if(NTDM_s_LastClickTick[playerid] != 0 && (GetTickCount() - NTDM_s_LastClickTick[playerid]) < NTDM_DEBOUNCE_MS)
	{
		return 1;
	}
	NTDM_s_LastClickTick[playerid] = GetTickCount();
	#endif

	#if defined NTDM_OnPlayerClickTextDraw
		return NTDM_OnPlayerClickTextDraw(playerid, clickedid);
	#else
		return 0;
	#endif
}
#if defined _ALS_OnPlayerClickTextDraw
	#undef OnPlayerClickTextDraw
#else
	#define _ALS_OnPlayerClickTextDraw
#endif
#define OnPlayerClickTextDraw NTDM_OnPlayerClickTextDraw
#if defined NTDM_OnPlayerClickTextDraw
	forward NTDM_OnPlayerClickTextDraw(playerid, Text:clickedid);
#endif
