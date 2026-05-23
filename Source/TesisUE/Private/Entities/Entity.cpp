#include "Entities/Entity.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GenericTeamAgentInterface.h"
#include "MotionWarpingComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/TargetingComponent.h"
#include "Features/SaveSystem/Data/SaveTypes.h"
#include "DataAssets/EntityData.h"
#include "Features/InventorySystem/Core/InventoryComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GAS/VelmaraAbilityInputID.h"
#include "GAS/VelmaraAttributeSet.h"
#include "GAS/VelmaraGameplayAbility.h"
#include "GAS/VelmaraGameplayTags.h"
#include "Interfaces/Weapon/WeaponInterface.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

AEntity::AEntity()
{
	PrimaryActorTick.bCanEverTick = false;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(false);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UVelmaraAttributeSet>(TEXT("AttributeSet"));
	
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));

	TargetingComponent = CreateDefaultSubobject<UTargetingComponent>(TEXT("Targeting"));

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArmComponent->SetupAttachment(GetRootComponent());

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(GetSpringArmComponent());

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarping"));
}

void AEntity::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		GiveDefaultAbilities();
		InitializeAttributeSet();
	}

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	
	GetSpringArmComponent()->bUsePawnControlRotation = true;

	if (GetMesh())
	{
		const TArray<UMaterialInterface*> TempArray = GetMesh()->GetMaterials();

		for (int32 MatIndex = 0; MatIndex < TempArray.Num(); MatIndex++)
		{
			UMaterialInterface* CurrentMaterial = GetMesh()->GetMaterial(MatIndex);
			DissolveMaterials.Add(UMaterialInstanceDynamic::Create(CurrentMaterial, this));
			GetMesh()->SetMaterial(MatIndex, DissolveMaterials[MatIndex]);
		}
	}

	if (GetController())
	{
		if (const TScriptInterface<IGenericTeamAgentInterface> TeamAgentInterface = GetController())
		{
			TeamId = TeamAgentInterface->GetGenericTeamId();
		}
	}

	//this only happens in case the entity has extra mesh's children collision boxes

	/*if (GetMesh()->GetAttachChildren().Num() > 0)
	{
		TArray<USceneComponent*> BoxComponents;
		GetMesh()->GetChildrenComponents(false, BoxComponents);

		for (USceneComponent* Child : BoxComponents)
		{
			if (UBoxComponent* ChildBox = Cast<UBoxComponent>(Child))
			{
				if (ChildBox->ComponentTags.Num() > 0 && ChildBox->ComponentTags[0].ToString().StartsWith("Hitbox."))
				{
					ChildBox->OnComponentBeginOverlap.AddDynamic(this, &AEntity::OnBodyPartOverlap);
					if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, FColor::Blue, "Child Hitbox bound...");
				}
			}
		}
	}*/
}

void AEntity::InitializeComponentsData() const
{
	if (EntityData)
	{
		InventoryComponent->InitializeValues(EntityData->InventoryData);
		
		GetSpringArmComponent()->TargetArmLength = EntityData->SpringArmData.SpringArmLength;
		GetSpringArmComponent()->SocketOffset = EntityData->SpringArmData.SocketOffset;
		GetSpringArmComponent()->bEnableCameraLag = EntityData->SpringArmData.CameraLag;
		GetSpringArmComponent()->CameraLagSpeed = EntityData->SpringArmData.CameraLagSpeed;
		GetSpringArmComponent()->CameraRotationLagSpeed = EntityData->SpringArmData.CameraRotationLagSpeed;

		GetCharacterMovement()->GravityScale = EntityData->CharMoveData.GravityScale;
		GetCharacterMovement()->MaxAcceleration = EntityData->CharMoveData.MaxAcceleration;
		GetCharacterMovement()->MaxWalkSpeed = EntityData->CharMoveData.MaxWalkSpeed;
		GetCharacterMovement()->BrakingDecelerationWalking = EntityData->CharMoveData.BrakingDecelerationWalking;
		GetCharacterMovement()->JumpZVelocity = EntityData->CharMoveData.JumpZVelocity;
		GetCharacterMovement()->AirControl = EntityData->CharMoveData.AirControl;
		GetCharacterMovement()->AirControlBoostMultiplier = EntityData->CharMoveData.AirControlBoostMultiplier;
		GetCharacterMovement()->RotationRate = EntityData->CharMoveData.RotationRate;
		GetCharacterMovement()->MaxJumpApexAttemptsPerSimulation = EntityData->CharMoveData.MaxJumpApexAttemptsPerSimulation;
	}
}

void AEntity::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	InitializeComponentsData();
}

void AEntity::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GetOwnedGameplayTags(TagContainer);
	}
}

UAbilitySystemComponent* AEntity::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AEntity::OnSaveGame_Implementation(FEntitySaveData& OutData)
{
	FMemoryWriter MemWriter(OutData.ByteData);
	FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);
	Ar.ArIsSaveGame = true;
    
	this->Serialize(Ar); 
    
	if (InventoryComponent)
	{
		InventoryComponent->SaveInventory(); 
		InventoryComponent->Serialize(Ar); 
	}
}

