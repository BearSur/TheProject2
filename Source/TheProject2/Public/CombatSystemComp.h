// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomEnum.h"
#include "Components/ActorComponent.h"
#include "RootMotionModifier.h"
#include "CombatSystemComp.generated.h"

class UHitboxData;
class ULockonPointComp;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent),Blueprintable)
class THEPROJECT2_API UCombatSystemComp : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCombatSystemComp();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly)
	ACharacter* OwnerCharacter;
	UPROPERTY(BlueprintReadOnly)
	AController* OwnerController;
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;
	/**
	 * 当被调用时，会进行一次检测然后找到最近的点
	 */
	UFUNCTION(BlueprintCallable)
	void TargetLockOn();
	UFUNCTION(BlueprintCallable)
	void QuitLockOn();


	/**
	 * 根据攻击打到的位置来计算出打到了角色的哪一个部位，应该由攻击发起者调用
	 * 有正面视角的上下左右（别人看你的方向的上下左右）和后面，可以根据这个来播放对应动画
	 * 计算完后会激活对应的GameplayTag，为：Character.HitState.Up/Down/Left/Right/Back...
	 * @param HitLocation 攻击打到角色身上的位置，绝对坐标
	 * @param TargetActor 被攻击的角色
	 */
	UFUNCTION(BlueprintCallable)
	void CalculateHitDirection(FVector HitLocation, AActor* TargetActor);

	UPROPERTY(EditAnywhere,blueprintReadWrite)
	ULockonPointComp* LockPoint;
	UPROPERTY(EditAnywhere,blueprintReadWrite)
	float Radius = 1000.0f;
	
	UFUNCTION(BlueprintCallable)
	void DoDirectionalRoll(FVector2D InputValue);

	UFUNCTION(BlueprintCallable)
	ERollAnimationType GetRollDirection(const FVector2D InputDirection);

	/**
	 * 根据MaxDistance计算出MotionWarping的Target，避免出现无限远吸附的情况
	 * @param MaxDistance 最大MotionWarping移动距离
	 * @param TargetLocation 目标的位置
	 * @param TargetRotation 目标的rotator
	 * @param WarpTargetName 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable,BlueprintPure)
	FMotionWarpingTarget CalculateMotionWarpingTarget(float MaxDistance, FVector TargetLocation, FRotator TargetRotation, FName WarpTargetName);

};
