// Copyright 2024 Tore Lervik. All Rights Reserved.

#include "MeshBlendActivator.h"

#include "AutoBlendUserData.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "MeshBlendShared.h"
#include "PrimitiveDataHelper.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/Level.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/EngineVersionComparison.h"

#if WITH_EDITOR
#include "Editor.h"
#include "EditorViewportClient.h"
#include "Engine/Selection.h"
#include "Logging/MessageLog.h"
#include "UObject/ObjectSaveContext.h"
#endif

#define LOCTEXT_NAMESPACE "AMeshBlendActivator"

static TAutoConsoleVariable<bool> CVarMeshBlendEnable(
	TEXT("r.MeshBlend.Enable"),
	true,
	TEXT("Enable/Disable the Activator. This command has no effect while the game is running."));

#if WITH_EDITOR

static TAutoConsoleVariable<bool> CVarMeshBlendRunInEditor(
	TEXT("r.MeshBlend.RunInEditor"),
	true,
	TEXT("Enable/Disable the plugin from running in the editor."));

static TAutoConsoleVariable<bool> CVarMeshBlendDisableRestrictions(
	TEXT("r.MeshBlend.DisableRestrictions"),
	false,
	TEXT("Enable/Disable the distance and time budget of the activator. (Used with sequencer movie rendering)"));

static TAutoConsoleVariable<bool> CVarMeshBlendCustomerGBufferChannel(
	TEXT("r.MeshBlend.CustomerGBufferChannel"),
	false,
	TEXT("When true will stop warnings related to Material AO workflow and hide the shader patcher tool."));

#endif

AMeshBlendActivator::AMeshBlendActivator(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_DuringPhysics;
	ActorsProcessingQueue.Reserve(100000);


#if WITH_EDITOR
	bIsSpatiallyLoaded = false;

	if (const UWorld* World = GetWorld())
	{
		if (World->WorldType == EWorldType::Editor)
		{
			FEditorDelegates::PreSaveWorldWithContext.AddUObject(this, &AMeshBlendActivator::OnPreSaveWorld);
			FEditorDelegates::PostSaveWorldWithContext.AddUObject(this, &AMeshBlendActivator::OnPostSaveWorld);
			FEditorDelegates::PreSaveExternalActors.AddUObject(this, &AMeshBlendActivator::OnPreSaveExternalActors);
		}
	}
#endif
}

AMeshBlendActivator::~AMeshBlendActivator()
{
#if WITH_EDITOR
	FEditorDelegates::PreSaveWorldWithContext.RemoveAll(this);
	FEditorDelegates::PostSaveWorldWithContext.RemoveAll(this);
	FEditorDelegates::PreSaveExternalActors.RemoveAll(this);
#endif

	FWorldDelegates::LevelAddedToWorld.RemoveAll(this);
}

bool AMeshBlendActivator::ShouldTickIfViewportsOnly() const
{
	return true;
}

void AMeshBlendActivator::CheckActivatorInitialization()
{
	if (IsInitialized)
	{
		return;
	}

	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	MeshBlendProcessor = NewObject<UMeshBlendProcessor>(this);
	MeshBlendProcessor->SmallBlendCounter = StencilStartSmall;
	MeshBlendProcessor->MediumBlendCounter = StencilStartMedium;
	MeshBlendProcessor->LargeBlendCounter = StencilStartLarge;
	MeshBlendProcessor->ExtraLargeBlendCounter = StencilStartExtraLarge;

	PooledActors.OnRefreshActor.BindUObject(this, &AMeshBlendActivator::RefreshPooledActor);

	CurrentMaxRadius = TNumericLimits<float>::Max();
	CurrentProcessStage = FindDistance;

	if (UWorld* World = GetWorld())
	{
		World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this, &AMeshBlendActivator::Level_OnActorSpawned));

		FWorldDelegates::LevelAddedToWorld.AddUObject(this, &AMeshBlendActivator::World_OnLevelAdded);
		FWorldDelegates::LevelRemovedFromWorld.AddUObject(this, &AMeshBlendActivator::World_OnLevelRemoved);

		for (ULevel* Level : World->GetLevels())
		{
			World_OnLevelAdded(Level, World);
		}

		IsInitialized = true;
	}
}

