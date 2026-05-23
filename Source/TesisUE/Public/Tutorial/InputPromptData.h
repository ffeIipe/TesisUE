#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "InputPromptData.generated.h"

USTRUCT(BlueprintType)
struct FInputPromptData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName InputName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UTexture2D* KeyboardIcon = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UTexture2D* GamepadIcon = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText DisplayText;
};
