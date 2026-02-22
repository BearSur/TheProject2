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
			if (GetHealth()<=0.0f&&oldHealth>0.0f)
			{
				//获取杀手和被害者的信息
				UAbilitySystemComponent* TargetASC = &Data.Target;
				UAbilitySystemComponent* SourceASC = Data.EffectSpec.GetContext().GetOriginalInstigatorAbilitySystemComponent();

				if (TargetASC)
				{
					// 构建事件负载 Payload
					FGameplayEventData Payload;
					Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Death")); // 确保你在编辑器里创建了这个 Tag
					Payload.Instigator = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
					Payload.Target = TargetASC->GetAvatarActor();
					Payload.ContextHandle = Data.EffectSpec.GetContext();
					Payload.EventMagnitude = LocalDamage; 
					UE_LOG(LogTemp,Warning,TEXT("Character Death Trigger Get From AI, remeber to Check!"));
					
					UE_LOG(LogTemp, Warning, TEXT("Sending Death Event to Actor: %s with Tag: %s"), 
						*TargetASC->GetAvatarActor()->GetName(), 
						*Payload.EventTag.ToString());
					
					// 发送事件 (Send Gameplay Event)
					TargetASC->HandleGameplayEvent(Payload.EventTag, &Payload);
					
				}
			}
			UE_LOG(LogTemp,Display,TEXT("Health: %f,Damage:%f"),GetHealth(),LocalDamage);
		}
		
		
	}
	
	
}
