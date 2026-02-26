// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/CombatSystemComp.h"

#include "Engine/OverlapResult.h"
#include "Kismet/KismetMathLibrary.h"
#include "LockonPointComp.h"
#include "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "CustomEnum.h"
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
void UCombatSystemComp::TickComponent(const float DeltaTime, const ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (LockPoint&&LockPoint->IsLockable)
	{
		DrawDebugSphere(GetWorld(),LockPoint->GetComponentLocation(),30,16,FColor::Red);
		FRotator TargetRotator =  UKismetMathLibrary::FindLookAtRotation(OwnerCharacter->GetActorLocation(),LockPoint->GetComponentLocation());
		if (OwnerCharacter->GetController())
		{
			//OwnerController->SetControlRotation(TargetRotator);
			TargetRotator = FRotator(TargetRotator.Roll,TargetRotator.Yaw,OwnerCharacter->GetActorRotation().Pitch);
			OwnerCharacter->SetActorRotation(TargetRotator);
		}
	}
	else if (LockPoint)
	{
		QuitLockOn();
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
					if (FoundComp->IsLockable&&CurrentWeight < MinWeight)
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

/**
 * 根据攻击打到的位置来计算出打到了角色的哪一个部位，应该由攻击发起者调用
 * 有正面视角的上下左右（别人看你的方向的上下左右）和后面，可以根据这个来播放对应动画
 * 计算完后会激活对应的GameplayTag，为：Character.HitState.Up/Down/Left/Right/Back...
 * @param HitLocation 攻击打到角色身上的位置，绝对坐标
 * @param TargetActor 被攻击的角色
 */
void UCombatSystemComp::CalculateHitDirection(const FVector HitLocation, AActor* TargetActor)
{
	
    //  将受击点的绝对坐标转换到角色的本地空间
    // 转换后：中心点为(0,0,0)，X代表前后，Y代表左右，Z代表上下
    FVector LocalHitLoc = TargetActor->GetActorTransform().InverseTransformPosition(HitLocation);
	
    FGameplayTag HitTag;

    //  首先判断是否是背后受击 (X轴负方向代表在角色中心后方)
    // 可以加一个微小的容差值，比如 < -10.0f，避免极度侧面被误判为背面
    if (LocalHitLoc.X < 0.0f) 
    {
        HitTag = FGameplayTag::RequestGameplayTag(FName("Character.HitState.Back"));
    }
    else 
    {
        // 3. 处理正面受击 (X >= 0)，切分上下左右四个象限
        // 【关键点】由于人形角色的胶囊体高度(Z)通常是宽度(Y)的两倍以上，
        // 如果直接比较 Y 和 Z 的绝对值，会导致几乎所有攻击都被判定为上下。
        // 因此我们给 Y 轴乘以一个补偿权重（例如2.0），让左右判定区域更合理。
        float HeightToWidthRatio = 2.0f; 
        float WeightedY = FMath::Abs(LocalHitLoc.Y) * HeightToWidthRatio;
        float AbsZ = FMath::Abs(LocalHitLoc.Z);

        // 对比横向与纵向的偏移程度，判断是倾向于打在侧面还是上下
        if (WeightedY > AbsZ)
        {
            if (LocalHitLoc.Y > 0.0f)
            {
                HitTag = FGameplayTag::RequestGameplayTag(FName("Character.HitState.Left"));
            }
            else
            {
                HitTag = FGameplayTag::RequestGameplayTag(FName("Character.HitState.Right"));
            }
        }
        else
        {
            if (LocalHitLoc.Z > 0.0f)
            {
                HitTag = FGameplayTag::RequestGameplayTag(FName("Character.HitState.Up"));
            }
            else
            {
                HitTag = FGameplayTag::RequestGameplayTag(FName("Character.HitState.Down"));
            }
        }
    }

    // 4. 激活对应的 GameplayTag
    if (HitTag.IsValid())
    {
    	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("Hit Part Tag: %s"), *HitTag.ToString()));
		FGameplayEventData Payload;
    	Payload.Instigator = GetOwner();
    	Payload.Target = TargetActor;
    	Payload.TargetTags.AddTag(HitTag);
    	
    	FGameplayTag HitEventTag = FGameplayTag::RequestGameplayTag(FName("Event.Character.HitReact"));
    	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor,HitEventTag,Payload);
    }
}

