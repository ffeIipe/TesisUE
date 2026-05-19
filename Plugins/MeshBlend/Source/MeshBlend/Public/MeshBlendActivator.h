// Copyright 2024 Tore Lervik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AutoBlendUserData.h"
#include "MeshBlendPCGHelper.h"
#include "MeshBlendPooledActors.h"
#include "MeshBlendProcessor.h"
#include "MeshBlendSceneViewExtension.h"
#include "MeshBlendShared.h"
#include "ProcessingQueue.h"
#include "GameFramework/Actor.h"
#include "Materials/Material.h"

#include "MeshBlendActivator.generated.h"

UENUM()
enum EProcessStage : uint8
{
	FindDistance = 0,
	ProcessActors = 1,
};

USTRUCT()
struct FActorInformation
{
	GENERATED_BODY()

	TWeakObjectPtr<AActor> Actor;

	UPROPERTY()
	FActorBounds Bounds;

	UPROPERTY()
	double SkipCheckingUntil = 0.0;
};

USTRUCT()
struct FActorCheckAgainItem
{
	GENERATED_BODY()

	TWeakObjectPtr<AActor> Actor;

	UPROPERTY()
	bool HasBeenCheckedOnce = false;

	UPROPERTY()
	double StopCheckingAfter = 0.0;
};

USTRUCT()
struct FActorProcessingItem
{
	GENERATED_BODY()

	TWeakObjectPtr<AActor> Actor;

	UPROPERTY()
	bool bReset = false;

	UPROPERTY()
	bool bSoftReset = false;

	UPROPERTY()
	bool bRefresh = false;
};

UCLASS()
class MESHBLEND_API AMeshBlendActivator : public AActor
{
	GENERATED_BODY()

public:
	AMeshBlendActivator(const FObjectInitializer& ObjectInitializer);
	virtual ~AMeshBlendActivator() override;
	void RefreshActorBlending(AActor* Actor, bool bSoftReset);
	bool ResetActor(AActor* Actor, bool bSoftReset);
	void CheckActor(const FActorProcessingItem& ActorProcessingItem);
	static AMeshBlendActivator* GetInstance(const UWorld* World);
	static void ToggleEnabled();
	static bool IsEnabled();
	static void ToggleRunInEditor();
	static bool IsRunInEditor();
	static void ToggleDebugVisualization();
	static bool GetDebugVisualization();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PostInitProperties() override;
	void EmptyProcessingQueues();
	virtual void BeginDestroy() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual bool ShouldTickIfViewportsOnly() const override;

	/* Used to tune how much time we have to process actors per tick. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshBlend", meta = (DisplayName = "Process Budget (ms)"))
	float ProcessBudget = 0.3;

private:
	void OnActorSpawned(AActor* Actor);
	void World_OnLevelAdded(ULevel* InLevel, UWorld* World);
	void World_OnLevelRemoved(ULevel* InLevel, UWorld* World);
	void Level_OnActorsAdded(const TArray<AActor*>& Actors);
	void Level_OnActorSpawned(AActor* Actor);
	bool CalculateMaxProcessingRadius(const UWorld* World, const FVector& Origin, double MaxProcessDuration);
	void ShowMissingScalarParameterWarning(const UMaterialInterface* Material);
	bool ProcessActors(const UWorld* World, const FVector& Origin, double ProcessTimeStart, double MaxProcessDuration);
	bool ProcessAddedLevels(double MaxProcessDuration);
	bool ProcessActorsToCheckQueue(double MaxProcessDuration);
	void HandleActorProcessingItem(const FActorProcessingItem& ActorProcessingItem);


	bool SyncConsoleVariables();
	void CheckActivatorInitialization();
	void ResetMeshActivator(const UWorld* World, bool bSoftReset);
	void DisableMeshBlendActivator(UWorld* World, bool bSoftReset);

	static bool IsLumenMaterialAOCVarCorrect();
	static bool IsAllowStaticLightingCVarCorrect();

	UPROPERTY(Transient, DuplicateTransient)
	float CurrentMaxRadius = 0;

	static constexpr int MinDistanceThreshold = 10000;
	static constexpr int MinDistanceThresholdSquared = MinDistanceThreshold * MinDistanceThreshold;

	UPROPERTY(Transient, DuplicateTransient)
	TEnumAsByte<EProcessStage> CurrentProcessStage = FindDistance;

	TSet<TWeakObjectPtr<AActor>> ActorsAlreadyProcessed;
	TProcessingQueue<FActorInformation> ActorsProcessingQueue;
	TProcessingQueue<FActorProcessingItem> ActorsToCheckQueue;
	TProcessingQueue<TWeakObjectPtr<ULevel>> LevelsProcessingQueue;
	int LevelActorProcessingQueueIndex = 0;

	UPROPERTY(Transient, DuplicateTransient)
	FMeshBlendPooledActors PooledActors;

	UPROPERTY(Transient, DuplicateTransient)
	FMeshBlendPCGHelper MeshBlendPCGHelper;

	void RefreshPooledActor(AActor* Actor);

	UPROPERTY(Transient, DuplicateTransient)
	TSet<FString> MaterialsMissingAutoBlendID;

	UPROPERTY(Transient, DuplicateTransient)
	bool CachedCVarMeshBlendEnable = true;

	UPROPERTY(Transient, DuplicateTransient)
	bool CachedMeshBlendRunInEditor = false;

	UPROPERTY(Transient, DuplicateTransient)
	bool IsBlendingEnabled = false;

	UPROPERTY(Transient, DuplicateTransient)
	bool IsInitialized = false;

	UPROPERTY(Transient, DuplicateTransient)
	TObjectPtr<UMeshBlendProcessor> MeshBlendProcessor;

#if WITH_EDITOR

public:
	static void TryResetMeshActivator();
	void RefreshActorsReferencingAsset(const UStaticMesh* StaticMeshAsset);

private:
	void OnPreSaveWorld(UWorld* World, FObjectPreSaveContext ObjectSaveContext);
	void OnPostSaveWorld(UWorld* World, FObjectPostSaveContext ObjectSaveContext);
	void OnPreSaveExternalActors(UWorld* World);

#endif
};
