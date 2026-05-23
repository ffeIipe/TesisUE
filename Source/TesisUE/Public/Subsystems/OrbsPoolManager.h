// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OrbsPoolManager.generated.h"

class AItem;

USTRUCT(BlueprintType)
struct FOrbsPool
{
    GENERATED_BODY()

public:
    UPROPERTY()
    TArray<AActor*> PooledItems;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Orbs Pooling")
    TSubclassOf<AActor> ItemClass;

    UPROPERTY()
    TArray<AActor*> ActiveItems;
};

UCLASS()
class TESISUE_API UOrbsPoolManager : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Orbs Pooling")
    void EnsurePoolInitialized(TSubclassOf<AActor> OrbsClass, int32 InitialPoolSize = 15);
    
    UFUNCTION(BlueprintCallable, Category = "Orbs Pooling")
    AActor* SpawnOrbFromPool(TSubclassOf<AActor> OrbsClass, const FVector& Location, const FRotator& Rotation, AActor* Owner = nullptr, APawn* Instigator = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Orbs Pooling")
    void ReturnOrbToPool(AActor* Orb);

private:
    UPROPERTY()
    TMap<TSubclassOf<AActor>, FOrbsPool> OrbsPools;

    FOrbsPool* GetOrCreatePool(const TSubclassOf<AActor>& OrbsClass);

    AActor* SpawnNewOrbForPool(const TSubclassOf<AActor>& OrbsClass, const FVector& Location, const FRotator& Rotation, AActor* Owner, APawn* Instigator);

};
