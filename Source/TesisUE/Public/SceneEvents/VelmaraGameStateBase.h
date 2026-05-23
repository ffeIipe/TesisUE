// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "VelmaraGameStateBase.generated.h"

class AItem;

UCLASS()
class TESISUE_API AVelmaraGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	//UFUNCTION()
	//void UpdateEntityState(AEntity* DeadEntity);

protected:
	bool bIsLoadingFromSave = false;
};