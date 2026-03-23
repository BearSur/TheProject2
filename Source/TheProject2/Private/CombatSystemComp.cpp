// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/CombatSystemComp.h"

#include "Engine/OverlapResult.h"
#include "HitboxData.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "LockonPointComp.h"
#include "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CustomEnum.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HAL/IConsoleManager.h"
#include "SkillData.h"
#include "UObject/UnrealType.h"

namespace
{
	const FName NAME_CurrentPlaySkill(TEXT("CurrentPlaySkill"));
	const FName NAME_CurrentDamageCount(TEXT("CurrentDamageCount"));
	const FName NAME_HitActorArray(TEXT("HitActorArray"));
	const FName NAME_GetWeaponMesh(TEXT("GetWeaponMesh"));
	const FName NAME_WeaponMesh(TEXT("WeaponMesh"));
	const FName NAME_DeliverDamageAndPoise(TEXT("DeliverDamageAndPoise"));
	const FName NAME_StartHitStop(TEXT("StartHitStop"));

	static TAutoConsoleVariable<int32> CVarCombatDebugHitbox(
		TEXT("theproject2.Combat.DebugHitbox"),
		1,
		TEXT("Enable debug drawing for ResolveHitDetection.\n")
		TEXT("0: Disabled\n")
		TEXT("1: Enabled"));

	static TAutoConsoleVariable<float> CVarCombatDebugHitboxDuration(
		TEXT("theproject2.Combat.DebugHitboxDuration"),
		1.5f,
		TEXT("Debug draw duration in seconds for ResolveHitDetection."));

	struct FGetWeaponMeshParams
	{
		UStaticMeshComponent* WeaponMesh = nullptr;
	};

	struct FDeliverDamageAndPoiseParams
	{
		AActor* TargetActor = nullptr;
		double Damage = 0.0;
		double Poise = 0.0;
	};

	struct FStartHitStopParams
	{
		double Duration = 0.0;
	};

	struct FResolvedHitTarget
	{
		AActor* HitActor = nullptr;
		FVector HitLocation = FVector::ZeroVector;
	};

	USkillData* GetBlueprintCurrentSkill(UObject* SourceObject)
	{
		if (!SourceObject)
		{
			return nullptr;
		}

		const FObjectProperty* SkillProperty = FindFProperty<FObjectProperty>(SourceObject->GetClass(), NAME_CurrentPlaySkill);
		if (!SkillProperty)
		{
			return nullptr;
		}

		return Cast<USkillData>(SkillProperty->GetObjectPropertyValue_InContainer(SourceObject));
	}

	int32 GetBlueprintCurrentDamageCount(UObject* SourceObject)
	{
		if (!SourceObject)
		{
			return INDEX_NONE;
		}

		const FIntProperty* DamageCountProperty = FindFProperty<FIntProperty>(SourceObject->GetClass(), NAME_CurrentDamageCount);
		return DamageCountProperty ? DamageCountProperty->GetPropertyValue_InContainer(SourceObject) : INDEX_NONE;
	}

	TArray<AActor*>* GetBlueprintHitActorArray(UObject* SourceObject)
	{
		if (!SourceObject)
		{
			return nullptr;
		}

		const FArrayProperty* HitActorArrayProperty = FindFProperty<FArrayProperty>(SourceObject->GetClass(), NAME_HitActorArray);
		if (!HitActorArrayProperty || !HitActorArrayProperty->Inner || !HitActorArrayProperty->Inner->IsA<FObjectProperty>())
		{
			return nullptr;
		}

		return HitActorArrayProperty->ContainerPtrToValuePtr<TArray<AActor*>>(SourceObject);
	}

