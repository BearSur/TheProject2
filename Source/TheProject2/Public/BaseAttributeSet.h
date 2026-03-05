// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "BaseAttributeSet.generated.h"


#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class THEPROJECT2_API UBaseAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
	
public:
	UBaseAttributeSet();

	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	//-----------variable----------------//
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, Health);
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, MaxHealth);

	// 耐力 (用于近战/奔跑)
	UPROPERTY(BlueprintReadOnly,Category = "Attributes")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, Stamina);
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, MaxStamina);
	UPROPERTY(BlueprintReadOnly,Category = "MetaAttributes")
	FGameplayAttributeData Poise;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, Poise);
	
	//Meta Attribute
	
	UPROPERTY(BlueprintReadOnly,Category = "MetaAttributes")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, Damage);
	UPROPERTY(BlueprintReadOnly,Category = "MetaAttributes")
	FGameplayAttributeData Healing;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, Healing);
	UPROPERTY(BlueprintReadOnly,Category = "MetaAttributes")
	FGameplayAttributeData PoiseDamage;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, PoiseDamage);
};
