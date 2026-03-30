// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BounceBackActor.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class THEPROJECT2_API ABounceBackActor : public AActor
{
	GENERATED_BODY()

public:
	ABounceBackActor();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DisplayMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* TriggerBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bounce")
	float HorizontalLaunchStrength = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bounce")
	float VerticalLaunchStrength = 250.0f;

	UFUNCTION()
	void HandleTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};
