// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HitboxData.generated.h"

/**
 * 
 */
UCLASS()
class THEPROJECT2_API UHitboxData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	
	UPROPERTY(BlueprintReadOnly,EditAnywhere)
	FName StartSocketName;
	UPROPERTY(BlueprintReadOnly,EditAnywhere)
	FName EndSocketName;
	UPROPERTY(BlueprintReadOnly,EditAnywhere)
	TArray<TEnumAsByte<EObjectTypeQuery>> HitObjectType;
	UPROPERTY(BlueprintReadOnly,EditAnywhere)
	float HitboxRadius;
	
	
};
