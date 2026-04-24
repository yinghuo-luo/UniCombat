#include "Framework/QuestUI/AtlasQuestUISubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Framework/QuestUI/AtlasQuestUISettings.h"
#include "QuestSystem/Core/QuestSubsystem.h"
#include "UI/Core/UIRootSubsystem.h"
#include "UI/HUD/HUDWidgets.h"
#include "UI/Menu/MenuWidgets.h"

namespace AtlasQuestUIPrivate
{
	int32 GetQuestStateSortPriority(const EQuestState QuestState)
	{
		switch (QuestState)
		{
		case EQuestState::Active:
			return 0;

		case EQuestState::Completed:
			return 1;

		case EQuestState::Failed:
			return 2;

		case EQuestState::Inactive:
		default:
			return 3;
		}
	}
}

void UAtlasQuestUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<UUIRootSubsystem>();

	const UAtlasQuestUISettings* Settings = GetDefault<UAtlasQuestUISettings>();
	if (Settings)
	{
		bAutoTrackFirstActiveQuestWhenUnset = Settings->bAutoTrackFirstActiveQuestWhenUnset;
		bAutoTrackNextActiveQuestWhenTrackedQuestStops = Settings->bAutoTrackNextActiveQuestWhenTrackedQuestStops;
	}

	if (UQuestSubsystem* QuestSubsystem = GetQuestSubsystem())
	{
		QuestSubsystem->OnQuestObjectiveUpdated.AddDynamic(this, &ThisClass::HandleQuestObjectiveUpdated);
		QuestSubsystem->OnQuestStateChanged.AddDynamic(this, &ThisClass::HandleQuestStateChanged);
		QuestSubsystem->OnQuestRestored.AddDynamic(this, &ThisClass::HandleQuestRestored);
	}

	if (UUIRootSubsystem* UIRootSubsystem = GetUIRootSubsystem())
	{
		UIRootSubsystem->OnHUDReady.AddDynamic(this, &ThisClass::HandleHUDReady);
		UIRootSubsystem->OnMenuReady.AddDynamic(this, &ThisClass::HandleMenuReady);
	}

	RefreshQuestEntries();
}

void UAtlasQuestUISubsystem::Deinitialize()
{
	if (UQuestSubsystem* QuestSubsystem = GetQuestSubsystem())
	{
		QuestSubsystem->OnQuestObjectiveUpdated.RemoveDynamic(this, &ThisClass::HandleQuestObjectiveUpdated);
		QuestSubsystem->OnQuestStateChanged.RemoveDynamic(this, &ThisClass::HandleQuestStateChanged);
		QuestSubsystem->OnQuestRestored.RemoveDynamic(this, &ThisClass::HandleQuestRestored);
	}

	if (UUIRootSubsystem* UIRootSubsystem = GetUIRootSubsystem())
	{
		UIRootSubsystem->OnHUDReady.RemoveDynamic(this, &ThisClass::HandleHUDReady);
		UIRootSubsystem->OnMenuReady.RemoveDynamic(this, &ThisClass::HandleMenuReady);
	}

	CachedQuestTitles.Reset();
	QuestEntries.Reset();
	TrackedQuestId = NAME_None;

	Super::Deinitialize();
}

bool UAtlasQuestUISubsystem::IsQuestTracked(const FName QuestId) const
{
	return TrackedQuestId == QuestId && QuestId != NAME_None;
}

FAtlasQuestUIEntry UAtlasQuestUISubsystem::GetTrackedQuestEntry() const
{
	return TrackedQuestId.IsNone() ? FAtlasQuestUIEntry() : BuildQuestEntry(TrackedQuestId);
}

bool UAtlasQuestUISubsystem::SetTrackedQuest(const FName QuestId)
{
	if (!IsQuestTrackable(QuestId))
	{
		return false;
	}

	CacheQuestTitle(QuestId);

	if (TrackedQuestId == QuestId)
	{
		return true;
	}

	TrackedQuestId = QuestId;
	if (!QuestEntries.ContainsByPredicate([QuestId](const FAtlasQuestUIEntry& Entry)
	{
		return Entry.QuestId == QuestId;
	}))
	{
		QuestEntries.Add(BuildQuestEntry(QuestId));
	}

	UpdateTrackedFlags();
	OnTrackedQuestChanged.Broadcast(TrackedQuestId);
	OnQuestUIDataChanged.Broadcast();
	RefreshWidgets();
	return true;
}

