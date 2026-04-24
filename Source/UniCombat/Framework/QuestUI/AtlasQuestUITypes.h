#pragma once

#include "CoreMinimal.h"
#include "QuestSystem/QuestTypes.h"
#include "AtlasQuestUITypes.generated.h"

USTRUCT(BlueprintType)
struct FAtlasQuestUIEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QuestUI")
	FName QuestId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QuestUI")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QuestUI")
	FText Objective;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QuestUI")
	EQuestState State = EQuestState::Inactive;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QuestUI")
	bool bTracked = false;

	bool IsValid() const
	{
		return QuestId != NAME_None;
	}
};
