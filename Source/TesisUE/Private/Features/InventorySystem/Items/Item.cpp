#include "Features/InventorySystem/Items/Item.h"
#include "Features/SaveSystem/Data/SaveTypes.h"
#include "Components/SphereComponent.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"


AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = false;
	
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Item Mesh"));
	RootComponent = ItemMesh;
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ItemMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel5, ECR_Block);

	SphereCollider = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collider"));
	SphereCollider->SetupAttachment(GetRootComponent());
	SphereCollider->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereCollider->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AItem::BeginPlay()
{
	Super::BeginPlay();
}

void AItem::OnEnteredInventory_Implementation(AActor* NewOwner)
{
	SetOwner(NewOwner);
	SetInstigator(Cast<APawn>(NewOwner));
	bWasUsed = true;

	DisableCollision();
}

void AItem::OnRemovedFromInventory_Implementation()
{
	bWasUsed = false;
}

bool AItem::ShouldConsumeOnUse_Implementation()
{
	return false;	
}

void AItem::EnableVisuals()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
}

void AItem::DisableVisuals()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void AItem::OnSaveGame_Implementation(FEntitySaveData& OutData)
{
	FMemoryWriter MemWriter(OutData.ByteData);
	FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);
	Ar.ArIsSaveGame = true;
	this->Serialize(Ar);
}

void AItem::OnLoadGame_Implementation(const FEntitySaveData& InData)
{
	FMemoryReader MemReader(InData.ByteData);
	FObjectAndNameAsStringProxyArchive Ar(MemReader, true);
	Ar.ArIsSaveGame = true;
	this->Serialize(Ar);
}

void AItem::OnPostGameLoaded_Implementation()
{
	Destroy();
}

void AItem::DisableCollision()
{
	ItemMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel5, ECR_Ignore);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
