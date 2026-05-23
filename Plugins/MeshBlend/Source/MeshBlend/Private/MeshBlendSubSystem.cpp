// Copyright 2024 Tore Lervik. All Rights Reserved.


#include "MeshBlendSubSystem.h"
#include "SceneViewExtension.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"

UMeshBlendSubSystem::UMeshBlendSubSystem()
{
	static ConstructorHelpers::FObjectFinder<UTexture2D> NoiseTextureObject(TEXT("/MeshBlend/Materials/TilingNoise05.TilingNoise05"));

	if (NoiseTextureObject.Succeeded())
	{
		NoiseTexture = NoiseTextureObject.Object;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load noise texture (/MeshBlend/Materials/TilingNoise05.TilingNoise05) for MeshBlend"));
	}
}

void UMeshBlendSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	MeshBlendSceneViewExtension = FSceneViewExtensions::NewExtension<FMeshBlendSceneViewExtension>();
	MeshBlendSceneViewExtension->NoiseTexture = NoiseTexture;
}

void UMeshBlendSubSystem::Deinitialize()
{
	Super::Deinitialize();

	if (MeshBlendSceneViewExtension.IsValid())
	{
		MeshBlendSceneViewExtension.Reset();
	}
}
