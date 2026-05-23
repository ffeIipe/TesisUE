#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Item.h"
#include "Interfaces/Weapon/WeaponInterface.h"
#include "Weapon.generated.h"


class UWeaponData;

UCLASS(Abstract)
class TESISUE_API AWeapon : public AItem, public IWeaponInterface
{
	GENERATED_BODY()

public:
	virtual void OnEnteredInventory_Implementation(AActor* NewOwner) override;

	virtual void OnRemovedFromInventory_Implementation() override; //TODO:

	virtual void Equip(); 

	virtual void Holster();

	virtual void EnableVisuals() override { Super::EnableVisuals(); }
	virtual void DisableVisuals() override { Super::DisableVisuals(); }

	virtual void SetDamageType_Implementation(const FGameplayTag& DamageTag, const FGameplayTag& CueTag) override;
	virtual void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled) override {}
	virtual void ClearIgnoreActors() override {}

	UFUNCTION(BlueprintCallable, Category = "GAS")
	void SetDamageEffectSpec(const FGameplayEffectSpecHandle& InSpecHandle);
	
protected:
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;
	
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag CurrentDamageTag = FGameplayTag::RequestGameplayTag("Damage.Default");

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag CurrentCueTag = FGameplayTag::RequestGameplayTag("GameplayCue.Damage.Default");
	
	FGameplayEffectSpecHandle DamageEffectSpecHandle = nullptr;

	UFUNCTION(BlueprintCallable)
	virtual void AttachMeshToSocket(USceneComponent* InParent, FName SocketInName = NAME_None) {}

	UFUNCTION(BlueprintImplementableEvent)
	void OnWeaponEquipped();

	UFUNCTION(BlueprintImplementableEvent)
	void OnWeaponHolstered();
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnActorsWeaponHit(const TArray<FHitResult>& HitResults);
};