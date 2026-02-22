// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "BaseAICharacter.generated.h"

class UBehaviorTree;
class UGameplayEffect;
class UGameplayAbility;
class UBaseAttributeSet;

UCLASS()
class THEPROJECT2_API ABaseAICharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseAICharacter();
	
	

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	//GAS
	void GiveDefaultAbilities();
	void InitAbilitySystem();
	virtual void InitializeAttribute();
	
	UPROPERTY(EditDefaultsOnly,blueprintreadwrite,Category="GAS")
	TSubclassOf<UGameplayEffect>  DefaultAttributeEffect;//用于给attribute赋初值
	
	UPROPERTY(editDefaultsOnly,BlueprintReadOnly, Category="GAS")
	TArray<TSubclassOf<UGameplayAbility>> CharacterAbilities;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="GAS")
	UAbilitySystemComponent* AbilitySystemComponent;
	UPROPERTY()
	UBaseAttributeSet* AttributeSet;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UPROPERTY(BlueprintReadOnly,EditAnywhere ,Category="AI")
	UBehaviorTree* BehaviorTree;
};
