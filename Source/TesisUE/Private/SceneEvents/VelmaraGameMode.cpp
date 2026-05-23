#include "SceneEvents/VelmaraGameMode.h"
#include "Features/SaveSystem/Subsystems/SaveGameSubsystem.h"

void AVelmaraGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (USaveGameSubsystem* SaveSystem = GI->GetSubsystem<USaveGameSubsystem>())
		{
			SaveSystem->RestoreCurrentLevelState();
		}
	}
}