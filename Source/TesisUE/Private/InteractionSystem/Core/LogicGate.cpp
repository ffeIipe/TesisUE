#include "InteractionSystem/Core/LogicGate.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "DrawDebugHelpers.h"
#include "InteractionSystem/Interfaces/Interactable.h"
#include "Kismet/KismetSystemLibrary.h"

ALogicGate::ALogicGate()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALogicGate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bDebugConnections)
	{
		const FColor LineColor = bOutputState ? FColor::Green : FColor::Red;
        
		for (const AActor* Target : LinkedTargets)
		{
			if (Target)
			{
				UKismetSystemLibrary::DrawDebugArrow(GetWorld(), GetActorLocation(), Target->GetActorLocation(), 
					50.0f, LineColor, -1.0f, 2.0f);
			}
		}

		const FString DebugMsg = FString::Printf(TEXT("GATE: %d / %d"), ActiveSources.Num(), RequiredActivations);
		const FColor TextColor = (ActiveSources.Num() >= RequiredActivations) ? FColor::Green : FColor::Yellow;
        
		DrawDebugString(GetWorld(), GetActorLocation() + FVector(0,0,50), DebugMsg, nullptr, TextColor, 0.0f);
	}
}

void ALogicGate::ResetSources()
{
	TArray<AActor*> SourcesToReset = ActiveSources.Array();

	ActiveSources.Empty();

	for (AActor* Source : SourcesToReset)
	{
		if (Source && Source->Implements<UInteractable>())
		{
			IInteractable::Execute_ExecuteInteraction(Source, this);
		}
	}

	UpdateLogicState();
}

void ALogicGate::BeginPlay()
{
	Super::BeginPlay();
	UpdateLogicState();
}

#if WITH_EDITOR
bool ALogicGate::ShouldTickIfViewportsOnly() const
{
	return bDebugConnections;
}
#endif

void ALogicGate::ReceiveSignal_Implementation(const bool bActive, AActor* Activator)
{
	if (!Activator) return;

	if (bIsByTimerDeactivation)
	{
		GetWorld()->GetTimerManager().SetTimer(DeactivationTimerHandle, this, &ALogicGate::ResetSources, DeactivationTime, false);
	}
	
	if (bActive)
	{
		ActiveSources.Add(Activator);
	}
	else
	{
		ActiveSources.Remove(Activator);
		GetWorld()->GetTimerManager().ClearTimer(DeactivationTimerHandle);
	}

	UpdateLogicState();
}

void ALogicGate::OnSaveGame_Implementation(FEntitySaveData& OutData)
{
	FMemoryWriter MemWriter(OutData.ByteData);
	FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);
	Ar.ArIsSaveGame = true;
    
	this->Serialize(Ar); 
}

void ALogicGate::OnLoadGame_Implementation(const FEntitySaveData& InData)
{
	FMemoryReader MemReader(InData.ByteData);
	FObjectAndNameAsStringProxyArchive Ar(MemReader, true);
	Ar.ArIsSaveGame = true;

	this->Serialize(Ar); 
}

void ALogicGate::UpdateLogicState()
{
	const bool bShouldBeActive = ActiveSources.Num() >= RequiredActivations;

	if (bOutputState != bShouldBeActive)
	{
		bOutputState = bShouldBeActive;
		BroadcastToTargets(bOutputState);
	}
}

void ALogicGate::BroadcastToTargets(const bool bActive)
{
	for (AActor* Target : LinkedTargets)
	{
		if (!Target) continue;
		
		if (Target->GetClass()->ImplementsInterface(USignalReceiver::StaticClass()))
		{
			Execute_ReceiveSignal(Target, bActive, this);
		}
	}

	GetWorld()->GetTimerManager().ClearTimer(DeactivationTimerHandle);
	
	if (bActive) DrawDebugString(GetWorld(), GetActorLocation(), "GATE OPEN", nullptr, FColor::Green, 2.0f);
}