// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponData.generated.h"

class USkillData;
/**
 * 
 */
UCLASS()
class THEPROJECT2_API UWeaponData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	UStaticMeshComponent* WeaponMesh;
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	TArray<USkillData*> LightAttackChain;
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	TArray<USkillData*> HeavyAttackChain;
	
	
};
