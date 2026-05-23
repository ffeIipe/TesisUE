#include "InteractionSystem/Core/Lever.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "InteractionSystem/Interfaces/SignalReceiver.h"
#include "Kismet/KismetSystemLibrary.h"

ALever::ALever()
{
	PrimaryActorTick.bCanEverTick = true;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
}

void ALever::BeginPlay()
{
	Super::BeginPlay();

	SetLeverState(bIsActive);
}

void ALever::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bDebugConnections)
	{
		const FColor LineColor = bIsActive ? FColor::Green : FColor::Red;

		for (const AActor* Target : LinkedTargets)
		{
			if (Target)
			{
				UKismetSystemLibrary::DrawDebugArrow(GetWorld(), GetActorLocation(), Target->GetActorLocation(), 
					30.0f, LineColor, -1.0f, 1.5f);
			}
		}
	}
}

#if WITH_EDITOR
bool ALever::ShouldTickIfViewportsOnly() const
{
	return bDebugConnections;
}
#endif

bool ALever::ExecuteInteraction_Implementation(AActor* Interactor)
{
	SetLeverState(!bIsActive);
	return true;
}

void ALever::SetLeverState(const bool bNewState)
{
	bIsActive = bNewState;

	for (AActor* Target : LinkedTargets)
	{
		if (!Target) continue;
		
		if (Target->GetClass()->ImplementsInterface(USignalReceiver::StaticClass()))
		{
			ISignalReceiver::Execute_ReceiveSignal(Target, bIsActive, this);
		}
	}
}

void ALever::OnSaveGame_Implementation(FEntitySaveData& OutData)
{
	FMemoryWriter MemWriter(OutData.ByteData);
	FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);
	Ar.ArIsSaveGame = true;
    
	this->Serialize(Ar);
}

void ALever::OnLoadGame_Implementation(const FEntitySaveData& InData)
{
	FMemoryReader MemReader(InData.ByteData);
	FObjectAndNameAsStringProxyArchive Ar(MemReader, true);
	Ar.ArIsSaveGame = true;

	this->Serialize(Ar);

	SetLeverState(bIsActive); 
}