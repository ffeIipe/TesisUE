#include "Features/SaveSystem/Core/PlayerProgressSaveGame.h"

UPlayerProgressSaveGame::UPlayerProgressSaveGame()
{
    CurrentLevelName = TEXT("");
    Timestamp = FDateTime(0); 
    SaveSlotIndex = -1;
    SaveSlotUserLabel = TEXT("Nueva Partida");
}