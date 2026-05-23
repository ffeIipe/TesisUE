// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Wizard/WizardAIController.h"

//void AWizardAIController::OnEnemyPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
//{
//    AEnemy* Enemy = Cast<AEnemy>(GetPawn());
//    APawn* PlayerPawn = Cast<APawn>(Actor);
//    UCharacterStateComponent* CharacterStateComponent = PlayerPawn ? PlayerPawn->FindComponentByClass<UCharacterStateComponent>() : nullptr;
//
//    if (!BlackboardComponent || !PlayerPawn || !Enemy || Enemy->GetCharacterStateComponent()->GetCurrentCharacterState().Action == ECharacterActions::ECA_Dead) return;
//    
//
//    if (!CharacterStateComponent)
//    {
//        BlackboardComponent->ClearValue(FName("TargetActor"));
//        BlackboardComponent->ClearValue(FName("DistToTarget"));
//
//        return;
//    }
//
//    if (Stimulus.WasSuccessfullySensed())
//    {
//
//        BlackboardComponent->SetValueAsObject(FName("TargetActor"), Actor);
//    }
//}
