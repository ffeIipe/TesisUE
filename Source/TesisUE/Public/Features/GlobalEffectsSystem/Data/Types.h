#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Types.generated.h"

USTRUCT()
struct FEffectProgressParameters
{
    GENERATED_BODY()

    UPROPERTY()
    float CurrentTime;

    UPROPERTY()
    float Duration;

    UPROPERTY()
    bool bIsActive;

	UPROPERTY()
	UCurveFloat* Curve;
	
    FEffectProgressParameters()
        : CurrentTime(0.0f)
        , Duration(0.0f)
        , bIsActive(false)
		, Curve(nullptr)
    {}

    void Start(const float InDuration)
    {
        Duration = FMath::Max(KINDA_SMALL_NUMBER, InDuration);
        CurrentTime = 0.0f;
        bIsActive = true;
    }

    void Stop()
    {
        bIsActive = false;
        CurrentTime = 0.0f;
    }
	
    float GetAlpha(const float DeltaTime)
    {
        if (!bIsActive)
        {
            return 0.0f;
        }

        CurrentTime += DeltaTime;
    	const float Alpha = FMath::Clamp(CurrentTime / Duration, 0.0f, 1.0f);
    	
        if (Alpha >= 1.0f)
        {
            Stop();
        }

    	if (Curve != nullptr)
    	{
    		const float CurvedAlpha = Curve->GetFloatValue(Alpha);
    		return CurvedAlpha;
    	}
    	
        return Alpha;
    }
};

USTRUCT()
struct FZoomEffectParameters : public FEffectProgressParameters
{
    GENERATED_BODY()

    UPROPERTY()
    float InitialFOV;

    UPROPERTY()
    float TargetFOV;

    FZoomEffectParameters()
        : InitialFOV(90.0f)
        , TargetFOV(0.0f)
    {}

    void StartZoom(UCurveFloat* InCurveAsset, const float InDuration, const float InInitialFOV)
    {
        Start(InDuration);
    	Curve = InCurveAsset;
        InitialFOV = InInitialFOV;
        // TargetFOV = InTargetFOV;
    }
};

USTRUCT()
struct FTimeWarpEffectParameters : public FEffectProgressParameters
{
	GENERATED_BODY()
	
	UPROPERTY()
	float MinTimeScaleValue; 

	UPROPERTY()
	float MaxTimeScaleValue; 

	FTimeWarpEffectParameters()
		: MinTimeScaleValue(0.0f)
		, MaxTimeScaleValue(1.0f)
	{}

	void StartWarp(UCurveFloat* InCurveAsset, const float InDuration)
	{
		Start(InDuration);
		
		if (InCurveAsset)
		{
			Curve = InCurveAsset;
		}
	}
};

USTRUCT()
struct FHitStopEffectParameters : public FEffectProgressParameters
{
	GENERATED_BODY()

	UPROPERTY()
	float MinTimeScaleValue;

	UPROPERTY()
	float MaxTimeScaleValue;

	FHitStopEffectParameters()
	: MinTimeScaleValue(0.f)
	, MaxTimeScaleValue(1.0f)
	{}

	void StartHitStop(const float InDuration, UCurveFloat* InCurveAsset)
	{
		Start(InDuration);
		
		if (InCurveAsset)
		{
			Curve = InCurveAsset;
		}
	}
};


USTRUCT(BlueprintType)
struct FCameraShakeType
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake")
	TSubclassOf<UCameraShakeBase> ShakeClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake", meta=(UIMin="0.0", ClampMin="0.0"))
	float InnerRadius;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake", meta=(UIMin="0.0", ClampMin="0.0"))
	float OuterRadius;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake", meta=(UIMin="0.0", ClampMin="0.0"))
	float Falloff;
	
	FCameraShakeType()
		: ShakeClass(nullptr)
		, InnerRadius(0.0f)
		, OuterRadius(500.0f)
		, Falloff(1.0f)
	{}
	
	FCameraShakeType(const TSubclassOf<UCameraShakeBase>& InShakeClass, const float InInnerRadius, const float InOuterRadius, const float InFalloff = 1.0f)
		: ShakeClass(InShakeClass)
		, InnerRadius(InInnerRadius)
		, OuterRadius(InOuterRadius)
		, Falloff(InFalloff)
	{}
};

USTRUCT(BlueprintType)
struct FEffectBaseType
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	UCurveFloat* Curve;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	float Duration;

	FEffectBaseType() : Curve(nullptr), Duration(0.0f) {}

	FEffectBaseType(UCurveFloat* InCurve, const float InDuration) : Curve(InCurve), Duration(InDuration) {}
};

USTRUCT(BlueprintType)
struct FCameraZoomType : public FEffectBaseType
{
	GENERATED_BODY()
	
	FCameraZoomType() : FEffectBaseType(nullptr, 0.0f) {}

	FCameraZoomType(UCurveFloat* CurveAsset, const float Duration) : FEffectBaseType(CurveAsset, Duration) {}
};

USTRUCT(BlueprintType)
struct FTimeWarpType : public FEffectBaseType
{
	GENERATED_BODY()

	FTimeWarpType() : FEffectBaseType(nullptr, 0.0f) {}

	FTimeWarpType(UCurveFloat* CurveAsset, const float Duration) : FEffectBaseType(CurveAsset, Duration) {}
};

USTRUCT(BlueprintType)
struct FHitStopType : public FEffectBaseType
{
	GENERATED_BODY()

	FHitStopType() : FEffectBaseType(nullptr, 0.0f) {}

	FHitStopType(UCurveFloat* InCurveAsset, const float Duration) : FEffectBaseType(InCurveAsset, Duration) {}
};