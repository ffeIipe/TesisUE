#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SettingsSaveGame.generated.h"

UCLASS()
class TESISUE_API USettingsSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    USettingsSaveGame();

    UPROPERTY(VisibleAnywhere, Category = "Settings|Graphics")
    FIntPoint ScreenResolution;

    UPROPERTY(VisibleAnywhere, Category = "Settings|Graphics")
    int32 TextureQuality;

    UPROPERTY(VisibleAnywhere, Category = "Settings|Graphics")
    int32 ShadowQuality;

    UPROPERTY(VisibleAnywhere, Category = "Settings|Graphics")
    int AntiAliasingQuality;

    UPROPERTY(VisibleAnywhere, Category = "Settings|Graphics")
    int ViewDistanceQuality;

    UPROPERTY(VisibleAnywhere, Category = "Settings|Graphics")
    int PostProcessingQuality;

    UPROPERTY(VisibleAnywhere, Category = "Settings|Graphics")
    int VisualEffectQuality;

    UPROPERTY(VisibleAnywhere, Category = "Settings|Graphics")
    int GlobalIlluminationQuality;

    UPROPERTY(VisibleAnywhere, Category = "Settings|Graphics")
    int ShadingQuality;

    UPROPERTY(VisibleAnywhere, Category = "Settings|Graphics")
    float FrameRateLimit = 120;
};
