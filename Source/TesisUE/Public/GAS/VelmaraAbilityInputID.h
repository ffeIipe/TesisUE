#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EVelmaraAbilityInputID : uint8
{
	None				UMETA(DisplayName = "None"),
	Confirm				UMETA(DisplayName = "Confirm"),
	Cancel				UMETA(DisplayName = "Cancel"),
    
	Attack				UMETA(DisplayName = "Attack"),
	Jump				UMETA(DisplayName = "Jump"),
	Dodge				UMETA(DisplayName = "Dodge"),
	Crouch				UMETA(DisplayName = "Crouch"),
	Interact			UMETA(DisplayName = "Interact"),
	Inventory			UMETA(DisplayName = "Inventory"),
	ToggleWeapon		UMETA(DisplayName = "ToggleWeapon"),
	Skill1				UMETA(DisplayName = "Skill 1"),
	Skill2				UMETA(DisplayName = "Skill 2"),
	Skill3				UMETA(DisplayName = "Skill 3"),
	Skill4				UMETA(DisplayName = "Skill 4"),
};
