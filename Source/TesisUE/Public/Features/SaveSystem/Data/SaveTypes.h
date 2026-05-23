#pragma once

#include "CoreMinimal.h"
#include "Misc/Guid.h"
#include "SaveTypes.generated.h"

USTRUCT(BlueprintType)
struct FEntitySaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FGuid UniqueSaveID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FTransform Transform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	UClass* ActorClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TArray<uint8> ByteData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TMap<FName, float> SavedStats;
};