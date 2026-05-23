#include "Subsystems/OrbsPoolManager.h"
#include "Interfaces/OrbPoolingInterface.h"
#include "Features/InventorySystem/Items/Item.h"

void UOrbsPoolManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UOrbsPoolManager::Deinitialize()
{
    for (auto& Pair : OrbsPools)
    {
        for (AActor* Item : Pair.Value.PooledItems)
        {
            if (IsValid(Item))
            {
                Item->SetActorHiddenInGame(true);
                Item->SetActorTickEnabled(false);
                Item->Destroy();
            }
        }
        Pair.Value.PooledItems.Empty();
    }
    OrbsPools.Empty();
    Super::Deinitialize();
}

void UOrbsPoolManager::EnsurePoolInitialized(TSubclassOf<AActor> OrbsClass, int32 InitialPoolSize)
{
    if (!OrbsClass)
    {
        return;
    }

    if (FOrbsPool* Pool = GetOrCreatePool(OrbsClass))
    {
        const int32 OrbsToCreate = InitialPoolSize - Pool->PooledItems.Num();
        for (int32 i = 0; i < OrbsToCreate; ++i)
        {
            if (AActor* NewItem = SpawnNewOrbForPool(OrbsClass, FVector::ZeroVector, FRotator::ZeroRotator, nullptr, nullptr))
            {
                if (NewItem->GetClass()->ImplementsInterface(UOrbPoolingInterface::StaticClass()))
                {
                    IOrbPoolingInterface::Execute_DeactivateOrb(NewItem);
                }

                Pool->PooledItems.Add(NewItem);
            }
        }
    }
}

AActor* UOrbsPoolManager::SpawnOrbFromPool(const TSubclassOf<AActor> OrbsClass, const FVector& Location, const FRotator& Rotation, AActor* Owner, APawn* Instigator)
{
    if (!OrbsClass)
    {
        return nullptr;
    }

    FOrbsPool* Pool = GetOrCreatePool(OrbsClass);
    AActor* ItemToSpawn;

    if (Pool && Pool->PooledItems.Num() > 0)
    {
        ItemToSpawn = Pool->PooledItems.Pop();
    }
    else
    {
        ItemToSpawn = SpawnNewOrbForPool(OrbsClass, Location, Rotation, Owner, Instigator);
        if (ItemToSpawn && Pool)
        {
            // Freshly spawned actor might have run its BeginPlay.
            // We ideally want to control this. For now, we assume it's okay and call Activate.
            // More robust solution: spawn deferred, then call custom Init, then FinishSpawning.
        }
    }

    if (ItemToSpawn)
    {
        ItemToSpawn->SetOwner(Owner);
        ItemToSpawn->SetInstigator(Instigator);

        if (ItemToSpawn->GetClass()->ImplementsInterface(UOrbPoolingInterface::StaticClass()))
        {
            IOrbPoolingInterface::Execute_ActivateOrb(ItemToSpawn, Location, Rotation);
        }
    }
    return ItemToSpawn;
}

void UOrbsPoolManager::ReturnOrbToPool(AActor* Orb)
{
    if (!Orb) return;

    const TSubclassOf<AItem> ItemClass = Orb->GetClass();

    if (FOrbsPool* Pool = GetOrCreatePool(ItemClass))
    {
        if (Orb->GetClass()->ImplementsInterface(UOrbPoolingInterface::StaticClass()))
        {
            IOrbPoolingInterface::Execute_DeactivateOrb(Orb);
        }
        Pool->PooledItems.Add(Orb);
    }
    else
    {
        Orb->Destroy();
    }
}

FOrbsPool* UOrbsPoolManager::GetOrCreatePool(const TSubclassOf<AActor>& OrbsClass)
{
    if (!OrbsClass) return nullptr;

    FOrbsPool* FoundPool = OrbsPools.Find(OrbsClass);
    if (!FoundPool)
    {
        FOrbsPool NewPool;
        NewPool.ItemClass = OrbsClass;
        OrbsPools.Add(OrbsClass, NewPool);
        FoundPool = OrbsPools.Find(OrbsClass);
    }
    return FoundPool;
}

AActor* UOrbsPoolManager::SpawnNewOrbForPool(const TSubclassOf<AActor>& OrbsClass, const FVector& Location, const FRotator& Rotation, AActor* Owner, APawn* Instigator)
{
    UWorld* World = GetWorld();
    if (!World || !OrbsClass)
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Owner;
    SpawnParams.Instigator = Instigator;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AActor* NewItem = World->SpawnActor<AActor>(OrbsClass, Location, Rotation, SpawnParams);

    return NewItem;
}