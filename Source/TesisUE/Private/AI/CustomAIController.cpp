#include "AI/CustomAIController.h"
#include "Perception/AIPerceptionComponent.h"

ACustomAIController::ACustomAIController(const FObjectInitializer& ObjectInitializer)
{
    AAIController::SetGenericTeamId(FGenericTeamId(0));
}

ETeamAttitude::Type ACustomAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
    const APawn* PawnToCheck = Cast<APawn>(&Other);

    if (const IGenericTeamAgentInterface* OtherTeamAgent = Cast<IGenericTeamAgentInterface>(PawnToCheck->GetController()))
    {
        const FGenericTeamId OtherTeamId = OtherTeamAgent->GetGenericTeamId();

        if (OtherTeamId == FGenericTeamId(0)) return ETeamAttitude::Neutral;
        if (OtherTeamId == FGenericTeamId(1)) return ETeamAttitude::Friendly;
        if (OtherTeamId == FGenericTeamId(2)) return ETeamAttitude::Hostile;
    }
    return ETeamAttitude::Neutral;
}

FGenericTeamId ACustomAIController::GetGenericTeamId() const
{
    return Super::GetGenericTeamId();
}

void ACustomAIController::SetGenericTeamId(const FGenericTeamId& TeamId)
{
    Super::SetGenericTeamId(TeamId);
}
