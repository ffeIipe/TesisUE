// Copyright 2024 Tore Lervik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MeshBlendSceneViewExtension.h"
#include "Subsystems/EngineSubsystem.h"
#include "MeshBlendSubSystem.generated.h"

/**
 * 
 */
UCLASS()
class MESHBLEND_API UMeshBlendSubSystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	UMeshBlendSubSystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	TSharedPtr<class FMeshBlendSceneViewExtension, ESPMode::ThreadSafe> MeshBlendSceneViewExtension;

private:
	UPROPERTY()
	TObjectPtr<UTexture2D> NoiseTexture;
};
