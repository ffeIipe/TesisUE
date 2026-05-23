#include "Items/Weapons/Sword.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"

#include "DataAssets/Items/Weapons/SwordData.h"
#include "Features/GlobalEffectsSystem/Interfaces/EffectManagerProvider.h"
#include "GameFramework/Character.h"
#include "GAS/VelmaraAbilityInputID.h"
#include "GAS/VelmaraGameplayAbility.h"

ASword::ASword()
{
	SphereCollider->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	WeaponDamageBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Box"));
	WeaponDamageBox->SetupAttachment(GetRootComponent());
	WeaponDamageBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponDamageBox->SetCollisionResponseToAllChannels(ECR_Overlap);
	WeaponDamageBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	WeaponDamageBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	WeaponDamageBox->OnComponentBeginOverlap.AddDynamic(this, &ASword::OnBoxOverlap);

	BoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
	BoxTraceStart->SetupAttachment(GetRootComponent());

	BoxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
	BoxTraceEnd->SetupAttachment(GetRootComponent());
}

void ASword::BeginPlay()
{
	Super::BeginPlay();

	if (!SwordData)
	{
		//if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, FColor::Red, "MISSING! Data asset of: " + GetName() + " is nullptr.");
	}
}

void ASword::Equip()
{
	Super::Equip();

	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner());
	if (!ASI) return;

	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC) return;
		
	if (SwordData && SwordData->WeaponTag.IsValid())
	{
		if (ASC->HasMatchingGameplayTag(SwordData->WeaponTag)) return;
		
		ASC->AddLooseGameplayTag(SwordData->WeaponTag);
	}

	if (SwordData)
	{
		for (TSubclassOf AbilityClass : SwordData->AbilitiesToGrant)
		{
			if (AbilityClass)
			{
				EVelmaraAbilityInputID InputID = EVelmaraAbilityInputID::None;
				if (UVelmaraGameplayAbility* VGA = Cast<UVelmaraGameplayAbility>(AbilityClass.GetDefaultObject()))
				{
					InputID = VGA->AbilityInputID;
				}

				FGameplayAbilitySpec Spec(AbilityClass, 1, static_cast<int32>(InputID), this); 
				FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
				GrantedAbilityHandles.Add(Handle);
			}
		}

		if (IsValid(SwordData->AnimLayer))
		{
			if (const ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner()))
			{
				CharacterOwner->GetMesh()->GetAnimInstance()->LinkAnimClassLayers(SwordData->AnimLayer);
			}
		}
	}
}

void ASword::Holster()
{
	Super::Holster();

	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner());
	if (!ASI) return;
	
	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC) return;
	
	for (const FGameplayAbilitySpecHandle& Handle : GrantedAbilityHandles)
	{
		if (Handle.IsValid())
		{
			ASC->ClearAbility(Handle);
		}
	}

	GrantedAbilityHandles.Empty();
	
	ASC->RemoveLooseGameplayTag(SwordData->WeaponTag);

	if (!ASC->HasMatchingGameplayTag(SwordData->HolsterTag))
	{
		ASC->AddLooseGameplayTag(SwordData->HolsterTag);
	}

	if (IsValid(SwordData->AnimLayer))
	{
		if (const ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner()))
		{
			CharacterOwner->GetMesh()->GetAnimInstance()->UnlinkAnimClassLayers(SwordData->AnimLayer);
		}
	}
}

void ASword::AttachMeshToSocket(USceneComponent* InParent, const FName InSocketName)
{
	Super::AttachMeshToSocket(InParent, InSocketName);

	const FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);

	if (InSocketName == NAME_None)
	{
		ItemMesh->AttachToComponent(InParent, TransformRules, SwordData->CustomInSocketName);
	}
	else
	{
		ItemMesh->AttachToComponent(InParent, TransformRules, InSocketName);
	}
}

void ASword::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const FVector Start = BoxTraceStart->GetComponentLocation();
	const FVector End = BoxTraceEnd->GetComponentLocation();

	IgnoreActors.Add(this);
	IgnoreActors.Add(GetOwner());

	TArray<FHitResult> HitResults;

	UKismetSystemLibrary::BoxTraceMulti(
		this,
		Start,
		End,
		FVector(25.f, 25.f, 25.f),
		BoxTraceStart->GetComponentRotation(),
		UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel3),
		false,
		IgnoreActors,
		EDrawDebugTrace::None,
		HitResults,
		true
	);

	for (const FHitResult& Hit : HitResults)
	{
		if (IgnoreActors.Contains(Hit.GetActor())) return;

		IgnoreActors.Add(Hit.GetActor());
		
		if (const TScriptInterface<IAbilitySystemInterface> TargetASI = Hit.GetActor())
		{
			UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();
			
			if (TargetASC && DamageEffectSpecHandle.IsValid())
			{
				//Set the damage type
				DamageEffectSpecHandle.Data.Get()->AddDynamicAssetTag(CurrentDamageTag);
				
				//Also sending the hit result
				FGameplayEffectContextHandle Context = GetOwner()->FindComponentByClass<UAbilitySystemComponent>()->MakeEffectContext();
				Context.AddHitResult(Hit);

				DamageEffectSpecHandle.Data->SetContext(Context);
				
				//Applying the gameplay effect to the hit actor
				GetOwner()->FindComponentByClass<UAbilitySystemComponent>()->ApplyGameplayEffectSpecToTarget(
					*DamageEffectSpecHandle.Data.Get(),
					TargetASC
				);
				
				//VFX call
				FGameplayCueParameters CueParameters;
				CueParameters.Location = Hit.ImpactPoint;
				CueParameters.Normal = Hit.ImpactNormal;
				CueParameters.Instigator = GetOwner();
				CueParameters.TargetAttachComponent = OverlappedComponent;

				TargetASC->ExecuteGameplayCue(CurrentCueTag, CueParameters);

				if (GetGameInstance()->Implements<UEffectManagerProvider>())
				{
					IEffectManagerProvider::Execute_PlayGameplayEffect(GetGameInstance(), CurrentDamageTag, Hit.ImpactPoint);
				}
			}
		}

		OnActorsWeaponHit(HitResults);
	}
}

void ASword::ClearIgnoreActors()
{
	IgnoreActors.Empty();
}

void ASword::SetWeaponCollisionEnabled(const ECollisionEnabled::Type CollisionEnabled)
{
	Super::SetWeaponCollisionEnabled(CollisionEnabled);
	
	WeaponDamageBox->SetCollisionEnabled(CollisionEnabled);
}