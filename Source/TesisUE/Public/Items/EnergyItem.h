#pragma once

#include "CoreMinimal.h"
#include "Features/InventorySystem/Items/Item.h"
#include "EnergyItem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChestUsedSpawnEnergy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChestUsedSpawnLife);

UCLASS()
class TESISUE_API AEnergyItem : public AItem
{
	GENERATED_BODY()
	
public:
	virtual void OnEnteredInventory_Implementation(AActor* NewOwner) override;

	UPROPERTY(BlueprintAssignable)
	FOnChestUsedSpawnEnergy OnUsedSpawnEnergy;

	UPROPERTY(BlueprintAssignable)
	FOnChestUsedSpawnLife OnUsedSpawnLife;

private:
	bool bWasUsed = false;

	UPROPERTY(EditAnywhere, Category = "Stats | Health")
	float HealthToIncrease;
	
	UPROPERTY(EditAnywhere, Category = "Stats | Energy")
	float EnergyToIncrease;
	
	UPROPERTY(EditAnywhere, Category = "Stats | FX")
	USoundBase* OpenSFX;

	UPROPERTY(EditAnywhere, Category = "Stats | FX")
	USoundBase* HealthSFX;

	UPROPERTY(EditAnywhere, Category = "Stats | FX")
	USoundBase* EnergySFX;
};
