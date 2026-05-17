#include "Components/TargetingComponent.h"

#include "Curves/CurveFloat.h"
#include "Entities/Entity.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UTargetingComponent::UTargetingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.01f;

    CurrentTarget = nullptr;
    CombatTargets.Empty();
}

void UTargetingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    PrimaryComponentTick.bStartWithTickEnabled = false;
    PrimaryComponentTick.TickInterval = 0.01f;
    SetComponentTickEnabled(false);

    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        OwnerController = Cast<AController>(OwnerCharacter->GetController());
    }
}

void UTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!CurrentTarget || !OwnerCharacter)
    {
        DisableLock();
        return;
    }

    const float DistSq = FVector::DistSquared(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    
    if (DistSq > FMath::Square(TargetingLostRadius))
    {
        DisableLock();
    }
    else
    {
        RotateTowardsTarget(CurrentTarget);
    }
}

void UTargetingComponent::HandleTargetDeath(AEntity* DeadEntity)
{
    if (DeadEntity == CurrentTarget)
    {
        DeadEntity->OnDead.RemoveDynamic(this, &UTargetingComponent::HandleTargetDeath);

        ChangeHardLockTarget();
    }
}

void UTargetingComponent::EnableLock(const float LockRange)
{
    CombatTargets = GetTargets(LockRange);
    CurrentTarget = SelectNearestTarget(CombatTargets);

    if (CurrentTarget)
    {
        bIsLocking = true;
        SetComponentTickEnabled(true);
    }
}

void UTargetingComponent::DisableLock()
{
    bIsLocking = false;
    SetComponentTickEnabled(false);
    
    CurrentTarget = nullptr;
    CombatTargets.Empty();
}

void UTargetingComponent::ChangeHardLockTarget()
{
    if (!bIsLocking || CombatTargets.Num() == 0) return;
    
    int32 Attempts = 0;
    AActor* NewCandidate = nullptr;

    while (Attempts < CombatTargets.Num())
    {
        CombatTargetIndex = (CombatTargetIndex + 1) % CombatTargets.Num();

        if (AActor* Candidate = CombatTargets[CombatTargetIndex]; Candidate && Candidate != CurrentTarget)
        {
            NewCandidate = Candidate;
            break;
        }
        Attempts++;
    }

    if (NewCandidate)
    {
        if (AEntity* OldTarget = Cast<AEntity>(CurrentTarget))
        {
            OldTarget->OnDead.RemoveDynamic(this, &UTargetingComponent::HandleTargetDeath);
        }

        CurrentTarget = NewCandidate;

        if (AEntity* NewTarget = Cast<AEntity>(CurrentTarget))
        {
            NewTarget->OnDead.AddUniqueDynamic(this, &UTargetingComponent::HandleTargetDeath);
        }
    }
}

AActor* UTargetingComponent::SelectNearestTarget(TArray<AActor*> Targets)
{
    if (Targets.Num() == 0) return nullptr;
    
    float MinDistance = TNumericLimits<float>::Max(); 
    AActor* ClosestCombatTarget = nullptr;
    
    const FVector OwnerLocation = GetOwner()->GetActorLocation();

    for (const auto CombatTarget : Targets)
    {
        if (CombatTarget)
        {
            if (const float DistSq = FVector::DistSquared(CombatTarget->GetActorLocation(), OwnerLocation); DistSq < MinDistance)
            {
                MinDistance = DistSq;
                ClosestCombatTarget = CombatTarget;
            }
        }
    }

    if (ClosestCombatTarget) 
    {
        if (AEntity* OldTarget = Cast<AEntity>(CurrentTarget))
        {
            OldTarget->OnDead.RemoveDynamic(this, &UTargetingComponent::HandleTargetDeath);
        }

        CurrentTarget = ClosestCombatTarget;
        
        if (AEntity* NewTarget = Cast<AEntity>(CurrentTarget))
        {
            NewTarget->OnDead.AddUniqueDynamic(this, &UTargetingComponent::HandleTargetDeath);
        }
        
        return ClosestCombatTarget;
    }

    return nullptr;
}

void UTargetingComponent::RotateTowardsTarget(AActor* Target)
{
    if (!OwnerController || !OwnerCharacter || !Target)
    {
        if (bIsLocking)
        {
            DisableLock();
        }
        
        return;
    }

    OwnerController = OwnerCharacter->GetController();
    
    const FVector StartLocation = OwnerCharacter->GetPawnViewLocation();
    const FVector TargetLocation = Target->GetActorLocation() + FVector(0.f, 0.f, 70.f);
    
    const FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(StartLocation, TargetLocation);

    if (OwnerController)
    {
        const FRotator CurrentControlRotation = OwnerController->GetControlRotation();
        const FRotator NewControlRotation = FMath::RInterpTo(CurrentControlRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), 15.f);
        OwnerController->SetControlRotation(NewControlRotation);
    }
}

TArray<AActor*> UTargetingComponent::GetTargets(const float Radius) const
{
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(GetOwner());

    TArray<AActor*> Hits;
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        GetOwner()->GetActorLocation(),
        Radius,
        ObjectTypes,
        TSubclassOf<AEntity>(),
        ActorsToIgnore,
        Hits
    );

    TArray<AActor*> ValidTargets;
    
    const AEntity* OwnerEntity = Cast<AEntity>(GetOwner());
    if (!OwnerEntity) 
    {
        return ValidTargets; 
    }

    for (const auto HitActor : Hits)
    {
        if (AEntity* TargetEntity = Cast<AEntity>(HitActor))
        {
            if (const bool bIsHostile = OwnerEntity->IsHostile(TargetEntity); TargetEntity->IsAlive() && bIsHostile)
            {
                ValidTargets.Add(TargetEntity);
            }
        }
    }
    
    return ValidTargets;
}

void UTargetingComponent::RemoveCombatTarget()
{
    if (!bIsLocking)
    {
        CurrentTarget = nullptr;
    }
}

AActor* UTargetingComponent::SearchCombatTarget(
    const FVector& Start, const FVector& End, const float SearchRadius) const
{
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(GetOwner());

    FHitResult ResultHit;
    const bool bHit = UKismetSystemLibrary::SphereTraceSingleForObjects(
        GetWorld(),
        Start,
        End,
        SearchRadius,
        ObjectTypes,
        false,
        ActorsToIgnore,
        EDrawDebugTrace::ForDuration,
        ResultHit,
        true
    );

    if (bHit)
    {
        return ResultHit.GetActor();
    }
    return nullptr;
}