
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WeaponInterface.generated.h"

class UBaseDamageType;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponUsed);

UINTERFACE(MinimalAPI, Blueprintable)
class UWeaponInterface : public UInterface
{
	GENERATED_BODY()
};

class TESISUE_API IWeaponInterface
{
	GENERATED_BODY()

public:
	FOnWeaponUsed OnWeaponUsed;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetDamageType(const FGameplayTag& DamageTypeTag, const FGameplayTag& CueTag);
	
	virtual void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled) = 0;

	virtual void ClearIgnoreActors() = 0;
};
