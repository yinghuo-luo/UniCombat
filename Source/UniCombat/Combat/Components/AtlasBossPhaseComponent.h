// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "AtlasBossPhaseComponent.generated.h"

class UAbilitySystemComponent;

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class UNICOMBAT_API UAtlasBossPhaseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAtlasBossPhaseComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Boss")
	void EnterPhase(FGameplayTag NewPhaseTag, bool bClearPreviousPhase = true);

	UFUNCTION(BlueprintCallable, Category = "Boss")
	void AddStateTag(FGameplayTag StateTag);

	UFUNCTION(BlueprintCallable, Category = "Boss")
	void RemoveStateTag(FGameplayTag StateTag);

	UFUNCTION(BlueprintPure, Category = "Boss")
	bool HasStateTag(FGameplayTag StateTag) const;

	UFUNCTION(BlueprintCallable, Category = "Boss")
	void OpenExecutionWindow(float DurationSeconds = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Boss")
	void CloseExecutionWindow();

	UFUNCTION(BlueprintPure, Category = "Boss")
	FGameplayTag GetCurrentPhaseTag() const { return CurrentPhaseTag; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss")
	FGameplayTag InitialPhaseTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss")
	FGameplayTagContainer ExecutionWindowTags;

	UPROPERTY(BlueprintReadOnly, Category = "Boss")
	FGameplayTag CurrentPhaseTag;

	UPROPERTY(BlueprintReadOnly, Category = "Boss")
	FGameplayTagContainer RuntimeStateTags;

private:
	UAbilitySystemComponent* GetOwnerAbilitySystemComponent() const;
	void ApplyLooseTag(FGameplayTag GameplayTag);
	void RemoveLooseTag(FGameplayTag GameplayTag);
	void HandleExecutionWindowExpired();

	FTimerHandle ExecutionWindowTimerHandle;
	bool bExecutionWindowOpen = false;
};
