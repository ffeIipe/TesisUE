#include "Features/InventorySystem/Core/InventoryComponent.h"

#include "EngineUtils.h"
#include "Features/InventorySystem/Interfaces/Pickable.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "HUD/Inventory.h"

#include "DataAssets/EntityData.h"
#include "Features/InventorySystem/Items/Weapon.h"
#include "Features/SaveSystem/Core/SaveComponent.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    
    PlayerControllerRef = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    OwnerController = PlayerControllerRef;

    InitializeInventoryWidget();
}

void UInventoryComponent::InitializeValues(const FInventoryData& InventoryData)
{
    MaxSlots = InventoryData.MaxSlots;

    InventorySlots.Init(nullptr, MaxSlots);
}

void UInventoryComponent::ToggleInventorySlot()
{
    EquippedSlotIndex++;
    if (EquippedSlotIndex >= MaxSlots) EquippedSlotIndex = 0;
    
     EquipItemFromSlot(EquippedSlotIndex);
}

void UInventoryComponent::InitializeInventoryWidget()
{
    if (InventoryWidgetClass && PlayerControllerRef && PlayerControllerRef->IsLocalController() && !InventoryWidgetInstance)
    {
        InventoryWidgetInstance = CreateWidget<UInventory>(PlayerControllerRef, InventoryWidgetClass);
        if (InventoryWidgetInstance)
        {
            InventoryWidgetInstance->AddToViewport();
            InventoryWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

bool UInventoryComponent::TryAddWeapon(const TScriptInterface<IPickable> PickableToAdd)
{
    if (!PickableToAdd) return false;

    if (PickableToAdd->Execute_ShouldConsumeOnUse(PickableToAdd.GetObject())) return false;
    
    for (int32 i = 0; i < InventorySlots.Num(); ++i)
    {
        if (InventorySlots[i] == nullptr)
        {
            InventorySlots[i] = PickableToAdd;
            EquipItemFromSlot(i);
            UpdateInventoryUI();
            return true;
        }
    }

    return false;
}

void UInventoryComponent::EquipItemFromSlot(const int32 SlotIndex)
{
    if (!InventorySlots.IsValidIndex(SlotIndex) || InventorySlots[SlotIndex] == nullptr) return;

    if (CurrentItem == InventorySlots[SlotIndex]) return;
    
    //UnequipCurrentWeapon();
    
    if (CurrentItem)
    {
        CurrentItem->Execute_OnRemovedFromInventory(CurrentItem.GetObject());
    }
    
    if (const TScriptInterface<IPickable> Pickable = InventorySlots[SlotIndex].GetObject())
        Pickable->Execute_OnEnteredInventory(Pickable.GetObject(), GetOwner());

    if (InventorySlots.IsValidIndex(SlotIndex))
    {
        CurrentItem = InventorySlots[SlotIndex];
        EquippedSlotIndex = SlotIndex;
        CurrentItem->Execute_OnEnteredInventory(CurrentItem.GetObject(), GetOwner());
    }
    
    //UpdateInventoryUI();
}

void UInventoryComponent::DropWeaponFromSlot(const int32 SlotIndex)
{
    if (!InventorySlots.IsValidIndex(SlotIndex) || InventorySlots[SlotIndex] == nullptr)
    {
        return;
    }

    InventorySlots[SlotIndex] = CurrentItem;
    
    if (CurrentItem)
    {
        UnequipCurrentWeapon();
    }

    InventorySlots[SlotIndex] = nullptr;

    UpdateInventoryUI();
}

void UInventoryComponent::UnequipCurrentWeapon()
{
    if (!CurrentItem) return;

    if (CurrentItem)
    {
        CurrentItem->Execute_OnRemovedFromInventory(CurrentItem.GetObject());
        CurrentItem = nullptr;
        EquippedSlotIndex = -1;

        if (OnItemEquipped.IsBound()) OnItemEquipped.Broadcast(nullptr);
    }
    else
    {
        if (EquippedSlotIndex != -1)
        {
            EquippedSlotIndex = -1;
        }
    }

    if (OnItemEquipped.IsBound()) 
    {
        OnItemEquipped.Broadcast(CurrentItem);
    }
}

void UInventoryComponent::ShowInventory()
{
    if (bIsInventoryOpen) return;
    if (!InventoryWidgetInstance)
    {
        InitializeInventoryWidget();
        if (!InventoryWidgetInstance) return;
    }

    bIsInventoryOpen = true;
    UpdateInventoryUI();
    InventoryWidgetInstance->SetVisibility(ESlateVisibility::Visible);
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.2f);

    if (PlayerControllerRef)
    {
        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(InventoryWidgetInstance->TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PlayerControllerRef->SetInputMode(InputMode);
        PlayerControllerRef->bShowMouseCursor = true;
    }
}

void UInventoryComponent::HideInventory()
{
    if (!bIsInventoryOpen) return;
    if (!InventoryWidgetInstance) return;

    bIsInventoryOpen = false;
    InventoryWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

    if (PlayerControllerRef)
    {
        const FInputModeGameOnly InputMode;
        PlayerControllerRef->SetInputMode(InputMode);
        PlayerControllerRef->bShowMouseCursor = false;
    }
}

void UInventoryComponent::UpdateInventoryUI()
{
    if (InventoryWidgetInstance)
    {
        InventoryWidgetInstance->RefreshInventoryUI(InventorySlots);
    }
}

void UInventoryComponent::SaveInventory()
{
    SavedInventoryData.Empty();

    for (const TScriptInterface<IPickable>& SlotItem : InventorySlots)
    {
        FInventoryItemSaveData Data;
        
        if (SlotItem && SlotItem.GetObject())
        {
            if (AActor* WeaponActor = Cast<AActor>(SlotItem.GetObject()))
            {
                if (USaveComponent* SaveComp = WeaponActor->FindComponentByClass<USaveComponent>())
                {
                    Data.UniqueSaveID = SaveComp->UniqueSaveID;
                }
            }
        }
        
        SavedInventoryData.Add(Data);
    }
}

void UInventoryComponent::LoadInventory()
{
    InventorySlots.Empty();
    InventorySlots.Init(nullptr, MaxSlots);
    CurrentItem = nullptr;
}

void UInventoryComponent::ReconstructInventory()
{
    TMap<FGuid, AActor*> WorldSaveActors;
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        if (USaveComponent* SaveComp = (*It)->FindComponentByClass<USaveComponent>())
        {
            WorldSaveActors.Add(SaveComp->UniqueSaveID, *It);
        }
    }

    for (int32 i = 0; i < SavedInventoryData.Num(); i++)
    {
        const FGuid& SavedID = SavedInventoryData[i].UniqueSaveID;

        if (SavedID.IsValid() && WorldSaveActors.Contains(SavedID))
        {
            AActor* RestoredActor = WorldSaveActors[SavedID];
        
            InventorySlots[i] = RestoredActor;
        
            if (RestoredActor->Implements<UPickable>())
            {
                const TScriptInterface<IPickable> Pickable = RestoredActor;
            
                Pickable->Execute_OnEnteredInventory(Pickable.GetObject(), GetOwner());

                if (i != EquippedSlotIndex)
                {
                    if (AWeapon* RestoredWeapon = Cast<AWeapon>(RestoredActor))
                    {
                        RestoredWeapon->Holster();
                    }
                }
            }
        }
    }

    if (EquippedSlotIndex >= 0 && EquippedSlotIndex < MaxSlots)
    {
        EquipItemFromSlot(EquippedSlotIndex);
    }

    UpdateInventoryUI();
}

void UInventoryComponent::PerformInteract(const FVector& StartTrace, const FVector& EndTrace, const float RadiusTrace)
{
    if (IsInventoryOpen()) return;
    
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(GetOwner());

    FHitResult ResultHit;
    
    const bool bHit = UKismetSystemLibrary::SphereTraceSingle(
        GetWorld(),
        StartTrace,
        EndTrace,
        RadiusTrace,
        UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel5), //item trace
        false,
        ActorsToIgnore,
        EDrawDebugTrace::ForDuration,
        ResultHit,
        true
    );

    if (bHit)
    {
        if (const TScriptInterface<IPickable> Pickable = ResultHit.GetActor())
        {
            if (TryAddWeapon(Pickable))
            {
                ActorsToIgnore.Add(Cast<AActor>(Pickable.GetObject()));
            }
            else
            {
                Pickable->Execute_OnEnteredInventory(Pickable.GetObject(), GetOwner());
            }
        }
    }
}
