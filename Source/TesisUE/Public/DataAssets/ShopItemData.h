#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ShopItemData.generated.h"

USTRUCT(BlueprintType)
struct FShopItemData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 BaseCost;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag GrantedTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag RequiredTag;
};
