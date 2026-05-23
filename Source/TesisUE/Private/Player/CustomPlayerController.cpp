#include "Player/CustomPlayerController.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GenericTeamAgentInterface.h"
#include "Blueprint/UserWidget.h"
#include "DataAssets/InputData.h"
#include "GAS/VelmaraAbilityInputID.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundMix.h"

#include "HUD/PlayerMainHUD.h"


ACustomPlayerController::ACustomPlayerController()
{
    CurrentPauseMenuInstance = nullptr;
    CurrentOptionsMenuInstance_Pause = nullptr;
    bIsGamePausedExplicitly = false;

    PauseSoundMix = nullptr;
    PlayerMainHUD = nullptr;
    OriginalBody = nullptr;
    
    TeamId = FGenericTeamId(2);
}

void ACustomPlayerController::PerformPossession(APawn* NewPawn)
{
    if (!NewPawn || NewPawn == GetPawn()) return;

    if (!OriginalBody)
    {
        OriginalBody = GetPawn();
    }

    UnPossess();

    Possess(NewPawn);

    if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(NewPawn))
    {
        if (UAbilitySystemComponent* NewASC = ASI->GetAbilitySystemComponent())
        {
            NewASC->InitAbilityActorInfo(this, NewPawn);
        }
    }
}

bool ACustomPlayerController::IsPossessing() const
{
    return OriginalBody != GetPawn();
}

void ACustomPlayerController::BeginPlay()
{
    Super::BeginPlay();

    OriginalBody = GetPawn();
    
    PlayerMainHUD = Cast<APlayerMainHUD>(GetHUD());

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (DefaultMappingContext)
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}

void ACustomPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (InputComponent)
    {
        InputComponent->BindAction("PauseGame", IE_Pressed, this, &ACustomPlayerController::TogglePauseMenu);
    }

    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
        /*if (InputData->Inputs.MoveAction)
        {
            EnhancedInputComponent->BindAction(InputData->Inputs.MoveAction, ETriggerEvent::Triggered, this, &ACustomPlayerController::Input_Move);
            EnhancedInputComponent->BindAction(InputData->Inputs.MoveAction, ETriggerEvent::Completed, this, &ACustomPlayerController::Input_Move);
        }
		
        if (InputData->Inputs.LookAction)
            EnhancedInputComponent->BindAction(InputData->Inputs.LookAction, ETriggerEvent::Triggered, this, &ACustomPlayerController::Input_Look);*/

        if (InputData->Inputs.CrouchAction)
        {
            EnhancedInputComponent->BindAction(InputData->Inputs.CrouchAction, ETriggerEvent::Started, this, &ACustomPlayerController::AbilityInputTagPressed, EVelmaraAbilityInputID::Crouch);
            EnhancedInputComponent->BindAction(InputData->Inputs.CrouchAction, ETriggerEvent::Completed, this, &ACustomPlayerController::AbilityInputTagReleased, EVelmaraAbilityInputID::Crouch);
        }
        
        if (InputData->Inputs.JumpAction)
        {
            EnhancedInputComponent->BindAction(InputData->Inputs.JumpAction, ETriggerEvent::Started, this, &ACustomPlayerController::AbilityInputTagPressed, EVelmaraAbilityInputID::Jump);
            EnhancedInputComponent->BindAction(InputData->Inputs.JumpAction, ETriggerEvent::Completed, this, &ACustomPlayerController::AbilityInputTagReleased, EVelmaraAbilityInputID::Jump);
        }
        
        if (InputData->Inputs.DodgeAction)
        {
            EnhancedInputComponent->BindAction(InputData->Inputs.DodgeAction, ETriggerEvent::Started, this, &ACustomPlayerController::AbilityInputTagPressed, EVelmaraAbilityInputID::Dodge);
            EnhancedInputComponent->BindAction(InputData->Inputs.DodgeAction, ETriggerEvent::Completed, this, &ACustomPlayerController::AbilityInputTagReleased, EVelmaraAbilityInputID::Dodge);
        }
        
        if (InputData->Inputs.InteractAction)
        {
            EnhancedInputComponent->BindAction(InputData->Inputs.InteractAction, ETriggerEvent::Started, this, &ACustomPlayerController::AbilityInputTagPressed, EVelmaraAbilityInputID::Interact);
            EnhancedInputComponent->BindAction(InputData->Inputs.InteractAction, ETriggerEvent::Completed, this, &ACustomPlayerController::AbilityInputTagReleased, EVelmaraAbilityInputID::Interact);
        }

        if (InputData->Inputs.InventoryAction)
        {
            EnhancedInputComponent->BindAction(InputData->Inputs.InventoryAction, ETriggerEvent::Started, this, &ACustomPlayerController::AbilityInputTagPressed, EVelmaraAbilityInputID::Interact);
            EnhancedInputComponent->BindAction(InputData->Inputs.InventoryAction, ETriggerEvent::Completed, this, &ACustomPlayerController::AbilityInputTagReleased, EVelmaraAbilityInputID::Interact);
        }

        if (InputData->Inputs.ToggleWeaponAction)
        {
            EnhancedInputComponent->BindAction(InputData->Inputs.ToggleWeaponAction, ETriggerEvent::Started, this, &ACustomPlayerController::AbilityInputTagPressed, EVelmaraAbilityInputID::Interact);
            EnhancedInputComponent->BindAction(InputData->Inputs.ToggleWeaponAction, ETriggerEvent::Completed, this, &ACustomPlayerController::AbilityInputTagReleased, EVelmaraAbilityInputID::Interact);
        }
        
        if (InputData->Inputs.AttackAction)
        {
            EnhancedInputComponent->BindAction(InputData->Inputs.AttackAction, ETriggerEvent::Started, this, &ACustomPlayerController::AbilityInputTagPressed, EVelmaraAbilityInputID::Attack);
            EnhancedInputComponent->BindAction(InputData->Inputs.AttackAction, ETriggerEvent::Completed, this, &ACustomPlayerController::AbilityInputTagReleased, EVelmaraAbilityInputID::Attack);
        }

        if (InputData->Inputs.InputAction_Skill1)
        {
            EnhancedInputComponent->BindAction(InputData->Inputs.InputAction_Skill1, ETriggerEvent::Started, this, &ACustomPlayerController::AbilityInputTagPressed, EVelmaraAbilityInputID::Skill1);
            EnhancedInputComponent->BindAction(InputData->Inputs.InputAction_Skill1, ETriggerEvent::Completed, this, &ACustomPlayerController::AbilityInputTagReleased, EVelmaraAbilityInputID::Skill1);
        }

        if (InputData->Inputs.InputAction_Skill2)
        {
            EnhancedInputComponent->BindAction(InputData->Inputs.InputAction_Skill2, ETriggerEvent::Started, this, &ACustomPlayerController::AbilityInputTagPressed, EVelmaraAbilityInputID::Skill2);
            EnhancedInputComponent->BindAction(InputData->Inputs.InputAction_Skill2, ETriggerEvent::Completed, this, &ACustomPlayerController::AbilityInputTagReleased, EVelmaraAbilityInputID::Skill2);
        }

        if (InputData->Inputs.InputAction_Skill3)
        {
            EnhancedInputComponent->BindAction(InputData->Inputs.InputAction_Skill3, ETriggerEvent::Started, this, &ACustomPlayerController::AbilityInputTagPressed, EVelmaraAbilityInputID::Skill3);
            EnhancedInputComponent->BindAction(InputData->Inputs.InputAction_Skill3, ETriggerEvent::Completed, this, &ACustomPlayerController::AbilityInputTagReleased, EVelmaraAbilityInputID::Skill3);
        }

        if (InputData->Inputs.InputAction_Skill4)
        {
            EnhancedInputComponent->BindAction(InputData->Inputs.InputAction_Skill4, ETriggerEvent::Started, this, &ACustomPlayerController::AbilityInputTagPressed, EVelmaraAbilityInputID::Skill4);
            EnhancedInputComponent->BindAction(InputData->Inputs.InputAction_Skill4, ETriggerEvent::Completed, this, &ACustomPlayerController::AbilityInputTagReleased, EVelmaraAbilityInputID::Skill4);
        }
    }
}

void ACustomPlayerController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
    if (TeamId != NewTeamID)
    {
        TeamId = NewTeamID;
    }
}

FGenericTeamId ACustomPlayerController::GetGenericTeamId() const
{
    return TeamId;
}

ETeamAttitude::Type ACustomPlayerController::GetTeamAttitudeTowards(const AActor& Other) const
{
    const IGenericTeamAgentInterface* OtherTeamAgent = Cast<const IGenericTeamAgentInterface>(&Other);
    if (!OtherTeamAgent) return ETeamAttitude::Neutral;

    const FGenericTeamId OtherTeamId = OtherTeamAgent->GetGenericTeamId();

    if (OtherTeamId == 255) return ETeamAttitude::Neutral;
    if (OtherTeamId == TeamId) return ETeamAttitude::Friendly;
    
    return ETeamAttitude::Hostile;
}

