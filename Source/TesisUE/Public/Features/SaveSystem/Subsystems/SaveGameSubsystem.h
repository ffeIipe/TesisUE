#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Features/SaveSystem/Core/PlayerProgressSaveGame.h"
#include "SaveGameSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameSaved, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameLoaded, bool, bSuccess);

UCLASS()
class TESISUE_API USaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	static void SerializeProperties(UObject* Object, TArray<uint8>& OutBytes);

	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	static void DeserializeProperties(UObject* Object, const TArray<uint8>& InBytes);
	
public:
	UFUNCTION(BlueprintCallable, Category = "Save System")
	void SaveGame(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Save System")
	void LoadGame(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Save System")
	void CreateNewGame(int32 SlotIndex, const FString MapName);

	UFUNCTION(BlueprintPure, Category = "Save System")
	bool DoesSaveGameExist(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool GetSaveSlotInfo(int32 SlotIndex, FString& OutLevelName, FDateTime& OutTimestamp, FString& OutSlotLabel) const;

	UFUNCTION(BlueprintCallable, Category = "Save System")
	void RestoreCurrentLevelState() const;
	
	UPROPERTY(BlueprintAssignable)
	FOnGameSaved OnGameSaved;

	UPROPERTY(BlueprintAssignable)
	FOnGameLoaded OnGameLoaded;

protected:
	UPROPERTY()
	int32 CurrentSlotIndex = -1;

	UPROPERTY()
	UPlayerProgressSaveGame* CurrentSaveGame;

private:
	FString GetSlotNameFromIndex(int32 SlotIndex) const;

	void SaveLevelActors(UPlayerProgressSaveGame* SaveObject) const;
	void LoadLevelActors(UPlayerProgressSaveGame* SaveObject) const;
};