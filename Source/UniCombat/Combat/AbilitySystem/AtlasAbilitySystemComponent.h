// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AtlasAbilitySystemComponent.generated.h"

class UAtlasAbilitySet;
class UGameplayEffect;
struct FAtlasAbilitySet_GrantedHandles;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UNICOMBAT_API UAtlasAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UAtlasAbilitySystemComponent();

	void InitializeAbilityActorInfo(AActor* OwnerActor, AActor* AvatarActor);

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	bool TryActivateAbilityByInputTag(FGameplayTag InputTag, bool bAllowRemoteActivation = true);

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void ReleaseAbilityByInputTag(FGameplayTag InputTag);

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	bool TryActivateAbilityByTagExact(FGameplayTag AbilityTag, bool bAllowRemoteActivation = true);

	void GrantAbilitySet(const UAtlasAbilitySet* AbilitySet, UObject* SourceObject,
		FAtlasAbilitySet_GrantedHandles* OutGrantedHandles = nullptr);

protected:
	bool AbilityInputTagPressed(FGameplayTag InputTag, bool bAllowRemoteActivation);
	void AbilityInputTagReleased(FGameplayTag InputTag);
};
