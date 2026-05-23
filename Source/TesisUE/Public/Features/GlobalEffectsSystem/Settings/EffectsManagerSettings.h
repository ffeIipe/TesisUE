#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Features/GlobalEffectsSystem/Data/EffectsManagerData.h"
#include "EffectsManagerSettings.generated.h"

UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Effects System"))
class TESISUE_API UEffectsSystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General")
	TSoftObjectPtr<UEffectsManagerData> EffectsDataAsset;
};
