#pragma once

#include "CoreMinimal.h"
#include "DetourCrowdAIController.h"
#include "CustomAIController.generated.h"

UCLASS()
class TESISUE_API ACustomAIController : public ADetourCrowdAIController
{
	GENERATED_BODY()

public:
	ACustomAIController(const FObjectInitializer& ObjectInitializer);

	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

	UFUNCTION(BlueprintCallable, Category = "Team")
	virtual FGenericTeamId GetGenericTeamId() const override;

	UFUNCTION(BlueprintCallable, Category = "Team")
	virtual void SetGenericTeamId(const FGenericTeamId& TeamId) override;
};
