#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractionSystem/Interfaces/Interactable.h"
#include "Features/SaveSystem/Interfaces/SaveInterface.h"
#include "Lever.generated.h"

UCLASS()
class TESISUE_API ALever : public AActor, public IInteractable, public ISaveInterface
{
	GENERATED_BODY()
	
public:	
	ALever();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Properties")
	TArray<AActor*> LinkedTargets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, Category = "Properties")
	bool bIsActive = false;

	UPROPERTY(EditAnywhere, Category = "Properties|Debug")
	bool bDebugConnections = true;

#if WITH_EDITOR
	virtual bool ShouldTickIfViewportsOnly() const override;
#endif
	
public:
	virtual bool ExecuteInteraction_Implementation(AActor* Interactor) override;

	virtual void OnSaveGame_Implementation(FEntitySaveData& OutData) override;

	virtual void OnLoadGame_Implementation(const FEntitySaveData& InData) override;
	
private:
	void SetLeverState(bool bNewState);
};