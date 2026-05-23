// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponData.h"
#include "Engine/DataAsset.h"
#include "PistolData.generated.h"

/**
 * 
 */

class UNiagaraSystem;
class USoundBase;
class UAnimMontage;

USTRUCT(BlueprintType)
struct FPistolEffects
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    UNiagaraSystem* MuzzleFlash = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    UNiagaraSystem* SparksEffect = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
    USoundBase* ShootSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
    USoundBase* OutOfBlood = nullptr;
};

USTRUCT(BlueprintType)
struct FPistolMontages
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montages")
    UAnimMontage* PrimaryFireMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montages")
    UAnimMontage* SecondaryFireMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montages")
    UAnimMontage* ReloadMontage = nullptr;
};

USTRUCT(BlueprintType)
struct FPistolProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float BaseDamage = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 MaxAmmo = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float FireEnableTime = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float ReloadTime = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float EnergyToDecrease = 10.f;
};

UCLASS(BlueprintType)
class TESISUE_API UPistolData : public UWeaponData
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties")
    FPistolProperties PistolTypeStats;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties")
    FPistolMontages Montages;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties")
    FPistolEffects Effects;
};