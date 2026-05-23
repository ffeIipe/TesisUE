#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/PlayerController.h"
#include "CustomPlayerController.generated.h"

struct FInputActionValue;
enum class EVelmaraAbilityInputID : uint8;
class UInputMappingContext;
class UInputData;
class UUserWidget;
class USoundMix;

UCLASS()
class TESISUE_API ACustomPlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
	
public:
	ACustomPlayerController();

	virtual void SetupInputComponent() override;
	
	UFUNCTION(BlueprintCallable, Category = "Pause")
	void TogglePauseMenu();
	
	UPROPERTY(BlueprintReadOnly)
	class APlayerMainHUD* PlayerMainHUD;

	UFUNCTION(BlueprintCallable, Category = "GAS|Possession")
	void PerformPossession(APawn* NewPawn);

	UFUNCTION(BlueprintCallable, Category = "GAS|Possession")
	bool IsPossessing() const;
	
	UPROPERTY(BlueprintReadWrite)
	FVector2D InputMovingVector = FVector2D::ZeroVector;
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> OptionsMenuWidgetClass_Pause;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	USoundMix* PauseSoundMix;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputData> InputData;

	UPROPERTY(BlueprintReadOnly, Category = "GAS|Possession")
	APawn* OriginalBody;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Team")
	FGenericTeamId TeamId;

	UFUNCTION(BlueprintCallable, Category = "AI | Team")
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

	UFUNCTION(BlueprintCallable, Category = "AI | Team")
	virtual FGenericTeamId GetGenericTeamId() const override;

	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

private:
	void AbilityInputTagPressed(EVelmaraAbilityInputID InputId);
	
	void AbilityInputTagReleased(EVelmaraAbilityInputID InputId);

	void Input_Move(const FInputActionValue& Value);
	
	void Input_Look(const FInputActionValue& Value);
	
	UPROPERTY()
	UUserWidget* CurrentPauseMenuInstance;

	UPROPERTY()
	UUserWidget* CurrentOptionsMenuInstance_Pause;

	bool bIsGamePausedExplicitly;
};
