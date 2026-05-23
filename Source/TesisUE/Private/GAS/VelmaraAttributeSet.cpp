#include "GAS/VelmaraAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "Entities/Entity.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/Damageable.h"

UVelmaraAttributeSet::UVelmaraAttributeSet() { }

void UVelmaraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
}

void UVelmaraAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}

	if (Attribute == GetMovementSpeedAttribute())
	{
		if (const ACharacter* TargetCharacter = Cast<ACharacter>(GetOwningActor()))
		{
			if (UCharacterMovementComponent* CharMovComp = TargetCharacter->GetCharacterMovement())
			{
				CharMovComp->MaxWalkSpeed = NewValue;
			}
		}
	}
}

void UVelmaraAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);
    
    if (Data.EvaluatedData.Attribute == GetDamageIncomingAttribute())
    {
        const float LocalDamageDone = GetDamageIncoming();
        
        SetDamageIncoming(0.0f);

        if (LocalDamageDone > 0.0f)
        {
            const float CurrentShield = GetShield();
            const float DamageToShield = FMath::Min(CurrentShield, LocalDamageDone);

            if (DamageToShield > 0.0f)
            {
                SetShield(CurrentShield - DamageToShield);
            }

            const float DamageToHealth = LocalDamageDone - DamageToShield;
            if (DamageToHealth > 0.0f)
            {
                const float CurrentHealth = GetHealth();
                SetHealth(FMath::Clamp(CurrentHealth - DamageToHealth, 0.0f, GetMaxHealth()));
            }

            if (GetHealth() <= 0.0f)
            {
                FGameplayTagContainer DeathTagContainer;
                Data.EffectSpec.GetAllAssetTags(DeathTagContainer);

                FGameplayTag DeathReason = FGameplayTag::RequestGameplayTag(FName("Death.Default"));

                FGameplayEventData DeathPayload;
                DeathPayload.EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Death"));
                DeathPayload.Instigator = Data.EffectSpec.GetContext().GetInstigator();
                DeathPayload.Target = GetOwningActor();
                DeathPayload.InstigatorTags.AddTag(DeathReason);
           
                if (const TScriptInterface<IDamageable> Damageable = GetOwningActor())
                {
                    Damageable->Execute_MortalDamage(GetOwningActor(), DeathPayload);
                }
            }
            else
            {
                FGameplayEventData DamagePayload;
                DamagePayload.EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Hit"));
                DamagePayload.Instigator = Data.EffectSpec.GetContext().GetInstigator();
                DamagePayload.EventMagnitude = LocalDamageDone; 
                DamagePayload.Target = GetOwningActor();

                Data.EffectSpec.GetAllAssetTags(DamagePayload.InstigatorTags);
              
                if (const FHitResult* HitResult = Data.EffectSpec.GetContext().GetHitResult())
                {
                    DamagePayload.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(*HitResult);
                }

                if (const TScriptInterface<IDamageable> Damageable = GetOwningActor())
                {
                    Damageable->Execute_ReceiveDamage(GetOwningActor(), DamagePayload);
                }
            }
        }
    }
    else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
    	float CurrentMax = GetMaxHealth();
    	if (CurrentMax > 0.0f)
    	{
    		SetHealth(FMath::Clamp(GetHealth(), 0.0f, CurrentMax));
    	}
    }
    else if (Data.EvaluatedData.Attribute == GetShieldAttribute())
    {
    	float CurrentMax = GetMaxShield();
    	if (CurrentMax > 0.0f)
    	{
    		SetShield(FMath::Clamp(GetShield(), 0.0f, CurrentMax));
    	}
    }
    else if (Data.EvaluatedData.Attribute == GetEnergyAttribute())
    {
    	float CurrentMax = GetMaxEnergy();
    	if (CurrentMax > 0.0f)
    	{
    		SetEnergy(FMath::Clamp(GetEnergy(), 0.0f, CurrentMax));
    	}
    }
}

void UVelmaraAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UVelmaraAttributeSet, Health, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UVelmaraAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UVelmaraAttributeSet, Shield, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UVelmaraAttributeSet, MaxShield, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UVelmaraAttributeSet, Energy, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UVelmaraAttributeSet, MaxEnergy, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UVelmaraAttributeSet, MovementSpeed, COND_None, REPNOTIFY_Always)
}

void UVelmaraAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVelmaraAttributeSet, Health, OldHealth);
}

void UVelmaraAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVelmaraAttributeSet, MaxHealth, OldMaxHealth)
}

void UVelmaraAttributeSet::OnRep_Shield(const FGameplayAttributeData& OldShield)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVelmaraAttributeSet, Shield, OldShield)
}

void UVelmaraAttributeSet::OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVelmaraAttributeSet, MaxShield, OldMaxShield)
}

void UVelmaraAttributeSet::OnRep_Energy(const FGameplayAttributeData& OldEnergy)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVelmaraAttributeSet, Energy, OldEnergy)
}

void UVelmaraAttributeSet::OnRep_MaxEnergy(const FGameplayAttributeData& OldMaxEnergy)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVelmaraAttributeSet, MaxEnergy, OldMaxEnergy)
}

void UVelmaraAttributeSet::OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVelmaraAttributeSet, MovementSpeed, OldMovementSpeed)
}