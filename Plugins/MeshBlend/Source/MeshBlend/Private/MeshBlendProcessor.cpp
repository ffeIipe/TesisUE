// Copyright 2024 Tore Lervik. All Rights Reserved.


#include "MeshBlendProcessor.h"

#include "MeshBlendActivator.h"
#include "PrimitiveDataHelper.h"
#include "Components/InstancedStaticMeshComponent.h"

void UMeshBlendProcessor::Reset()
{
	BlendValueBucket.Reset();
}

uint8 UMeshBlendProcessor::GetAutoBlendID(const EAutoBlendOption MeshAutoBlendState)
{
	uint8 StencilValue = 0;

	if (MeshAutoBlendState == Small)
	{
		StencilValue = SmallBlendCounter;
		SmallBlendCounter += 2;
		SmallBlendCounter = (SmallBlendCounter - StencilStartSmall) % ClampedStencilRange + StencilStartSmall;
	}
	else if (MeshAutoBlendState == Medium)
	{
		StencilValue = MediumBlendCounter;
		MediumBlendCounter += 2;
		MediumBlendCounter = (MediumBlendCounter - StencilStartMedium) % ClampedStencilRange + StencilStartMedium;
	}
	else if (MeshAutoBlendState == Large)
	{
		StencilValue = LargeBlendCounter;
		LargeBlendCounter += 2;
		LargeBlendCounter = (LargeBlendCounter - StencilStartLarge) % ClampedStencilRange + StencilStartLarge;
	}
	else if (MeshAutoBlendState == Extra_Large)
	{
		StencilValue = ExtraLargeBlendCounter;
		ExtraLargeBlendCounter += 2;
		ExtraLargeBlendCounter = (ExtraLargeBlendCounter - StencilStartExtraLarge) % ClampedStencilRange + StencilStartExtraLarge;
	}

	return StencilValue;
}

uint8 UMeshBlendProcessor::GetReservedAutoBlendID(const EAutoBlendOption MeshAutoBlendState, const uint8 Offset)
{
	if (MeshAutoBlendState == Small)
	{
		return StencilStartSmall + ClampedStencilRange + 1 + Offset;
	}
	else if (MeshAutoBlendState == Medium)
	{
		return StencilStartMedium + ClampedStencilRange + 1 + Offset;
	}
	else if (MeshAutoBlendState == Large)
	{
		return StencilStartLarge + ClampedStencilRange + 1 + Offset;
	}
	else if (MeshAutoBlendState == Extra_Large)
	{
		return StencilStartExtraLarge + ClampedStencilRange + 1 + Offset;
	}

	return 0;
}

uint8 UMeshBlendProcessor::AssignAutoBlendId(const FBox& Box, const EAutoBlendOption MeshAutoBlendState)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

	// We try N times to find an available AutoBlendID that doesn't overlap with any other bounds
	const int32 MaxAttempts = IsCooking ? ClampedStencilRange : 20;

	for (int32 AttemptIndex = 0; AttemptIndex <= MaxAttempts; AttemptIndex++)
	{
		const uint8 AutoBlendID = GetAutoBlendID(MeshAutoBlendState);

		if (!BlendValueBucket.Contains(AutoBlendID))
		{
			BlendValueBucket.Add(AutoBlendID, FBlendValueBucket());
		}

		FBlendValueBucket& Bucket = BlendValueBucket[AutoBlendID];

		if (Bucket.Intersect(Box))
		{
			continue;
		}

		Bucket.Add(Box);
		return AutoBlendID;
	}

	// If we have exhausted all attempts we try to find a AutoBlendID in the reserved range.
	for (int32 ReservedRangeOffset = 0; ReservedRangeOffset < ReservedStencilRange; ReservedRangeOffset++)
	{
		const uint8 AutoBlendID = GetReservedAutoBlendID(MeshAutoBlendState, ReservedRangeOffset);

		if (!BlendValueBucket.Contains(AutoBlendID))
		{
			BlendValueBucket.Add(AutoBlendID, FBlendValueBucket());
		}

		FBlendValueBucket& Bucket = BlendValueBucket[AutoBlendID];

		if (Bucket.Intersect(Box))
		{
			continue;
		}

		Bucket.Add(Box);
		return AutoBlendID;
	}

	return GetAutoBlendID(MeshAutoBlendState);
}

