#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Features/SaveSystem/Data/SaveTypes.h"
#include "SaveInterface.generated.h"

UINTERFACE()
class USaveInterface : public UInterface
{
	GENERATED_BODY()
};

class TESISUE_API ISaveInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SaveInterface")
	void OnSaveGame(FEntitySaveData& OutData);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SaveInterface")
	void OnLoadGame(const FEntitySaveData& InData);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SaveInterface")
	void OnPostGameLoaded();
};
