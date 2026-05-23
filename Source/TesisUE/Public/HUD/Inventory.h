// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory.generated.h"

class AItem;
class IPickable;

UCLASS()
class TESISUE_API UInventory : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void RefreshInventoryUI(const TArray<TScriptInterface<IPickable>>& Items);
};
