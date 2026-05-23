// Copyright 2024 Tore Lervik. All Rights Reserved.

#include "MeshBlendPooledActors.h"

#include "PrimitiveDataHelper.h"
#include "Components/InstancedStaticMeshComponent.h"

void FMeshBlendPooledActors::AddActor(AActor* Actor)
{
	ActorProcessingQueue.AddUnique(Actor);
}

bool FMeshBlendPooledActors::ProcessActorQueue(const double MaxProcessDuration)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	if (IterationIndex != 0)
	{
		IterationIndex = (IterationIndex + 1) % 100;
		return true;
	}

	TWeakObjectPtr<AActor> WeakActor;

	while (ActorProcessingQueue.Next(WeakActor))
	{
		if (FMeshBlendShared::IsOverProcessBudget(MaxProcessDuration))
		{
			ActorProcessingQueue.KeepCurrentAsNext();
			return false;
		}

		if (!WeakActor.IsValid())
		{
			ActorProcessingQueue.RemoveCurrent();
			continue;
		}

		AActor* Actor = WeakActor.Get();

		if (CheckIfActorShouldUpdate(Actor))
		{
			OnRefreshActor.ExecuteIfBound(Actor);
		}
	}

	IterationIndex = (IterationIndex + 1) % 100;
	return true;
}

bool FMeshBlendPooledActors::CheckIfActorShouldUpdate(const AActor* Actor)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	TArray<UActorComponent*> Components;
	Actor->GetComponents(UMeshComponent::StaticClass(), Components, false);
	bool bShouldRefresh = false;

	for (UActorComponent* Component : Components)
	{
		if (UMeshComponent* MeshComponent = Cast<UMeshComponent>(Component))
		{
			if (CheckMeshComponent(MeshComponent))
			{
				bShouldRefresh = true;
			}
		}
	}

	return bShouldRefresh;
}

bool FMeshBlendPooledActors::CheckMeshComponent(UMeshComponent* Component)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	int InstanceCount = 1;

	if (const UInstancedStaticMeshComponent* InstancedStaticMeshComponent = Cast<UInstancedStaticMeshComponent>(Component))
	{
		InstanceCount = InstancedStaticMeshComponent->GetInstanceCount();
	}

	if (!BoundMeshComponents.Contains(Component))
	{
		BoundMeshComponents.Add(Component, InstanceCount);

		if (Component->ComponentHasTag(GName_AutoBlendHasPrimitiveData))
		{
			return false;
		}

		return true;
	}

	if (BoundMeshComponents[Component] != InstanceCount)
	{
		BoundMeshComponents[Component] = InstanceCount;
		Component->ComponentTags.Remove(GName_AutoBlendHasPrimitiveData);
		return true;
	}

	return false;
}
