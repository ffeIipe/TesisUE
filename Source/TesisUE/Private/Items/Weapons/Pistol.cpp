#include "Items/Weapons/Pistol.h"

#include "NiagaraFunctionLibrary.h"
#include "DataAssets/Items/Weapons/PistolData.h"
#include "Features/GlobalEffectsSystem/Interfaces/EffectManagerProvider.h"
#include "Kismet/GameplayStatics.h"

void APistol::AttachMeshToSocket(USceneComponent* InParent, FName SocketInName)
{
	Super::AttachMeshToSocket(InParent);
	
	FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
	ItemMesh->AttachToComponent(InParent, TransformRules, PistolData->CustomInSocketName);
}

APistol::APistol()
{
	OnFire.AddDynamic(this, &APistol::PlayEffects);
}

void APistol::BeginPlay()
{
	Super::BeginPlay();
	if (PistolData)
	{
		CurrentAmmo = PistolData->PistolTypeStats.MaxAmmo;
	}
	else
	{
		//if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, FColor::Red, "MISSING! Data asset is nullptr.");
	}
}

void APistol::PrimaryShoot()
{
	Fire();
}

void APistol::SecondaryShoot()
{
	
}

void APistol::PlayEffects()
{
	const FVector SocketLocation = ItemMesh->GetSocketLocation(FName("MuzzleFlashSocket"));
	const FRotator SocketRotation = ItemMesh->GetSocketRotation(FName("MuzzleFlashSocket"));

	if (PistolData)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			PistolData->Effects.MuzzleFlash,
			SocketLocation,
			SocketRotation,
			FVector(1)
		);
		
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			PistolData->Effects.ShootSound,
			SocketLocation,
			SocketRotation,
			1.f
		);
	}
}

void APistol::Fire()
{
	//if (!AttributeProvider->RequiresEnergy(PistolData->Stats.EnergyToDecrease)) return;

	if (CurrentAmmo >= 1 && !bIsReloading && bIsFireEnabled)
	{
		bIsFireEnabled = false;
		CurrentAmmo--;
		SetTimer(TimerHandle_BetweenShots, PistolData->PistolTypeStats.FireEnableTime, &APistol::EnableFire);

		if (OnFire.IsBound()) OnFire.Broadcast();

		//AnimatorProvider->Execute_PlayAnimMontage(GetOwner(), PistolData->Montages.PrimaryFireMontage, 1.f, "Default");
		
		FVector TraceStart;
		FRotator CameraRotation;
		//ControllerProvider->GetEntityController()->GetPlayerViewPoint(TraceStart, CameraRotation);

		const FVector TraceDirection = CameraRotation.Vector();
		const FVector TraceEnd = TraceStart + TraceDirection * 1000000.f;

		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(GetOwner());
		CollisionParams.bTraceComplex = true;
		CollisionParams.bTraceComplex = false;
		
		FHitResult HitResult;

		const bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			TraceStart,
			TraceEnd,
			ECC_Visibility,
			CollisionParams
		);
		
		if (!bHit) return;
		
		/*if (const TScriptInterface<IHitInterface> HitInterface = HitResult.GetActor())
		{
			UGameplayStatics::ApplyPointDamage(
			HitResult.GetActor(),
			PistolData->PistolTypeStats.BaseDamage,
			TraceDirection,
			HitResult,
			/*ControllerProvider->GetEntityController()#1# nullptr,
			GetOwner(),
			nullptr
			);

			const FDamageEvent DamageEvent(UDamageType::StaticClass());
			HitInterface->GetHit(GetOwner(), HitResult.ImpactPoint, DamageEvent, PistolData->PistolTypeStats.BaseDamage);
		}*/

		if (GetGameInstance()->Implements<UEffectManagerProvider>())
		{
			IEffectManagerProvider::Execute_PlayGameplayEffect(GetGameInstance(), CurrentDamageTag, HitResult.ImpactPoint);
		}
	}
	else if (CurrentAmmo <= 0)
		Reload();
}

void APistol::EnableFire()
{
	bIsFireEnabled = true;
}

void APistol::Reload()
{
	if (bIsReloading || CurrentAmmo == PistolData->PistolTypeStats.MaxAmmo) return;

	bIsReloading = true;
	
	//AnimatorProvider->Execute_PlayAnimMontage(GetOwner(), PistolData->Montages.ReloadMontage, 1.f, "Default");
	SetTimer(TimerHandle_Reload, 1.f, &APistol::FinishReload);
}

void APistol::FinishReload()
{
	CurrentAmmo = PistolData->PistolTypeStats.MaxAmmo;
	bIsReloading = false;
}

void APistol::SetTimer(FTimerHandle TimerHandle, const float Time, void (APistol::*InTimerMethod)())
{
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.ClearTimer(TimerHandle);
	TimerManager.SetTimer(TimerHandle, this, InTimerMethod, Time, false);
}
