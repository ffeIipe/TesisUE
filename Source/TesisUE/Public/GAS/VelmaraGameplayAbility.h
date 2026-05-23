#pragma once

#include "CoreMinimal.h"
#include "VelmaraAbilityInputID.h"
#include "Abilities/GameplayAbility.h"
#include "VelmaraGameplayAbility.generated.h"

enum class EVelmaraAbilityInputID : uint8;

UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class TESISUE_API UVelmaraGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UVelmaraGameplayAbility();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	EVelmaraAbilityInputID AbilityInputID = EVelmaraAbilityInputID::None;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	bool bActivateOnGiven = false;

protected:
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

#if WITH_EDITOR
	
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
