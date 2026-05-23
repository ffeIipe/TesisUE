#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Misc/Guid.h"
#include "SaveComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TESISUE_API USaveComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USaveComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save System")
	FGuid UniqueSaveID;

	UFUNCTION(BlueprintCallable, Category = "Save System")
	void GenerateSaveID();

	UFUNCTION(BlueprintPure, Category = "Save System")
	FString GetSaveIDAsString() const;
    
	UFUNCTION(BlueprintCallable, Category = "Save System")
	void InvalidateSaveID();
};