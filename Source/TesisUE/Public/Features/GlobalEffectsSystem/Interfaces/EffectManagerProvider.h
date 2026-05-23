#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTags.h"
#include "EffectManagerProvider.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UEffectManagerProvider : public UInterface
{
	GENERATED_BODY()
};

class TESISUE_API IEffectManagerProvider
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Effects")
	void PlayGameplayEffect(FGameplayTag EffectTag, FVector Location = FVector::ZeroVector);
};
