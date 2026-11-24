#include <a_samp>
#define NTDM_DEBOUNCE_MS 150
#include "not_textdraw_manager.inc"

main() {}

public OnGameModeInit()
{
	new Text:gtd = Text:INVALID_TEXT_DRAW;
	SafeDestroyGlobalTD(gtd);
	return 1;
}

public OnPlayerSpawn(playerid)
{
	new PlayerText:ptd = PlayerText:INVALID_TEXT_DRAW;
	ShowPlayerTDWindow(playerid, 1, 0xFFFFFFFF);
	SetPlayerTDWindow(playerid, 2);
	new w = GetPlayerTDWindow(playerid);
	if(IsPlayerInTDWindow(playerid, w) && IsPlayerSelectingTD(playerid))
	{
		HidePlayerTDWindow(playerid);
	}
	SafeDestroyPlayerTD(playerid, ptd);
	return 1;
}

public OnPlayerCancelTDWindow(playerid, windowid)
{
	#pragma unused playerid, windowid
	return 1;
}

public OnPlayerCancelTD(playerid)
{
	#pragma unused playerid
	return 1;
}
