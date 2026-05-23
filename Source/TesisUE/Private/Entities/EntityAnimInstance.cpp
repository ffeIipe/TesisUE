#include "Entities/EntityAnimInstance.h"

#include "AbilitySystemComponent.h"
#include "KismetAnimationLibrary.h"
#include "Entities/Entity.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UEntityAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	EntityOwner = Cast<AEntity>(TryGetPawnOwner());

	if (EntityOwner)
	{
		AbilitySystemComponent = EntityOwner->GetAbilitySystemComponent();
		
		//CharacterMovementComponent = EntityOwner->GetCharacterMovementComponent();
	}
	
	/*if (CharacterMovementComponent)
	{
		MaxWalkSpeed = CharacterMovementComponent->MaxWalkSpeed;
	}*/
}

void UEntityAnimInstance::NativeUpdateAnimation(const float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	/*if (CharacterMovementComponent)
	{
		GroundSpeed = UKismetMathLibrary::VSizeXY(CharacterMovementComponent->Velocity);

		bIsFalling = CharacterMovementComponent->IsFalling();

		bHasAcceleration = CharacterMovementComponent->GetCurrentAcceleration().SizeSquared2D() > SMALL_NUMBER;
		
		Direction = UKismetAnimationLibrary::CalculateDirection(CharacterMovementComponent->Velocity, EntityOwner->GetActorRotation());
	}*/

	if (AbilitySystemComponent)
	{
		bIsEquipped = AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Weapon")));
		bIsAttacking = AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Attacking")));
		bIsLocking = AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Locking")));
		bIsDodging = AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Dodging")));
	}
}