// Copyright 2024 Tore Lervik. All Rights Reserved.

#include "MeshBlendBPLibrary.h"

#include "MeshBlendActivator.h"
#include "MeshBlendShared.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/UObjectToken.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Package.h"

#if WITH_EDITOR
#include "Logging/MessageLog.h"
#endif

#define LOCTEXT_NAMESPACE "UMeshBlendBPLibrary"

#if WITH_EDITOR
bool MaterialHasMeshBlendActivator(const UMaterialInterface* Material)
{
	if (Material)
	{
		bool bIsChecked = false;
		FGuid ExpressionGuid;

		if (Material->GetStaticSwitchParameterValue(TEXT("Use Static Value"), bIsChecked, ExpressionGuid))
		{
			return true;
		}
	}

	return false;
}
#endif

void UMeshBlendBPLibrary::SetBlendUserDataOnMesh(UStaticMesh* Mesh, const EAutoBlendOption NewBlendOption)
{
	if (Mesh)
	{
#if WITH_EDITOR
		const TArray<FStaticMaterial>& StaticMaterials = Mesh->GetStaticMaterials();
		bool bHasMaterialWithMeshBlendActivator = false;

		for (const FStaticMaterial& StaticMaterial : StaticMaterials)
		{
			if (MaterialHasMeshBlendActivator(StaticMaterial.MaterialInterface))
			{
				bHasMaterialWithMeshBlendActivator = true;
			}
		}

		if (!bHasMaterialWithMeshBlendActivator)
		{
			FMessageLog("MapCheck")
				.Warning()
				->AddToken(FAssetNameToken::Create(Mesh->GetPackage()->GetPathName()))
				->AddToken(FTextToken::Create(LOCTEXT(
					"MapCheck_MeshBlendMaterialNotSetup",
					"The mesh does not have any materials set up with the MeshBlend activator. MeshBlend needs this to work.")))
				->AddToken(FURLToken::Create(
					"https://meshblend.lervik.com",
					LOCTEXT("MapCheck_MeshBlendDocumentation_UrlText", "Click to open documentation")));

			FMessageLog("MapCheck").Open(EMessageSeverity::Warning, true);
		}
#endif

		bool bHasChanged = false;

		if (UAssetUserData* ExistingData = Mesh->GetAssetUserDataOfClass(UAutoBlendUserData::StaticClass()))
		{
			if (UAutoBlendUserData* ExistingAutoBlendUserData = Cast<UAutoBlendUserData>(ExistingData))
			{
				if (ExistingAutoBlendUserData->AutoBlendOption != NewBlendOption)
				{
					ExistingAutoBlendUserData->AutoBlendOption = NewBlendOption;
					bHasChanged = true;
				}
			}
		}
		else
		{
			UAutoBlendUserData* NewAssetUserData = NewObject<UAutoBlendUserData>(Mesh, NAME_None, RF_Public | RF_Transactional);
			NewAssetUserData->AutoBlendOption = NewBlendOption;
			Mesh->AddAssetUserData(NewAssetUserData);
			bHasChanged = true;
		}

		if (bHasChanged)
		{
			Mesh->MarkPackageDirty();

#if WITH_EDITOR
			if (GWorld)
			{
				if (AMeshBlendActivator* Activator = AMeshBlendActivator::GetInstance(GWorld))
				{
					Activator->RefreshActorsReferencingAsset(Mesh);
				}
			}
#endif
		}
	}
}

void UMeshBlendBPLibrary::ClearBlendUserDataFromMesh(UStaticMesh* Mesh)
{
	if (Mesh)
	{
		if (Mesh->GetAssetUserDataOfClass(UAutoBlendUserData::StaticClass()))
		{
			Mesh->RemoveUserDataOfClass(UAutoBlendUserData::StaticClass());
			Mesh->MarkPackageDirty();

#if WITH_EDITOR

			if (GWorld)
			{
				if (AMeshBlendActivator* Activator = AMeshBlendActivator::GetInstance(GWorld))
				{
					Activator->RefreshActorsReferencingAsset(Mesh);
				}
			}

#endif
		}
	}
}

