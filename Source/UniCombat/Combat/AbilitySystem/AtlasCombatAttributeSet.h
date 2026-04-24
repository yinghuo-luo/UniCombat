// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AtlasCombatAttributeSet.generated.h"

/*#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)*/

UCLASS()
class UNICOMBAT_API UAtlasCombatAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UAtlasCombatAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//在属性最终值实际改变前调用，不要在那里面做最终值修正，而应该在 PreAttributeChange() 里做。
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	//GameplayEffect 执行后、并且修改了属性的 base value 之后调用；GAS 属性文档也建议在 AttributeSet 中重写它来对属性变化作出反应
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UAtlasCombatAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UAtlasCombatAttributeSet, MaxHealth)

	//@stamina 耐力
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "Attributes")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS_BASIC(UAtlasCombatAttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina, Category = "Attributes")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS_BASIC(UAtlasCombatAttributeSet, MaxStamina)

	//@sporit 灵力/法力
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Spirit, Category = "Attributes")
	FGameplayAttributeData Spirit;
	ATTRIBUTE_ACCESSORS_BASIC(UAtlasCombatAttributeSet, Spirit)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxSpirit, Category = "Attributes")
	FGameplayAttributeData MaxSpirit;
	ATTRIBUTE_ACCESSORS_BASIC(UAtlasCombatAttributeSet, MaxSpirit)

	//攻击力
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttackPower, Category = "Attributes")
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS_BASIC(UAtlasCombatAttributeSet, AttackPower)

	//技能力量
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AbilityPower, Category = "Attributes")
	FGameplayAttributeData AbilityPower;
	ATTRIBUTE_ACCESSORS_BASIC(UAtlasCombatAttributeSet, AbilityPower)

	//防御
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Defense, Category = "Attributes")
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS_BASIC(UAtlasCombatAttributeSet, Defense)

	//韧性
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Poise, Category = "Attributes")
	FGameplayAttributeData Poise;
	ATTRIBUTE_ACCESSORS_BASIC(UAtlasCombatAttributeSet, Poise)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxPoise, Category = "Attributes")
	FGameplayAttributeData MaxPoise;
	ATTRIBUTE_ACCESSORS_BASIC(UAtlasCombatAttributeSet, MaxPoise)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MoveSpeed, Category = "Attributes")
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UAtlasCombatAttributeSet, MoveSpeed)

	//抗性（火抗等用一个）
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_StatusResistance, Category = "Attributes")
	FGameplayAttributeData StatusResistance;
	ATTRIBUTE_ACCESSORS_BASIC(UAtlasCombatAttributeSet, StatusResistance)

	//即将造成的伤害
	UPROPERTY(BlueprintReadOnly, Category = "Meta")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS_BASIC(UAtlasCombatAttributeSet, IncomingDamage)

	//即将到来的疗愈
	UPROPERTY(BlueprintReadOnly, Category = "Meta")
	FGameplayAttributeData IncomingHealing;
	ATTRIBUTE_ACCESSORS_BASIC(UAtlasCombatAttributeSet, IncomingHealing)

	//即将到来的平衡伤害
	UPROPERTY(BlueprintReadOnly, Category = "Meta")
	FGameplayAttributeData IncomingPoiseDamage;
	ATTRIBUTE_ACCESSORS_BASIC(UAtlasCombatAttributeSet, IncomingPoiseDamage)

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_Spirit(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_MaxSpirit(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_AttackPower(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_AbilityPower(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_Defense(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_Poise(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_MaxPoise(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_StatusResistance(const FGameplayAttributeData& OldValue) const;

private:
	/*
	 * Clamp 钳制/夹具
	 * Primary 主要的
	 */
	void ClampPrimaryAttributes();
};