	UStaticMeshComponent* ResolveWeaponMesh(UObject* SourceObject)
	{
		if (!SourceObject)
		{
			return nullptr;
		}

		if (UFunction* GetWeaponMeshFunction = SourceObject->FindFunction(NAME_GetWeaponMesh))
		{
			FGetWeaponMeshParams Params;
			SourceObject->ProcessEvent(GetWeaponMeshFunction, &Params);
			if (Params.WeaponMesh)
			{
				return Params.WeaponMesh;
			}
		}

		if (const FObjectProperty* WeaponMeshProperty = FindFProperty<FObjectProperty>(SourceObject->GetClass(), NAME_WeaponMesh))
		{
			if (UStaticMeshComponent* WeaponMesh = Cast<UStaticMeshComponent>(WeaponMeshProperty->GetObjectPropertyValue_InContainer(SourceObject)))
			{
				return WeaponMesh;
			}
		}

		AActor* OwnerActor = Cast<AActor>(SourceObject);
		if (!OwnerActor)
		{
			return nullptr;
		}

		TArray<UStaticMeshComponent*> StaticMeshComponents;
		OwnerActor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
		for (UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
		{
			if (StaticMeshComponent && StaticMeshComponent->GetName().Contains(TEXT("Weapon")))
			{
				return StaticMeshComponent;
			}
		}

		return StaticMeshComponents.Num() > 0 ? StaticMeshComponents[0] : nullptr;
	}

	FVector ResolveClosestHitLocation(AActor* HitActor, const FVector& ReferencePoint)
	{
		if (!IsValid(HitActor))
		{
			return ReferencePoint;
		}

		FVector BestPoint = HitActor->GetActorLocation();
		float BestDistanceSq = FVector::DistSquared(BestPoint, ReferencePoint);

		TArray<UPrimitiveComponent*> PrimitiveComponents;
		HitActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

		for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
		{
			if (!PrimitiveComponent || !PrimitiveComponent->IsCollisionEnabled())
			{
				continue;
			}

			FVector ClosestPoint = BestPoint;
			const float Distance = PrimitiveComponent->GetClosestPointOnCollision(ReferencePoint, ClosestPoint);
			if (Distance < 0.0f)
			{
				continue;
			}

			const float DistanceSq = FVector::DistSquared(ClosestPoint, ReferencePoint);
			if (DistanceSq < BestDistanceSq)
			{
				BestDistanceSq = DistanceSq;
				BestPoint = ClosestPoint;
			}
		}

		return BestPoint;
	}

	bool ResolveDamageAndPoiseValues(const USkillData* SkillData, const int32 DamageIndex, float& OutDamage, float& OutPoise)
	{
		OutDamage = 0.0f;
		OutPoise = 0.0f;

		if (!SkillData || DamageIndex < 0)
		{
			return false;
		}

		const bool bHasDamage = SkillData->Damage.IsValidIndex(DamageIndex);
		const bool bHasPoise = SkillData->Poise.IsValidIndex(DamageIndex);

		if (bHasDamage)
		{
			OutDamage = SkillData->Damage[DamageIndex];
		}

		if (bHasPoise)
		{
			OutPoise = SkillData->Poise[DamageIndex];
		}

		if (!bHasDamage && !bHasPoise)
		{
			UE_LOG(LogTemp, Warning, TEXT("Skill '%s' does not have damage or poise data for index %d."),
				*SkillData->GetName(), DamageIndex);
			return false;
		}

		if (!bHasDamage)
		{
			UE_LOG(LogTemp, Warning, TEXT("Skill '%s' is missing damage data for index %d, using 0."),
				*SkillData->GetName(), DamageIndex);
		}

		if (!bHasPoise)
		{
			UE_LOG(LogTemp, Warning, TEXT("Skill '%s' is missing poise data for index %d, using 0."),
				*SkillData->GetName(), DamageIndex);
		}

		return true;
	}

	void CallBlueprintDeliverDamageAndPoise(UObject* SourceObject, AActor* TargetActor, const float Damage, const float Poise)
	{
		if (!SourceObject || !IsValid(TargetActor))
		{
			return;
		}

		if (UFunction* DeliverDamageFunction = SourceObject->FindFunction(NAME_DeliverDamageAndPoise))
		{
			FDeliverDamageAndPoiseParams Params;
			Params.TargetActor = TargetActor;
			Params.Damage = Damage;
			Params.Poise = Poise;
			SourceObject->ProcessEvent(DeliverDamageFunction, &Params);
		}
	}

	void CallBlueprintStartHitStop(UObject* SourceObject, const float Duration)
	{
		if (!SourceObject)
		{
			return;
		}

		if (UFunction* StartHitStopFunction = SourceObject->FindFunction(NAME_StartHitStop))
		{
			FStartHitStopParams Params;
			Params.Duration = Duration;
			SourceObject->ProcessEvent(StartHitStopFunction, &Params);
		}
	}

	bool ShouldDrawCombatHitboxDebug()
	{
		return CVarCombatDebugHitbox.GetValueOnGameThread() != 0;
	}

	float GetCombatHitboxDebugDuration()
	{
		return FMath::Max(0.0f, CVarCombatDebugHitboxDuration.GetValueOnGameThread());
	}
}


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

void UCombatSystemComp::ResolveHitDetection(UHitboxData* HitboxData)
{
	// 1. 基础合法性校验。没有数据、没有角色主体时，不进行任何判定。
	if (!HitboxData || !OwnerCharacter)
	{
		return;
	}

	// 2. 从蓝图组件实例里读取当前攻击上下文。
	// CurrentPlaySkill / CurrentDamageCount / HitActorArray 仍然由 BP_CombatSystremComp 维护，
	// 这里只是把命中检测与结算逻辑统一下沉到 C++。
	USkillData* CurrentPlaySkill = GetBlueprintCurrentSkill(this);
	if (!CurrentPlaySkill)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s failed to resolve hit detection because CurrentPlaySkill is not set."),
			*GetName());
		return;
	}

	const int32 CurrentDamageCount = GetBlueprintCurrentDamageCount(this);
	float Damage = 0.0f;
	float Poise = 0.0f;
	// 3. 读取当前这一段攻击应结算的 Damage / Poise。
	if (!ResolveDamageAndPoiseValues(CurrentPlaySkill, CurrentDamageCount, Damage, Poise))
	{
		return;
	}

	// 4. 组织忽略列表，避免打到自己，也避免一次攻击窗口里重复结算同一个目标。
	// HitActorArray 是“整段攻击窗口”的判重容器，它会跨多次 HitDetection 调用持续存在，
	// 直到攻击结束时由蓝图重置。
	TArray<AActor*>* HitActorArray = GetBlueprintHitActorArray(this);
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwnerCharacter);
	if (HitActorArray)
	{
		ActorsToIgnore.Append(*HitActorArray);
	}

	const bool bDrawDebug = ShouldDrawCombatHitboxDebug();
	const float DebugDrawTime = GetCombatHitboxDebugDuration();
	TArray<FResolvedHitTarget> ResolvedHitTargets;

	// 5. 按 HitboxType 执行不同的命中采样逻辑。
	switch (HitboxData->HitboxType)
	{
	case EHitboxType::WeaponSocketSweep:
		{
			// 武器型 hitbox：读取武器 Mesh 上的两个 socket，做球形扫掠。
			UStaticMeshComponent* WeaponMesh = ResolveWeaponMesh(OwnerCharacter);
			if (!WeaponMesh)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s failed to resolve a weapon mesh for weapon-socket hit detection."),
					*GetName());
				return;
			}

			const FVector TraceStart = HitboxData->StartSocketName.IsNone()
				? WeaponMesh->GetComponentLocation()
				: WeaponMesh->GetSocketLocation(HitboxData->StartSocketName);
			const FVector TraceEnd = HitboxData->EndSocketName.IsNone()
				? WeaponMesh->GetComponentLocation()
				: WeaponMesh->GetSocketLocation(HitboxData->EndSocketName);

			TArray<FHitResult> HitResults;
			UKismetSystemLibrary::SphereTraceMultiForObjects(
				this,
				TraceStart,
				TraceEnd,
				HitboxData->HitboxRadius,
				HitboxData->HitObjectType,
				false,
				ActorsToIgnore,
				bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
				HitResults,
				true,
				FLinearColor::Yellow,
				FLinearColor::Red,
				DebugDrawTime);

			// 统一转成后续结算使用的“命中目标 + 命中点”结构。
			for (const FHitResult& HitResult : HitResults)
			{
				if (AActor* HitActor = HitResult.GetActor())
				{
					const FVector HitLocation = HitResult.ImpactPoint.IsZero() ? HitResult.Location : HitResult.ImpactPoint;
					ResolvedHitTargets.Add({HitActor, HitLocation});
				}
			}
			break;
		}

	case EHitboxType::OwnerFollowSphere:
		{
			// 角色跟随型 hitbox：球心使用角色世界变换后的偏移位置，始终跟随角色移动。
			const FVector SphereCenter = OwnerCharacter->GetActorTransform().TransformPosition(HitboxData->FollowSphereOffset);

			TArray<AActor*> OverlappedActors;
			UKismetSystemLibrary::SphereOverlapActors(
				this,
				SphereCenter,
				HitboxData->HitboxRadius,
				HitboxData->HitObjectType,
				AActor::StaticClass(),
				ActorsToIgnore,
				OverlappedActors);

			if (bDrawDebug && GetWorld())
			{
				DrawDebugLine(
					GetWorld(),
					OwnerCharacter->GetActorLocation(),
					SphereCenter,
					FColor::Cyan,
					false,
					DebugDrawTime,
					0,
					1.5f);

				DrawDebugSphere(
					GetWorld(),
					SphereCenter,
					HitboxData->HitboxRadius,
					24,
					OverlappedActors.Num() > 0 ? FColor::Orange : FColor::Cyan,
					false,
					DebugDrawTime,
					0,
					1.5f);
			}

			for (AActor* OverlappedActor : OverlappedActors)
			{
				if (IsValid(OverlappedActor))
				{
					ResolvedHitTargets.Add({OverlappedActor, ResolveClosestHitLocation(OverlappedActor, SphereCenter)});
				}
			}
			break;
		}

	default:
		return;
	}

	//注意：ProcessedActors意义不明，
	// 6. 判重分两层：
	// HitActorArray 负责“跨多次 HitDetection 调用”的判重，防止一次攻击动画的多帧检测反复打中同一目标；
	// ProcessedActors 负责“单次 ResolveHitDetection 调用内”的判重，防止一次 trace / overlap 结果里，
	// 同一个 Actor 因多个组件或多个命中结果被重复结算。
	bool bHasAppliedHit = false;
	TSet<AActor*> ProcessedActors;

	for (const FResolvedHitTarget& ResolvedHitTarget : ResolvedHitTargets)
	{
		AActor* HitActor = ResolvedHitTarget.HitActor;
		const bool bAlreadyHitInAttackWindow = HitActorArray && HitActorArray->Contains(HitActor);
		if (!IsValid(HitActor) || HitActor == OwnerCharacter || bAlreadyHitInAttackWindow || ProcessedActors.Contains(HitActor))
		{
			continue;
		}

		ProcessedActors.Add(HitActor);

		if (HitActorArray)
		{
			HitActorArray->AddUnique(HitActor);
		}

		// 7. 结算仍然复用原蓝图链路：送伤害、算受击方向、触发 hit stop。
		CallBlueprintDeliverDamageAndPoise(this, HitActor, Damage, Poise);
		CalculateHitDirection(ResolvedHitTarget.HitLocation, HitActor);

		if (bDrawDebug && GetWorld())
		{
			DrawDebugSphere(
				GetWorld(),
				ResolvedHitTarget.HitLocation,
				12.0f,
				12,
				FColor::Red,
				false,
				DebugDrawTime,
				0,
				2.0f);
		}

		bHasAppliedHit = true;
	}

	// 8. 只要这次检测至少有一个有效命中，就触发一次停顿反馈。
	if (bHasAppliedHit)
	{
		CallBlueprintStartHitStop(this, 0.2f);
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


void UCombatSystemComp::CalculateHitDirection(const FVector HitLocation, AActor* TargetActor)
{
	IAbilitySystemInterface* ASCOwner = Cast<IAbilitySystemInterface>(TargetActor);
	if (!ASCOwner)
	{
		return;
	}
	UAbilitySystemComponent* ASC = ASCOwner->GetAbilitySystemComponent();
	
	if (!ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.HitState"))))
	{
		//若没有受击大小的tag，则说明韧性值过低，无需专门播放受击动画
		return;
	}
	
	
    // 将受击点的绝对坐标转换到角色的本地空间
    // 转换后：中心点为(0,0,0)，X代表前后，Y代表左右，Z代表上下
    FVector LocalHitLoc = TargetActor->GetActorTransform().InverseTransformPosition(HitLocation);
    FGameplayTag HitTag;

    // 判断是否后背受击
    if (LocalHitLoc.X < -10.0f) 
    {
    	if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.HitState.Heavy"))))
    	{	
    		HitTag = FGameplayTag::RequestGameplayTag(FName("Character.HitState.Heavy.Back"));
    	}
    	else if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.HitState.Light"))))
    	{	
    		HitTag = FGameplayTag::RequestGameplayTag(FName("Character.HitState.Light.Back"));
    	}
    }
    else 
    {
        /* 3. 处理正面受击 (X >= 0)，切分上下左右四个象限
         由于人形角色的胶囊体高度(Z)通常是宽度(Y)的两倍以上，
         如果直接比较 Y 和 Z 的绝对值，会导致几乎所有攻击都被判定为上下。
         因此我们给 Y 轴乘以一个补偿权重（例如2.0），让左右判定区域更合理。
        */
        float HeightToWidthRatio = 2.0f; 
        float WeightedY = FMath::Abs(LocalHitLoc.Y) * HeightToWidthRatio;
        float AbsZ = FMath::Abs(LocalHitLoc.Z);

        // 对比横向与纵向的偏移程度，判断是倾向于打在侧面还是上下
        if (WeightedY > AbsZ)
        {
            if (LocalHitLoc.Y > 0.0f)
            {
            	if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.HitState.Heavy"))))
            	{	
            		HitTag = FGameplayTag::RequestGameplayTag(FName("Character.HitState.Heavy.Left"));
            	}
            	else if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.HitState.Light"))))
            	{	
            		HitTag = FGameplayTag::RequestGameplayTag(FName("Character.HitState.Light.Left"));
            	}
            }
            else
            {
            	if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.HitState.Heavy"))))
            	{	
            		HitTag = FGameplayTag::RequestGameplayTag(FName("Character.HitState.Heavy.Right"));
            	}
            	else if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.HitState.Light"))))
            	{	
            		HitTag = FGameplayTag::RequestGameplayTag(FName("Character.HitState.Light.Right"));
            	}
            }
        }
        else
        {
            if (LocalHitLoc.Z > 0.0f)
            {
            	if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.HitState.Heavy"))))
            	{	
            		HitTag = FGameplayTag::RequestGameplayTag(FName("Character.HitState.Heavy.Up"));
            	}
            	else if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.HitState.Light"))))
            	{	
            		HitTag = FGameplayTag::RequestGameplayTag(FName("Character.HitState.Light.Up"));
            	}
            }
            else
            {
            	if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.HitState.Heavy"))))
            	{	
            		HitTag = FGameplayTag::RequestGameplayTag(FName("Character.HitState.Heavy.Down"));
            	}
            	else if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.HitState.Light"))))
            	{	
            		HitTag = FGameplayTag::RequestGameplayTag(FName("Character.HitState.Light.Down"));
            	}
            }
        }
    }
	ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.HitState.Heavy")));
	ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.HitState.Light")));
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