void AMeshBlendActivator::BeginPlay()
{
	Super::BeginPlay();
	IsBlendingEnabled = CVarMeshBlendEnable.GetValueOnGameThread();
	CheckActivatorInitialization();
}

void AMeshBlendActivator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

#if WITH_EDITOR
	const bool ShouldStopBlending = !CVarMeshBlendRunInEditor.GetValueOnGameThread();

	if (UWorld* World = GetWorld())
	{
		if (ShouldStopBlending)
		{
			DisableMeshBlendActivator(World, true);
		}
	}
#endif
}

void AMeshBlendActivator::PostInitProperties()
{
	Super::PostInitProperties();
}

void AMeshBlendActivator::EmptyProcessingQueues()
{
	ActorsAlreadyProcessed.Empty();
	ActorsProcessingQueue.Empty();
	ActorsToCheckQueue.Empty();
	LevelsProcessingQueue.Empty();
	CurrentProcessStage = FindDistance;

	if (MeshBlendProcessor)
	{
		MeshBlendProcessor->Reset();
	}
}

void AMeshBlendActivator::BeginDestroy()
{
	Super::BeginDestroy();

	EmptyProcessingQueues();
	MaterialsMissingAutoBlendID.Reset();
}


void AMeshBlendActivator::World_OnLevelAdded(ULevel* InLevel, UWorld* World)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	if (World && IsValid(InLevel))
	{
#if WITH_EDITOR
		InLevel->OnLoadedActorAddedToLevelPostEvent.AddUObject(this, &AMeshBlendActivator::Level_OnActorsAdded);
#endif

		LevelsProcessingQueue.Add(InLevel);
	}
}

void AMeshBlendActivator::World_OnLevelRemoved(ULevel* InLevel, UWorld* World)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	if (World && IsValid(InLevel))
	{
#if WITH_EDITOR
		InLevel->OnLoadedActorAddedToLevelPostEvent.RemoveAll(this);
#endif
	}
}

void AMeshBlendActivator::Level_OnActorsAdded(const TArray<AActor*>& Actors)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	for (AActor* Actor : Actors)
	{
		Level_OnActorSpawned(Actor);
	}
}

void AMeshBlendActivator::Level_OnActorSpawned(AActor* Actor)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	if (ActorsAlreadyProcessed.Contains(Actor))
	{
		return;
	}

	FActorProcessingItem Item;
	Item.bRefresh = true;
	Item.Actor = Actor;
	CheckActor(Item);
}

void AMeshBlendActivator::OnActorSpawned(AActor* Actor)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	if (!IsValid(Actor))
	{
		return;
	}

	if (Actor->Tags.Contains(GName_AutoBlendHasPrimitiveData))
	{
#if WITH_EDITOR
		ResetActor(Actor, true); // Reset actor since the tags are copied to PIE while the CPD values are not
#else
		return; // Opt out early if the actor is not relevant for blending
#endif
	}

	if (FUAutoBlendHelper::DoesItBlend(Actor))
	{
		FActorInformation ActorInformation;
		ActorInformation.Actor = Actor;
		ActorInformation.Bounds = FMeshBlendShared::GetBounds(Actor);
		ActorsProcessingQueue.Add(ActorInformation);
	}
}

void AMeshBlendActivator::Tick(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
	Super::Tick(DeltaTime);

	if (!GEngine)
	{
		return;
	}

	const UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	if (SyncConsoleVariables())
	{
		return;
	}

	const double ProcessTimeStart = FPlatformTime::Seconds();
	double ProcessBudgetInSeconds = ProcessBudget / 1000.0;

#if WITH_EDITOR
	// Used to ensure blending is instant when rendering a movie in sequencer
	if (CVarMeshBlendDisableRestrictions.GetValueOnGameThread())
	{
		ProcessBudgetInSeconds = 1000000;
	}
#endif


	if (!IsBlendingEnabled)
	{
		return;
	}

	FVector Origin;

	if (!FMeshBlendShared::GetCurrentCameraLocation(World, Origin))
	{
		return;
	}

	const double MaxProcessDuration = ProcessTimeStart + ProcessBudgetInSeconds;

	CheckActivatorInitialization();

	if (!ProcessAddedLevels(MaxProcessDuration))
	{
		return;
	};

	if (!PooledActors.ProcessActorQueue(MaxProcessDuration))
	{
		return;
	}

	if (!ProcessActorsToCheckQueue(MaxProcessDuration))
	{
		return;
	};

	if (CurrentProcessStage == FindDistance)
	{
		if (CalculateMaxProcessingRadius(World, Origin, MaxProcessDuration))
		{
			CurrentProcessStage = EProcessStage::ProcessActors;
			ActorsProcessingQueue.ResetIndexToStart();
		}
	}
	else if (CurrentProcessStage == EProcessStage::ProcessActors)
	{
		if (ProcessActors(World, Origin, ProcessTimeStart, MaxProcessDuration))
		{
			CurrentProcessStage = FindDistance;
			CurrentMaxRadius = TNumericLimits<float>::Max();
			ActorsProcessingQueue.ResetIndexToStart();
		}
	}
}

