#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "UObject/Interface.h"
#include "Damageable.generated.h"

UINTERFACE(Blueprintable)
class UDamageable : public UInterface
{
	GENERATED_BODY()
};

class TESISUE_API IDamageable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReceiveDamage(FGameplayEventData DamagePayload);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void MortalDamage(FGameplayEventData DeathPayload);
};
