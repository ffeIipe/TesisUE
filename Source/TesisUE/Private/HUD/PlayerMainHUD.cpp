// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/PlayerMainHUD.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "HUD/PlayerMainWidget.h"

void APlayerMainHUD::BeginPlay()
{
    Super::BeginPlay();

    /*PlayerMainWidgetInstance = CreateWidget<UPlayerMainWidget>(UGameplayStatics::GetPlayerController(GetWorld(), 0), PlayerMainWidgetClass);

    if (PlayerMainWidgetInstance)
    {
        PlayerMainWidgetInstance->AddToViewport();
    }*/
}

void APlayerMainHUD::TogglePlayerUI(bool Bool)
{
    if (Bool)
    {
        if (!PlayerMainWidgetInstance)
        {
            PlayerMainWidgetInstance = CreateWidget<UPlayerMainWidget>(UGameplayStatics::GetPlayerController(GetWorld(), 0), PlayerMainWidgetClass);
            PlayerMainWidgetInstance->AddToViewport();
        }
        else
        {
            PlayerMainWidgetInstance->AddToViewport();
        }
    }
    else
    {
        if (PlayerMainWidgetInstance)
        {
            PlayerMainWidgetInstance->RemoveFromParent();
            PlayerMainWidgetInstance = nullptr;
        }
    }
}
