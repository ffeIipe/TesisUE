#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
#include "GenericTeamAgentInterface.h"
#include "Features/SaveSystem/Interfaces/SaveInterface.h"
#include "Interfaces/Damageable.h"

#include "Entity.generated.h"

struct FGameplayAbilitySpecHandle;
class IPickable;
class UInventoryComponent;
class UTargetingComponent;
class UCameraComponent;
class UMotionWarpingComponent;
struct FHitReactDefinition;
struct FInputActionValue;
class UVelmaraAttributeSet;
class UInputMappingContext;
class UGameplayEffect;
class USpringArmComponent;
class UEntityData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEntityDead, AEntity*, EntityDead);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEntityDamaged, AEntity*, LastDamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEntityCanBeFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEntityShieldTakeDamage);

UCLASS()
class TESISUE_API AEntity : public ACharacter, public ISaveInterface, public IGameplayTagAssetInterface,
                            public IAbilitySystemInterface, public IDamageable
{
	GENERATED_BODY()

public:
	AEntity();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TObjectPtr<UEntityData> EntityData;

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

	UFUNCTION(BlueprintCallable)
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	virtual void OnSaveGame_Implementation(FEntitySaveData& OutData) override;

	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	virtual void OnLoadGame_Implementation(const FEntitySaveData& InData) override;

	virtual void ReceiveDamage_Implementation(FGameplayEventData DamagePayload) override;

	virtual void MortalDamage_Implementation(FGameplayEventData DeathPayload) override;
	
	UFUNCTION(BlueprintPure, Category = "Components")
	FORCEINLINE UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

	UFUNCTION(BlueprintPure, Category = "Components")
	FORCEINLINE UTargetingComponent* GetTargetingComponent() const { return TargetingComponent; }

	UFUNCTION(BlueprintPure, Category = "Components")
	FORCEINLINE USpringArmComponent* GetSpringArmComponent() const { return SpringArmComponent; }

	UFUNCTION(BlueprintPure, Category = "Components")
	FORCEINLINE UCameraComponent* GetCameraComponent() const { return CameraComponent; }

	UFUNCTION(BlueprintPure, Category = "Components")
	FORCEINLINE UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarpingComponent; }

	UPROPERTY(BlueprintAssignable)
	FOnEntityCanBeFinished OnCanBeFinished;

	UPROPERTY(BlueprintAssignable)
	FOnEntityShieldTakeDamage OnShieldTakeDamage;
	
	UFUNCTION(BlueprintPure)
	TScriptInterface<IPickable> GetCurrentItem() const; 

	UFUNCTION(BlueprintPure)
	AActor* GetLastDamageCauser() const { return LastDamageCauser; }

	virtual ACharacter* GetCharacter() { return this; }
	virtual UCharacterMovementComponent* GetCharacterMovementComponent() { return GetCharacterMovement(); }
	
	UFUNCTION(BlueprintCallable)
	virtual FGameplayTag GetDirectionalReact(const FVector& ImpactPoint); 

	UFUNCTION(BlueprintCallable, Category = "Combat | Weapon")
	virtual void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnEntityDead OnDead;

	UFUNCTION(BlueprintCallable, Category = "Entity Flag")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "Entity Flag")
	bool IsShielded() const;

	UFUNCTION(BlueprintCallable, Category = "Entity Flag")
	bool HasEnergy() const;

	UFUNCTION(BlueprintCallable, Category = "Entity Flag")
	bool IsStunned() const;

	UFUNCTION(BlueprintCallable, Category = "Entity Flag")
	bool IsBlocking() const;

	UFUNCTION(BlueprintCallable, Category = "Entity Flag")
	bool IsLaunchable() const;

	UFUNCTION(BlueprintCallable, Category = "Entity Flag")
	bool CanBeFinished();

	UFUNCTION(BlueprintCallable, Category = "Entity Flag")
	bool IsEquipped() const;

	UFUNCTION(BlueprintCallable, Category = "Entity Flag")
	bool IsInAir();

	UFUNCTION()
	void OnBodyPartOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable, Category = "Entity|Unarmed")
	void ActivateBodyHitbox(FName ComponentTag, FGameplayTag DamageType);

	UFUNCTION(BlueprintCallable, Category = "Entity|Unarmed")
	void DeactivateBodyHitbox(FName ComponentTag) const;

	UFUNCTION(BlueprintCallable, Category = "Entity|Effects")
	void HitFlash(float Duration, float Amount);

	void DeactivateHitFlash();

	UPROPERTY()
	FTimerHandle HitFlashTimerHandle;
	
	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> DissolveMaterials;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	FGenericTeamId TeamId; 

	UFUNCTION(BlueprintCallable, Category = "Team")
	bool IsHostile(const AEntity* OtherEntity) const;

	UFUNCTION(BlueprintCallable, Category = "Team")
	void SetTeam(ETeamAttitude::Type NewTeam);

	UFUNCTION(BlueprintCallable, Category = "Team")
	FGenericTeamId GetTeamId() const {return TeamId;}
	
protected:
	UFUNCTION(BlueprintCallable)
	virtual void InitializeAttributeSet();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayEffect> DefaultAttributeEffect;

	UPROPERTY(BlueprintReadOnly, Category = "Abilities")
	TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;
	
	virtual void BeginPlay() override;
	
	virtual void OnConstruction(const FTransform &Transform) override;
	
	void InitializeComponentsData() const;

	UFUNCTION(BlueprintCallable)
	void GiveDefaultAbilities();
	
	UPROPERTY()
	AActor* LastDamageCauser;

	UPROPERTY(Transient)
	APlayerController* PlayerControllerRef = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input | Mapping")
	TObjectPtr<UInputMappingContext> CharacterContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	ACameraActor* FollowCamera;
	
	UPROPERTY()
	TArray<AActor*> IgnoreActors;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	UAbilitySystemComponent* AbilitySystemComponent;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintGetter = GetInventoryComponent)
	UInventoryComponent* InventoryComponent;

	UPROPERTY(VisibleDefaultsOnly,BlueprintGetter = GetTargetingComponent)
	UTargetingComponent* TargetingComponent;
	
	UPROPERTY(VisibleDefaultsOnly,BlueprintGetter = GetSpringArmComponent)
	USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleDefaultsOnly,BlueprintGetter = GetCameraComponent)
	UCameraComponent* CameraComponent;

	UPROPERTY(VisibleDefaultsOnly,BlueprintGetter = GetMotionWarpingComponent)
	UMotionWarpingComponent* MotionWarpingComponent;

	UPROPERTY(VisibleDefaultsOnly)
	UVelmaraAttributeSet* AttributeSet;
};