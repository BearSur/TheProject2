// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "WeaponData.generated.h"


enum class EWeaponAnimationType : uint8;
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
	UStaticMesh* WeaponMesh;
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	TArray<USkillData*> LightAttackChain;
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	TArray<USkillData*> HeavyAttackChain;
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	EWeaponAnimationType WeaponAnimationType;
	
	//问题：如果使用前向声明FGameplayTag，会出问题，为什么
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	TMap<FGameplayTag,UAnimMontage*> TagsToAnimationMap;
	
};
