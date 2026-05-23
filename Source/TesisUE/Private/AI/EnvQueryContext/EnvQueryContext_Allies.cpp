// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EnvQueryContext/EnvQueryContext_Allies.h"

#include "AIController.h"
#include "AI/Core/CombatDirectorSubsystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UEnvQueryContext_Allies::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	AActor* QueryOwner = Cast<AActor>(QueryInstance.Owner.Get());
	if (!QueryOwner) return;

	AAIController* AICon = Cast<AAIController>(QueryOwner->GetInstigatorController());
	if (!AICon || !AICon->GetBlackboardComponent()) return;

	AActor* TargetActor = Cast<AActor>(AICon->GetBlackboardComponent()->GetValueAsObject("TargetActor"));

	if (!TargetActor) return;

	const UWorld* World = QueryOwner->GetWorld();
	if (!World) return;

	const UCombatDirectorSubsystem* CombatDirector = World->GetSubsystem<UCombatDirectorSubsystem>();
	if (!CombatDirector) return;

	TArray<AActor*> Allies = CombatDirector->GetAlliesForTarget(TargetActor);

	Allies.Remove(Cast<AActor>(QueryInstance.Owner.Get()));

	UEnvQueryItemType_Actor::SetContextHelper(ContextData, Allies);
}
