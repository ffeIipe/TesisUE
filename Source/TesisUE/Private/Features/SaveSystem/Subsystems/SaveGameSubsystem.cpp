#include "Features/SaveSystem/Subsystems/SaveGameSubsystem.h"
#include "Features/SaveSystem/Data/SaveTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Features/SaveSystem/Interfaces/SaveInterface.h"
#include "EngineUtils.h"
#include "Features/SaveSystem/Core/SaveComponent.h"
#include "GameFramework/Actor.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

void USaveGameSubsystem::SerializeProperties(UObject* Object, TArray<uint8>& OutBytes)
{
    if (!IsValid(Object)) return;

    FMemoryWriter Writer(OutBytes);
    FObjectAndNameAsStringProxyArchive Ar(Writer, false);
    Ar.ArIsSaveGame = true;

    for (TFieldIterator<FProperty> PropIt(Object->GetClass()); PropIt; ++PropIt)
    {
        FProperty* Property = *PropIt;
        if (Property->HasAnyPropertyFlags(CPF_SaveGame) && 
        !Property->IsA<FObjectPropertyBase>() && 
        !Property->IsA<FInterfaceProperty>())
        {
            void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Object);
    
            Property->SerializeItem(FStructuredArchiveFromArchive(Ar).GetSlot(), ValuePtr);
        }
    }
}

void USaveGameSubsystem::DeserializeProperties(UObject* Object, const TArray<uint8>& InBytes)
{
    if (!IsValid(Object) || InBytes.Num() == 0) return;

    FMemoryReader Reader(InBytes);
    FObjectAndNameAsStringProxyArchive Ar(Reader, false);
    Ar.ArIsSaveGame = true;

    for (TFieldIterator<FProperty> PropIt(Object->GetClass()); PropIt; ++PropIt)
    {
        FProperty* Property = *PropIt;
        
        if (Property->HasAnyPropertyFlags(CPF_SaveGame) && 
    !Property->IsA<FObjectPropertyBase>() && 
    !Property->IsA<FInterfaceProperty>())
        {
            void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Object);
    
            Property->SerializeItem(FStructuredArchiveFromArchive(Ar).GetSlot(), ValuePtr);
        }
    }
}

void USaveGameSubsystem::SaveGame(int32 SlotIndex)
{
    CurrentSlotIndex = SlotIndex;
    FString TargetSlotName = GetSlotNameFromIndex(SlotIndex);

    UPlayerProgressSaveGame* SaveObject = Cast<UPlayerProgressSaveGame>(UGameplayStatics::CreateSaveGameObject(UPlayerProgressSaveGame::StaticClass()));
    
    SaveObject->CurrentLevelName = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
    SaveObject->SaveSlotUserLabel = TargetSlotName;
    SaveObject->Timestamp = FDateTime::Now();

    SaveLevelActors(SaveObject);

    const bool bSaved = UGameplayStatics::SaveGameToSlot(SaveObject, TargetSlotName, 0);
    
    if(bSaved) 
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("SUCCESS: Partida GUARDADA en slot: %s"), *TargetSlotName));
        //OnSaveDataUpdated.Broadcast();
    }
    OnGameSaved.Broadcast(bSaved);
}

void USaveGameSubsystem::LoadGame(int32 SlotIndex)
{
    FString TargetSlotName = GetSlotNameFromIndex(SlotIndex);

    if (!UGameplayStatics::DoesSaveGameExist(TargetSlotName, 0))
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Red, FString::Printf(TEXT("LOAD ABORTED: No existe %s.sav"), *TargetSlotName));
        return;
    }

    CurrentSaveGame = Cast<UPlayerProgressSaveGame>(UGameplayStatics::LoadGameFromSlot(TargetSlotName, 0));
    
    if (!CurrentSaveGame) return;

    CurrentSlotIndex = SlotIndex;

    const FString SavedLevelName = CurrentSaveGame->CurrentLevelName;
    const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);

    if (CurrentLevelName.Equals(SavedLevelName) || SavedLevelName.IsEmpty())
    {
        RestoreCurrentLevelState();
        OnGameLoaded.Broadcast(true);
    }
    else
    {
        UGameplayStatics::OpenLevel(this, FName(*SavedLevelName));
    }
}

void USaveGameSubsystem::CreateNewGame(int32 SlotIndex, const FString MapName)
{
    if (MapName.IsEmpty())
    {
        return;
    }

    CurrentSlotIndex = SlotIndex;
    FString TargetSlotName = GetSlotNameFromIndex(SlotIndex);

    CurrentSaveGame = Cast<UPlayerProgressSaveGame>(UGameplayStatics::CreateSaveGameObject(UPlayerProgressSaveGame::StaticClass()));
	
    if (CurrentSaveGame)
    {
        CurrentSaveGame->SaveSlotUserLabel = TargetSlotName;
        CurrentSaveGame->Timestamp = FDateTime::Now();
		
        CurrentSaveGame->CurrentLevelName = MapName;

        const bool bSaved = UGameplayStatics::SaveGameToSlot(CurrentSaveGame, TargetSlotName, 0);
		
        if (bSaved)
        {
            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("Nueva Partida en Slot %d. Cargando: %s"), SlotIndex, *MapName));
            //OnSaveDataUpdated.Broadcast();
        }

        UGameplayStatics::OpenLevel(GetWorld(), FName(*MapName));
    }
}