void UMeshBlendBPLibrary::SetBlendOptionOnActor(AActor* Actor, const EAutoBlendOption NewBlendOption)
{
	if (Actor)
	{
#if WITH_EDITOR

		bool bHasMaterialWithMeshBlendActivator = false;
		TArray<UMeshComponent*> Meshes;
		Actor->GetComponents<UMeshComponent>(Meshes, false);

		for (const UMeshComponent* MeshComponent : Meshes)
		{
			for (const UMaterialInterface* Material : MeshComponent->GetMaterials())
			{
				if (MaterialHasMeshBlendActivator(Material))
				{
					bHasMaterialWithMeshBlendActivator = true;
				}
			}
		}

		if (!bHasMaterialWithMeshBlendActivator)
		{
			FMessageLog("MapCheck")
				.Warning()
				->AddToken(FUObjectToken::Create(Actor))
				->AddToken(FTextToken::Create(LOCTEXT(
					"MapCheck_MeshBlendMaterialNotSetup",
					"The mesh does not have any materials set up with the MeshBlend activator. MeshBlend needs this to work.")))
				->AddToken(FURLToken::Create(
					"https://meshblend.lervik.com",
					LOCTEXT("MapCheck_MeshBlendDocumentation_UrlText", "Click to open documentation")));

			FMessageLog("MapCheck").Open(EMessageSeverity::Warning, true);
		}

#endif

		FName AutoBlendTag;

		switch (NewBlendOption)
		{
		case Small:
			AutoBlendTag = GName_AutoBlendSmall;
			break;
		case Medium:
			AutoBlendTag = GName_AutoBlendMedium;
			break;
		case Large:
			AutoBlendTag = GName_AutoBlendLarge;
			break;
		case Extra_Large:
			AutoBlendTag = GName_AutoBlendExtraLarge;
			break;
		case Disabled:
			AutoBlendTag = GName_Disabled;
			break;
		default:
			return;
		}

		TArray<FName> Tags = Actor->Tags;

		if (Tags.Contains(AutoBlendTag))
		{
			return;
		}

		const auto Property = Actor->GetClass()->FindPropertyByName("Tags");
		const auto TagsProperty = CastField<FArrayProperty>(Property);
		const auto ArrayPtr = TagsProperty->ContainerPtrToValuePtr<TArray<FName>>(Actor);
		FScriptArrayHelper ArrayHelper(TagsProperty, ArrayPtr);

		Tags.Remove(GName_AutoBlendSmall);
		Tags.Remove(GName_AutoBlendMedium);
		Tags.Remove(GName_AutoBlendLarge);
		Tags.Remove(GName_AutoBlendExtraLarge);
		Tags.Remove(GName_Disabled);
		Tags.Add(AutoBlendTag);

		ArrayHelper.Resize(Tags.Num());

		for (int32 i = 0; i < Tags.Num(); i++)
		{
			FName* NamePtr = reinterpret_cast<FName*>(ArrayHelper.GetRawPtr(i));
			*NamePtr = Tags[i];
		}

		Actor->MarkPackageDirty();

		if (AMeshBlendActivator* Activator = AMeshBlendActivator::GetInstance(Actor->GetWorld()))
		{
			Activator->RefreshActorBlending(Actor, false);
		}
	}
}

void UMeshBlendBPLibrary::DisableBlendOnActor(AActor* Actor)
{
	SetBlendOptionOnActor(Actor, Disabled);
}

void UMeshBlendBPLibrary::RefreshBlendOnActor(AActor* Actor, const bool bSoftReset)
{
	if (AMeshBlendActivator* Activator = AMeshBlendActivator::GetInstance(Actor->GetWorld()))
	{
		Activator->RefreshActorBlending(Actor, bSoftReset);
	}
}

void UMeshBlendBPLibrary::ClearBlendOptionFromActor(AActor* Actor)
{
	if (Actor)
	{
		TArray<FName> Tags = Actor->Tags;
		const auto Property = Actor->GetClass()->FindPropertyByName("Tags");
		const auto TagsProperty = CastField<FArrayProperty>(Property);
		const auto ArrayPtr = TagsProperty->ContainerPtrToValuePtr<TArray<FName>>(Actor);
		FScriptArrayHelper ArrayHelper(TagsProperty, ArrayPtr);

		Tags.Remove(GName_AutoBlendSmall);
		Tags.Remove(GName_AutoBlendMedium);
		Tags.Remove(GName_AutoBlendLarge);
		Tags.Remove(GName_AutoBlendExtraLarge);
		Tags.Remove(GName_Disabled);

		ArrayHelper.Resize(Tags.Num());

		for (int32 i = 0; i < Tags.Num(); i++)
		{
			FName* NamePtr = reinterpret_cast<FName*>(ArrayHelper.GetRawPtr(i));
			*NamePtr = Tags[i];
		}

		Actor->MarkPackageDirty();

		if (AMeshBlendActivator* Activator = AMeshBlendActivator::GetInstance(Actor->GetWorld()))
		{
			Activator->RefreshActorBlending(Actor, false);
		}
	}
}

#undef LOCTEXT_NAMESPACE
