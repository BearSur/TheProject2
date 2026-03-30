// Copyright Epic Games, Inc. All Rights Reserved.

#include "Public/BounceBackActor.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"

ABounceBackActor::ABounceBackActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	DisplayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayMesh"));
	DisplayMesh->SetupAttachment(SceneRoot);
	DisplayMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(SceneRoot);
	TriggerBox->SetBoxExtent(FVector(60.0f, 60.0f, 60.0f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ABounceBackActor::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ABounceBackActor::HandleTriggerBeginOverlap);
}

void ABounceBackActor::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	ACharacter* OverlappingCharacter = Cast<ACharacter>(OtherActor);
	if (!OverlappingCharacter)
	{
		return;
	}

	FVector LaunchDirection = OverlappingCharacter->GetActorLocation() - GetActorLocation();
	LaunchDirection.Z = 0.0f;

	if (!LaunchDirection.Normalize())
	{
		LaunchDirection = -GetActorForwardVector();
		LaunchDirection.Z = 0.0f;
		LaunchDirection.Normalize();
	}

	const FVector LaunchVelocity = (LaunchDirection * HorizontalLaunchStrength) + FVector::UpVector * VerticalLaunchStrength;
	OverlappingCharacter->LaunchCharacter(LaunchVelocity, true, true);
}
