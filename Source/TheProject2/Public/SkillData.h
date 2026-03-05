// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SkillData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class THEPROJECT2_API USkillData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	UAnimMontage* SkillAnim;

	/**
	 * 使用列表是因为有时候一些攻击会有多段伤害，这样可以方便处理，
	 * 该攻击的第n段伤害会读取第n个元素，多了不读取
	 * 少了报错并默认为0
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	TArray<float> Damage{50};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	TArray<float> Poise{210};
	
	/**
	 * 用于设置在开启MotionWarping时最大的吸附距离，避免无限距离吸附，过于出戏，小于
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	bool ShouldDoMotionWarping=true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	float MaxMotionWarpingDistance=200.0f;
	
};
