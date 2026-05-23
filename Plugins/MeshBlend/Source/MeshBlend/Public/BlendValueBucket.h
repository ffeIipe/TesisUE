// Copyright 2024 Tore Lervik. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/ChunkedArray.h"
#include "BlendValueBucket.generated.h"

constexpr float GMeshBlend_GridCellSize = 20000.0f;
constexpr float GMeshBlend_GridCellSizeInverse = 1 / GMeshBlend_GridCellSize;
constexpr uint32 GMeshBlend_MeshBoundsChunks = 20;

USTRUCT()
struct MESHBLEND_API FGridCell
{
	GENERATED_BODY()

	TChunkedArray<FBox, GMeshBlend_MeshBoundsChunks * sizeof(FBox)> MeshBounds;

	FGridCell()
	{
	}
};

USTRUCT()
struct MESHBLEND_API FBlendValueBucket
{
	GENERATED_BODY()

	TChunkedArray<FBox, GMeshBlend_MeshBoundsChunks * sizeof(FBox)> MeshBounds;

	UPROPERTY()
	TMap<FIntPoint, FGridCell> GridCells;

	FBlendValueBucket()
	{
	}

	bool Intersect(const FBox& InBox)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

		for (const FBox& Box : MeshBounds)
		{
			if (Box.Intersect(InBox))
			{
				return true;
			}
		}

		for (const auto& [GridCoords, GridCell] : GridCells)
		{
			const FBox GridCellBounds = GetBoundsXY(GridCoords);

			if (GridCellBounds.IntersectXY(InBox))
			{
				for (const FBox& Box : GridCell.MeshBounds)
				{
					if (Box.Intersect(InBox))
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	void Add(const FBox& InBox)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

		const FIntPoint GridCoords = GetBoundsXYKey(InBox);
		const FBoxSphereBounds GridCellBounds = GetBoundsXY(GridCoords);

		/* Add the bounds to the "big array" if it doesn't fit inside a single grid cell.
		 * Avoids the need to add bounds to multiple cells by storing the oversize ones in a separate array. */
		if (!GridCellBounds.GetBox().IsInsideXY(InBox))
		{
			MeshBounds.AddElement(InBox);
			return;
		}

		if (GridCells.Contains(GridCoords))
		{
			FGridCell& ExistingCell = GridCells[GridCoords];
			ExistingCell.MeshBounds.AddElement(InBox);
		}
		else
		{
			FGridCell NewGridCell = FGridCell();
			NewGridCell.MeshBounds.AddElement(InBox);
			GridCells.Add(GridCoords, NewGridCell);
		}
	}

private:
	FIntPoint GetBoundsXYKey(const FBox& Box)
	{
		const FVector BoxCenter = Box.GetCenter();

		return FIntPoint(
			FMath::FloorToInt(BoxCenter.X * GMeshBlend_GridCellSizeInverse),
			FMath::FloorToInt(BoxCenter.Y * GMeshBlend_GridCellSizeInverse)
		);
	}

	FBox GetBoundsXY(const FIntPoint GridCoords)
	{
		FBoxSphereBounds GridCellBounds;
		GridCellBounds.Origin.X = GridCoords.X * GMeshBlend_GridCellSize + GMeshBlend_GridCellSize * 0.5f;
		GridCellBounds.Origin.Y = GridCoords.Y * GMeshBlend_GridCellSize + GMeshBlend_GridCellSize * 0.5f;
		GridCellBounds.Origin.Z = 0.0f;
		GridCellBounds.BoxExtent = FVector(GMeshBlend_GridCellSize * 0.5f);
		return GridCellBounds.GetBox();
	}
};
