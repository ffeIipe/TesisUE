// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class APlayerMain;
class UBoxComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;

UCLASS()
class TESISUE_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:
	AProjectile();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite);
	float Damage;

	UPROPERTY(EditAnywhere, Category = "Projectile | Collider")
	UBoxComponent* BoxCollider;

	UPROPERTY(EditAnywhere, Category = "Projectile | Mesh")
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(EditAnywhere, Category = "Projectile | Niagara")
	UNiagaraComponent* ProjectileNiagaraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile | ProjectileMovement")
	UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile | Stats")
	float Speed = 500.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile | Stats")
	float Gravity = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile | Stats")
	float ProjectileLifetime = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile | Stats")
	bool IsHoming = true;

	bool bEnableDestroyOnCollision;

	UFUNCTION()
	virtual void OnBoxOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	/*
	* Effects
	*/
	UPROPERTY(EditAnywhere, Category = "Effects");
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = "Effects");
	UParticleSystem* HitParticles;
};