bool USaveGameSubsystem::DoesSaveGameExist(int32 SlotIndex) const
{
    FString TargetSlotName = GetSlotNameFromIndex(SlotIndex);
    return UGameplayStatics::DoesSaveGameExist(TargetSlotName, 0);
}

bool USaveGameSubsystem::GetSaveSlotInfo(int32 SlotIndex, FString& OutLevelName, FDateTime& OutTimestamp, FString& OutSlotLabel) const
{
    FString TargetSlotName = GetSlotNameFromIndex(SlotIndex);

    if (!UGameplayStatics::DoesSaveGameExist(TargetSlotName, 0)) return false;

    USaveGame* LoadedSave = UGameplayStatics::LoadGameFromSlot(TargetSlotName, 0);
    if (UPlayerProgressSaveGame* ProgressSave = Cast<UPlayerProgressSaveGame>(LoadedSave))
    {
        OutLevelName = ProgressSave->CurrentLevelName;
        OutTimestamp = ProgressSave->Timestamp;
        OutSlotLabel = ProgressSave->SaveSlotUserLabel;
        return true;
    }
    return false;
}

void USaveGameSubsystem::RestoreCurrentLevelState() const
{
    if (CurrentSaveGame)
    {
        FString CurrentMap = GetWorld()->GetMapName();
        CurrentMap.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

        LoadLevelActors(CurrentSaveGame);
    }
}

FString USaveGameSubsystem::GetSlotNameFromIndex(int32 SlotIndex) const
{
    return FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);
}

void USaveGameSubsystem::SaveLevelActors(UPlayerProgressSaveGame* SaveObject) const
{
    int32 SavedCount = 0;
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        AActor* Actor = *It;
        
        if (USaveComponent* SaveComp = Actor->FindComponentByClass<USaveComponent>())
        {
            if (!SaveComp->UniqueSaveID.IsValid()) continue;

            FEntitySaveData ActorData;
            ActorData.UniqueSaveID = SaveComp->UniqueSaveID;
            ActorData.Transform = Actor->GetActorTransform();
            ActorData.ActorClass = Actor->GetClass();

            if (Actor->Implements<USaveInterface>())
            {
                FEntitySaveData InterfaceData; 
                ISaveInterface::Execute_OnSaveGame(Actor, InterfaceData);

                ActorData.ByteData = InterfaceData.ByteData;
                ActorData.SavedStats = InterfaceData.SavedStats;
            }

            SaveObject->SavedActors.Add(ActorData.UniqueSaveID, ActorData);
            SavedCount++;
        }
    }
    
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("DATA: Se guardaron %d actores válidos."), SavedCount));
}

void USaveGameSubsystem::LoadLevelActors(UPlayerProgressSaveGame* SaveObject) const
{
    if (!SaveObject) return;

    int32 RestoredStaticCount = 0;
    int32 RestoredDynamicCount = 0;

    TArray<FGuid> ProcessedIDs;

    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        AActor* Actor = *It;

        if (USaveComponent* SaveComp = Actor->FindComponentByClass<USaveComponent>())
        {
            FGuid ActorID = SaveComp->UniqueSaveID;

            if (SaveObject->SavedActors.Contains(ActorID))
            {
                const FEntitySaveData& Data = SaveObject->SavedActors[ActorID];
                Actor->SetActorTransform(Data.Transform, false, nullptr, ETeleportType::TeleportPhysics);
                
                if (Actor->Implements<USaveInterface>())
                {
                    ISaveInterface::Execute_OnLoadGame(Actor, Data);
                }

                ProcessedIDs.Add(ActorID);
                RestoredStaticCount++;
            }
        }
    }

    //Dynamics objects
    for (const auto& Pair : SaveObject->SavedActors)
    {
        FGuid SavedID = Pair.Key;

        if (!ProcessedIDs.Contains(SavedID))
        {
            const FEntitySaveData& Data = Pair.Value;

            if (Data.ActorClass)
            {
                FActorSpawnParameters SpawnParams;
                SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

                AActor* NewActor = GetWorld()->SpawnActor<AActor>(Data.ActorClass, Data.Transform, SpawnParams);

                if (NewActor)
                {
                    if (USaveComponent* SaveComp = NewActor->FindComponentByClass<USaveComponent>())
                    {
                        SaveComp->UniqueSaveID = SavedID;
                    }

                    if (NewActor->Implements<USaveInterface>())
                    {
                        ISaveInterface::Execute_OnLoadGame(NewActor, Data);
                    }
                    RestoredDynamicCount++;
                }
            }
        }
    }

    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Emerald, FString::Printf(TEXT("SUCCESS: Se restauraron %d Estáticos y %d Dinámicos!"), RestoredStaticCount, RestoredDynamicCount));
}