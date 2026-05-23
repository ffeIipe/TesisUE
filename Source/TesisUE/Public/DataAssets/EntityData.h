#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "EntityData.generated.h"

class UVelmaraGameplayAbility;
class UGameplayEffect;
class UInputAction;
class UNiagaraSystem;

USTRUCT(BlueprintType)
struct FInventoryData
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 MaxSlots = 2;

    /** This property is optional, so if you are filling these stats and do not
      want to spawn with a weapon, leave it in blank. **/
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<AActor> InitialWeapon;
};

USTRUCT(BlueprintType)
struct FSpringArmData
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float SpringArmLength = 300.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FVector SocketOffset = FVector(0, 90.f, 60.f);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    bool CameraLag = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float CameraLagSpeed = 8.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float CameraRotationLagSpeed = 30.f;
};

USTRUCT(BlueprintType)
struct FCharMoveStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GravityScale = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxAcceleration = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxJumpApexAttemptsPerSimulation = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxWalkSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BrakingDecelerationWalking = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float JumpZVelocity = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AirControl = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AirControlBoostMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float JumpOffJumpZFactor = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator RotationRate = FRotator(0.0f, 400.0f, 0.0f);
};

USTRUCT(BlueprintType)
struct FCharacterModeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly)
    int32 TeamID = 0;

    UPROPERTY(EditDefaultsOnly)
    bool bShowWeapon = false;
    
    UPROPERTY(EditDefaultsOnly)
    bool bUseControllerRotationYaw = false;
};

USTRUCT(BlueprintType)
struct FDeathAnimDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTag DeathTag;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    bool bUseRagdoll = false;
};

USTRUCT(BlueprintType)
struct FHitReactDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTag DamageTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float MagnitudeReaction = 0.f;
};

UCLASS()
class TESISUE_API UEntityData : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Abilities")
    TArray<TSubclassOf<UVelmaraGameplayAbility>> DefaultAbilities;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Inventory")
    FInventoryData InventoryData;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|CharacterMovement")
    FCharMoveStats CharMoveData;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category = "Entity|SpringArm")
    FSpringArmData SpringArmData;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Unarmed")
    FGameplayTag UnarmedDamageTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Unarmed")
    TSubclassOf<UGameplayEffect> UnarmedDamageEffectSpecClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Unarmed")
    FGameplayTag UnarmedGameplayCueTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Unarmed")
    float UnarmedDamage;
};