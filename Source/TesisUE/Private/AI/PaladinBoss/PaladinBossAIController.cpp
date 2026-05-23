#include "AI/PaladinBoss/PaladinBossAIController.h"

// void APaladinBossAIController::OnEnemyPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
// {
//     AEnemy* Enemy = Cast<AEnemy>(GetPawn());
//     APawn* PlayerPawn = Cast<APawn>(Actor);
//     UCharacterStateComponent* PlayerCharStateComp = PlayerPawn ? PlayerPawn->FindComponentByClass<UCharacterStateComponent>() : nullptr;
//
//     if (!BlackboardComponent || !PlayerPawn || !Enemy) return;
//     
//     if (Stimulus.WasSuccessfullySensed())
//     {
//         if (PlayerCharStateComp->GetCurrentCharacterState().Form != ECharacterForm::ECF_Possessing)
//         {
//             BlackboardComponent->SetValueAsObject(FName("TargetActor"), Actor);
//         }
//         else
//         {
//             if (DamageCauser != Actor)
//             {
//                 BlackboardComponent->SetValueAsObject(FName("TargetActor"), DamageCauser);
//             }
//             else
//             {
//                 BlackboardComponent->SetValueAsObject(FName("TargetActor"), nullptr);
//                 DamageCauser = nullptr;
//             }
//         }
//     }
// }