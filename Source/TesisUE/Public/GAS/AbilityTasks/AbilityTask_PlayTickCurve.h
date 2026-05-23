#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "UObject/ObjectMacros.h"
#include "Curves/CurveFloat.h"
#include "AbilityTask_PlayTickCurve.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayTickCurveDelegate, float, CurrentValue, float, CurrentTime);

UCLASS()
class UAbilityTask_PlayTickCurve : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAbilityTask_PlayTickCurve();

	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_PlayTickCurve* PlayTickCurve(UGameplayAbility* OwningAbility, UCurveFloat* CurveAsset, float DurationMultiplier = 1.0f, float UpdateInterval = 1.0f, bool bReverse = false);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

	UPROPERTY(BlueprintAssignable)
	FPlayTickCurveDelegate OnUpdate;

	UPROPERTY(BlueprintAssignable)
	FPlayTickCurveDelegate OnFinished;

protected:
	UPROPERTY()
	UCurveFloat* CurveFloat;

	float DurationMultiplier;
	float UpdateInterval;
	bool bReverse;

	float MaxTime;
	float CurrentTime;
	float TimeSinceLastUpdate;
};