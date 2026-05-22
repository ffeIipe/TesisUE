#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class IPickable;
struct FInventoryData;
class UInventory;
class APlayerController;
class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemEquipped, TScriptInterface<IPickable>, NewWeapon);

USTRUCT()
struct FInventoryItemSaveData
{
    GENERATED_BODY()

    UPROPERTY(SaveGame)
    FGuid LastOwnerSaveID;
    
    UPROPERTY(SaveGame)
    FGuid UniqueSaveID;
    
    UPROPERTY(SaveGame)
    TSubclassOf<AActor> ActorClass;

    // Si tenías munición descomentada en tu código antiguo, agrega esto:
    // UPROPERTY(SaveGame)
    // int32 AmmoCount;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TESISUE_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "Inventory | Events")
    FOnItemEquipped OnItemEquipped;
    
    UInventoryComponent();

    UPROPERTY(VisibleAnywhere, Category = "Inventory", SaveGame)
    int32 EquippedSlotIndex = -1;

    UPROPERTY(VisibleAnywhere, Category = "Inventory")
    TScriptInterface<IPickable> CurrentItem;

    void InitializeValues(const FInventoryData& InventoryData);

    UFUNCTION(BlueprintPure, Category = "Inventory")
    TScriptInterface<IPickable> GetCurrentWeapon() const { return CurrentItem; }

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryAddWeapon(TScriptInterface<IPickable> PickableToAdd);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void EquipItemFromSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void DropWeaponFromSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UnequipCurrentWeapon();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ToggleInventorySlot();
    
    UFUNCTION(BlueprintPure, Category = "Inventory")
    const TArray<TScriptInterface<IPickable>>& GetInventoryItems() const { return InventorySlots; }

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void PerformInteract(const FVector& StartTrace, const FVector& EndTrace, float RadiusTrace);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory|Widget")
    void ShowInventory();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Widget")
    void HideInventory();

    UFUNCTION(BlueprintPure, Category = "Inventory|Widget")
    bool IsInventoryOpen() const { return bIsInventoryOpen; }
    
   UFUNCTION(BlueprintCallable, Category = "Inventory|Widget") 
    void UpdateInventoryUI();

    UFUNCTION(BlueprintCallable, Category = "Inventory|SaveGame")
    void SaveInventory();

    UFUNCTION(BlueprintCallable, Category = "Inventory|SaveGame")
    void LoadInventory();

    UFUNCTION(BlueprintCallable, Category = "Inventory|SaveGame")
    void ReconstructInventory();

protected:
    virtual void BeginPlay() override;
    
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Widget")
    TSubclassOf<UUserWidget> InventoryWidgetClass;

    UPROPERTY(Transient)
    UInventory* InventoryWidgetInstance = nullptr;

    UPROPERTY(Transient)
    APlayerController* PlayerControllerRef = nullptr;

    bool bIsInventoryOpen = false;

public:
    UPROPERTY(EditAnywhere, Category = "Inventory", Transient)
    TArray<TScriptInterface<IPickable>> InventorySlots;
    
private:
    void InitializeInventoryWidget();

    int32 MaxSlots;
    
    UPROPERTY()
    AController* OwnerController = nullptr;

    UPROPERTY(SaveGame)
    TArray<FInventoryItemSaveData> SavedInventoryData;
};