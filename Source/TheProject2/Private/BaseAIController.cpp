// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/BaseAIController.h"

#include "BaseAICharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISenseConfig_Sight.h"


// Sets default values
ABaseAIController::ABaseAIController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComp"));
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
	
	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	
	if (SightConfig)
	{
		SightConfig->SightRadius=1500.0f;
		SightConfig->LoseSightRadius=2000.0f;
		SightConfig->PeripheralVisionAngleDegrees = 90.0f;
		
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true; 
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		UE_LOG(LogTemp,Warning,TEXT("需要修改在BaseAIController处修改敌我识别逻辑！"));
	}
	
	if (PerceptionComp&&SightConfig)
	{
		PerceptionComp->ConfigureSense(*SightConfig);
		PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
		PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this,&ABaseAIController::OnTargetDetected);
	}

}

// Called when the game starts or when spawned
void ABaseAIController::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseAIController::OnPossess(APawn* InPawn)
{
	
	
	Super::OnPossess(InPawn);
	ABaseAICharacter* ai=Cast<ABaseAICharacter>(InPawn);
	if (ai&&this->BehaviorTreeComp&&BlackboardComp)
	{
		BlackboardComp->InitializeBlackboard(*(ai->BehaviorTree->BlackboardAsset));
		
		RunBehaviorTree(ai->BehaviorTree);
	}
	else
	{
		UE_LOG(LogTemp,Error,TEXT("AI Controller,Character,BehaviorTreeComp or BlackboardComp is Not Valid"));
	}
}

void ABaseAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	if (Stimulus.WasSuccessfullySensed())
	{
		
		UE_LOG(LogTemp,Display,TEXT("I see you:%s"),*Actor->GetName());
		
		if (BlackboardComp)
		{
			GetBlackboardComponent()->SetValueAsObject("Target",Actor);
		}
	}
	else
	{
		UE_LOG(LogTemp,Display,TEXT("Lost Sight of:%s"),*Actor->GetName());
	}
}

// Called every frame
void ABaseAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
