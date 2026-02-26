// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BaseAIController.generated.h"

class UBehaviorTreeComponent;
class UAISenseConfig_Sight;

UCLASS()
class THEPROJECT2_API ABaseAIController : public AAIController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABaseAIController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void OnPossess(APawn* InPawn) override;
	
	UPROPERTY(BlueprintReadOnly,VisibleAnywhere,Category = "AI")
	UBehaviorTreeComponent* BehaviorTreeComp;
	
	UPROPERTY(BlueprintReadOnly,VisibleAnywhere,Category = "AI")
	UBlackboardComponent* BlackboardComp;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "AI")
	UAIPerceptionComponent* PerceptionComp;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "AI")
	UAISenseConfig_Sight* SightConfig;
	
	UFUNCTION()
	void OnTargetDetected(AActor* Actor,FAIStimulus Stimulus);
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	
};

