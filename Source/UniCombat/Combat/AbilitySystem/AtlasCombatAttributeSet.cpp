// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/AbilitySystem/AtlasCombatAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "Combat/AtlasCombatTypes.h"
#include "Combat/AtlasGameplayTags.h"
#include "Combat/Interfaces/AtlasHitReceiverInterface.h"
#include "Net/UnrealNetwork.h"

UAtlasCombatAttributeSet::UAtlasCombatAttributeSet()
{
	InitMaxHealth(100.0f);
	InitHealth(100.0f);
	
	InitMaxStamina(100.0f);
	InitStamina(100.0f);
	
	InitMaxSpirit(100.0f);
	InitSpirit(100.0f);
	
	InitAttackPower(15.0f);
	InitAbilityPower(15.0f);
	
	InitDefense(5.0f);
	
	InitMaxPoise(50.0f);
	InitPoise(50.0f);
	
	InitMoveSpeed(600.0f);
	
	InitStatusResistance(0.0f);
}

void UAtlasCombatAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAtlasCombatAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAtlasCombatAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAtlasCombatAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAtlasCombatAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAtlasCombatAttributeSet, Spirit, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAtlasCombatAttributeSet, MaxSpirit, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAtlasCombatAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAtlasCombatAttributeSet, AbilityPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAtlasCombatAttributeSet, Defense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAtlasCombatAttributeSet, Poise, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAtlasCombatAttributeSet, MaxPoise, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAtlasCombatAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAtlasCombatAttributeSet, StatusResistance, COND_None, REPNOTIFY_Always);
}

void UAtlasCombatAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetMaxStaminaAttribute() 
		|| Attribute == GetMaxSpiritAttribute() 
		|| Attribute == GetMaxPoiseAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetMoveSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

void UAtlasCombatAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	AActor* TargetActor = Data.Target.AbilityActorInfo.IsValid() ? Data.Target.AbilityActorInfo->AvatarActor.Get() : nullptr;
	AActor* SourceActor = Data.EffectSpec.GetContext().GetOriginalInstigator();

	FGameplayTagContainer SourceTags;
	FGameplayTagContainer TargetTags;
	Data.EffectSpec.GetAllAssetTags(SourceTags);
	if (const FGameplayTagContainer* CapturedTargetTags = Data.EffectSpec.CapturedTargetTags.GetAggregatedTags())
	{
		TargetTags.AppendTags(*CapturedTargetTags);
	}

	auto BuildHitData = [&](const float AppliedDamage, const float AppliedPoiseDamage, const bool bIsFatal)
	{
		FAtlasCombatHitData HitData;
		HitData.InstigatorActor = SourceActor;
		HitData.EffectCauser = Data.EffectSpec.GetContext().GetEffectCauser();
		HitData.Damage = AppliedDamage;
		HitData.PoiseDamage = AppliedPoiseDamage;
		HitData.bIsFatal = bIsFatal;
		HitData.SourceTags = SourceTags;
		HitData.TargetTags = TargetTags;

		if (const FHitResult* HitResult = Data.EffectSpec.GetContext().GetHitResult())
		{
			HitData.HitResult = *HitResult;
		}

		for (const FGameplayTag& Tag : SourceTags)
		{
			if (Tag.MatchesTag(TAG_Damage_Physical) || Tag.MatchesTag(TAG_Damage_Spell) || Tag.MatchesTag(TAG_Damage_Exorcise))
			{
				HitData.DamageTypeTag = Tag;
				break;
			}
		}

		return HitData;
	};

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float LocalDamage = GetIncomingDamage();
		SetIncomingDamage(0.0f);

		if (LocalDamage > 0.0f)
		{
			SetHealth(FMath::Clamp(GetHealth() - LocalDamage, 0.0f, GetMaxHealth()));
			const bool bIsFatal = GetHealth() <= 0.0f;

			if (TargetActor != nullptr && TargetActor->GetClass()->ImplementsInterface(UAtlasHitReceiverInterface::StaticClass()))
			{
				const FAtlasCombatHitData HitData = BuildHitData(LocalDamage, 0.0f, bIsFatal);
				IAtlasHitReceiverInterface::Execute_HandleGameplayHit(TargetActor, HitData);

				if (bIsFatal)
				{
					IAtlasHitReceiverInterface::Execute_HandleGameplayDeath(TargetActor, HitData);
				}
			}
		}
	}
	else if (Data.EvaluatedData.Attribute == GetIncomingHealingAttribute())
	{
		const float LocalHealing = GetIncomingHealing();
		SetIncomingHealing(0.0f);

		if (LocalHealing > 0.0f)
		{
			SetHealth(FMath::Clamp(GetHealth() + LocalHealing, 0.0f, GetMaxHealth()));
		}
	}
	else if (Data.EvaluatedData.Attribute == GetIncomingPoiseDamageAttribute())
	{
		const float LocalPoiseDamage = GetIncomingPoiseDamage();
		SetIncomingPoiseDamage(0.0f);

		if (LocalPoiseDamage > 0.0f)
		{
			SetPoise(FMath::Clamp(GetPoise() - LocalPoiseDamage, 0.0f, GetMaxPoise()));
		}
	}

	ClampPrimaryAttributes();
}

void UAtlasCombatAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAtlasCombatAttributeSet, Health, OldValue);
}

void UAtlasCombatAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAtlasCombatAttributeSet, MaxHealth, OldValue);
}

void UAtlasCombatAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAtlasCombatAttributeSet, Stamina, OldValue);
}

void UAtlasCombatAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAtlasCombatAttributeSet, MaxStamina, OldValue);
}

void UAtlasCombatAttributeSet::OnRep_Spirit(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAtlasCombatAttributeSet, Spirit, OldValue);
}

void UAtlasCombatAttributeSet::OnRep_MaxSpirit(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAtlasCombatAttributeSet, MaxSpirit, OldValue);
}

void UAtlasCombatAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAtlasCombatAttributeSet, AttackPower, OldValue);
}

void UAtlasCombatAttributeSet::OnRep_AbilityPower(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAtlasCombatAttributeSet, AbilityPower, OldValue);
}

void UAtlasCombatAttributeSet::OnRep_Defense(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAtlasCombatAttributeSet, Defense, OldValue);
}

void UAtlasCombatAttributeSet::OnRep_Poise(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAtlasCombatAttributeSet, Poise, OldValue);
}

void UAtlasCombatAttributeSet::OnRep_MaxPoise(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAtlasCombatAttributeSet, MaxPoise, OldValue);
}

void UAtlasCombatAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAtlasCombatAttributeSet, MoveSpeed, OldValue);
}

void UAtlasCombatAttributeSet::OnRep_StatusResistance(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAtlasCombatAttributeSet, StatusResistance, OldValue);
}

void UAtlasCombatAttributeSet::ClampPrimaryAttributes()
{
	SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	SetSpirit(FMath::Clamp(GetSpirit(), 0.0f, GetMaxSpirit()));
	SetPoise(FMath::Clamp(GetPoise(), 0.0f, GetMaxPoise()));
	//SetMoveSpeed(FMath::Max(GetMoveSpeed(), 0.0f));
}
