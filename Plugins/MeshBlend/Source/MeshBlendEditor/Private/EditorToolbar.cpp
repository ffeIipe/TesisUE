// Copyright 2024 Tore Lervik. All Rights Reserved.

#if WITH_EDITOR

#include "EditorToolbar.h"
#include "CoreMinimal.h"
#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "MeshBlendActivator.h"
#include "MeshBlendEditor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Styling/SlateStyleRegistry.h"

#define LOCTEXT_NAMESPACE "FMeshBlendModule"

void FEditorToolbar::Startup(FToolMenuOwner Owner)
{
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(&FEditorToolbar::RegisterMenus));
}

void FEditorToolbar::Shutdown(const FToolMenuOwner Owner)
{
	UToolMenus::UnregisterOwner(Owner);
}

void FEditorToolbar::RegisterMenus()
{
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
	FToolMenuSection& Section = Menu->FindOrAddSection("MeshBlend");

	Section.AddEntry(FToolMenuEntry::InitComboButton(
		NAME_None,
		FUIAction(),
		FOnGetContent::CreateLambda([]()
		{
			FMenuBuilder MenuBuilder(true, TSharedPtr<FUICommandList>());

			MenuBuilder.AddMenuEntry(
				LOCTEXT("ToolbarButtonText", "Enable"),
				LOCTEXT("ToolbarButtonTip", "Run MeshBlend (r.MeshBlend.Enable 1)"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&AMeshBlendActivator::ToggleEnabled),
				          FCanExecuteAction(),
				          FIsActionChecked::CreateStatic(&AMeshBlendActivator::IsEnabled)),
				NAME_None,
				EUserInterfaceActionType::ToggleButton);

			// MenuBuilder.AddMenuEntry(
			// 	LOCTEXT("ToolbarButtonText", "Run in Editor"),
			// 	LOCTEXT("ToolbarButtonTip", "Run MeshBlend in Editor (r.MeshBlend.RunInEditor 1)"),
			// 	FSlateIcon(),
			// 	FUIAction(FExecuteAction::CreateStatic(&AMeshBlendActivator::ToggleRunInEditor),
			// 	          FCanExecuteAction(),
			// 	          FIsActionChecked::CreateStatic(&AMeshBlendActivator::IsRunInEditor)),
			// 	NAME_None,
			// 	EUserInterfaceActionType::ToggleButton);

			MenuBuilder.AddMenuEntry(
				LOCTEXT("ToolbarButtonText", "Debug View"),
				LOCTEXT("ToolbarButtonTip", "Show debug visualization (r.MeshBlend.PP.Visualize 1)"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&AMeshBlendActivator::ToggleDebugVisualization),
				          FCanExecuteAction(),
				          FIsActionChecked::CreateStatic(&AMeshBlendActivator::GetDebugVisualization)),
				NAME_None,
				EUserInterfaceActionType::ToggleButton);

			MenuBuilder.AddMenuEntry(
				LOCTEXT("ToolbarButtonText", "Refresh Actors"),
				LOCTEXT("ToolbarButtonTip", "Refresh blending for all actors in the level"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FMeshBlendEditorModule::TryResetMeshActivator),
				          FCanExecuteAction::CreateStatic(&AMeshBlendActivator::IsEnabled)),
				NAME_None,
				EUserInterfaceActionType::Button);

			MenuBuilder.AddMenuEntry(
				LOCTEXT("ToolbarButtonText", "Readme"),
				LOCTEXT("ToolbarButtonTip", "Show readme for MeshBlend"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&FEditorToolbar::OpenReadme)),
				NAME_None,
				EUserInterfaceActionType::Button);

			return MenuBuilder.MakeWidget();
		}),
		LOCTEXT("ToolbarButtonText", ""),
		LOCTEXT("ToolbarButtonTip", "MeshBlend"),
		FSlateIcon(TEXT("MeshBlendStyle"), "MeshBlend.Icon")));
}

void FEditorToolbar::OpenReadme()
{
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	const IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
	const FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(FSoftObjectPath(TEXT("/MeshBlend/Editor/Readme.Readme")));

	if (!AssetData.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("MeshBlend: Readme asset not found!"));
		return;
	}

	UObject* Asset = AssetData.GetAsset();

	if (UEditorUtilityWidgetBlueprint* EditorUtilityWidgetBlueprint = Cast<UEditorUtilityWidgetBlueprint>(Asset))
	{
		EditorUtilitySubsystem->SpawnAndRegisterTab(EditorUtilityWidgetBlueprint);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MeshBlend: Readme asset is not a valid UEditorUtilityWidgetBlueprint!"));
	}
}

#undef LOCTEXT_NAMESPACE

#endif
