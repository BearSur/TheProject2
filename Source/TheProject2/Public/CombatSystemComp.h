// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatSystemComp.generated.h"


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
	
	UFUNCTION(BlueprintCallable)
	void CalculateHitDirection(FVector HitLocation, AActor* TargetActor);

	UPROPERTY(EditAnywhere,blueprintReadWrite)
	ULockonPointComp* LockPoint;
	UPROPERTY(EditAnywhere,blueprintReadWrite)
	float Radius = 1000.0f;
	
	
};