bool UAtlasQuestUISubsystem::CycleTrackedQuest(const int32 Direction)
{
	const TArray<FName> ActiveQuestIds = GetActiveQuestIds();
	if (ActiveQuestIds.IsEmpty())
	{
		if (TrackedQuestId != NAME_None)
		{
			TrackedQuestId = NAME_None;
			UpdateTrackedFlags();
			OnTrackedQuestChanged.Broadcast(TrackedQuestId);
			OnQuestUIDataChanged.Broadcast();
			RefreshWidgets();
		}

		return false;
	}

	const int32 Step = Direction < 0 ? -1 : 1;
	const int32 CurrentIndex = ActiveQuestIds.IndexOfByKey(TrackedQuestId);

	int32 NextIndex = Step > 0 ? 0 : ActiveQuestIds.Num() - 1;
	if (CurrentIndex != INDEX_NONE)
	{
		NextIndex = (CurrentIndex + Step + ActiveQuestIds.Num()) % ActiveQuestIds.Num();
	}

	return SetTrackedQuest(ActiveQuestIds[NextIndex]);
}

void UAtlasQuestUISubsystem::RefreshQuestEntries()
{
	TArray<FAtlasQuestUIEntry> NewEntries;
	const TArray<FName> RuntimeQuestIds = GetRuntimeQuestIds();
	NewEntries.Reserve(RuntimeQuestIds.Num());

	for (const FName QuestId : RuntimeQuestIds)
	{
		if (QuestId.IsNone())
		{
			continue;
		}

		CacheQuestTitle(QuestId);
		NewEntries.Add(BuildQuestEntry(QuestId));
	}

	NewEntries.Sort([](const FAtlasQuestUIEntry& Left, const FAtlasQuestUIEntry& Right)
	{
		const int32 LeftPriority = AtlasQuestUIPrivate::GetQuestStateSortPriority(Left.State);
		const int32 RightPriority = AtlasQuestUIPrivate::GetQuestStateSortPriority(Right.State);
		if (LeftPriority != RightPriority)
		{
			return LeftPriority < RightPriority;
		}

		return Left.QuestId.ToString() < Right.QuestId.ToString();
	});

	QuestEntries = MoveTemp(NewEntries);

	const FName PreviousTrackedQuestId = TrackedQuestId;
	const bool bTrackedQuestChanged = UpdateTrackedQuestFromEntries();
	UpdateTrackedFlags();

	if (bTrackedQuestChanged || PreviousTrackedQuestId != TrackedQuestId)
	{
		OnTrackedQuestChanged.Broadcast(TrackedQuestId);
	}

	OnQuestUIDataChanged.Broadcast();
	RefreshWidgets();
}

bool UAtlasQuestUISubsystem::IsQuestTrackable(const FName QuestId) const
{
	if (QuestId.IsNone())
	{
		return false;
	}

	if (const UQuestSubsystem* QuestSubsystem = GetQuestSubsystem())
	{
		return QuestSubsystem->IsQuestActive(QuestId);
	}

	return false;
}

void UAtlasQuestUISubsystem::HandleQuestObjectiveUpdated(const FName QuestId, const FText& ObjectiveText)
{
	(void)ObjectiveText;
	CacheQuestTitle(QuestId);
	RefreshQuestEntries();
}

void UAtlasQuestUISubsystem::HandleQuestStateChanged(const FName QuestId, const EQuestState QuestState)
{
	if (QuestState != EQuestState::Inactive)
	{
		CacheQuestTitle(QuestId);
	}

	RefreshQuestEntries();
}

void UAtlasQuestUISubsystem::HandleQuestRestored(const FName QuestId)
{
	CacheQuestTitle(QuestId);
	RefreshQuestEntries();
}

void UAtlasQuestUISubsystem::HandleHUDReady(UGameHUDWidget* HUD)
{
	if (HUD)
	{
		HUD->RefreshHUD();
	}
}

void UAtlasQuestUISubsystem::HandleMenuReady(UGameMenuWidget* Menu)
{
	if (Menu)
	{
		Menu->RefreshMenu();
	}
}

UQuestSubsystem* UAtlasQuestUISubsystem::GetQuestSubsystem() const
{
	UGameInstance* GameInstance = nullptr;
	if (const ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		GameInstance = LocalPlayer->GetGameInstance();
	}

	return GameInstance ? GameInstance->GetSubsystem<UQuestSubsystem>() : nullptr;
}

