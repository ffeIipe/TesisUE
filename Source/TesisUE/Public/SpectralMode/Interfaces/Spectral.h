#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Spectral.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class USpectral : public UInterface
{
	GENERATED_BODY()
};

class TESISUE_API ISpectral
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void RegisterSpectralObject();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ActivateVisibility();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DeactivateVisibility();
};
