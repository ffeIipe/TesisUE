#include "GenericPool/Core/ActorPoolSubsystem.h"


void UActorPoolSubsystem::ReturnToPool(AActor* ActorToReturn)
{
	if (!IsValid(ActorToReturn)) return;

	if (ActorToReturn->Implements<UPoolableActor>())
	{
		IPoolableActor::Execute_OnPoolDeactivate(ActorToReturn);
	}

	ActorToReturn->SetActorHiddenInGame(true);
	ActorToReturn->SetActorEnableCollision(false);
	ActorToReturn->SetActorTickEnabled(false);

	InactivePools.FindOrAdd(ActorToReturn->GetClass()).Push(ActorToReturn);
}

AActor* UActorPoolSubsystem::GetActorFromPool(const TSubclassOf<AActor>& ClassToSpawn, const FTransform& SpawnTransform)
{
	if (!ClassToSpawn) return nullptr;

	AActor* ResultActor = nullptr;
	
	if (TArray<AActor*>* PoolArray = InactivePools.Find(ClassToSpawn))
	{
		while (PoolArray->Num() > 0)
		{
			if (AActor* Candidate = PoolArray->Pop(); IsValid(Candidate))
			{
				ResultActor = Candidate;
				break;
			}
		}
	}

	if (!ResultActor)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ResultActor = GetWorld()->SpawnActor<AActor>(ClassToSpawn, SpawnTransform, SpawnParams);
	}

	if (ResultActor)
	{
		ResultActor->SetActorTransform(SpawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
		
		ResultActor->SetActorHiddenInGame(false);
		ResultActor->SetActorEnableCollision(true);
		ResultActor->SetActorTickEnabled(true);

		if (ResultActor->Implements<UPoolableActor>())
		{
			IPoolableActor::Execute_OnPoolActivate(ResultActor);
		}
	}

	return ResultActor;
}

void UActorPoolSubsystem::InitializePool(const TSubclassOf<AActor> ClassToSpawn, const int32 Quantity)
{
	if (!ClassToSpawn || Quantity <= 0)
	{
		return;
	}

	TArray<AActor*>& Pool = InactivePools.FindOrAdd(ClassToSpawn);
	Pool.Reserve(Pool.Num() + Quantity);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	UWorld* World = GetWorld();
	if (!World) return;

	for (int32 i = 0; i < Quantity; ++i)
	{
		if (AActor* NewActor = World->SpawnActor<AActor>(ClassToSpawn, FTransform::Identity, SpawnParams); IsValid(NewActor))
		{
			ReturnToPool(NewActor);
		}
	}
}

AActor* UActorPoolSubsystem::RetrieveActor(const UClass* ClassType)
{
	if (TArray<AActor*>* PoolArray = InactivePools.Find(ClassType))
	{
		while (PoolArray->Num() > 0)
		{
			if (AActor* Candidate = PoolArray->Pop(); IsValid(Candidate))
			{
				Candidate->SetActorHiddenInGame(false);
				Candidate->SetActorEnableCollision(true);
				Candidate->SetActorTickEnabled(true);
				return Candidate;
			}
		}
	}
	
	return nullptr;
}
