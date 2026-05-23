// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ArenaPreset.generated.h"

UCLASS(Blueprintable)
class TESISUE_API UArenaPreset : public UDataAsset
{
	GENERATED_BODY()

	public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner Config")
	TArray<TSubclassOf<AActor>> AgentClasses;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave Rules")
	int32 TotalEnemiesToSpawn = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave Rules")
	int32 MaxEnemiesAliveAtOnce = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave Rules")
	bool bShouldSurvivePerTime = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave Rules", meta = (EditCondition = "bShouldSurvivePerTime", EditConditionHides))
	float TimeInSeconds = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pacing")
	float MinSpawnInterval = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pacing")
	float MaxSpawnInterval = 3.0f;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NavMesh")
	float NavMeshProjectionExtent = 200.0f;
};
