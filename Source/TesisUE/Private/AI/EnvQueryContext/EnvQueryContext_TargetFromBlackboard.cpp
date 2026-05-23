// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EnvQueryContext/EnvQueryContext_TargetFromBlackboard.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UEnvQueryContext_TargetFromBlackboard::ProvideContext(FEnvQueryInstance& QueryInstance,
                                                           FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);

	const AActor* QueryOwner = Cast<AActor>(QueryInstance.Owner.Get());
    
	if (AAIController* AIController = Cast<AAIController>(QueryOwner->GetInstigatorController()))
	{
		if (const UBlackboardComponent* BB = AIController->GetBlackboardComponent())
		{
			if (const AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject("TargetActor")))
			{
				UEnvQueryItemType_Actor::SetContextHelper(ContextData, TargetActor);
			}
		}
	}
}
