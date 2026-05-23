#include "Subsystems/SpectralObjectsSubsystem.h"
#include "SpectralMode/Interfaces/Spectral.h"


void USpectralObjectsSubsystem::AddSpectralObject(AActor* Actor)
{
	if (Actor->GetClass()->ImplementsInterface(USpectral::StaticClass()))
	{
		const TScriptInterface<ISpectral> SpectralInterface = Actor;
		SpectralObjects.AddUnique(SpectralInterface);
	}
}

void USpectralObjectsSubsystem::ActivateSpectralObjects()
{
	for (const TScriptInterface SpectralObject : SpectralObjects)
	{
		SpectralObject->Execute_ActivateVisibility(SpectralObject.GetObject());
	}
}

void USpectralObjectsSubsystem::DeactivateSpectralObjects()
{
	for (const TScriptInterface SpectralObject : SpectralObjects)
	{
		SpectralObject->Execute_DeactivateVisibility(SpectralObject.GetObject());
	}
}

void USpectralObjectsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	DeactivateSpectralObjects();
}