#include "Framework/QuestUI/AtlasQuestUIBlueprintLibrary.h"

#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Framework/QuestUI/AtlasQuestUISubsystem.h"
#include "GameFramework/PlayerController.h"

namespace AtlasQuestUIBlueprintLibraryPrivate
{
	ULocalPlayer* ResolveLocalPlayer(const UObject* WorldContextObject)
	{
		if (!WorldContextObject)
		{
			return nullptr;
		}

		if (const UWorld* World = WorldContextObject->GetWorld())
		{
			if (APlayerController* PlayerController = World->GetFirstPlayerController())
			{
				return PlayerController->GetLocalPlayer();
			}

			return World->GetFirstLocalPlayerFromController();
		}

		return nullptr;
	}
}

UAtlasQuestUISubsystem* UAtlasQuestUIBlueprintLibrary::GetQuestUISubsystem(const UObject* WorldContextObject)
{
	if (ULocalPlayer* LocalPlayer = AtlasQuestUIBlueprintLibraryPrivate::ResolveLocalPlayer(WorldContextObject))
	{
		return LocalPlayer->GetSubsystem<UAtlasQuestUISubsystem>();
	}

	return nullptr;
}

FAtlasQuestUIEntry UAtlasQuestUIBlueprintLibrary::GetTrackedQuestEntry(const UObject* WorldContextObject)
{
	if (const UAtlasQuestUISubsystem* QuestUISubsystem = GetQuestUISubsystem(WorldContextObject))
	{
		return QuestUISubsystem->GetTrackedQuestEntry();
	}

	return FAtlasQuestUIEntry();
}

TArray<FAtlasQuestUIEntry> UAtlasQuestUIBlueprintLibrary::GetQuestEntries(const UObject* WorldContextObject)
{
	if (const UAtlasQuestUISubsystem* QuestUISubsystem = GetQuestUISubsystem(WorldContextObject))
	{
		return QuestUISubsystem->GetQuestEntriesCopy();
	}

	return TArray<FAtlasQuestUIEntry>();
}

bool UAtlasQuestUIBlueprintLibrary::SetTrackedQuest(const UObject* WorldContextObject, const FName QuestId)
{
	if (UAtlasQuestUISubsystem* QuestUISubsystem = GetQuestUISubsystem(WorldContextObject))
	{
		return QuestUISubsystem->SetTrackedQuest(QuestId);
	}

	return false;
}

bool UAtlasQuestUIBlueprintLibrary::CycleTrackedQuest(const UObject* WorldContextObject, const int32 Direction)
{
	if (UAtlasQuestUISubsystem* QuestUISubsystem = GetQuestUISubsystem(WorldContextObject))
	{
		return QuestUISubsystem->CycleTrackedQuest(Direction);
	}

	return false;
}

void UAtlasQuestUIBlueprintLibrary::RefreshQuestEntries(const UObject* WorldContextObject)
{
	if (UAtlasQuestUISubsystem* QuestUISubsystem = GetQuestUISubsystem(WorldContextObject))
	{
		QuestUISubsystem->RefreshQuestEntries();
	}
}
