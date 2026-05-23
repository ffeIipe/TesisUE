#pragma once

#include "CoreMinimal.h"
#include "Features/InventorySystem/Interfaces/Pickable.h"
#include "Features/SaveSystem/Interfaces/SaveInterface.h"
#include "Item.generated.h"

class USphereComponent;
class UBoxComponent;
class UPromptWidgetComponent;

USTRUCT(BlueprintType)
struct FUIItemProperties
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties")
	FText ItemDisplayName = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (MultiLine = true))
	FText ItemDescription = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties")
	UTexture2D* ItemIcon = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemUsed, ACharacter*, UserCharacter);

UCLASS()
class TESISUE_API AItem : public AActor, public IPickable, public ISaveInterface
{
	GENERATED_BODY()

public:	
	AItem();
	
	UPROPERTY(BlueprintAssignable)
	FOnItemUsed OnUsed;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame)
	bool bWasUsed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName RestoredSaveID;

	virtual void BeginPlay() override;
	
	virtual void OnEnteredInventory_Implementation(AActor* NewOwner) override;

	virtual void OnRemovedFromInventory_Implementation() override;

	virtual bool ShouldConsumeOnUse_Implementation() override;

	virtual void EnableVisuals();

	virtual void DisableVisuals();
	
	virtual void OnSaveGame_Implementation(FEntitySaveData& OutData) override;
	
	virtual void OnLoadGame_Implementation(const FEntitySaveData& InData) override;

	virtual void OnPostGameLoaded_Implementation() override;

protected:
	virtual void DisableCollision();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FUIItemProperties UIItemProperties;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item Properties")
	UStaticMeshComponent* ItemMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USphereComponent* SphereCollider;
};