#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TargetingComponent.generated.h"

class AEntity;
class IWeaponInterface;
struct FTargetingData;
class UTimelineComponent;
class UCurveFloat;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetedEntityDead, AEntity*, EntityDead);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetedEntityChanged, AEntity*, NewEntity);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TESISUE_API UTargetingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTargetingComponent();

    UFUNCTION()
    void HandleTargetDeath(AEntity* DeadEntity);
    
    UFUNCTION(BlueprintCallable, Category = "Targeting")
    void EnableLock(float LockRange);

    UFUNCTION(BlueprintCallable, Category = "Targeting")
    void DisableLock();
    
    void RefreshTargets();

    UFUNCTION(BlueprintCallable, Category = "Targeting")
    void ChangeHardLockTarget();

    UFUNCTION(BlueprintCallable, Category = "Targeting")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintCallable, Category = "Targeting")
    void RemoveCombatTarget();

    UFUNCTION(BlueprintCallable, Category = "Targeting")
    AActor* SearchCombatTarget(const FVector& Start, const FVector& End, float SearchRadius) const;
    
    UFUNCTION(BlueprintCallable, Category = "Targeting")
    AActor* SelectNearestTarget(TArray<AActor*> Targets);

    UFUNCTION(BlueprintCallable, Category = "Targeting")
    void RotateTowardsTarget(AActor* Target);
    
    UFUNCTION(BlueprintCallable, Category = "Targeting")
    TArray<AActor*> GetTargets(const float Radius) const;

    UPROPERTY(BlueprintReadWrite, Category = "Targeting")
    float TargetingLostRadius = 2000.f;

protected:
    virtual void BeginPlay() override;

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintAssignable, Category = "Targeting")
    FOnTargetedEntityDead OnTargetedEntityDead;

    UPROPERTY(EditAnywhere, BlueprintAssignable, Category = "Targeting")
    FOnTargetedEntityChanged OnTargetedEntityChanged;

private:
    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<AController> OwnerController;

    UPROPERTY()
    AActor* CurrentTarget;

    UPROPERTY()
    TArray<AActor*> CombatTargets;

    int32 CombatTargetIndex = 0;
    
    bool bIsLocking = false;

    float ActiveLockRange;
    FTimerHandle TargetRefreshTimerHandle;
};
