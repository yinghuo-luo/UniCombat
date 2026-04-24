// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/AbilitySystem/AtlasDamageExecution.h"

#include "Combat/AbilitySystem/AtlasCombatAttributeSet.h"
#include "Combat/AtlasGameplayTags.h"

/*
* 这个结构体预先声明并初始化了 4 个用于伤害计算的属性捕获定义：
AttackPower 和 AbilityPower 从施法者一侧快照捕获，
Defense 和 StatusResistance 从目标一侧实时捕获。
其中 DECLARE_ATTRIBUTE_CAPTUREDEF 负责声明每个属性对应的 Property 和 CaptureDefinition 成员，
DEFINE_ATTRIBUTE_CAPTUREDEF 负责真正把这些成员绑定到 
UAtlasCombatAttributeSet 中对应的字段，并指定捕获来源与是否快照。
 */
struct FAtlasDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(AbilityPower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Defense);
	DECLARE_ATTRIBUTE_CAPTUREDEF(StatusResistance);

	FAtlasDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAtlasCombatAttributeSet, AttackPower, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAtlasCombatAttributeSet, AbilityPower, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAtlasCombatAttributeSet, Defense, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAtlasCombatAttributeSet, StatusResistance, Target, false);
	}
};

static const FAtlasDamageStatics& DamageStatics()
{
	static FAtlasDamageStatics Statics;
	return Statics;
}

UAtlasDamageExecution::UAtlasDamageExecution()
{
	RelevantAttributesToCapture.Add(DamageStatics().AttackPowerDef);
	RelevantAttributesToCapture.Add(DamageStatics().AbilityPowerDef);
	RelevantAttributesToCapture.Add(DamageStatics().DefenseDef);
	RelevantAttributesToCapture.Add(DamageStatics().StatusResistanceDef);
}

void UAtlasDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	//初始化四个捕获属性变量
	float AttackPower = 0.0f; //物理/普通攻击强度
	float AbilityPower = 0.0f; //法术/技能强度
	float Defense = 0.0f; //伤害减免用防御
	float StatusResistance = 0.0f; //状态/削韧抗性

	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackPowerDef,
		EvaluationParameters, AttackPower);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AbilityPowerDef, 
		EvaluationParameters, AbilityPower);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DefenseDef, 
		EvaluationParameters, Defense);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().StatusResistanceDef, 
		EvaluationParameters, StatusResistance);

	//防止负数污染公式
	AttackPower = FMath::Max(AttackPower, 0.0f);
	AbilityPower = FMath::Max(AbilityPower, 0.0f);
	Defense = FMath::Max(Defense, 0.0f);
	StatusResistance = FMath::Max(StatusResistance, 0.0f);

	//读取 SetByCaller 的基础数值
	const float BaseDamage = Spec.GetSetByCallerMagnitude(TAG_SetByCaller_Data_Damage, 
		false, 0.0f);
	const float BasePoiseDamage = Spec.GetSetByCallerMagnitude(TAG_SetByCaller_Data_PoiseDamage, 
		false, 0.0f);
	const float BaseHealing = Spec.GetSetByCallerMagnitude(TAG_SetByCaller_Data_Healing, 
		false, 0.0f);

	FGameplayTagContainer AssetTags;
	Spec.GetAllAssetTags(AssetTags);

	//@Exact 精确的/确切的
	//@Invincible 无敌
	if (TargetTags != nullptr && (TargetTags->HasTagExact(TAG_State_Dead) 
		|| TargetTags->HasTagExact(TAG_State_Invincible)))
	{
		return;
	}

	//Contribution 贡献
	float PowerContribution = AttackPower;
	//如果当前效果带有：TAG_Damage_Spell或 TAG_Damage_Exorcise则改用：AbilityPower
	if (AssetTags.HasTag(TAG_Damage_Spell) || AssetTags.HasTag(TAG_Damage_Exorcise))
	{
		PowerContribution = AbilityPower;
	}

	float FinalDamage = FMath::Max(BaseDamage + PowerContribution - Defense, 0.0f);
	//计算最终削韧伤害
	float FinalPoiseDamage = FMath::Max(BasePoiseDamage - (StatusResistance * 0.25f), 0.0f);

	//如果目标是 Ghost并且这次效果本身带有“对 Ghost 增伤”标签，那么最终伤害 × 1.25。
	//也就是 对幽灵额外增伤 25%。
	if (TargetTags != nullptr && TargetTags->HasTag(TAG_Type_Ghost) && 
		AssetTags.HasTag(TAG_Damage_BonusVsGhost))
	{
		FinalDamage *= 1.25f;
	}

	//同理：目标是 Corpse，效果带有“对尸体增伤”标签
	//则伤害再乘 1.25。
	if (TargetTags != nullptr && TargetTags->HasTag(TAG_Type_Corpse) && 
		AssetTags.HasTag(TAG_Damage_BonusVsCorpse))
	{
		FinalDamage *= 1.25f;
	}

	if (FinalDamage > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UAtlasCombatAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, FinalDamage));
	}

	if (FinalPoiseDamage > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UAtlasCombatAttributeSet::GetIncomingPoiseDamageAttribute(), EGameplayModOp::Additive, FinalPoiseDamage));
	}

	if (BaseHealing > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UAtlasCombatAttributeSet::GetIncomingHealingAttribute(), EGameplayModOp::Additive, BaseHealing));
	}
}