bool AMeshBlendActivator::ProcessAddedLevels(const double MaxProcessDuration)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	TWeakObjectPtr<ULevel> LevelPointer;

	while (LevelsProcessingQueue.Next(LevelPointer))
	{
		if (FMeshBlendShared::IsOverProcessBudget(MaxProcessDuration))
		{
			LevelsProcessingQueue.KeepCurrentAsNext();
			return false;
		}

		if (!LevelPointer.IsValid())
		{
			LevelsProcessingQueue.RemoveCurrent();
			continue;
		}

		// We reset the process stage to ensure we update the CurrentMaxRadius after a level has been added
		CurrentProcessStage = FindDistance;
		CurrentMaxRadius = TNumericLimits<float>::Max();
		ActorsProcessingQueue.ResetIndexToStart();

		ULevel* Level = LevelPointer.Get();

		for (int y = LevelActorProcessingQueueIndex; y < Level->Actors.Num(); y++)
		{
			Level_OnActorSpawned(Level->Actors[y]);

			if (FMeshBlendShared::IsOverProcessBudget(MaxProcessDuration))
			{
				LevelsProcessingQueue.KeepCurrentAsNext();
				LevelActorProcessingQueueIndex = y;
				return false;
			}
		}

		LevelActorProcessingQueueIndex = 0;
		LevelsProcessingQueue.RemoveCurrent();
	}

	return true;
}

bool AMeshBlendActivator::ProcessActorsToCheckQueue(const double MaxProcessDuration)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	FActorProcessingItem ActorProcessingItem;

	while (ActorsToCheckQueue.Next(ActorProcessingItem))
	{
		if (FMeshBlendShared::IsOverProcessBudget(MaxProcessDuration))
		{
			ActorsToCheckQueue.KeepCurrentAsNext();
			return false;
		}

		if (!ActorProcessingItem.Actor.IsValid())
		{
			ActorsToCheckQueue.RemoveCurrent();
			continue;
		}

		HandleActorProcessingItem(ActorProcessingItem);
		ActorsToCheckQueue.RemoveCurrent();
	}

	return true;
}

void AMeshBlendActivator::HandleActorProcessingItem(const FActorProcessingItem& ActorProcessingItem)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	AActor* Actor = ActorProcessingItem.Actor.Get();

	if (ActorProcessingItem.bReset)
	{
		ResetActor(Actor, ActorProcessingItem.bSoftReset);
	}

	if (ActorProcessingItem.bRefresh)
	{
		OnActorSpawned(Actor);
	}

	if (MeshBlendPCGHelper.ActorIsPCGActor(Actor))
	{
		PooledActors.AddActor(Actor);
	}
}

void AMeshBlendActivator::RefreshPooledActor(AActor* Actor)
{
	Actor->Tags.Remove(GName_AutoBlendHasPrimitiveData);

	FActorInformation ActorInformation;
	ActorInformation.Actor = Actor;
	ActorInformation.Bounds = FMeshBlendShared::GetBounds(Actor);
	ActorsProcessingQueue.Add(ActorInformation);
}

