// Copyright 2024 Tore Lervik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MeshBlendShared.h"
#include "ProcessingQueue.h"
#include "GameFramework/Actor.h"
#include "MeshBlendPooledActors.generated.h"

DECLARE_DELEGATE_OneParam(FRefreshActorDelegate, AActor*);

/**
 * FMeshBlendPooledActors is a structure that manages a pool of actors for processing.
 * It checks the UMeshComponent of each actor to determine if it should be updated.
 */
USTRUCT()
struct MESHBLEND_API FMeshBlendPooledActors
{
	GENERATED_BODY()

	FMeshBlendPooledActors()
	{
	}

	FRefreshActorDelegate OnRefreshActor;

	void AddActor(AActor* Actor);
	bool ProcessActorQueue(const double MaxProcessDuration);

private:
	int IterationIndex = 0;
	TProcessingQueue<TWeakObjectPtr<AActor>> ActorProcessingQueue;

	UPROPERTY()
	TMap<TWeakObjectPtr<UMeshComponent>, int> BoundMeshComponents;

	bool CheckIfActorShouldUpdate(const AActor* Actor);
	bool CheckMeshComponent(UMeshComponent* Component);
};
