// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/CombatSystemComp.h"
#include "Engine/OverlapResult.h"
#include "Kismet/KismetMathLibrary.h"
#include "LockonPointComp.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values for this component's properties
UCombatSystemComp::UCombatSystemComp()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCombatSystemComp::BeginPlay()
{
	Super::BeginPlay();
	
	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		OwnerController=OwnerCharacter->GetController();
	}
	
}


// Called every frame
void UCombatSystemComp::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (LockPoint)
	{
		DrawDebugSphere(GetWorld(),LockPoint->GetComponentLocation(),30,16,FColor::Red);
		FRotator TargetRotator =  UKismetMathLibrary::FindLookAtRotation(OwnerCharacter->GetActorLocation(),LockPoint->GetComponentLocation());
		if (OwnerCharacter->GetController())
		{
			OwnerController->SetControlRotation(TargetRotator);
		}
	}
}




void UCombatSystemComp::TargetLockOn()
{
	//配置扫描参数
	UWorld* World = GetWorld();
	if (!World) return;
	FVector Center = OwnerCharacter->GetActorLocation();
	
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(Radius);
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);
	QueryParams.bTraceComplex = false; 
	
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn); 
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	
	TArray<FOverlapResult> OverlapResults;
	//扫描
	bool bHit = World->OverlapMultiByObjectType(OverlapResults,Center,FQuat::Identity,
		ObjectQueryParams,SphereShape,QueryParams);
	
	
	//此处得到最符合权重的锁定点
	if (bHit)
	{
		ULockonPointComp* BestComp=nullptr;//选出最符合权重的Point
		float MinWeight = 1000000.0f;
		for (auto Result:OverlapResults)
		{
			if (AActor* OverlappedActor =  Result.GetActor())
			{
				TArray<ULockonPointComp*> CompsOnActor;
				OverlappedActor->GetComponents<ULockonPointComp>(CompsOnActor);
				for (ULockonPointComp* FoundComp :CompsOnActor)
				{
					float CurrentWeight = FVector::DistSquared(Center, FoundComp->GetComponentLocation());
					if (CurrentWeight < MinWeight)
					{
						MinWeight=CurrentWeight;
						BestComp = FoundComp;
					}
				}
			}
		}
		if (BestComp)
		{
			LockPoint=BestComp;
			OwnerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false; 
			OwnerCharacter->GetCharacterMovement()->bUseControllerDesiredRotation = true;
		}
	}
	
}

void UCombatSystemComp::QuitLockOn()
{
	LockPoint=nullptr;
	OwnerCharacter->GetCharacterMovement()->bOrientRotationToMovement = true; 
	OwnerCharacter->GetCharacterMovement()->bUseControllerDesiredRotation = false;
}

