#include "AI/Core/CombatDirectorSubsystem.h"
#include "Engine/Engine.h" // Necesario para AddOnScreenDebugMessage

void UCombatDirectorSubsystem::RegisterEnemy(AActor* Enemy, AActor* TargetActor)
{
    if (!Enemy || !TargetActor) return;

    FCombatTargetData& Data = CombatMap.FindOrAdd(TargetActor);
    
    // Evitamos duplicados
    if (!Data.RegisteredEnemies.Contains(Enemy))
    {
        Data.RegisteredEnemies.Add(Enemy);
    }
}

void UCombatDirectorSubsystem::UnregisterEnemy(AActor* Enemy, AActor* TargetActor)
{
    if (!TargetActor) return;

    if (FCombatTargetData* Data = CombatMap.Find(TargetActor))
    {
        Data->RegisteredEnemies.Remove(Enemy);

        if (Data->AttackingEnemies.Contains(Enemy))
        {
            Data->AttackingEnemies.Remove(Enemy);
            
            // DEBUG: Alguien se desregistró y devolvió el token forzosamente
            if (GEngine)
            {
                FString Msg = FString::Printf(TEXT("DIRECTOR: %s Unregistered & Forced Token Return. Slots: %d / %d"), 
                    *Enemy->GetName(), 
                    Data->AttackingEnemies.Num(), 
                    Data->MaxConcurrentAttackers);
                GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.f, FColor::Orange, Msg);
            }
        }

        if (Data->RegisteredEnemies.Num() == 0)
        {
            CombatMap.Remove(TargetActor);
        }
    }
}

TArray<AActor*> UCombatDirectorSubsystem::GetAlliesForTarget(AActor* TargetActor) const
{
    TArray<AActor*> Result;
    if (!TargetActor) return Result;

    if (const FCombatTargetData* Data = CombatMap.Find(TargetActor))
    {
        for (const TWeakObjectPtr<AActor>& WeakEnemy : Data->RegisteredEnemies)
        {
            if (AActor* LiveEnemy = WeakEnemy.Get())
            {
                Result.Add(LiveEnemy);
            }
        }
    }
    return Result;
}

bool UCombatDirectorSubsystem::RequestAttackToken(AActor* Enemy, AActor* TargetActor)
{
    if (!Enemy || !TargetActor) return false;

    FCombatTargetData& Data = CombatMap.FindOrAdd(TargetActor);

    // 1. Limpieza preventiva
    int32 RemovedCount = 0;
    Data.AttackingEnemies.RemoveAll([&RemovedCount](const TWeakObjectPtr<AActor>& Ptr)
    {
        if (!Ptr.IsValid())
        {
            RemovedCount++;
            return true;
        }
        return false;
    });

    if (RemovedCount > 0 && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, FColor::Red, 
            FString::Printf(TEXT("DIRECTOR: Cleaned %d invalid enemies holding tokens!"), RemovedCount));
    }

    // 2. Idempotencia
    if (Data.AttackingEnemies.Contains(Enemy))
    {
        // DEBUG: Ya tenía token
        if (GEngine)
        {
            // Usamos key aleatoria para no spamear, o un tiempo corto
            // FString Msg = FString::Printf(TEXT("DIRECTOR: %s already has token."), *Enemy->GetName());
            // GEngine->AddOnScreenDebugMessage(INDEX_NONE, 1.f, FColor::Yellow, Msg);
        }
        return true;
    }

    // 3. Chequeo de cupo (AQUÍ ESTÁ LA CLAVE DEL DEBUG)
    if (Data.AttackingEnemies.Num() < Data.MaxConcurrentAttackers)
    {
        Data.AttackingEnemies.Add(Enemy);
        
        // DEBUG: ÉXITO
        if (GEngine)
        {
            FString Msg = FString::Printf(TEXT("DIRECTOR: Token GRANTED to %s. Slots Used: %d / %d"), 
                *Enemy->GetName(), 
                Data.AttackingEnemies.Num(), 
                Data.MaxConcurrentAttackers);
            GEngine->AddOnScreenDebugMessage(INDEX_NONE, 2.f, FColor::Green, Msg);
        }
        return true;
    }

    // DEBUG: FALLO (Cupo lleno)
    if (GEngine)
    {
        // Nota: Esto puede spamear mucho si llamas a Request cada tick.
        // Recomiendo verlo una vez y luego comentar esta línea si molesta.
        FString Msg = FString::Printf(TEXT("DIRECTOR: Token DENIED to %s. Full: %d / %d"), 
            *Enemy->GetName(), 
            Data.AttackingEnemies.Num(), 
            Data.MaxConcurrentAttackers);
        // Color Rojo para denegado
        GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0.0f, FColor::Red, Msg); 
    }

    return false; 
}

void UCombatDirectorSubsystem::ReturnAttackToken(AActor* Enemy, AActor* TargetActor)
{
    if (!TargetActor) return;

    if (FCombatTargetData* Data = CombatMap.Find(TargetActor))
    {
        int32 Removed = Data->AttackingEnemies.Remove(Enemy);
        
        // DEBUG: Retorno voluntario
        if (Removed > 0 && GEngine)
        {
            FString Msg = FString::Printf(TEXT("DIRECTOR: %s Returned Token. Slots Free: %d / %d"), 
                *Enemy->GetName(), 
                Data->AttackingEnemies.Num(), 
                Data->MaxConcurrentAttackers);
            GEngine->AddOnScreenDebugMessage(INDEX_NONE, 2.f, FColor::Cyan, Msg);
        }
    }
}

void UCombatDirectorSubsystem::CleanInvalidEntries(FCombatTargetData& Data)
{
    // Esta función ya está integrada con debugs dentro de RequestAttackToken arriba
    // para no duplicar lógica en el ejemplo de debug.
    // En producción deberías mantenerla separada.
}