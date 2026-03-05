// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatSystem.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UCombatSystem : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class THEPROJECT2_API ICombatSystem
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Interaction")
	void GetCombatSystemComp();

	// 情景 B：C++ 默认实现 + 蓝图可重写 (蓝图如果不重写，就执行 C++ 的默认逻辑)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	UStaticMeshComponent* GetWeaponMesh();
};
