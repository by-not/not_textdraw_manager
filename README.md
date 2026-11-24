# not_textdraw_manager

> **Language:** English | [Español](README.es.md)

A lightweight, zero-dependency TextDraw cursor and active window manager for **SA-MP** and **open.mp**.

## Features

- Real-time TextDraw cursor selection tracking (`IsPlayerSelectingTD`).
- Single active UI window registry per player (`ShowPlayerTDWindow` / `HidePlayerTDWindow`).
- Clean ESC cancellation routing (`OnPlayerCancelTDWindow` and `OnPlayerCancelTD`).
- Distinguishes programmatic closes (`HidePlayerTDWindow`) from user ESC presses (no false cancel callbacks).
- Configurable click debounce (`NTDM_DEBOUNCE_MS`) that never blocks ESC cancellation.
- Safe TextDraw destruction helpers (`SafeDestroyPlayerTD`, `SafeDestroyGlobalTD`).
- Standard ALS hooks, compatible with YSI (`y_hooks`) and other includes.

## Installation

1. Copy `not_textdraw_manager.inc` to your `pawno/include/` (or `qawno/include/`) folder.
2. Include it in your gamemode or filterscript after `<a_samp>`:

```pawn
#include <a_samp>

// Optional configuration (define before including):
#define NTDM_DEBOUNCE_MS 150                // Min ms between clicks (0 = disabled)
// #define NTDM_FORWARD_PLAYER_TD_CANCEL    // Optional: forward ESC to OnPlayerClickPlayerTextDraw

#include <not_textdraw_manager>
```

## Quick Start

```pawn
enum {
    TD_WINDOW_NONE = 0,
    TD_WINDOW_INVENTORY,
    TD_WINDOW_FACTION_PANEL
};

stock OpenInventoryUI(playerid)
{
    // Create/show your TextDraws here...
    ShowPlayerTDWindow(playerid, TD_WINDOW_INVENTORY, 0xFFFFFFFF);
}

stock CloseInventoryUI(playerid)
{
    // Destroy/hide your TextDraws here...
    HidePlayerTDWindow(playerid); // Will NOT fire OnPlayerCancelTDWindow
}

public OnPlayerCancelTDWindow(playerid, windowid)
{
    switch(windowid)
    {
        case TD_WINDOW_INVENTORY: CloseInventoryUI(playerid);
        case TD_WINDOW_FACTION_PANEL: CloseFactionPanel(playerid);
    }
    return 1;
}
```

## API Reference

### Functions

| Function | Description |
|---|---|
| `ShowPlayerTDWindow(playerid, windowid, hovercolor = 0xFFFFFFFF)` | Registers the active window ID and enables cursor selection. |
| `HidePlayerTDWindow(playerid)` | Clears the active window and cancels selection without firing cancel callbacks. |
| `GetPlayerTDWindow(playerid)` | Returns the player's current window ID (`0` if none). |
| `SetPlayerTDWindow(playerid, windowid)` | Updates the active window ID without changing cursor state. |
| `bool:IsPlayerInTDWindow(playerid, windowid = 0)` | Checks if a specific window (or any window if `0`) is open. |
| `bool:IsPlayerSelectingTD(playerid)` | Returns `true` if the player currently has TextDraw cursor mode enabled. |
| `SafeDestroyPlayerTD(playerid, &PlayerText:text)` | Destroys a `PlayerTextDraw` if valid and resets the variable to `INVALID_TEXT_DRAW`. |
| `SafeDestroyGlobalTD(&Text:text)` | Destroys a global `TextDraw` if valid and resets the variable to `INVALID_TEXT_DRAW`. |

### Callbacks

| Callback | Description |
|---|---|
| `OnPlayerCancelTDWindow(playerid, windowid)` | Called when the player presses ESC while a registered (`windowid != 0`) window is open. Return `1` to consume. |
| `OnPlayerCancelTD(playerid)` | Global fallback called when ESC is pressed and not consumed by `OnPlayerCancelTDWindow`. |

## License

Released under the [MIT License](LICENSE).