bool AMeshBlendActivator::CalculateMaxProcessingRadius(const UWorld* World, const FVector& Origin, const double MaxProcessDuration)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	FActorInformation ActorInformation;
	while (ActorsProcessingQueue.Next(ActorInformation, 50))
	{
		if (FMeshBlendShared::IsOverProcessBudget(MaxProcessDuration))
		{
			ActorsProcessingQueue.KeepCurrentAsNext();
			return false;
		}

		if (!ActorInformation.Actor.IsValid())
		{
			ActorsProcessingQueue.RemoveCurrent();
			continue;
		}

		const AActor* Actor = ActorInformation.Actor.Get();
		FActorBounds ActorBounds = ActorInformation.Bounds;
		const float ActorDistance = FVector::DistSquared(Origin, ActorBounds.Origin) * (1.0 + FMath::Min(10.0, World->TimeSince(Actor->GetLastRenderTime())));

		if (ActorDistance < CurrentMaxRadius * CurrentMaxRadius)
		{
			CurrentMaxRadius = FMath::Sqrt(ActorDistance);

			// Abort early if we're already at the minimum distance threshold
			if (CurrentMaxRadius < MinDistanceThreshold)
			{
				CurrentMaxRadius = MinDistanceThreshold;
				ActorsProcessingQueue.ResetIndexToStart();
				return true;
			}
		}
	}

	CurrentMaxRadius = FMath::CeilToFloat(CurrentMaxRadius / MinDistanceThreshold) * MinDistanceThreshold;
	ActorsProcessingQueue.ResetIndexToStart();
	return true;
}

void AMeshBlendActivator::ShowMissingScalarParameterWarning(const UMaterialInterface* Material)
{
	// Only show this warning once per material
	if (!MaterialsMissingAutoBlendID.Contains(*Material->GetFullName()))
	{
		MaterialsMissingAutoBlendID.Add(*Material->GetFullName());
		const FString WarningMessage = FString::Printf(
			TEXT("MeshBlend warning: Material \"%s\" is missing scalar parameter %s"),
			*Material->GetFullName(),
			*GName_AutoBlendID.ToString());
		UE_LOG(LogTemp, Warning, TEXT("%s"), *WarningMessage);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, WarningMessage);
	}
}

bool AMeshBlendActivator::ProcessActors(const UWorld* World, const FVector& Origin, const double ProcessTimeStart, const double MaxProcessDuration)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	const double SkipCheckTime = ProcessTimeStart + 5.0;

	FActorInformation ActorInformation;
	while (ActorsProcessingQueue.Next(ActorInformation))
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("AMeshBlendActivator::ProcessActors::ActorInformation");

		if (FMeshBlendShared::IsOverProcessBudget(MaxProcessDuration))
		{
			ActorsProcessingQueue.KeepCurrentAsNext();
			return false;
		}

		if (ActorInformation.SkipCheckingUntil > ProcessTimeStart)
		{
			continue;
		}

		if (!ActorInformation.Actor.IsValid())
		{
			ActorsProcessingQueue.RemoveCurrent();
			continue;
		}


		AActor* Actor = ActorInformation.Actor.Get();
		FActorBounds ActorBounds = ActorInformation.Bounds;
		float ActorDistanceSquared = FVector::DistSquared(Origin, ActorBounds.Origin) * (1.0 + FMath::Min(10.0, World->TimeSince(Actor->GetLastRenderTime())));
		const float SkipCheckDistanceThreshold = CurrentMaxRadius + ActorBounds.Radius + MinDistanceThreshold * 2;

#if WITH_EDITOR
		// Used to ensure blending is instant when rendering a movie in sequencer
		if (CVarMeshBlendDisableRestrictions.GetValueOnGameThread())
		{
			ActorDistanceSquared = 0;
		}
#endif

		if (ActorDistanceSquared > SkipCheckDistanceThreshold * SkipCheckDistanceThreshold)
		{
			ActorInformation.SkipCheckingUntil = SkipCheckTime;
			continue;
		}

		const float DistanceThreshold = CurrentMaxRadius + ActorBounds.Radius;

		if (ActorDistanceSquared < DistanceThreshold * DistanceThreshold)
		{
			if (!MeshBlendProcessor->ActivateActor(Actor, MaxProcessDuration))
			{
				ActorsProcessingQueue.KeepCurrentAsNext();
				return false;
			}

			ActorsProcessingQueue.RemoveCurrent();
		}
	}

	ActorsProcessingQueue.ResetIndexToStart();
	return true;
}

void AMeshBlendActivator::ResetMeshActivator(const UWorld* World, const bool bSoftReset = false)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	EmptyProcessingQueues();

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (AActor* Actor = *It)
		{
			FActorProcessingItem Item;
			Item.Actor = Actor;
			Item.bReset = true;
			Item.bSoftReset = bSoftReset;
			Item.bRefresh = true;
			HandleActorProcessingItem(Item);
		}
	}

	IsBlendingEnabled = CVarMeshBlendEnable.GetValueOnGameThread();
}

