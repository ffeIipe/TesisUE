// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EntityAnimInstance.generated.h"

class UAbilitySystemComponent;
class UCharacterMovementComponent;
class AEntity;

UCLASS()
class TESISUE_API UEntityAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	UPROPERTY(BlueprintReadOnly)
	AEntity* EntityOwner;

	UPROPERTY(BlueprintReadOnly)
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly)
	bool bIsEquipped = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsAttacking = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsLocking = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsDodging = false;
	
	/*UPROPERTY(BlueprintReadOnly)
	UCharacterMovementComponent* CharacterMovementComponent;*/
	
	/*UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsFalling;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bHasAcceleration;

	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	float Direction;
	
	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	float WalkRight;
	
	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	float WalkForward;
	
	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	float MaxWalkSpeed;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bIsLocking;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement | Animation States")
	bool bIsMeleeWeapon;*/
};
