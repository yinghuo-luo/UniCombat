// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/Components/AtlasBossPhaseComponent.h"

#include "AbilitySystemComponent.h"
#include "Combat/AtlasGameplayTags.h"
#include "Combat/Library/AtlasCombatBlueprintLibrary.h"
#include "TimerManager.h"

UAtlasBossPhaseComponent::UAtlasBossPhaseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAtlasBossPhaseComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!ExecutionWindowTags.HasTagExact(TAG_State_Executable))
	{
		ExecutionWindowTags.AddTag(TAG_State_Executable);
	}

	if (!ExecutionWindowTags.HasTagExact(TAG_State_ExorciseWindow))
	{
		ExecutionWindowTags.AddTag(TAG_State_ExorciseWindow);
	}

	if (InitialPhaseTag.IsValid())
	{
		EnterPhase(InitialPhaseTag);
	}
}

void UAtlasBossPhaseComponent::EnterPhase(const FGameplayTag NewPhaseTag, const bool bClearPreviousPhase)
{
	if (!NewPhaseTag.IsValid())
	{
		return;
	}

	if (bClearPreviousPhase && CurrentPhaseTag.IsValid() && !(CurrentPhaseTag == NewPhaseTag))
	{
		RemoveLooseTag(CurrentPhaseTag);
		RuntimeStateTags.RemoveTag(CurrentPhaseTag);
	}

	CurrentPhaseTag = NewPhaseTag;
	ApplyLooseTag(CurrentPhaseTag);
	RuntimeStateTags.AddTag(CurrentPhaseTag);
}

void UAtlasBossPhaseComponent::AddStateTag(const FGameplayTag StateTag)
{
	if (!StateTag.IsValid())
	{
		return;
	}

	ApplyLooseTag(StateTag);
	RuntimeStateTags.AddTag(StateTag);
}

void UAtlasBossPhaseComponent::RemoveStateTag(const FGameplayTag StateTag)
{
	if (!StateTag.IsValid())
	{
		return;
	}

	RemoveLooseTag(StateTag);
	RuntimeStateTags.RemoveTag(StateTag);
}

bool UAtlasBossPhaseComponent::HasStateTag(const FGameplayTag StateTag) const
{
	if (!StateTag.IsValid())
	{
		return false;
	}

	if (const UAbilitySystemComponent* AbilitySystemComponent = GetOwnerAbilitySystemComponent())
	{
		if (AbilitySystemComponent->HasMatchingGameplayTag(StateTag))
		{
			return true;
		}
	}

	return RuntimeStateTags.HasTag(StateTag);
}

void UAtlasBossPhaseComponent::OpenExecutionWindow(const float DurationSeconds)
{
	if (!bExecutionWindowOpen)
	{
		for (const FGameplayTag& GameplayTag : ExecutionWindowTags)
		{
			AddStateTag(GameplayTag);
		}
	}

	bExecutionWindowOpen = true;

	if (DurationSeconds > 0.0f && GetWorld() != nullptr)
	{
		GetWorld()->GetTimerManager().SetTimer(
			ExecutionWindowTimerHandle,
			this,
			&ThisClass::HandleExecutionWindowExpired,
			DurationSeconds,
			false);
	}
}

void UAtlasBossPhaseComponent::CloseExecutionWindow()
{
	if (!bExecutionWindowOpen)
	{
		return;
	}

	bExecutionWindowOpen = false;

	for (const FGameplayTag& GameplayTag : ExecutionWindowTags)
	{
		RemoveStateTag(GameplayTag);
	}

	if (GetWorld() != nullptr)
	{
		GetWorld()->GetTimerManager().ClearTimer(ExecutionWindowTimerHandle);
	}
}

UAbilitySystemComponent* UAtlasBossPhaseComponent::GetOwnerAbilitySystemComponent() const
{
	return UAtlasCombatBlueprintLibrary::GetAbilitySystemFromActor(GetOwner());
}

void UAtlasBossPhaseComponent::ApplyLooseTag(const FGameplayTag GameplayTag)
{
	if (UAbilitySystemComponent* AbilitySystemComponent = GetOwnerAbilitySystemComponent())
	{
		AbilitySystemComponent->AddLooseGameplayTag(GameplayTag);
	}
}

void UAtlasBossPhaseComponent::RemoveLooseTag(const FGameplayTag GameplayTag)
{
	if (UAbilitySystemComponent* AbilitySystemComponent = GetOwnerAbilitySystemComponent())
	{
		if (AbilitySystemComponent->HasMatchingGameplayTag(GameplayTag))
		{
			AbilitySystemComponent->RemoveLooseGameplayTag(GameplayTag);
		}
	}
}

void UAtlasBossPhaseComponent::HandleExecutionWindowExpired()
{
	CloseExecutionWindow();
}
