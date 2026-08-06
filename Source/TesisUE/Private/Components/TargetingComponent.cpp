#include "Components/TargetingComponent.h"
#include "Curves/CurveFloat.h"
#include "Entities/Entity.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h" // Necesario para los debugs visuales de C++

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
    SetComponentTickEnabled(false);

    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        OwnerController = OwnerCharacter->GetController();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[UTargetingComponent] BeginPlay: Owner no es un ACharacter."));
    }
}

void UTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    // CRÍTICA RESUELTA: Había dos Super::TickComponent aquí.
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!CurrentTarget || !OwnerCharacter)
    {
        DisableLock();
        return;
    }

    const float DistSq = FVector::DistSquared(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    
    if (DistSq > FMath::Square(TargetingLostRadius))
    {
        UE_LOG(LogTemp, Warning, TEXT("[UTargetingComponent] Target perdido por distancia. Distancia actual: %f, Radio máximo: %f"), FMath::Sqrt(DistSq), TargetingLostRadius);
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
        UE_LOG(LogTemp, Log, TEXT("[UTargetingComponent] El target actual (%s) ha muerto. Cambiando target."), *DeadEntity->GetName());
        DeadEntity->OnDead.RemoveDynamic(this, &UTargetingComponent::HandleTargetDeath);

        if (OnTargetedEntityDead.IsBound()) OnTargetedEntityDead.Broadcast(DeadEntity);

        ChangeHardLockTarget();
    }
}

void UTargetingComponent::EnableLock(const float LockRange)
{
    UE_LOG(LogTemp, Log, TEXT("[UTargetingComponent] EnableLock llamado con rango: %f"), LockRange);
    ActiveLockRange = LockRange;
    CombatTargets = GetTargets(ActiveLockRange);
    CurrentTarget = SelectNearestTarget(CombatTargets);

    if (CurrentTarget)
    {
        UE_LOG(LogTemp, Log, TEXT("[UTargetingComponent] Target fijado: %s"), *CurrentTarget->GetName());
        bIsLocking = true;
        SetComponentTickEnabled(true);

        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                TargetRefreshTimerHandle, 
                this, 
                &UTargetingComponent::RefreshTargets, 
                0.5f, 
                true
            );
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[UTargetingComponent] EnableLock: No se encontró ningún target válido en el rango."));
    }
}

void UTargetingComponent::DisableLock()
{
    UE_LOG(LogTemp, Log, TEXT("[UTargetingComponent] DisableLock llamado."));
    bIsLocking = false;
    SetComponentTickEnabled(false);
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TargetRefreshTimerHandle);
    }
    
    CurrentTarget = nullptr;
    CombatTargets.Empty();
}

void UTargetingComponent::RefreshTargets()
{
    if (!bIsLocking) return;

    CombatTargets = GetTargets(ActiveLockRange);

    if (CombatTargets.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UTargetingComponent] RefreshTargets: Ya no hay targets en combate. Desactivando Lock."));
        DisableLock();
        return;
    }

    if (CurrentTarget && !CombatTargets.Contains(CurrentTarget))
    {
        UE_LOG(LogTemp, Log, TEXT("[UTargetingComponent] RefreshTargets: El target actual ya no es válido/está fuera de rango. Seleccionando uno nuevo."));
        CurrentTarget = SelectNearestTarget(CombatTargets);
    }
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
        UE_LOG(LogTemp, Log, TEXT("[UTargetingComponent] ChangeHardLockTarget: Cambiado a %s"), *CurrentTarget->GetName());

        if (AEntity* NewTarget = Cast<AEntity>(CurrentTarget))
        {
            NewTarget->OnDead.AddUniqueDynamic(this, &UTargetingComponent::HandleTargetDeath);
        }

        if (OnTargetedEntityChanged.IsBound()) OnTargetedEntityChanged.Broadcast(Cast<AEntity>(NewCandidate));
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

    if (ClosestCombatTarget && ClosestCombatTarget != CurrentTarget) 
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
    }

    return ClosestCombatTarget;
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
    
    const FVector StartLocation = OwnerCharacter->GetPawnViewLocation();
    const FVector TargetLocation = Target->GetActorLocation() + FVector(0.f, 0.f, 70.f);
    
    const FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(StartLocation, TargetLocation);
    const FRotator CurrentControlRotation = OwnerController->GetControlRotation();
    const FRotator NewControlRotation = FMath::RInterpTo(CurrentControlRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), 15.f);
    
    OwnerController->SetControlRotation(NewControlRotation);
}

TArray<AActor*> UTargetingComponent::GetTargets(const float Radius) const
{
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(GetOwner());

    TArray<AActor*> Hits;
    
    // DEBUG VISUAL: Dibuja la esfera para que confirmes que el tamaño de búsqueda es correcto
    DrawDebugSphere(GetWorld(), GetOwner()->GetActorLocation(), Radius, 24, FColor::Red, false, 2.0f);

    // CRÍTICA RESUELTA: TSubclassOf<AEntity>() instanciaba a NULL. Se cambió a AEntity::StaticClass()
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        GetOwner()->GetActorLocation(),
        Radius,
        ObjectTypes,
        AEntity::StaticClass(), 
        ActorsToIgnore,
        Hits
    );

    UE_LOG(LogTemp, Warning, TEXT("[UTargetingComponent] GetTargets: Esfera ejecutada. Actores interceptados (brutos): %d"), Hits.Num());

    TArray<AActor*> ValidTargets;
    
    const AEntity* OwnerEntity = Cast<AEntity>(GetOwner());
    if (!OwnerEntity) 
    {
        UE_LOG(LogTemp, Error, TEXT("[UTargetingComponent] GetTargets: El Owner no es un AEntity válido."));
        return ValidTargets; 
    }

    for (const auto HitActor : Hits)
    {
        if (AEntity* TargetEntity = Cast<AEntity>(HitActor))
        {
            const bool bIsAlive = TargetEntity->IsAlive();
            const bool bIsHostile = OwnerEntity->IsHostile(TargetEntity);

            UE_LOG(LogTemp, Warning, TEXT("[UTargetingComponent] Evaluando a %s | ¿Está vivo?: %s | ¿Es hostil?: %s"), 
                *HitActor->GetName(), 
                bIsAlive ? TEXT("True") : TEXT("False"), 
                bIsHostile ? TEXT("True") : TEXT("False"));

            if (bIsAlive && bIsHostile)
            {
                ValidTargets.Add(TargetEntity);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[UTargetingComponent] El actor detectado %s no casteó correctamente a AEntity."), *HitActor->GetName());
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[UTargetingComponent] GetTargets: Total de targets válidos retornados: %d"), ValidTargets.Num());
    return ValidTargets;
}

void UTargetingComponent::RemoveCombatTarget()
{
    if (!bIsLocking)
    {
        CurrentTarget = nullptr;
    }
}

AActor* UTargetingComponent::SearchCombatTarget(const FVector& Start, const FVector& End, const float SearchRadius) const
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
        EDrawDebugTrace::ForDuration, // Si quieres ver este debug, tienes que asegurarte de ver los canales en pantalla
        ResultHit,
        true
    );

    if (bHit)
    {
        UE_LOG(LogTemp, Log, TEXT("[UTargetingComponent] SearchCombatTarget: Impacto con %s"), *ResultHit.GetActor()->GetName());
        return ResultHit.GetActor();
    }

    UE_LOG(LogTemp, Warning, TEXT("[UTargetingComponent] SearchCombatTarget: No se impactó nada."));
    return nullptr;
}