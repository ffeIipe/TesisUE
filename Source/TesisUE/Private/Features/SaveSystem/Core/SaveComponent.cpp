#include "Features/SaveSystem/Core/SaveComponent.h"

USaveComponent::USaveComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USaveComponent::GenerateSaveID()
{
	if (!UniqueSaveID.IsValid())
	{
		UniqueSaveID = FGuid::NewGuid();
	}
}

FString USaveComponent::GetSaveIDAsString() const
{
	return UniqueSaveID.ToString();
}

void USaveComponent::InvalidateSaveID()
{
	UniqueSaveID.Invalidate();
}