void AEntity::OnLoadGame_Implementation(const FEntitySaveData& InData)
{
	FMemoryReader MemReader(InData.ByteData);
	FObjectAndNameAsStringProxyArchive Ar(MemReader, true);
	Ar.ArIsSaveGame = true;

	this->Serialize(Ar); 

	if (InventoryComponent)
	{
		InventoryComponent->Serialize(Ar); 
		InventoryComponent->LoadInventory();
	}

	StopAnimMontage();
}

void AEntity::ReceiveDamage_Implementation(FGameplayEventData DamagePayload)
{
	IDamageable::ReceiveDamage_Implementation(DamagePayload);
}

void AEntity::MortalDamage_Implementation(FGameplayEventData DeathPayload)
{
	IDamageable::MortalDamage_Implementation(DeathPayload);
}

TScriptInterface<IPickable> AEntity::GetCurrentItem() const
{
	return GetInventoryComponent()->GetCurrentWeapon();
}

FGameplayTag AEntity::GetDirectionalReact(const FVector& ImpactPoint)
{
	const FVector Forward = GetActorForwardVector();
	const FVector ToHit = (ImpactPoint - GetActorLocation()).GetSafeNormal();

	const double CosAngle = FVector::DotProduct(Forward, ToHit);

	double Angle = FMath::Acos(CosAngle);

	Angle = FMath::RadiansToDegrees(Angle);

	if (const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit); CrossProduct.Z < 0)
	{
		Angle *= -1.f;
	}

	FName Section("Damage.Default.FromBack");

	if (Angle >= -45.f && Angle < 45.f)
	{
		Section = FName("Damage.Default.FromFront");
	}

	else if (Angle >= -135.f && Angle < -45.f)
	{
		Section = FName("Damage.Default.FromLeft");
	}

	else if (Angle >= 45.f && Angle < 135.f)
	{
		Section = FName("Damage.Default.FromRight");
	}

	return FGameplayTag::RequestGameplayTag(Section);
}

bool AEntity::IsEquipped() const
{
	if (!AbilitySystemComponent) return false;

	const bool bIsEquipped = AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Weapon")));

	return bIsEquipped;
}

bool AEntity::IsInAir()
{
	if (!GetCharacterMovementComponent()) return false;
	
	const bool bIsInAir = GetCharacterMovementComponent()->MovementMode == MOVE_Flying
	|| GetCharacterMovementComponent()->MovementMode == MOVE_Falling;

	return bIsInAir;
}

bool AEntity::IsBlocking() const
{
	if (!AbilitySystemComponent) return false;

	const bool bIsBlocking = AbilitySystemComponent->HasMatchingGameplayTag(FVelmaraGameplayTags::Get().State_Blocking);
	
	return bIsBlocking;
}

bool AEntity::CanBeFinished()
{
	const float HealthPercentage = FMath::Clamp(AttributeSet->GetHealth(), 0.f, 1.f);
	
	const bool bHasReachedLowHealth = HealthPercentage <= .4f;

	if (bHasReachedLowHealth)
	{
		if (OnCanBeFinished.IsBound()) OnCanBeFinished.Broadcast();
	}
	
	return bHasReachedLowHealth;
}

bool AEntity::IsLaunchable() const
{
	return !IsShielded();
}

void AEntity::SetWeaponCollisionEnabled(const ECollisionEnabled::Type CollisionEnabled)
{
	if (GetCurrentItem())
	{
		if (const TScriptInterface<IWeaponInterface> Weapon = GetCurrentItem().GetObject())
		{
			Weapon->SetWeaponCollisionEnabled(CollisionEnabled);
			
			if (CollisionEnabled != ECollisionEnabled::NoCollision)
			{
				Weapon->ClearIgnoreActors();
			}
		}
	}
}

bool AEntity::IsHostile(const AEntity* OtherEntity) const
{
	if (!OtherEntity) return false;
	return TeamId != OtherEntity->TeamId;
}

void AEntity::SetTeam(const ETeamAttitude::Type NewTeam)
{
	TeamId = NewTeam;
	
	if (const TScriptInterface<IGenericTeamAgentInterface> TeamAgent = GetController())
	{
		if (TeamAgent)
		{
			TeamAgent->SetGenericTeamId(NewTeam);
		}
	}
}

void AEntity::InitializeAttributeSet()
{
	if (AbilitySystemComponent && DefaultAttributeEffect)
	{
		FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
		ContextHandle.AddSourceObject(this);

		const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributeEffect, 1.f, ContextHandle);

		if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, FColor::Red, "Default Attributes Set");
		
		if (SpecHandle.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), AbilitySystemComponent);
		}
	}
	
}

