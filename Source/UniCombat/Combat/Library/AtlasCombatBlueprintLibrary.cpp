// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/Library/AtlasCombatBlueprintLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Combat/AtlasCombatTypes.h"
#include "Combat/AtlasGameplayTags.h"
#include "Combat/Interfaces/AtlasCombatantInterface.h"
#include "Components/ActorComponent.h"
#include "GameplayEffect.h"

UAbilitySystemComponent* UAtlasCombatBlueprintLibrary::GetAbilitySystemFromActor(const AActor* Actor)
{
	////从Actor获取能力系统
	if (Actor == nullptr)
	{
		return nullptr;
	}

	if (const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(Actor))
	{
		return AbilitySystemInterface->GetAbilitySystemComponent();
	}

	if (Actor->GetClass()->ImplementsInterface(UAtlasCombatantInterface::StaticClass()))
	{
		return IAtlasCombatantInterface::Execute_GetCombatAbilitySystemComponent(const_cast<AActor*>(Actor));
	}

	return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(const_cast<AActor*>(Actor));
}

bool UAtlasCombatBlueprintLibrary::AreActorsHostile(const AActor* SourceActor, const AActor* TargetActor)
{
	////Actor是否敌对
	if (SourceActor == nullptr || TargetActor == nullptr || SourceActor == TargetActor)
	{
		return false;
	}

	if (!SourceActor->GetClass()->ImplementsInterface(UAtlasCombatantInterface::StaticClass())
		|| !TargetActor->GetClass()->ImplementsInterface(UAtlasCombatantInterface::StaticClass()))
	{
		return false;
	}

	const EAtlasCombatFaction SourceFaction = IAtlasCombatantInterface::Execute_GetCombatFaction(const_cast<AActor*>(SourceActor));
	const EAtlasCombatFaction TargetFaction = IAtlasCombatantInterface::Execute_GetCombatFaction(const_cast<AActor*>(TargetActor));

	if (SourceFaction == EAtlasCombatFaction::Neutral || TargetFaction == EAtlasCombatFaction::Neutral)
	{
		return false;
	}

	return SourceFaction != TargetFaction;
}

bool UAtlasCombatBlueprintLibrary::ApplyDamageEffect(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC,
	TSubclassOf<UGameplayEffect> DamageEffectClass, const float AbilityLevel, const float BaseDamage, const float BasePoiseDamage,
	const FGameplayTag DamageTypeTag, const FHitResult& HitResult, AActor* EffectCauser,
	const FGameplayTagContainer& AdditionalAssetTags)
{
	//应用伤害效果
	if (SourceASC == nullptr || TargetASC == nullptr || DamageEffectClass == nullptr)
	{
		return false;
	}

	/**
	* 封装 FGameplayEffectContext 或其子类的处理程序，使其能够实现多态并正确复制
	* @MakeEffectContext 为该 AbilitySystemComponent 的所有者创建一个 EffectContext。
	*/
	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	//AvatarActor 能力系统当前附着并实际驱动的那个表现实体 / 载体 Actor
	/*这里的 Avatar 不是“头像”，也不是社交软件里的 profile image。
	 *它更接近：化身 / 在世界中的具体代理体 玩家/系统在场景中的实体表现
	 */
	EffectContext.AddSourceObject(SourceASC->GetAvatarActor());
	EffectContext.AddInstigator(
		SourceASC->GetOwnerActor(),
		EffectCauser != nullptr ? EffectCauser : SourceASC->GetAvatarActor());

	if (HitResult.GetActor() != nullptr)
	{
		EffectContext.AddHitResult(HitResult);
	}

	/**
	 * 允许蓝图生成一次 GameplayEffectSpec，然后通过句柄引用它，以便多次应用到多个目标。 
	 * @MakeOutgoingSpec 获取一个可以应用于其他对象的外部 GameplayEffectSpec。
	 */
	const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, AbilityLevel, EffectContext);
	if (!SpecHandle.IsValid())
	{
		return false;
	}

	//设置 SetByCaller 修饰符的大小
	SpecHandle.Data->SetSetByCallerMagnitude(TAG_SetByCaller_Data_Damage, BaseDamage);
	SpecHandle.Data->SetSetByCallerMagnitude(TAG_SetByCaller_Data_PoiseDamage, BasePoiseDamage);

	if (DamageTypeTag.IsValid())
	{
		SpecHandle.Data->AddDynamicAssetTag(DamageTypeTag);
	}

	for (const FGameplayTag& AdditionalTag : AdditionalAssetTags)
	{
		SpecHandle.Data->AddDynamicAssetTag(AdditionalTag);
	}

	TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	return true;
}

