#include "GAS/AbilityTasks/AbilityTask_PlayTickCurve.h"
#include "Math/UnrealMathUtility.h"

UAbilityTask_PlayTickCurve::UAbilityTask_PlayTickCurve()
{
    bTickingTask = true;
    bReverse = false;
    DurationMultiplier = 1.0f;
    UpdateInterval = 1.0f;
    TimeSinceLastUpdate = 0.0f;
}

UAbilityTask_PlayTickCurve* UAbilityTask_PlayTickCurve::PlayTickCurve(UGameplayAbility* OwningAbility, UCurveFloat* CurveAsset, float DurationMultiplier, float UpdateInterval, bool bReverse)
{
    UAbilityTask_PlayTickCurve* MyTask = NewAbilityTask<UAbilityTask_PlayTickCurve>(OwningAbility);
    MyTask->CurveFloat = CurveAsset;
    MyTask->DurationMultiplier = DurationMultiplier;
    MyTask->UpdateInterval = UpdateInterval;
    MyTask->bReverse = bReverse;
    return MyTask;
}

void UAbilityTask_PlayTickCurve::Activate()
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

    TimeSinceLastUpdate = UpdateInterval;
}

void UAbilityTask_PlayTickCurve::TickTask(float DeltaTime)
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

       TimeSinceLastUpdate += DeltaTime;

       bool bShouldFinish = false;

       if (bReverse)
       {
          if (CurrentTime <= 0.0f)
          {
             bShouldFinish = true;
             CurrentTime = 0.0f; 
          }
       }
       else
       {
          if (CurrentTime >= MaxTime)
          {
             bShouldFinish = true;
             CurrentTime = MaxTime;
          }
       }

       if (TimeSinceLastUpdate >= UpdateInterval || bShouldFinish)
       {
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
           
           TimeSinceLastUpdate = FMath::Fmod(TimeSinceLastUpdate, UpdateInterval); 
       }

       if (bShouldFinish)
       {
          float FinalValue = 0.0f;
          if (CurveFloat)
          {
              FinalValue = CurveFloat->GetFloatValue(CurrentTime);
          }
          
          OnFinished.Broadcast(FinalValue, CurrentTime);
          EndTask();
       }
    }
}

void UAbilityTask_PlayTickCurve::OnDestroy(bool bInOwnerFinished)
{
    Super::OnDestroy(bInOwnerFinished);
}