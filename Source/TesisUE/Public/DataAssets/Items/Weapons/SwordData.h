#pragma once

#include "CoreMinimal.h"
#include "WeaponData.h"
#include "Engine/DataAsset.h"
#include "SwordData.generated.h"

class UComboNode;

UCLASS(BlueprintType)
class TESISUE_API USwordData : public UWeaponData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Ground Combo")
	UComboNode* FirstGroundAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Air Combo")
	UComboNode* FirstAirAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Launch")
	UComboNode* LaunchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Skill")
	UComboNode* SkillAction;
};