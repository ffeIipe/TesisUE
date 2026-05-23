#include "Features/InventorySystem/Items/Weapon.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"

void AWeapon::SetDamageType_Implementation(const FGameplayTag& DamageTag, const FGameplayTag& CueTag)
{
	if (DamageTag == FGameplayTag::EmptyTag)
	{
		CurrentDamageTag = FGameplayTag::RequestGameplayTag("Damage.Default");
		CurrentCueTag = FGameplayTag::RequestGameplayTag("GameplayCue.Damage.Default");
	}
	else
	{
		CurrentCueTag = CueTag;
		CurrentDamageTag = DamageTag; 	
	}
}

void AWeapon::SetDamageEffectSpec(const FGameplayEffectSpecHandle& InSpecHandle)
{
	DamageEffectSpecHandle = InSpecHandle;
}

void AWeapon::OnEnteredInventory_Implementation(AActor* NewOwner)
{
	Super::OnEnteredInventory_Implementation(NewOwner);

	Equip();
}

void AWeapon::OnRemovedFromInventory_Implementation()
{
	Super::OnRemovedFromInventory_Implementation();

	Holster();
}

void AWeapon::Equip()
{
	if (USkeletalMeshComponent* SkeletalMesh = Cast<ACharacter>(GetOwner())->GetMesh())
	{
		AttachMeshToSocket(SkeletalMesh, FName("RightHandSword"));
		OnWeaponEquipped();
	}
}

void AWeapon::Holster()
{
	if (USkeletalMeshComponent* SkeletalMesh = Cast<ACharacter>(GetOwner())->GetMesh())
	{
		AttachMeshToSocket(SkeletalMesh, FName("RightClavicleSocket"));
		OnWeaponHolstered();
	}
}