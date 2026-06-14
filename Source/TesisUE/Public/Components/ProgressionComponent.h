#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "ProgressionComponent.generated.h"

// Delegados para notificar a la UI y al personaje (GAS)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrencyChangedSignature, int32, NewCurrencyAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboUnlockedSignature, FGameplayTag, UnlockedTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatUpgradedSignature, FGameplayTag, StatTag, int32, NewLevel);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TESISUE_API UProgressionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UProgressionComponent();

protected:
	virtual void BeginPlay() override;

	/* * DATOS GUARDADOS EN DISCO 
	 * Se utiliza el especificador SaveGame para facilitar la serialización.
	 */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression | State", SaveGame)
	int32 AvailableCurrency;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression | State", SaveGame)
	FGameplayTagContainer UnlockedCombos;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression | State", SaveGame)
	TMap<FGameplayTag, int32> StatUpgrades;

	/*
	 * CONFIGURACIÓN
	 * Para no hardcodear strings, definimos qué tags base identifican a un combo y a un stat.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Progression | Config")
	FGameplayTag BaseComboTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Progression | Config")
	FGameplayTag BaseStatTag;

public:

	// DELEGADOS (Para que la UI y el Character se suscriban)
	UPROPERTY(BlueprintAssignable, Category = "Progression | Events")
	FOnCurrencyChangedSignature OnCurrencyChanged;

	UPROPERTY(BlueprintAssignable, Category = "Progression | Events")
	FOnComboUnlockedSignature OnComboUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Progression | Events")
	FOnStatUpgradedSignature OnStatUpgraded;

	/* FUNCIONES PRINCIPALES */

	// Llamado por la UI para intentar realizar la compra
	UFUNCTION(BlueprintCallable, Category = "Progression")
	bool TryBuyUpgrade(FGameplayTag TagToGrant, int32 Cost);

	// Agrega o quita moneda y notifica
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void AddCurrency(int32 Amount);

	// Consultas (BlueprintPure porque no modifican estado)
	UFUNCTION(BlueprintPure, Category = "Progression | Queries")
	bool HasComboUnlocked(FGameplayTag ComboTag) const;

	UFUNCTION(BlueprintPure, Category = "Progression | Queries")
	int32 GetStatLevel(FGameplayTag StatTag) const;

	UFUNCTION(BlueprintPure, Category = "Progression | Queries")
	int32 GetAvailableCurrency() const { return AvailableCurrency; }
};