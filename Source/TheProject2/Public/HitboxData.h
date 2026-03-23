// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HitboxData.generated.h"

UENUM(BlueprintType)
enum class EHitboxType : uint8
{
	WeaponSocketSweep UMETA(DisplayName = "Weapon Socket Sweep"),
	OwnerFollowShape UMETA(DisplayName = "Owner Follow Shape")
};

UENUM(BlueprintType)
enum class EOwnerFollowHitboxShape : uint8
{
	Sphere UMETA(DisplayName = "Sphere"),
	Box UMETA(DisplayName = "Box"),
	Cylinder UMETA(DisplayName = "Cylinder")
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
		meta = (EditCondition = "HitboxType == EHitboxType::OwnerFollowShape", EditConditionHides))
	EOwnerFollowHitboxShape OwnerFollowShape;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Hitbox",
		meta = (EditCondition = "HitboxType == EHitboxType::OwnerFollowShape", EditConditionHides))
	FVector FollowShapeOffset;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Hitbox",
		meta = (EditCondition = "HitboxType == EHitboxType::OwnerFollowShape && OwnerFollowShape == EOwnerFollowHitboxShape::Box", EditConditionHides))
	FVector BoxHalfExtent;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Hitbox", meta = (ClampMin = "0.0",
		EditCondition = "HitboxType == EHitboxType::OwnerFollowShape && OwnerFollowShape == EOwnerFollowHitboxShape::Cylinder", EditConditionHides))
	float CylinderRadius;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Hitbox", meta = (ClampMin = "0.0",
		EditCondition = "HitboxType == EHitboxType::OwnerFollowShape && OwnerFollowShape == EOwnerFollowHitboxShape::Cylinder", EditConditionHides))
	float CylinderHalfHeight;
};