bool UAtlasCombatBlueprintLibrary::ApplyGameplayEffectToTarget(UAbilitySystemComponent* SourceASC, AActor* TargetActor,
	TSubclassOf<UGameplayEffect> EffectClass, const float AbilityLevel, const FGameplayTagContainer& DynamicAssetTags)
{
	if (SourceASC == nullptr || TargetActor == nullptr || EffectClass == nullptr)
	{
		return false;
	}

	UAbilitySystemComponent* TargetASC = GetAbilitySystemFromActor(TargetActor);
	if (TargetASC == nullptr)
	{
		return false;
	}

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddSourceObject(SourceASC->GetAvatarActor());

	const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(EffectClass, AbilityLevel, EffectContext);
	if (!SpecHandle.IsValid())
	{
		return false;
	}

	for (const FGameplayTag& Tag : DynamicAssetTags)
	{
		SpecHandle.Data->AddDynamicAssetTag(Tag);
	}

	TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	return true;
}

bool UAtlasCombatBlueprintLibrary::ApplyGameplayEffectToSelf(UAbilitySystemComponent* AbilitySystemComponent,
	TSubclassOf<UGameplayEffect> EffectClass, const float AbilityLevel, const FGameplayTagContainer& DynamicAssetTags)
{
	if (AbilitySystemComponent == nullptr || EffectClass == nullptr)
	{
		return false;
	}

	const FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, AbilityLevel, EffectContext);
	if (!SpecHandle.IsValid())
	{
		return false;
	}

	for (const FGameplayTag& Tag : DynamicAssetTags)
	{
		SpecHandle.Data->AddDynamicAssetTag(Tag);
	}

	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	return true;
}

UObject* UAtlasCombatBlueprintLibrary::FindInterfaceImplementer(const AActor* Actor, UClass* InterfaceClass)
{
	if (Actor == nullptr || InterfaceClass == nullptr || !InterfaceClass->IsChildOf(UInterface::StaticClass()))
	{
		return nullptr;
	}

	if (Actor->GetClass()->ImplementsInterface(InterfaceClass))
	{
		return const_cast<AActor*>(Actor);
	}

	TInlineComponentArray<UActorComponent*> Components;
	const_cast<AActor*>(Actor)->GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		if (Component != nullptr && Component->GetClass()->ImplementsInterface(InterfaceClass))
		{
			return Component;
		}
	}

	return nullptr;
}

void UAtlasCombatBlueprintLibrary::GetOwnedGameplayTagsFromActor(const AActor* Actor, FGameplayTagContainer& OutTags)
{
	OutTags.Reset();

	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemFromActor(Actor))
	{
		AbilitySystemComponent->GetOwnedGameplayTags(OutTags);
	}
}

bool UAtlasCombatBlueprintLibrary::ActorHasMatchingGameplayTag(const AActor* Actor, const FGameplayTag GameplayTag)
{
	if (Actor == nullptr || !GameplayTag.IsValid())
	{
		return false;
	}

	FGameplayTagContainer OwnedTags;
	GetOwnedGameplayTagsFromActor(Actor, OwnedTags);
	return OwnedTags.HasTag(GameplayTag);
}
