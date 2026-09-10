// Fill out your copyright notice in the Description page of Project Settings.

#include "Zom/Libraries/ZomInterpolatedMovementLibrary.h"

float UZomInterpolatedMovementLibrary::Sperp1D(float Position, float Target, float Stiffness, float Force, float& Speed)
{
	Speed = FMath::Lerp(Speed, (Target - Position) * Force / 100.f, Stiffness / 100.f);
	return Position + Speed;
}

FVector2D UZomInterpolatedMovementLibrary::Sperp2D(FVector2D Position, FVector2D Target, float Stiffness, float Force, FVector2D& Speed)
{
	Speed = FMath::Lerp(Speed, (Target - Position) * Force / 100.f, Stiffness / 100.f);
	return Position + Speed;
}

FVector UZomInterpolatedMovementLibrary::Sperp3D(FVector Position, FVector Target, float Stiffness, float Force, FVector& Speed)
{
	Speed = FMath::Lerp(Speed, (Target - Position) * Force / 100.f, Stiffness / 100.f);
	return Position + Speed;
}

FVector4 UZomInterpolatedMovementLibrary::Sperp4D(FVector4 Position, FVector4 Target, float Stiffness, float Force, FVector4& Speed)
{
	Speed = FMath::Lerp(Speed, (Target - Position) * Force / 100.f, Stiffness / 100.f);
	return Position + Speed;
}
