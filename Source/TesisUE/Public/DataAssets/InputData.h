#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputData.generated.h"

class UInputAction;

USTRUCT(BlueprintType)
struct FInputs
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Input | Movement")
	UInputAction* MoveAction = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input | Movement")
	UInputAction* LookAction = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input | Movement")
	UInputAction* CrouchAction = nullptr;	

	UPROPERTY(EditDefaultsOnly, Category = "Input | Movement")
	UInputAction* JumpAction = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input | Movement")
	UInputAction* DodgeAction = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input | Actions")
	UInputAction* InteractAction = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input | Actions")
	UInputAction* InventoryAction = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input | Actions")
	UInputAction* ToggleWeaponAction = nullptr;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input | Actions")
	UInputAction* AttackAction = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input | Actions")
	UInputAction* InputAction_Skill1 = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input | Actions")
	UInputAction* InputAction_Skill2 = nullptr;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input | Actions")
	UInputAction* InputAction_Skill3 = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input | Actions")
	UInputAction* InputAction_Skill4 = nullptr;
};

UCLASS()
class TESISUE_API UInputData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inputs")
	FInputs Inputs;
};