void UCombatSystemComp::DoDirectionalRoll(FVector2D InputValue)
{
	// 3. 构建 Gameplay Event Data
	FGameplayEventData Payload;
	Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Character.Roll"));
	Payload.Instigator = OwnerCharacter;
	Payload.Target = OwnerCharacter;
    
	// 4. 将 FVector2D 转换为 FVector，并存入 TargetData
	// 使用 LocationInfo 是 GAS 传递位置/向量数据的标准方式
	FGameplayAbilityTargetData_LocationInfo* LocationData = new FGameplayAbilityTargetData_LocationInfo();
	LocationData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	// 将 2D 向量存入 Transform 的 Location 中
	LocationData->TargetLocation.LiteralTransform = FTransform(FVector(InputValue.X, InputValue.Y, 0.f));
	Payload.TargetData.Add(LocationData);
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerCharacter, Payload.EventTag, Payload);
}

ERollAnimationType UCombatSystemComp::GetRollDirection(const FVector2D InputDirection)
{
	// 如果没有输入，默认返回向前翻滚
	if (InputDirection.IsNearlyZero())
	{
		return ERollAnimationType::Front;
	}
    
	FVector2D Dir(InputDirection.X, InputDirection.Y);
	Dir.Normalize();

	float Angle = FMath::RadiansToDegrees(FMath::Atan2(Dir.Y, Dir.X));

	if (Angle >= -22.5f && Angle < 22.5f)
	{
		return ERollAnimationType::Right;       // 右 (0度附近)
	}
	else if (Angle >= 22.5f && Angle < 67.5f)
	{
		return ERollAnimationType::FrontRight;  // 右前 (45度附近)
	}
	else if (Angle >= 67.5f && Angle < 112.5f)
	{
		return ERollAnimationType::Front;       // 前 (90度附近)
	}
	else if (Angle >= 112.5f && Angle < 157.5f)
	{
		return ERollAnimationType::FrontLeft;   // 左前 (135度附近)
	}
	else if (Angle >= -67.5f && Angle < -22.5f)
	{
		return ERollAnimationType::BackRight;   // 右后 (-45度附近)
	}
	else if (Angle >= -112.5f && Angle < -67.5f)
	{
		return ERollAnimationType::Back;        // 后 (-90度附近)
	}
	else if (Angle >= -157.5f && Angle < -112.5f)
	{
		return ERollAnimationType::BackLeft;    // 左后 (-135度附近)
	}
	else
	{
		return ERollAnimationType::Left;        // 左 (180或-180度附近，Angle >= 157.5f 或 Angle < -157.5f)
	}
}

FMotionWarpingTarget UCombatSystemComp::CalculateMotionWarpingTarget(float MaxDistance, FVector TargetLocation, FRotator TargetRotation, FName WarpTargetName)
{
	FMotionWarpingTarget WarpTarget;
	WarpTarget.Name = WarpTargetName;
	WarpTarget.Rotation = TargetRotation;
	WarpTarget.bFollowComponent = false; 
	
	AActor* OwnerActor = GetOwner();
	
	if (!OwnerActor)
	{
		
		WarpTarget.Location = TargetLocation;
		return WarpTarget;
	}

	/* * 备注：如果你的逻辑严格依赖 ACharacter 特有的属性，可以在这里进行 Cast 强转：
	 * ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerActor);
	 * 但因为我们仅仅需要获取位置 (GetActorLocation)，使用原生的 AActor 指针就足够了。
	 */

	// 3. 获取起点位置
	FVector StartLocation = OwnerActor->GetActorLocation();
    
	// 4. 计算从起点指向目标点的向量
	FVector Direction = TargetLocation - StartLocation;
	
	if (MaxDistance > 0.f && Direction.SizeSquared() > FMath::Square(MaxDistance))
	{
		WarpTarget.Location = StartLocation + (Direction.GetSafeNormal() * MaxDistance);
	}
	else
	{
		WarpTarget.Location = TargetLocation;
	}

	return WarpTarget;
}
