#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GAS/ComboNode.h"
#include "ComboChain.generated.h"

UCLASS(BlueprintType)
class TESISUE_API UComboChain : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Ground Combo")
	UComboNode* FirstGroundAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Air Combo")
	UComboNode* FirstAirAction;
};