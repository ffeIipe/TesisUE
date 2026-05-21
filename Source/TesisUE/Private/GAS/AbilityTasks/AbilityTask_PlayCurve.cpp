#include "GAS/AbilityTasks/AbilityTask_PlayCurve.h"

UAbilityTask_PlayCurve::UAbilityTask_PlayCurve()
{
	bTickingTask = true;
	bReverse = false;
}

UAbilityTask_PlayCurve* UAbilityTask_PlayCurve::PlayCurve(UGameplayAbility* OwningAbility, UCurveFloat* CurveAsset, float DurationMultiplier, bool bReverse)
{
	UAbilityTask_PlayCurve* MyTask = NewAbilityTask<UAbilityTask_PlayCurve>(OwningAbility);
	MyTask->CurveFloat = CurveAsset;
	MyTask->DurationMultiplier = DurationMultiplier;
	MyTask->bReverse = bReverse;
	return MyTask;
}

void UAbilityTask_PlayCurve::Activate()
{
	Super::Activate();
	
	if (CurveFloat)
	{
		float MinTime;
		CurveFloat->GetTimeRange(MinTime, MaxTime);
	}
	else
	{
		MaxTime = 1.0f;
	}
	
	if (bReverse)
	{
		CurrentTime = MaxTime;
	}
	else
	{
		CurrentTime = 0.0f;
	}
}

void UAbilityTask_PlayCurve::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		float TimeStep = DeltaTime * DurationMultiplier;

		if (bReverse)
		{
			CurrentTime -= TimeStep;
		}
		else
		{
			CurrentTime += TimeStep;
		}
		
		float CurrentValue = 0.0f;
		if (CurveFloat)
		{
			CurrentValue = CurveFloat->GetFloatValue(CurrentTime);
		}
		else
		{
			CurrentValue = FMath::Clamp(CurrentTime / MaxTime, 0.0f, 1.0f);
		}

		OnUpdate.Broadcast(CurrentValue, CurrentTime);

		bool bShouldFinish = false;

		if (bReverse)
		{
			if (CurrentTime <= 0.0f)
			{
				bShouldFinish = true;
				CurrentTime = 0.0f; 
				if(CurveFloat) CurrentValue = CurveFloat->GetFloatValue(0.0f);
			}
		}
		else
		{
			if (CurrentTime >= MaxTime)
			{
				bShouldFinish = true;
			}
		}

		if (bShouldFinish)
		{
			OnFinished.Broadcast(CurrentValue, CurrentTime);
			EndTask();
		}
	}
}

void UAbilityTask_PlayCurve::OnDestroy(bool bInOwnerFinished)
{
	Super::OnDestroy(bInOwnerFinished);
}