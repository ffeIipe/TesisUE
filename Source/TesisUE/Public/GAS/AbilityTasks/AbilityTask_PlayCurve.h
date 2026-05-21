#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_PlayCurve.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayCurveDelegate, float, CurrentValue, float, CurrentTime);

UCLASS()
class TESISUE_API UAbilityTask_PlayCurve : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAbilityTask_PlayCurve();

	UPROPERTY(BlueprintAssignable)
	FPlayCurveDelegate OnUpdate;

	UPROPERTY(BlueprintAssignable)
	FPlayCurveDelegate OnFinished;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_PlayCurve* PlayCurve(UGameplayAbility* OwningAbility, UCurveFloat* CurveAsset, float DurationMultiplier = 1.0f, bool bReverse = false);

protected:
	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	UPROPERTY()
	UCurveFloat* CurveFloat;

	float CurrentTime;
	float DurationMultiplier;
	float MaxTime;
	bool bReverse;
};