bool UMeshBlendProcessor::ActivateActor(AActor* Actor, const double MaxProcessDuration)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
	TInlineComponentArray<UMeshComponent*> Meshes;
	Actor->GetComponents<UMeshComponent>(Meshes, false);

	EAutoBlendOption ActorAutoBlendState = Disabled;
	FUAutoBlendHelper::TryGetBlendOptionFromActorTag(Actor, ActorAutoBlendState);

	for (UMeshComponent* MeshComponent : Meshes)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("UMeshBlendProcessor::ActivateActor::StaticMeshComponent");

		if (FMeshBlendShared::IsOverProcessBudget(MaxProcessDuration))
		{
			return false;
		}

		if (MeshComponent->ComponentHasTag(GName_AutoBlendHasPrimitiveData))
		{
			continue;
		}

		EAutoBlendOption ComponentAutoBlendState = Disabled;
		if (!FUAutoBlendHelper::TryGetBlendOptionFromComponentTag(MeshComponent, ComponentAutoBlendState))
		{
			ComponentAutoBlendState = ActorAutoBlendState;
		}

		if (ComponentAutoBlendState == Disabled)
		{
			if (const UAutoBlendUserData* AutoBlendUserData = FMeshBlendShared::GetAutoBlendUserData(MeshComponent))
			{
				ComponentAutoBlendState = AutoBlendUserData->AutoBlendOption;
			}
		}

		if (ComponentAutoBlendState == Disabled)
		{
			continue;
		}

		const int32 CustomPrimitiveDataIndex = MeshComponent->GetCustomPrimitiveDataIndexForScalarParameter(GName_AutoBlendID);

		if (CustomPrimitiveDataIndex == INDEX_NONE)
		{
			continue;
		}

#if WITH_EDITORONLY_DATA
		bool bIsOnlyStaticBlends = true;

		for (const UMaterialInterface* Material : MeshComponent->GetMaterials())
		{
			if (Material)
			{
				bool bIsChecked = false;
				FGuid ExpressionGuid;

				if (!Material->GetStaticSwitchParameterValue(TEXT("Use Static Value"), bIsChecked, ExpressionGuid) || !bIsChecked)
				{
					bIsOnlyStaticBlends = false;
				}
			}
		}

		// If all materials are using static values, we skip processing this component
		if (bIsOnlyStaticBlends)
		{
			continue;
		}
#endif

		MeshComponent->ComponentTags.Add(GName_AutoBlendHasPrimitiveData);
		FPrimitiveDataHelper::EnsurePrimitiveDataSlot(MeshComponent, CustomPrimitiveDataIndex);
		const FBoxSphereBounds MeshAssetBounds = FMeshBlendShared::GetMeshBounds(MeshComponent);

		if (UInstancedStaticMeshComponent* InstancedStaticMeshComponent = Cast<UInstancedStaticMeshComponent>(MeshComponent))
		{
			// To avoid hitches we skip checking bounds for each instance if we have more than N instances
			if (!IsCooking && InstancedStaticMeshComponent->GetInstanceCount() > 1000)
			{
				const FBox Bounds = MeshAssetBounds.TransformBy(MeshComponent->GetComponentTransform()).GetBox();
				uint8 AutoBlendID = AssignAutoBlendId(Bounds, ComponentAutoBlendState);
				const float Value = static_cast<float>(AutoBlendID) / 255.0;

				for (int32 i = 0; i < InstancedStaticMeshComponent->GetInstanceCount(); i++)
				{
					FPrimitiveDataHelper::SetCustomDataValue(InstancedStaticMeshComponent, i, CustomPrimitiveDataIndex, Value);
				}
			}
			else
			{
				for (int32 i = 0; i < InstancedStaticMeshComponent->GetInstanceCount(); i++)
				{
					FTransform InstanceTransform;
					InstancedStaticMeshComponent->GetInstanceTransform(i, InstanceTransform, true);
					const FBox Bounds = MeshAssetBounds.TransformBy(InstanceTransform).GetBox();
					const float Value = static_cast<float>(AssignAutoBlendId(Bounds, ComponentAutoBlendState)) / 255.0;
					FPrimitiveDataHelper::SetCustomDataValue(InstancedStaticMeshComponent, i, CustomPrimitiveDataIndex, Value);
				}
			}

			InstancedStaticMeshComponent->MarkRenderStateDirty();
		}
		else
		{
			const FBox Bounds = MeshAssetBounds.TransformBy(MeshComponent->GetComponentTransform()).GetBox();
			uint8 AutoBlendID = AssignAutoBlendId(Bounds, ComponentAutoBlendState);

			if (IsCooking)
			{
				MeshComponent->SetDefaultCustomPrimitiveDataFloat(CustomPrimitiveDataIndex, static_cast<float>(AutoBlendID) / 255.0);
			}
			else
			{
				MeshComponent->SetCustomPrimitiveDataFloat(CustomPrimitiveDataIndex, static_cast<float>(AutoBlendID) / 255.0);
			}
		}
	}

	Actor->Tags.Add(GName_AutoBlendHasPrimitiveData);
	return true;
}
