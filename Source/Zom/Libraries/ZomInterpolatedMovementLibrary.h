// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ZomInterpolatedMovementLibrary.generated.h"


/**
 * Spring-style interpolation ("Sperp") toward a target position/value. Each call blends the current Speed
 * toward the value needed to close the gap (scaled by Force) using Stiffness as the Lerp alpha, then advances
 * Position by the resulting Speed. Ported from a C# InterpolatedMovement utility (float/Vector2/Vector3/Vector4).
 */
UCLASS()
class ZOM_API UZomInterpolatedMovementLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Zom|Math|InterpolatedMovement")
	static float Sperp1D(float Position, float Target, float Stiffness, float Force, UPARAM(ref) float& Speed);

	UFUNCTION(BlueprintCallable, Category = "Zom|Math|InterpolatedMovement")
	static FVector2D Sperp2D(FVector2D Position, FVector2D Target, float Stiffness, float Force, UPARAM(ref) FVector2D& Speed);

	UFUNCTION(BlueprintCallable, Category = "Zom|Math|InterpolatedMovement")
	static FVector Sperp3D(FVector Position, FVector Target, float Stiffness, float Force, UPARAM(ref) FVector& Speed);

	UFUNCTION(BlueprintCallable, Category = "Zom|Math|InterpolatedMovement")
	static FVector4 Sperp4D(FVector4 Position, FVector4 Target, float Stiffness, float Force, UPARAM(ref) FVector4& Speed);
};