void AMeshBlendActivator::DisableMeshBlendActivator(UWorld* World, const bool bSoftReset = false)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	IsBlendingEnabled = false;

	EmptyProcessingQueues();

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (AActor* Actor = *It)
		{
			FActorProcessingItem Item;
			Item.Actor = Actor;
			Item.bReset = true;
			Item.bSoftReset = bSoftReset;
			CheckActor(Item);
		}
	}
}

bool AMeshBlendActivator::IsLumenMaterialAOCVarCorrect()
{
	static const auto CVarMaterialAO = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Lumen.ScreenProbeGather.MaterialAO"));
	return !CVarMaterialAO || CVarMaterialAO->GetInt() == 0;
}

bool AMeshBlendActivator::IsAllowStaticLightingCVarCorrect()
{
	static const auto CVarAllowStaticLighting = IConsoleManager::Get().FindConsoleVariable(TEXT("r.AllowStaticLighting"));
	return !CVarAllowStaticLighting || CVarAllowStaticLighting->GetInt() == 0;
}

void AMeshBlendActivator::RefreshActorBlending(AActor* Actor, const bool bSoftReset = false)
{
	ResetActor(Actor, bSoftReset);
	OnActorSpawned(Actor);
}

bool AMeshBlendActivator::ResetActor(AActor* Actor, const bool bSoftReset = false)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	if (!Actor->Tags.Contains(GName_AutoBlendHasPrimitiveData))
	{
		return false;
	}

	TArray<UMeshComponent*> Meshes;
	Actor->Tags.RemoveSwap(GName_AutoBlendHasPrimitiveData);
	Actor->GetComponents<UMeshComponent>(Meshes, false);
	bool FoundActorsToReset = false;

	for (int j = 0; j < Meshes.Num(); j++)
	{
		if (UMeshComponent* MeshComponent = Meshes[j])
		{
			const int BlendTagIndex = MeshComponent->ComponentTags.IndexOfByKey(GName_AutoBlendHasPrimitiveData);

			if (BlendTagIndex != INDEX_NONE)
			{
#if UE_VERSION_OLDER_THAN(5, 5, 0)
				MeshComponent->ComponentTags.RemoveAtSwap(BlendTagIndex, 1, false);
#else
				MeshComponent->ComponentTags.RemoveAtSwap(BlendTagIndex, 1, EAllowShrinking::No);
#endif

				FoundActorsToReset = true;

				if (!bSoftReset)
				{
					const int32 CustomPrimitiveDataIndex = MeshComponent->GetCustomPrimitiveDataIndexForScalarParameter(GName_AutoBlendID);

					if (CustomPrimitiveDataIndex != INDEX_NONE)
					{
						FPrimitiveDataHelper::EnsurePrimitiveDataSlot(MeshComponent, CustomPrimitiveDataIndex);

						if (UInstancedStaticMeshComponent* InstancedStaticMeshComponent = Cast<UInstancedStaticMeshComponent>(MeshComponent))
						{
							for (int32 i = 0; i < InstancedStaticMeshComponent->GetInstanceCount(); i++)
							{
								FPrimitiveDataHelper::SetCustomDataValue(InstancedStaticMeshComponent, i, CustomPrimitiveDataIndex, 0.0);
							}
						}
						else
						{
							MeshComponent->SetCustomPrimitiveDataFloat(CustomPrimitiveDataIndex, 0.0);
						}
					}
				}
			}
		}
	}

	return FoundActorsToReset;
}

void AMeshBlendActivator::CheckActor(const FActorProcessingItem& ActorProcessingItem)
{
	if (!ActorProcessingItem.Actor.IsValid())
	{
		return;
	}

	AActor* Actor = ActorProcessingItem.Actor.Get();

	if (ActorsAlreadyProcessed.Contains(Actor))
	{
		return;
	}

	ActorsAlreadyProcessed.Add(Actor);
	ActorsToCheckQueue.Add(ActorProcessingItem);
}

