#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerMainHUD.generated.h"

class UInputPromptWidget;
class UInputPromptData;
class UPlayerMainWidget;
class UPaladinBossHealthBar;

UCLASS()
class TESISUE_API APlayerMainHUD : public AHUD
{
	GENERATED_BODY()
	
protected:
    virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	TSubclassOf<UPlayerMainWidget> PlayerMainWidgetClass;

public:
	UFUNCTION(BlueprintCallable)
	void TogglePlayerUI(bool Bool);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	UPlayerMainWidget* PlayerMainWidgetInstance = nullptr;

private:
	bool bAlreadySetted = false;
};