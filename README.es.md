# not_textdraw_manager

> **Idioma:** [English](README.md) | Español

Gestor ligero y sin dependencias para el cursor de selección de TextDraws, ventanas UI activas y destrucción segura en **SA-MP** y **open.mp**.

## Características

- Seguimiento en tiempo real del cursor de TextDraws (`IsPlayerSelectingTD`).
- Registro de ventana activa por jugador (`ShowPlayerTDWindow` / `HidePlayerTDWindow`).
- Enrutamiento centralizado de la tecla ESC (`OnPlayerCancelTDWindow` y `OnPlayerCancelTD`).
- Distingue cierres programáticos (`HidePlayerTDWindow`) de cancelaciones reales con ESC (evita disparos falsos).
- Debounce configurable de clics (`NTDM_DEBOUNCE_MS`) que nunca bloquea la tecla ESC.
- Helpers de destrucción segura (`SafeDestroyPlayerTD`, `SafeDestroyGlobalTD`).
- Hooks ALS estándar, compatibles con YSI (`y_hooks`) y otros includes.

## Instalación

1. Copia `not_textdraw_manager.inc` en tu carpeta `pawno/include/` (o `qawno/include/`).
2. Inclúyelo en tu gamemode o filterscript después de `<a_samp>`:

```pawn
#include <a_samp>

// Configuración opcional (definir antes del #include):
#define NTDM_DEBOUNCE_MS 150                // Tiempo mínimo entre clics en ms (0 = desactivado)
// #define NTDM_FORWARD_PLAYER_TD_CANCEL    // Opcional: reenviar ESC a OnPlayerClickPlayerTextDraw

#include <not_textdraw_manager>
```

## Uso rápido

```pawn
enum {
    TD_WINDOW_NONE = 0,
    TD_WINDOW_INVENTORY,
    TD_WINDOW_FACTION_PANEL
};

stock OpenInventoryUI(playerid)
{
    // Crea/muestra tus TextDraws aquí...
    ShowPlayerTDWindow(playerid, TD_WINDOW_INVENTORY, 0xFFFFFFFF);
}

stock CloseInventoryUI(playerid)
{
    // Destruye/oculta tus TextDraws aquí...
    HidePlayerTDWindow(playerid); // NO disparará OnPlayerCancelTDWindow
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

## Referencia de API

### Funciones

| Función | Descripción |
|---|---|
| `ShowPlayerTDWindow(playerid, windowid, hovercolor = 0xFFFFFFFF)` | Registra la ventana activa y activa el cursor de selección. |
| `HidePlayerTDWindow(playerid)` | Limpia la ventana activa y desactiva el cursor sin disparar callbacks de cancelación. |
| `GetPlayerTDWindow(playerid)` | Devuelve el ID de ventana activa del jugador (`0` si no hay ninguna). |
| `SetPlayerTDWindow(playerid, windowid)` | Cambia el ID de ventana activa sin alterar el estado del cursor. |
| `bool:IsPlayerInTDWindow(playerid, windowid = 0)` | Comprueba si el jugador está en una ventana específica (o en cualquiera si `0`). |
| `bool:IsPlayerSelectingTD(playerid)` | Devuelve `true` si el jugador tiene el cursor de TextDraws activo. |
| `SafeDestroyPlayerTD(playerid, &PlayerText:text)` | Destruye un `PlayerTextDraw` si es válido y lo resetea a `INVALID_TEXT_DRAW`. |
| `SafeDestroyGlobalTD(&Text:text)` | Destruye un `TextDraw` global si es válido y lo resetea a `INVALID_TEXT_DRAW`. |

### Callbacks

| Callback | Descripción |
|---|---|
| `OnPlayerCancelTDWindow(playerid, windowid)` | Se llama al presionar ESC con una ventana registrada (`windowid != 0`). Retorna `1` para consumir el evento. |
| `OnPlayerCancelTD(playerid)` | Fallback global al presionar ESC si `OnPlayerCancelTDWindow` no lo consumió. |

## Licencia

Distribuido bajo la [Licencia MIT](LICENSE).