bool AMeshBlendActivator::SyncConsoleVariables()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	const bool MeshBlendEnable = CVarMeshBlendEnable.GetValueOnGameThread();
	bool bHasChanged = false;
	bool bShouldReset = false;
	bool bShouldDisable = false;

	if (CachedCVarMeshBlendEnable != MeshBlendEnable)
	{
		CachedCVarMeshBlendEnable = MeshBlendEnable;
		bHasChanged = true;

		if (MeshBlendEnable)
		{
			bShouldReset = true;
		}
		else
		{
			bShouldDisable = true;
		}
	}

#if WITH_EDITOR
	const bool MeshBlendRunInEditor = CVarMeshBlendRunInEditor.GetValueOnGameThread();

	if (CachedMeshBlendRunInEditor != MeshBlendRunInEditor)
	{
		CachedMeshBlendRunInEditor = MeshBlendRunInEditor;

		if (!bHasChanged)
		{
			bHasChanged = true;

			if (const UWorld* World = GetWorld())
			{
				if (!World->HasBegunPlay())
				{
					if (MeshBlendRunInEditor)
					{
						bShouldReset = true;
					}
					else
					{
						bShouldDisable = true;
					}
				}
			}
		}
	}
#endif

	if (UWorld* World = GetWorld())
	{
		if (bShouldReset)
		{
			ResetMeshActivator(World);
		}
		else if (bShouldDisable)
		{
			DisableMeshBlendActivator(World);
		}
	}

	return bHasChanged;
}

AMeshBlendActivator* AMeshBlendActivator::GetInstance(const UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	TArray<AActor*> ExistingActors;
	UGameplayStatics::GetAllActorsOfClass(World, StaticClass(), ExistingActors);

	if (ExistingActors.Num() > 0)
	{
		return Cast<AMeshBlendActivator>(ExistingActors[0]);
	}

	return nullptr;
}

#if WITH_EDITOR

void AMeshBlendActivator::TryResetMeshActivator()
{
	const UWorld* World = GWorld;

	if (GEditor && GEditor->PlayWorld != nullptr)
	{
		World = GEditor->PlayWorld;
	}

	if (World)
	{
		if (AMeshBlendActivator* Activator = GetInstance(World))
		{
			Activator->ResetMeshActivator(World, false);
		}
		else
		{
			FMessageLog("MapCheck")
				.Warning()
				->AddToken(FTextToken::Create(LOCTEXT(
					"MapCheck_MeshBlendActivatorNotFound",
					"BP_MeshBlend_Activator not found in the current level. MeshBlend needs this to work.")))
				->AddToken(FURLToken::Create(
					"https://meshblend.lervik.com",
					LOCTEXT("MapCheck_MeshBlendActivatorNotFound_UrlText", "Click to open documentation")));

			FMessageLog("MapCheck").Open(EMessageSeverity::Warning, true);
		}
	}

	if (GEditor && GEditor->PlayWorld == nullptr && !GEditor->IsAnyViewportRealtime())
	{
		FMessageLog("MapCheck")
			.Warning()
			->AddToken(FTextToken::Create(LOCTEXT(
				"MapCheck_MeshBlendViewportNotRealtime",
				"MeshBlend will only update blending in the editor when the viewport is realtime.")));

		FMessageLog("MapCheck").Open(EMessageSeverity::Warning, true);
	}

	if (GEditor)
	{
		if (CVarMeshBlendCustomerGBufferChannel.GetValueOnGameThread())
		{
			return;
		}

		if (!IsLumenMaterialAOCVarCorrect())
		{
			FMessageLog("MapCheck")
				.Warning()
				->AddToken(FTextToken::Create(LOCTEXT(
					"MapCheck_MeshBlendScreenProbeGatherMaterialAO",
					"r.Lumen.ScreenProbeGather.MaterialAO needs to be 0 to avoid shadow artifacts with MeshBlend.")))
				->AddToken(FURLToken::Create(
					"https://meshblend.lervik.com",
					LOCTEXT("MapCheck_MeshBlendActivatorNotFound_UrlText", "Click to open documentation")));

			FMessageLog("MapCheck").Open(EMessageSeverity::Warning, true);
		}

		if (!IsAllowStaticLightingCVarCorrect())
		{
			FMessageLog("MapCheck")
				.Warning()
				->AddToken(FTextToken::Create(LOCTEXT(
					"MapCheck_MeshBlendAllowStaticLighting",
					"MeshBlend: r.AllowStaticLighting needs to be 0 for MeshBlend to work. Adjust project settings.")))
				->AddToken(FURLToken::Create(
					"https://meshblend.lervik.com",
					LOCTEXT("MapCheck_MeshBlendActivatorNotFound_UrlText", "Click to open documentation")));

			FMessageLog("MapCheck").Open(EMessageSeverity::Warning, true);
		}
	}
}

