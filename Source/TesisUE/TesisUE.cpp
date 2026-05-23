// Copyright Epic Games, Inc. All Rights Reserved.

#include "TesisUE.h"
#include "Modules/ModuleManager.h"

#include "GAS/VelmaraGameplayTags.h" 

class FTesisUEModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override
	{
		FVelmaraGameplayTags::InitializeGameplayTags();
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, TesisUE, "TesisUE" );
