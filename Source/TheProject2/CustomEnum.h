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
