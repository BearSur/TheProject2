// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseAttributeSet.h"
#include "GameplayEffectExtension.h"

UBaseAttributeSet::UBaseAttributeSet()
{
	
}

void UBaseAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if(Data.EvaluatedData.Attribute==GetDamageAttribute())
	{
		const float LocalDamage=Data.EvaluatedData.Magnitude;
		//存储伤害值后，设置伤害值为0，以避免伤害叠加
		UE_LOG(LogTemp,Display,TEXT("Damage: %f"),GetDamage());
		Damage=0.0f;
		if (LocalDamage > 0.0f)
		{
			float oldHealth=GetHealth();
			float NewHealth = GetHealth() - LocalDamage;
			SetHealth(NewHealth);
			//处理死亡逻辑
			if (GetHealth()<=0.0f&&oldHealth>0.0f)
			{
				//获取杀手和被害者的信息
				UAbilitySystemComponent* TargetASC = &Data.Target;
				UAbilitySystemComponent* KillerASC = Data.EffectSpec.GetContext().GetOriginalInstigatorAbilitySystemComponent();
				if (TargetASC)
				{
					// 构建事件负载 Payload
					FGameplayEventData Payload;
					Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Character.Death")); //确保你在编辑器里创建了这个 Tag
					Payload.Instigator = KillerASC ? KillerASC->GetAvatarActor() : nullptr;
					Payload.Target = TargetASC->GetAvatarActor();
					Payload.ContextHandle = Data.EffectSpec.GetContext();
					Payload.EventMagnitude = LocalDamage; 
					
					FString DebugMessage = FString::Printf(TEXT("Sending Death Event to Actor: %s with Tag: %s"), 
						*TargetASC->GetAvatarActor()->GetName(), 
						*Payload.EventTag.ToString());
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DebugMessage);
					
					TargetASC->HandleGameplayEvent(Payload.EventTag, &Payload);
				}
			}
			UE_LOG(LogTemp,Display,TEXT("Health: %f,Damage:%f"),GetHealth(),LocalDamage);
		}
		
		
	}
	else if (Data.EvaluatedData.Attribute==GetHealingAttribute())
	{
		const float LocalHealing=Data.EvaluatedData.Magnitude;
		Healing=0.0f;
		if (LocalHealing > 0.0f)
		{
			float oldHealth=GetHealth();
			float NewHealth = GetHealth() + LocalHealing;
			if (oldHealth>0.0f&&NewHealth<GetMaxHealth())
			{
				SetHealth(NewHealth);
			}
			else if (NewHealth>GetMaxHealth())
			{
				SetHealth(GetMaxHealth());
			}
		}
	}
	else if (Data.EvaluatedData.Attribute==GetPoiseDamageAttribute())
	{
		const float localPoiseDamage=Data.EvaluatedData.Magnitude;
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,FString::Printf(TEXT("PoiseDamage: %f, Poise: %f"), localPoiseDamage, GetPoise()));
		SetPoiseDamage(0);
		UAbilitySystemComponent* VictimASC = &Data.Target;
		//poiseDamage大于poise才播放动画
		if (localPoiseDamage > GetPoise()+100)
		{
			VictimASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.HitState.Heavy")));
		}
		else if (localPoiseDamage>GetPoise())
		{
			VictimASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Character.HitState.Light")));
		}
		
	}
	
	
}
