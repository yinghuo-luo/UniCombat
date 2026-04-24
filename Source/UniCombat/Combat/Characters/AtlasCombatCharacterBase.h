// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Combat/AtlasCombatTypes.h"
#include "Combat/Interfaces/AtlasCombatantInterface.h"
#include "Combat/Interfaces/AtlasHitReceiverInterface.h"
#include "Combat/Interfaces/AtlasTargetableInterface.h"
#include "GameFramework/Character.h"
#include "AtlasCombatCharacterBase.generated.h"

class UAtlasAbilitySystemComponent;
class UAtlasCombatAttributeSet;
class UAtlasCharacterConfigData;
class UAtlasTalentTreeComponent;
class UAtlasWeaponComponent;
struct FOnAttributeChangeData;

UCLASS(Abstract)
class UNICOMBAT_API AAtlasCombatCharacterBase : public ACharacter, public IAbilitySystemInterface,
	public IAtlasCombatantInterface, public IAtlasTargetableInterface, public IAtlasHitReceiverInterface
{
	GENERATED_BODY()

public:
	AAtlasCombatCharacterBase();

	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//此处用于绑定按键等，激活/结束能力
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool TryActivateAbilityByInputTag(FGameplayTag InputTag);
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ReleaseAbilityInputTag(FGameplayTag InputTag);

	//通过标签直接执行能力
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool TryActivateAbilityByTag(FGameplayTag AbilityTag);

	//基本属性及组件
	UFUNCTION(BlueprintPure, Category = "Combat")
	UAtlasAbilitySystemComponent* GetAtlasAbilitySystemComponent() const { return AbilitySystemComponent; }
	UFUNCTION(BlueprintPure, Category = "Combat")
	const UAtlasCombatAttributeSet* GetCombatAttributes() const { return CombatAttributes; }
	UFUNCTION(BlueprintPure, Category = "Combat")
	UAtlasWeaponComponent* GetWeaponComponent() const { return WeaponComponent; }
	UFUNCTION(BlueprintPure, Category = "Combat")
	UAtlasTalentTreeComponent* GetTalentTreeComponent() const { return TalentTreeComponent; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetCurrentHealth() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetMaxHealthValue() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void BP_OnCombatHit(const FAtlasCombatHitData& HitData);

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void BP_OnCombatDeath(const FAtlasCombatHitData& HitData);

protected:
	//基本属性及组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAtlasAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAtlasCombatAttributeSet> CombatAttributes;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAtlasWeaponComponent> WeaponComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAtlasTalentTreeComponent> TalentTreeComponent;

	//默认技能及其他设置
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UAtlasCharacterConfigData> CharacterConfig = nullptr;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IsDead, Category = "Combat")
	bool bIsDead = false;

	//阵营
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	EAtlasCombatFaction CombatFaction = EAtlasCombatFaction::Neutral;
	
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	EAtlasTargetCategory TargetCategory = EAtlasTargetCategory::Normal;

	//默认标签区分（boss等）使用TAG_Type_XXX
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayTagContainer DefaultCombatTags;

	//初始化ASC的相关设置以及根据Config设计默认技能等其他相关参数（ApplyCharacterConfig）
	virtual void InitializeCombatActor();
	virtual void ApplyCharacterConfig();

	/*IAtlasCombatantInterface接口为战斗参与者通用接口
	GetCombatFaction()：阵营
	IsAlive()：是否存活
	IsCombatTargetable()：能不能被当战斗目标
	GetCombatAbilitySystemComponent()：拿到 ASC
	GetOwnedCombatTags()：拿到当前战斗标签
	GetCombatAimPoint()：拿到瞄准点/命中参考点*/
	//IAtlasCombatantInterface接口重写，
	virtual EAtlasCombatFaction GetCombatFaction_Implementation() const override;
	virtual bool IsAlive_Implementation() const override;
	virtual bool IsCombatTargetable_Implementation() const override;
	virtual UAbilitySystemComponent* GetCombatAbilitySystemComponent_Implementation() const override;
	virtual FGameplayTagContainer GetOwnedCombatTags_Implementation() const override;
	virtual FVector GetCombatAimPoint_Implementation(FName PointName) const override;

	/*IAtlasTargetableInterface可以理解成“可被选中/锁定目标接口”。
	它关注的不是“这个对象会不会打架”，而是“这个对象作为目标时，
	外部系统该怎么锁、锁哪里、显示什么名字、属于什么目标类型”。
	CanBeLockedon（)：能不能被锁定
	GetLockTargetLocation(）:锁定点坐标
	GetTargetDisplayName（）:目标显示名
	GetTargetCategory():目标分类，类型来自EAtlasTargetCategory，
	比如Normat/ Elite/ Boss/ Ghost Corpse / Summon/ Device.*/
	//IAtlasTargetableInterface接口重写
	virtual bool CanBeLockedOn_Implementation() const override;
	virtual FVector GetLockTargetLocation_Implementation() const override;
	virtual FText GetTargetDisplayName_Implementation() const override;
	virtual EAtlasTargetCategory GetTargetCategory_Implementation() const override;

	/*RecelverInterface理解成 果接收接： 不负责算伤害，而是在
	历害已经结算出来之后，告诉目标对象：“你被打中了你死了没有这次命
	中能不能把你打出硬直"。
	HandleGamepLayHit：收到一次命中
	HandLeGamepLayDeath：这次命中导致死亡
	CanBeInterruptedByHit：这次命中能不能打断当前状态*/
	//IAtlasHitReceiverInterface接口重写
	virtual void HandleGameplayHit_Implementation(const FAtlasCombatHitData& HitData) override;
	virtual void HandleGameplayDeath_Implementation(const FAtlasCombatHitData& HitData) override;
	virtual bool CanBeInterruptedByHit_Implementation(const FAtlasCombatHitData& HitData) const override;

	//开始死亡处理
	UFUNCTION(BlueprintNativeEvent, Category = "Combat")
	void OnDeathStarted(const FAtlasCombatHitData& HitData);
	virtual void OnDeathStarted_Implementation(const FAtlasCombatHitData& HitData);

private:
	UFUNCTION()
	void OnRep_IsDead();

	
	bool bCombatInitialized = false;
	FGameplayTag GetFactionTag() const;
};
