#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PoolManager.generated.h"

UCLASS()
class TESISUE_API APoolManager : public AActor
{
	GENERATED_BODY()

public:
	APoolManager();

protected:
	UPROPERTY()
	TArray<AActor*> Pool;

public:
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void InitializePool(TSubclassOf<AActor> ActorClass, int32 PoolSize);

	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	AActor* GetActorFromPool(FTransform SpawnTransform);

	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void ReturnActorToPool(AActor* ActorToReturn);
};
