#include "SceneEvents/TriggerComponent.h"
#include "GameplayTagAssetInterface.h" 
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"

UTriggerComponent::UTriggerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetCollisionProfileName(TEXT("Trigger"));
}

void UTriggerComponent::BeginPlay()
{
	Super::BeginPlay();
	OnComponentBeginOverlap.AddDynamic(this, &UTriggerComponent::OnBoxBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UTriggerComponent::OnBoxEndOverlap);
	
}

bool UTriggerComponent::HasMatchingGameplayTags(AActor* Actor) const
{
	if (!Actor) return false;

	const IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Actor);
	FGameplayTagContainer ActorTags;

	if (TagInterface)
	{
		TagInterface->GetOwnedGameplayTags(ActorTags);
	}
	else 
	{
		if (const APawn* Pawn = Cast<APawn>(Actor))
		{
			if (const AController* Controller = Pawn->GetController())
			{
				if (const IGameplayTagAssetInterface* ControllerInterface = Cast<IGameplayTagAssetInterface>(Controller))
				{
					ControllerInterface->GetOwnedGameplayTags(ActorTags);
				}
			}
		}
	}

	if (ActorTags.IsEmpty()) return false;

	if (IgnoredTags.Num() > 0 && ActorTags.HasAny(IgnoredTags))
	{
		return false;
	}

	return ActorTags.HasAny(AcceptedTags);
}

void UTriggerComponent::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasMatchingGameplayTags(OtherActor))
	{
		OnActorEntered.Broadcast(OtherActor);
	}
}

void UTriggerComponent::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (HasMatchingGameplayTags(OtherActor))
	{
		OnActorExit.Broadcast(OtherActor);
	}
}