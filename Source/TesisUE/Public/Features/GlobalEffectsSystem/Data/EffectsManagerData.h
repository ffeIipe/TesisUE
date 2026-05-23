// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Features/GlobalEffectsSystem/Data/Types.h"
#include "EffectsManagerData.generated.h"

UCLASS()
class TESISUE_API UEffectsManagerData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Shake")
	TMap<FGameplayTag, FCameraShakeType> CameraShakes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera Zoom")
	TMap<FGameplayTag, FCameraZoomType> CameraZooms;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time Warp")
	TMap<FGameplayTag, FTimeWarpType> TimeWarps;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Stop")
	TMap<FGameplayTag, FHitStopType> HitStops;
};
