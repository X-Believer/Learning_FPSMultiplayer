#pragma once

#include "ShooterTypes.Generated.h"

UENUM(BlueprintType)
enum class ETurnInPlace : uint8
{
	Left UMETA(DisplayName = "TurningLeft"),
	Right UMETA(DisplayName = "TurningRight"),
	NotTurning UMETA(DisplayName = "NotTurning")
};


USTRUCT(BlueprintType)
struct FReticleParams
{
	GENERATED_BODY()
	
	// Shape Cut Factor
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShapeCutFactor_RoundFired = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShapeCutFactor_Aiming = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShapeCutFactor_NotAiming = 0.f;
	
	// Corner Scale Factor
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScaleFactor_RoundFired = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScaleFactor_Aiming = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScaleFactor_NotAiming = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScaleFactor_TargetingPlayer = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScaleFactor_NotTargetingPlayer = 0.f;
	
	// Interp Speed
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RoundFiredInterpSpeed = 20.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AimingInterpSpeed = 15.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetingPlayerInterpSpeed = 10.f;
};
