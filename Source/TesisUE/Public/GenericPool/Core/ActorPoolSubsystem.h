#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GenericPool/Interfaces/PoolableActor.h"
#include "Subsystems/WorldSubsystem.h"
#include "ActorPoolSubsystem.generated.h"

UCLASS()
class TESISUE_API UActorPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	template <typename T>
	T* SpawnOrRetrieveFromPool(TSubclassOf<T> ClassToSpawn, const FTransform& SpawnTransform)
	{
		if (!ClassToSpawn) return nullptr;

		AActor* RecycledActor = RetrieveActor(ClassToSpawn);

		if (RecycledActor)
		{
			RecycledActor->SetActorTransform(SpawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
			
			if (RecycledActor->Implements<UPoolableActor>())
			{
				IPoolableActor::Execute_OnPoolActivate(RecycledActor);
			}
			
			return Cast<T>(RecycledActor);
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		T* NewActor = GetWorld()->SpawnActor<T>(ClassToSpawn, SpawnTransform, SpawnParams);
		
		if (NewActor && NewActor->Implements<UPoolableActor>())
		{
			IPoolableActor::Execute_OnPoolActivate(NewActor);
		}

		return NewActor;
	}

public:
	UFUNCTION(BlueprintCallable, Category = "Pooling")
	void ReturnToPool(AActor* ActorToReturn);

	UFUNCTION(BlueprintCallable, Category = "Pooling", meta = (DeterminesOutputType = "ClassToSpawn", DisplayName = "Spawn Actor From Pool"))
	AActor* GetActorFromPool(const TSubclassOf<AActor>& ClassToSpawn, const FTransform& SpawnTransform);

	UFUNCTION(BlueprintCallable, Category = "Pooling")
	void InitializePool(TSubclassOf<AActor> ClassToSpawn, int32 Quantity);

protected:
	AActor* RetrieveActor(const UClass* ClassType);

private:
	UPROPERTY()
	TMap<UClass*, FGameplayTagContainer> PoolStats;
	
	TMap<UClass*, TArray<AActor*>> InactivePools;
};