void ACustomPlayerController::AbilityInputTagPressed(EVelmaraAbilityInputID InputId)
{
    if (APawn* ControlledPawn = GetPawn())
    {
        if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(ControlledPawn))
        {
            if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
            {
                if (ASC)
                {
                    ASC->AbilityLocalInputPressed(static_cast<int32>(InputId));
                }
            }
        }
    }
}

void ACustomPlayerController::AbilityInputTagReleased(EVelmaraAbilityInputID InputId)
{
    if (APawn* ControlledPawn = GetPawn())
    {
        if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(ControlledPawn))
        {
            if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
            {
                if (ASC)
                {
                    ASC->AbilityLocalInputReleased(static_cast<int32>(InputId));
                }
            }
        }
    }
}

/*void ACustomPlayerController::Input_Move(const FInputActionValue& Value)
{
    if (APawn* ControlledPawn = GetPawn())
    {
        if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(ControlledPawn))
        {
            if (ASI->GetAbilitySystemComponent())
            {
                const FVector2D FlatVector = Value.Get<FVector2D>();
	
                const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);

                const FVector DirectionForward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
                const FVector DirectionSideward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

                ControlledPawn->AddMovementInput(DirectionForward, FlatVector.Y);
                ControlledPawn->AddMovementInput(DirectionSideward, FlatVector.X);

                if (Value.Get<bool>())
                {
                    InputMovingVector = FlatVector;
                }
                else
                {
                    InputMovingVector = FVector2D::ZeroVector;
                }
            }
        }
    }
}

void ACustomPlayerController::Input_Look(const FInputActionValue& Value)
{
    const FVector2D LookAxisVector = Value.Get<FVector2D>();
	
    AddYawInput(LookAxisVector.X);
    AddPitchInput(LookAxisVector.Y);
}*/

void ACustomPlayerController::TogglePauseMenu()
{
    if (bIsGamePausedExplicitly)
    {
        if (CurrentOptionsMenuInstance_Pause && CurrentOptionsMenuInstance_Pause->IsInViewport())
        {
            CurrentOptionsMenuInstance_Pause->RemoveFromParent();
            CurrentOptionsMenuInstance_Pause = nullptr;

            if (CurrentPauseMenuInstance)
            {
                CurrentPauseMenuInstance->SetFocus();
            }
            return;
        }

        if (CurrentPauseMenuInstance)
        {
            CurrentPauseMenuInstance->RemoveFromParent();
            CurrentPauseMenuInstance = nullptr;
        }

        SetShowMouseCursor(false);
        SetInputMode(FInputModeGameOnly());
        UGameplayStatics::SetGamePaused(GetWorld(), false);
        bIsGamePausedExplicitly = false;

        if (PauseSoundMix)
        {
            UGameplayStatics::PopSoundMixModifier(GetWorld(), PauseSoundMix);
        }
        UE_LOG(LogTemp, Log, TEXT("Game Unpaused"));
    }
    else
    {
        if (!PauseMenuWidgetClass)
        {
            UE_LOG(LogTemp, Error, TEXT("PauseMenuWidgetClass not set in PlayerController!"));
            return;
        }

        if (CurrentPauseMenuInstance && CurrentPauseMenuInstance->IsInViewport())
        {
            return;
        }

        CurrentPauseMenuInstance = CreateWidget<UUserWidget>(this, PauseMenuWidgetClass);
        if (CurrentPauseMenuInstance)
        {
            CurrentPauseMenuInstance->AddToViewport();
            SetShowMouseCursor(true);

            FInputModeGameAndUI InputModeData;
            InputModeData.SetWidgetToFocus(CurrentPauseMenuInstance->TakeWidget()); // Enfocar el men� de pausa
            InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            SetInputMode(InputModeData);

            UGameplayStatics::SetGamePaused(GetWorld(), true);
            bIsGamePausedExplicitly = true;

            if (PauseSoundMix)
            {
                UGameplayStatics::PushSoundMixModifier(GetWorld(), PauseSoundMix);
            }
            UE_LOG(LogTemp, Log, TEXT("Game Paused"));
        }
    }
}
