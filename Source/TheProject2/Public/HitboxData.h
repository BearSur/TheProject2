// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HitboxData.generated.h"

UENUM(BlueprintType)
enum class EHitboxType : uint8
{
	WeaponSocketSweep UMETA(DisplayName = "Weapon Socket Sweep"),
	OwnerFollowSphere UMETA(DisplayName = "Owner Follow Sphere")
};

/**
 * 
 */
UCLASS()
class THEPROJECT2_API UHitboxData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UHitboxData();
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Hitbox")
	EHitboxType HitboxType;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Hitbox",
		meta = (EditCondition = "HitboxType == EHitboxType::WeaponSocketSweep", EditConditionHides))
	FName StartSocketName;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Hitbox",
		meta = (EditCondition = "HitboxType == EHitboxType::WeaponSocketSweep", EditConditionHides))
	FName EndSocketName;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Hitbox")
	TArray<TEnumAsByte<EObjectTypeQuery>> HitObjectType;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Hitbox", meta = (ClampMin = "0.0"))
	float HitboxRadius;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Hitbox",
		meta = (EditCondition = "HitboxType == EHitboxType::OwnerFollowSphere", EditConditionHides))
	FVector FollowSphereOffset;
};
