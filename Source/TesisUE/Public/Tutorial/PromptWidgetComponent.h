#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "PromptWidgetComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TESISUE_API UPromptWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
    UPromptWidgetComponent();

    virtual void BeginPlay() override;
   
    UFUNCTION(BlueprintCallable, Category = "Prompt")
    void LoadAndApplyPrompt();

    UFUNCTION()
    void EnablePromptWidget();

    UFUNCTION()
    void DisablePromptWidget();
    
    UPromptWidgetComponent* GetPromptWidgetComponent() { return this; };

    void SetPromptRowName(const FName& RowName) { PromptRowName = RowName; }
    
protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prompt")
    UDataTable* PromptDataTable = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prompt")
    FName PromptRowName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prompt")
    bool bUseGamepadIcon;
};
