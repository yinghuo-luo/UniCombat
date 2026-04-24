// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Combat/Interfaces/AtlasExorcisableTargetInterface.h"
#include "Combat/Interfaces/AtlasRevealableTargetInterface.h"
#include "Combat/Interfaces/AtlasRitualTargetInterface.h"
#include "Combat/Interfaces/AtlasSoulBellReactiveInterface.h"
#include "Components/ActorComponent.h"
#include "AtlasOccultInteractableComponent.generated.h"

class UAbilitySystemComponent;

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class UNICOMBAT_API UAtlasOccultInteractableComponent : public UActorComponent,
	public IAtlasRevealableTargetInterface,
	public IAtlasSoulBellReactiveInterface,
	public IAtlasExorcisableTargetInterface,
	public IAtlasRitualTargetInterface
{
	GENERATED_BODY()

public:
	UAtlasOccultInteractableComponent();

	virtual void BeginPlay() override;

	//所有必需的仪式步骤是否完成
	UFUNCTION(BlueprintPure, Category = "Occult")
	bool AreAllRequiredRitualStepsComplete() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FGameplayTagContainer GrantedInteractionTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reveal")
	bool bCanBeRevealedByWater = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reveal")
	FGameplayTagContainer RevealGrantedTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoulBell")
	bool bCanReactToBell = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoulBell")
	bool bBellOpensExorciseWindow = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoulBell")
	FGameplayTagContainer SoulBellGrantedTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exorcise")
	bool bCanBeExorcisedDirectly = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ritual")
	bool bIsRitualTarget = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ritual")
	FGameplayTagContainer RequiredRitualStepTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ritual")
	FGameplayTagContainer RitualCompletedGrantedTags;

	UPROPERTY(BlueprintReadOnly, Category = "Occult")
	FGameplayTagContainer RuntimeStateTags;

	UPROPERTY(BlueprintReadOnly, Category = "Occult")
	FGameplayTagContainer CompletedRitualStepTags;

	virtual bool CanBeRevealed_Implementation(const FAtlasRevealContext& RevealContext) const override;
	virtual void HandleRevealTriggered_Implementation(const FAtlasRevealContext& RevealContext) override;
	virtual FGameplayTagContainer GetRevealResponseTags_Implementation() const override;

	virtual bool CanReactToSoulBell_Implementation(const FAtlasSoulBellContext& BellContext) const override;
	virtual void HandleSoulBellTriggered_Implementation(const FAtlasSoulBellContext& BellContext) override;
	virtual bool CanOpenExecutionWindowFromBell_Implementation() const override;
	virtual FGameplayTagContainer GetSoulBellResponseTags_Implementation() const override;

	virtual bool CanBeExorcised_Implementation(const FAtlasExorciseContext& ExorciseContext) const override;
	virtual bool IsExorciseWindowOpen_Implementation() const override;
	virtual void HandleExorcised_Implementation(const FAtlasExorciseContext& ExorciseContext) override;

	virtual bool CanAcceptRitualStep_Implementation(FGameplayTag RitualStepTag, const FAtlasRitualContext& RitualContext) const override;
	virtual void HandleRitualStepApplied_Implementation(FGameplayTag RitualStepTag, const FAtlasRitualContext& RitualContext) override;
	virtual FGameplayTagContainer GetCompletedRitualStepTags_Implementation() const override;

private:
	UAbilitySystemComponent* GetOwnerAbilitySystemComponent() const;
	void AddStateTags(const FGameplayTagContainer& TagsToAdd);
	void RemoveStateTags(const FGameplayTagContainer& TagsToRemove);
	bool HasStateTag(FGameplayTag TagToFind) const;
};
