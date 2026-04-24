// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/Characters/AtlasEnemyCombatCharacter.h"

#include "Combat/Data/AtlasEnemyCombatConfigData.h"

bool AAtlasEnemyCombatCharacter::TryUseCombatAbilityByTag(const FGameplayTag AbilityTag)
{
	return TryActivateAbilityByTag(AbilityTag);
}

bool AAtlasEnemyCombatCharacter::TryUseConfiguredAbilityForDistance(const float DistanceToTarget, FGameplayTag& OutChosenAbilityTag)
{
	OutChosenAbilityTag = FGameplayTag::EmptyTag;

	if (EnemyCombatConfig == nullptr)
	{
		return false;
	}

	for (const FAtlasEnemyAbilityEntry& AbilityEntry : EnemyCombatConfig->AbilityEntries)
	{
		if (!AbilityEntry.AbilityTag.IsValid())
		{
			continue;
		}

		if (DistanceToTarget >= AbilityEntry.MinRange && DistanceToTarget <= AbilityEntry.MaxRange)
		{
			OutChosenAbilityTag = AbilityEntry.AbilityTag;
			return TryUseCombatAbilityByTag(OutChosenAbilityTag);
		}
	}

	return false;
}
