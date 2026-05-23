#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "VelmaraGameMode.generated.h"

class UPlayerProgressSaveGame;

UCLASS()
class TESISUE_API AVelmaraGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
};