void AMeshBlendActivator::OnPreSaveWorld(UWorld* World, FObjectPreSaveContext ObjectSaveContext)
{
	if (!ObjectSaveContext.IsCooking() && CVarMeshBlendRunInEditor.GetValueOnGameThread())
	{
		DisableMeshBlendActivator(World, true);
	}
}

void AMeshBlendActivator::OnPostSaveWorld(UWorld* World, FObjectPostSaveContext ObjectSaveContext)
{
	if (!ObjectSaveContext.IsCooking() && CVarMeshBlendRunInEditor.GetValueOnGameThread())
	{
		ResetMeshActivator(World, true);
	}
}

void AMeshBlendActivator::OnPreSaveExternalActors(UWorld* World)
{
	for (const UPackage* ExternalPackage : World->PersistentLevel->GetLoadedExternalObjectPackages())
	{
		if (!ExternalPackage->IsDirty())
		{
			continue;
		}

		TArray<UObject*> ExternalObjects;
		GetObjectsWithOuter(ExternalPackage, ExternalObjects);

		for (UObject* ExternalObject : ExternalObjects)
		{
			AActor* Actor = Cast<AActor>(ExternalObject);

			if (IsValid(Actor))
			{
				ActorsAlreadyProcessed.Remove(Actor);
				FActorProcessingItem Item;
				Item.Actor = Actor;
				Item.bReset = true;
				Item.bRefresh = true;
				CheckActor(Item);
			}
		}
	}
}

void AMeshBlendActivator::RefreshActorsReferencingAsset(const UStaticMesh* StaticMeshAsset)
{
	if (StaticMeshAsset && StaticMeshAsset->IsAsset())
	{
		if (const UWorld* World = GetWorld())
		{
			for (TActorIterator<AActor> It(World); It; ++It)
			{
				if (AActor* Actor = *It)
				{
					TArray<UObject*> ReferencedObjects;
					Actor->GetReferencedContentObjects(ReferencedObjects);

					for (const UObject* ReferencedObject : ReferencedObjects)
					{
						if (ReferencedObject == StaticMeshAsset)
						{
							ActorsAlreadyProcessed.Remove(Actor);
							FActorProcessingItem Item;
							Item.Actor = Actor;
							Item.bReset = true;
							Item.bRefresh = true;
							CheckActor(Item);
							break;
						}
					}
				}
			}
		}
	}
}

void AMeshBlendActivator::ToggleEnabled()
{
	CVarMeshBlendEnable->Set(!CVarMeshBlendEnable.GetValueOnGameThread(), ECVF_SetByConsole);

	if (GEditor)
	{
		GEditor->RedrawAllViewports(true);
	}
}

bool AMeshBlendActivator::IsEnabled()
{
	return CVarMeshBlendEnable.GetValueOnGameThread();
}

void AMeshBlendActivator::ToggleRunInEditor()
{
	CVarMeshBlendRunInEditor->Set(!CVarMeshBlendRunInEditor.GetValueOnGameThread(), ECVF_SetByConsole);

	if (GEditor)
	{
		GEditor->RedrawAllViewports(true);
	}
}

bool AMeshBlendActivator::IsRunInEditor()
{
	return CVarMeshBlendRunInEditor.GetValueOnGameThread();
}

void AMeshBlendActivator::ToggleDebugVisualization()
{
	static const auto CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MeshBlend.Visualize"));

	if (CVar->GetInt() == 0)
	{
		CVar->Set(1, ECVF_SetByConsole);
	}
	else
	{
		CVar->Set(0, ECVF_SetByConsole);
	}

	if (GEditor)
	{
		GEditor->RedrawAllViewports(true);
	}
}

bool AMeshBlendActivator::GetDebugVisualization()
{
	static const auto CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MeshBlend.Visualize"));
	return CVar->GetInt() == 1;
}

#endif

#undef LOCTEXT_NAMESPACE
