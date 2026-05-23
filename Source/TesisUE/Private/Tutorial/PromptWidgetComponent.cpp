#include "Tutorial/PromptWidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Tutorial/InputPromptWidget.h"

UPromptWidgetComponent::UPromptWidgetComponent()
{
    bUseGamepadIcon = false;

    static ConstructorHelpers::FObjectFinder<UDataTable> InputDataTable(TEXT("/Game/Blueprints/DataTables/DT_InputPrompts.DT_InputPrompts"));
    if (InputDataTable.Succeeded())
    {  
        PromptDataTable = InputDataTable.Object;
    }

    /*static ConstructorHelpers::FObjectFinder<UInputPromptWidget> InputPromptWidgetClass(TEXT("/Game/Blueprints/Tutorial/WBP_InputPrompt.WBP_InputPrompt"));
    if (InputPromptWidgetClass.Succeeded())
    {  
        SetWidgetClass(InputPromptWidgetClass.Object->GetClass());
    }*/
    /*else if (GEngine) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, FColor::Red, "Missing! Input Prompt Widget");*/
    
    SetDrawAtDesiredSize(true);
    SetWidgetSpace(EWidgetSpace::Screen);
}

void UPromptWidgetComponent::BeginPlay()
{
    Super::BeginPlay();
    LoadAndApplyPrompt();
}

void UPromptWidgetComponent::LoadAndApplyPrompt()
{
    if (!PromptDataTable || PromptRowName.IsNone()) return;

    const FInputPromptData* PromptRow = PromptDataTable->FindRow<FInputPromptData>(PromptRowName, TEXT("Prompt Load"));

    if (!PromptRow) return;

    if (!GetWidget())
    {
        UUserWidget* NewWidget = CreateWidget<UUserWidget>(GetWorld(), GetWidgetClass());
        if (NewWidget)
        {
            SetWidget(NewWidget);
        }
    }

    if (UInputPromptWidget* PromptWidget = Cast<UInputPromptWidget>(GetWidget()))
    {
        PromptWidget->InitializePrompt(*PromptRow, bUseGamepadIcon);
    }
}

void UPromptWidgetComponent::EnablePromptWidget()
{
    if (GetWidget())
    {
        GetWidget()->SetVisibility(ESlateVisibility::Visible);
    }
    SetHiddenInGame(false);
    SetVisibility(true);

    /*GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, FColor::Green, "Prompt Widget Enabled");*/
}

void UPromptWidgetComponent::DisablePromptWidget()
{
    if (GetWidget())
    {
        GetWidget()->SetVisibility(ESlateVisibility::Hidden);
    }
    SetHiddenInGame(true);
    SetVisibility(false);

    /*GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, FColor::Red, "Prompt Widget Disabled");*/
}
