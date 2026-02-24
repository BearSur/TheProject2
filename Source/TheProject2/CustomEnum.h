// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Class.h"
#include "CustomEnum.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EWeaponAnimationType : uint8
{
	Sword UMETA(DisplayName = "Sword"),
	LongSword UMETA(DisplayName = "Long Sword"),

};
UENUM(BlueprintType)
enum class ERollAnimationType: uint8
{
	Front UMETA(DisplayName = "Front"),
	Back UMETA(DisplayName = "Back"),
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right"),
	FrontLeft UMETA(DisplayName = "Front Left"),
	FrontRight UMETA(DisplayName = "Front Right"),
	BackLeft UMETA(DisplayName = "Back Left"),
	BackRight UMETA(DisplayName = "Back Right"),
	Unlocking UMETA(DisplayName = "Unlocking"),
	Default UMETA(DisplayName = "Default"),
};
