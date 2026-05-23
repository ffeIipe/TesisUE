// Copyright 2024 Tore Lervik. All Rights Reserved.

#pragma once
#include "Misc/EngineVersionComparison.h"

template <typename ElementType>
struct TProcessingQueue
{
	void Empty()
	{
		Queue.Reset();
		ResetIndexToStart();
	}

	void Reserve(const int Size)
	{
		Queue.Reserve(Size);
	}

	void Add(ElementType Item)
	{
		Queue.Add(Item);
	}

	void AddUnique(ElementType Item)
	{
		Queue.AddUnique(Item);
	}

	bool Next(ElementType& Item, const int Advance = 1)
	{
		if (Index < Queue.Num())
		{
			Item = Queue[Index];
			CurrentIndex = Index;
			Index += Advance;
			return true;
		}

		Index = 0;
		return false;
	}

	void ResetIndexToStart()
	{
		Index = 0;
		CurrentIndex = 0;
	}

	void KeepCurrentAsNext()
	{
		Index = CurrentIndex;
	}

	void RemoveCurrent()
	{
		KeepCurrentAsNext();

#if UE_VERSION_OLDER_THAN(5, 5, 0)
		Queue.RemoveAtSwap(Index, 1, false);
#else
		Queue.RemoveAtSwap(Index, 1, EAllowShrinking::No);
#endif
	}

	int Num()
	{
		return Queue.Num();
	}

private:
	TArray<ElementType> Queue = TArray<ElementType>();
	int Index = 0;
	int CurrentIndex = 0;
};
