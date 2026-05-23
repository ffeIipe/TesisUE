#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Features/GlobalEffectsSystem/Data/EffectsManagerData.h"
#include "Features/GlobalEffectsSystem/Data/Types.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "Features/GlobalEffectsSystem/Interfaces/EffectManagerProvider.h"
#include "EffectsManager.generated.h"

class UCameraShakeBase;
class UCameraComponent;

UCLASS()
class TESISUE_API UEffectsManager : public UWorldSubsystem, public FTickableGameObject, public IEffectManagerProvider
{
	GENERATED_BODY()

public:
	virtual void PlayGameplayEffect_Implementation(FGameplayTag EffectTag, FVector Location) override;
	
	UFUNCTION(BlueprintCallable)
	void CameraShake(FGameplayTag EffectTag, const FVector& Epicenter) const;

	UFUNCTION(BlueprintCallable)
	void CameraZoom(FGameplayTag CameraZoomTag);

	UFUNCTION(BlueprintCallable)
	void HitStop(FGameplayTag HitStopTag);

	UFUNCTION(BlueprintCallable)
	void TimeWarp(FGameplayTag TimeWarpTag);
	
private:
	/** Initialization */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;
	
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	virtual void Tick(float DeltaTime) override;

	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; }
	
	virtual TStatId GetStatId() const override;

	virtual bool IsTickableInEditor() const override { return false; }

	virtual bool IsTickable() const override;

	/** Refs */
	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(Transient)
	TObjectPtr<ACameraActor> FollowCamera;
	
	UPROPERTY(Transient)
	TObjectPtr<UEffectsManagerData> Data;
	
	/** Variables */
	UPROPERTY()
	FZoomEffectParameters ZoomEffectParams;

	UPROPERTY()
	FTimeWarpEffectParameters TimeWarpEffectParams;

	UPROPERTY()
	FHitStopEffectParameters HitStopEffectParams;

	bool bHasActiveTickableEffects;

	void UpdateActiveTickableEffectsStatus();
};