UUIRootSubsystem* UAtlasQuestUISubsystem::GetUIRootSubsystem() const
{
	return GetLocalPlayer() ? GetLocalPlayer()->GetSubsystem<UUIRootSubsystem>() : nullptr;
}

FAtlasQuestUIEntry UAtlasQuestUISubsystem::BuildQuestEntry(const FName QuestId) const
{
	FAtlasQuestUIEntry Entry;
	Entry.QuestId = QuestId;
	Entry.bTracked = TrackedQuestId == QuestId;

	if (const UQuestSubsystem* QuestSubsystem = GetQuestSubsystem())
	{
		Entry.State = QuestSubsystem->GetQuestState(QuestId);
		Entry.Objective = QuestSubsystem->GetCurrentObjectiveText(QuestId);

		if (const FQuestRow* QuestRow = QuestSubsystem->FindQuestDefinition(QuestId))
		{
			Entry.Title = QuestRow->Title;
		}
	}

	if (Entry.Title.IsEmpty())
	{
		if (const FText* CachedTitle = CachedQuestTitles.Find(QuestId))
		{
			Entry.Title = *CachedTitle;
		}
	}

	if (Entry.Title.IsEmpty())
	{
		Entry.Title = FText::FromString(QuestId.ToString());
	}

	return Entry;
}

TArray<FName> UAtlasQuestUISubsystem::GetRuntimeQuestIds() const
{
	TArray<FName> RuntimeQuestIds;

	if (const UQuestSubsystem* QuestSubsystem = GetQuestSubsystem())
	{
		FQuestPersistenceSnapshot Snapshot;
		QuestSubsystem->ExportPersistenceSnapshot(Snapshot);
		Snapshot.RuntimeQuestStates.GenerateKeyArray(RuntimeQuestIds);
	}

	return RuntimeQuestIds;
}

TArray<FName> UAtlasQuestUISubsystem::GetActiveQuestIds() const
{
	TArray<FName> ActiveQuestIds;
	for (const FAtlasQuestUIEntry& Entry : QuestEntries)
	{
		if (Entry.State == EQuestState::Active)
		{
			ActiveQuestIds.Add(Entry.QuestId);
		}
	}

	return ActiveQuestIds;
}

void UAtlasQuestUISubsystem::CacheQuestTitle(const FName QuestId)
{
	if (QuestId.IsNone() || CachedQuestTitles.Contains(QuestId))
	{
		return;
	}

	if (const UQuestSubsystem* QuestSubsystem = GetQuestSubsystem())
	{
		if (const FQuestRow* QuestRow = QuestSubsystem->FindQuestDefinition(QuestId))
		{
			if (!QuestRow->Title.IsEmpty())
			{
				CachedQuestTitles.Add(QuestId, QuestRow->Title);
			}
		}
	}
}

void UAtlasQuestUISubsystem::RefreshWidgets() const
{
	if (UUIRootSubsystem* UIRootSubsystem = GetUIRootSubsystem())
	{
		if (UGameHUDWidget* HUD = UIRootSubsystem->GetHUD())
		{
			HUD->RefreshHUD();
		}

		if (UGameMenuWidget* Menu = UIRootSubsystem->GetMenu())
		{
			Menu->RefreshMenu();
		}
	}
}

bool UAtlasQuestUISubsystem::UpdateTrackedQuestFromEntries()
{
	const FName PreviousTrackedQuestId = TrackedQuestId;

	if (!TrackedQuestId.IsNone())
	{
		if (IsQuestTrackable(TrackedQuestId))
		{
			return false;
		}

		if (!bAutoTrackNextActiveQuestWhenTrackedQuestStops)
		{
			TrackedQuestId = NAME_None;
			return TrackedQuestId != PreviousTrackedQuestId;
		}
	}
	else if (!bAutoTrackFirstActiveQuestWhenUnset)
	{
		return false;
	}

	TrackedQuestId = NAME_None;
	for (const FAtlasQuestUIEntry& Entry : QuestEntries)
	{
		if (Entry.State == EQuestState::Active)
		{
			TrackedQuestId = Entry.QuestId;
			break;
		}
	}

	return TrackedQuestId != PreviousTrackedQuestId;
}

void UAtlasQuestUISubsystem::UpdateTrackedFlags()
{
	for (FAtlasQuestUIEntry& Entry : QuestEntries)
	{
		Entry.bTracked = Entry.QuestId == TrackedQuestId;
	}
}
