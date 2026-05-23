#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Features/GlobalEffectsSystem/Interfaces/EffectManagerProvider.h"
#include "VelmaraGameInstance.generated.h"

struct FGameplayTag;
class UPlayerProgressSaveGame;
class USettingsSaveGame;
class ACharacter;
class AEnemy;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaveDataUpdated);

UCLASS()
class TESISUE_API UVelmaraGameInstance : public UGameInstance, public IEffectManagerProvider
{
    GENERATED_BODY()

public:
    UVelmaraGameInstance();

    virtual void Init() override;
    virtual void Shutdown() override;

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetScreenResolution(FIntPoint NewResolution);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetTextureQuality(int32 NewQuality);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetShadowQuality(int32 NewQuality);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetAntiAliasing(int32 NewQuality);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetViewDistance(int32 NewQuality);
	
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetPostProcess(int32 NewQuality);
	
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetVisualEffect(int32 NewQuality);
	
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetGlobalIllumination(int32 NewQuality);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetShadingQuality(int32 NewQuality);

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetFrameRateLimit(int32 NewFrameRateLimit);

    UFUNCTION(BlueprintPure, Category = "Settings")
    FIntPoint GetScreenResolution() const;

    UFUNCTION(BlueprintPure, Category = "Settings")
    int32 GetTextureQuality() const;

    UFUNCTION(BlueprintPure, Category = "Settings")
    int32 GetShadowQuality() const;
    
    UFUNCTION(BlueprintPure, Category = "Settings")
    int32 GetAntiAliasingQuality() const;

    UFUNCTION(BlueprintPure, Category = "Settings")
    int32 GetViewDistanceQuality() const;

    UFUNCTION(BlueprintPure, Category = "Settings")
    int32 GetPostProcessQuality() const;

    UFUNCTION(BlueprintPure, Category = "Settings")
    int32 GetGlobalIlluminationQuality() const;

    UFUNCTION(BlueprintPure, Category = "Settings")
    int32 GetShadingQuality() const;

    UFUNCTION(BlueprintPure, Category = "Settings")
    int32 GetVisualEffectsQuality() const;

    UFUNCTION(BlueprintPure, Category = "Settings")
    int32 GetFrameRateLimit() const;
    
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SaveGameSettings();

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void LoadGameSettings();

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void ApplyGraphicsSettings();

    UPROPERTY(BlueprintReadOnly, Category = "Settings")
    USettingsSaveGame* CurrentSettings;
	
    UFUNCTION(BlueprintCallable, Category = "GameFlow")
    void CreateNewGame(int32 SlotIndex, FString StartLevelName);

    UFUNCTION(BlueprintCallable, Category= "GameFlow")
    bool DoesProgressSaveExist(int32 SlotIndex) const;
	
    UFUNCTION(BlueprintCallable, Category = "SaveSystem")
    void SaveGame(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "SaveSystem")
    void LoadGame(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "SaveSystem")
    UPlayerProgressSaveGame* GetSaveGameInfo(int32 SlotIndex) const;

    FString GetSlotNameByIndex(int32 SlotIndex) const;

	UPROPERTY(BlueprintReadWrite, Category = "SaveSystem")
	int32 ActiveSaveSlotIndex;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Loading")
	TSubclassOf<UUserWidget> LoadingScreenWidgetClass;

	UFUNCTION(BlueprintCallable, Category = "UI|Loading")
	void ShowLoadingScreen();

	UFUNCTION(BlueprintCallable, Category = "UI|Loading")
	void HideLoadingScreen();

	UPROPERTY(BlueprintAssignable, Category = "SaveSystem")
	FOnSaveDataUpdated OnSaveDataUpdated;

	virtual void PlayGameplayEffect_Implementation(FGameplayTag EffectTag, FVector Location) override;
	
private:
    const FString SettingsSlotName = TEXT("GameSettings");

    void SetDefaultGameSettings();

	UPROPERTY(VisibleAnywhere, Category = "Metadata")
	FString CurrentSlotName;
	
	const FString ProgressSaveSlotPrefix = TEXT("PlayerProgressSlot_");

	UPROPERTY(VisibleAnywhere, Category = "Metadata")
	uint32 UserIndex;

	const int32 DefaultUserIndex = 0;
	
	UPROPERTY()
	UUserWidget* CurrentLoadingScreenInstance;
};