void AEntity::GiveDefaultAbilities()
{
	if (!AbilitySystemComponent || !HasAuthority()) return;

	if (!EntityData) return;
	
	for (const auto Ability : EntityData->DefaultAbilities)
	{
		if (Ability)
		{
			EVelmaraAbilityInputID InputID = Ability.GetDefaultObject()->AbilityInputID;
			FGameplayAbilitySpec Spec(Ability, 1, static_cast<int32>(InputID), this);
			FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(Spec);

			GrantedAbilityHandles.Add(Handle);
		}
	}
}

bool AEntity::IsAlive() const
{
	if (!AbilitySystemComponent) return false;

	//const bool bHasHealth = AttributeSet && AttributeSet->GetHealth() > 0.0f;
	const bool bIsNotMarkedDead = !AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("State.Dead"));

	return /*bHasHealth*/ /*&&*/ bIsNotMarkedDead; 
}

bool AEntity::IsShielded() const
{
	if (!AbilitySystemComponent) return false;
	
	const bool bHasShield = AttributeSet && AttributeSet->GetShield() > 0.0f;
	const bool bIsNotMarkedUnshielded = !AbilitySystemComponent->HasMatchingGameplayTag(FVelmaraGameplayTags::Get().State_Shielded);

	return bHasShield && bIsNotMarkedUnshielded;
}

bool AEntity::HasEnergy() const
{
	if (!AbilitySystemComponent) return false;

	const bool bHasEnergy = AttributeSet && AttributeSet->GetEnergy() > 0.0f;

	return bHasEnergy;
}

bool AEntity::IsStunned() const
{
	if (!AbilitySystemComponent) return false;

	const bool bIsStunned = AbilitySystemComponent->HasMatchingGameplayTag(FVelmaraGameplayTags::Get().State_Stunned);

	return bIsStunned;
}

void AEntity::OnBodyPartOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || SweepResult.GetActor() == this) return;

	if (IgnoreActors.Contains(SweepResult.GetActor())) return;

	IgnoreActors.Add(SweepResult.GetActor());
	IgnoreActors.Add(this);

	if (const AEntity* TargetEntity = Cast<AEntity>(OtherActor))
	{
		if (!TargetEntity->IsAlive()) return;
	}
	
	//if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, FColor::Red, "Dealing body damage");
	
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (!TargetASC) return;
		
	if (EntityData && EntityData->UnarmedDamageEffectSpecClass)
	{
		//Context
		FGameplayEffectContextHandle Context = GetAbilitySystemComponent()->MakeEffectContext();
		Context.AddHitResult(SweepResult);

		//Spec + Context
		FGameplayEffectSpecHandle DamageEffectSpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(
		EntityData->UnarmedDamageEffectSpecClass, 
		1.0f, 
		Context
		);

		//Set Damage type
		if (DamageEffectSpecHandle.IsValid())
		{
			DamageEffectSpecHandle.Data.Get()->AddDynamicAssetTag(EntityData->UnarmedDamageTag);

			DamageEffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Damage")),
				-EntityData->UnarmedDamage
			);
		}
			
		//Applying the gameplay effect to the hit actor
		GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(
			*DamageEffectSpecHandle.Data.Get(),
			TargetASC
		);

		//VFX call
		FGameplayCueParameters CueParameters;
		CueParameters.Location = SweepResult.ImpactPoint;
		CueParameters.Normal = SweepResult.ImpactNormal;
		CueParameters.Instigator = this;

		TargetASC->ExecuteGameplayCue(EntityData->UnarmedGameplayCueTag, CueParameters);
	}
}

void AEntity::ActivateBodyHitbox(const FName ComponentTag, FGameplayTag DamageType)
{
	for (UActorComponent* Comp : GetComponents())
	{
		if (Comp->ComponentHasTag(ComponentTag))
		{
			if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Comp))
			{
				//if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, FColor::Emerald, "Collision Enabled...");
				
				Prim->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				Prim->IgnoreActorWhenMoving(this, true);
			}
		}
	}
}

void AEntity::DeactivateBodyHitbox(const FName ComponentTag) const
{
	for (UActorComponent* Comp : GetComponents())
	{
		if (Comp->ComponentHasTag(ComponentTag))
		{
			if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Comp))
			{
				Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}
}

void AEntity::HitFlash(const float Duration, const float Amount)
{
	for (UMaterialInstanceDynamic* DissolveMaterial : DissolveMaterials)
	{
		if (DissolveMaterial)
		{
			DissolveMaterial->SetScalarParameterValue(FName("HitFlashAmount"), Amount);
			GetWorldTimerManager().SetTimer(HitFlashTimerHandle, this, &AEntity::DeactivateHitFlash, Duration, false);
		}
	}
}

void AEntity::DeactivateHitFlash()
{
	for (UMaterialInstanceDynamic* DissolveMaterial : DissolveMaterials)
	{
		if (DissolveMaterial)
		{
			DissolveMaterial->SetScalarParameterValue(FName("HitFlashAmount"), 0);
		}
	}
}