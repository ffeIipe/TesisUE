#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ComboNode.generated.h"

class UAnimMontage;

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class TESISUE_API UComboNode : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CurrentNode | Animation")
	UAnimMontage* MontageToPlay;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CurrentNode | Data")
	float DamageMultiplier = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Branches", meta = (DisplayThumbnail = "false"))
	UComboNode* NextActionInputTap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Branches", meta = (DisplayThumbnail = "false"))
	UComboNode* NextActionInputHold;
};