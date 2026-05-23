#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameplayTagContainer.h"
#include "TriggerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTagTriggerDelegate, AActor*, Actor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTagTriggerExitDelegate, AActor*, Actor);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TESISUE_API UTriggerComponent : public UBoxComponent
{
	GENERATED_BODY()

public:	
	UTriggerComponent();

	UPROPERTY(BlueprintAssignable)
	FOnTagTriggerDelegate OnActorEntered;

	UPROPERTY(BlueprintAssignable)
	FOnTagTriggerExitDelegate OnActorExit;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties")
	FGameplayTagContainer AcceptedTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties")
	FGameplayTagContainer IgnoredTags;

	UFUNCTION()
	virtual void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
private:
	bool HasMatchingGameplayTags(AActor* Actor) const;
};