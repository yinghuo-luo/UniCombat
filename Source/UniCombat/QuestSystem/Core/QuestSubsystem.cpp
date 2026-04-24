// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestSubsystem.h"

#include "QuestSystem/QuestTypes.h"

#include "Engine/DataTable.h"
#include "QuestActionExecutor.h"
#include "QuestConditionEvaluator.h"
#include "QuestSystem/Objects/QuestInterfaces.h"
#include "QuestSystem/Setting/QuestSystemSettings.h"

DEFINE_LOG_CATEGORY(LogQuest);

UQuestSubsystem::UQuestSubsystem()
{
}

void UQuestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//读取数据
	const UQuestSystemSettings* Settings = GetDefault<UQuestSystemSettings>();
	if (Settings)
	{
		QuestRegistryTableAsset = Settings->QuestRegistryTableAsset;
	}
	
	//条件评估器
	ConditionEvaluator = NewObject<UQuestConditionEvaluator>(this);
	ActionExecutor = NewObject<UQuestActionExecutor>(this);

	if (ConditionEvaluator)
	{
		ConditionEvaluator->Initialize(this);
	}

	if (ActionExecutor)
	{
		ActionExecutor->Initialize(this);
	}

	LoadConfiguredDataTables();
}

void UQuestSubsystem::Deinitialize()
{
	RegisteredTargets.Reset();
	Super::Deinitialize();
}

void UQuestSubsystem::InitializeQuestRegistryTable(UDataTable* InQuestRegistryTable)
{
	QuestRegistryTable = InQuestRegistryTable;
	QuestDataSources.Reset();
	LoadedQuestBundles.Reset();
	RebuildQuestSourceCache();
	RefreshLoadedRuntimeQuests();
}

void UQuestSubsystem::InitializeQuestDataTables(UDataTable* InQuestTable, UDataTable* InNodeTable, UDataTable* InConditionTable, UDataTable* InActionTable)
{
	FQuestDataBundle Bundle;
	FString FailureReason;
	if (!BuildQuestBundle(NAME_None, InQuestTable, InNodeTable, InConditionTable, InActionTable, Bundle, &FailureReason))
	{
		UE_LOG(LogQuest, Warning, TEXT("Failed to initialize quest data tables: %s"), *FailureReason);
		return;
	}

	const FName QuestId = Bundle.QuestDefinition.QuestId;
	LoadedQuestBundles.Add(QuestId, MoveTemp(Bundle));
	UE_LOG(LogQuest, Log, TEXT("Manually initialized quest data tables for '%s'."), *QuestId.ToString());

	RefreshLoadedRuntimeQuests();
}

bool UQuestSubsystem::AcceptQuest(const FName QuestId)
{
	if (const FQuestRuntimeState* ExistingState = RuntimeQuestStates.Find(QuestId))
	{
		switch (ExistingState->State)
		{
		case EQuestState::Completed:
			UE_LOG(LogQuest, Warning, TEXT("Quest '%s' is already completed and cannot be accepted again."), *QuestId.ToString());
			break;

		case EQuestState::Failed:
			UE_LOG(LogQuest, Warning, TEXT("Quest '%s' is already failed and cannot be accepted again."), *QuestId.ToString());
			break;

		case EQuestState::Active:
			UE_LOG(LogQuest, Warning, TEXT("Quest '%s' is already active and cannot be accepted again."), *QuestId.ToString());
			break;

		default:
			UE_LOG(LogQuest, Warning, TEXT("Quest '%s' already exists in runtime state."), *QuestId.ToString());
			break;
		}

		return false;
	}

	FString FailureReason;
	if (!EnsureQuestBundleLoaded(QuestId, &FailureReason))
	{
		UE_LOG(LogQuest, Warning, TEXT("Failed to accept quest '%s': %s"), *QuestId.ToString(), *FailureReason);
		return false;
	}

	const FQuestRow* QuestRow = FindQuestDefinition(QuestId);
	if (!QuestRow)
	{
		UE_LOG(LogQuest, Warning, TEXT("Quest definition '%s' was not found."), *QuestId.ToString());
		return false;
	}

	FQuestRuntimeState RuntimeState;
	RuntimeState.QuestId = QuestId;
	RuntimeState.State = EQuestState::Active;
	RuntimeQuestStates.Add(QuestId, RuntimeState);

	const FQuestRuntimeState* RuntimeStatePtr = RuntimeQuestStates.Find(QuestId);
	if (!RuntimeStatePtr)
	{
		return false;
	}

	if (ConditionEvaluator && !ConditionEvaluator->EvaluateConditionList(QuestRow->StartConditionIds, *RuntimeStatePtr, nullptr))
	{
		UE_LOG(LogQuest, Warning, TEXT("Start conditions for quest '%s' were not met."), *QuestId.ToString());
		RuntimeQuestStates.Remove(QuestId);
		return false;
	}

	UE_LOG(LogQuest, Log, TEXT("Accepted quest '%s'."), *QuestId.ToString());
	OnQuestStateChanged.Broadcast(QuestId, EQuestState::Active);

	if (!QuestRow->RootNodeId.IsNone())
	{
		QueueNodeForActivation(QuestId, QuestRow->RootNodeId);
		TryActivatePendingNodes(QuestId, nullptr);
	}

	RefreshQuestObjective(QuestId);
	return true;
}

bool UQuestSubsystem::StartQuestNode(const FName QuestId, const FName NodeId)
{
	return TryActivateNode(QuestId, NodeId, nullptr);
}

void UQuestSubsystem::SubmitQuestEvent(const FQuestEventPayload& EventPayload)
{
	UE_LOG(LogQuest, Log, TEXT("Received quest event. Type=%s Target=%s Quest=%s Event=%s"),
		*StaticEnum<EQuestEventType>()->GetNameStringByValue(static_cast<int64>(EventPayload.EventType)),
		*EventPayload.TargetId.ToString(),
		*EventPayload.QuestId.ToString(),
		*EventPayload.EventName.ToString());

	TArray<FName> QuestIdsToEvaluate;
	if (!EventPayload.QuestId.IsNone())
	{
		QuestIdsToEvaluate.Add(EventPayload.QuestId);
	}
	else
	{
		RuntimeQuestStates.GenerateKeyArray(QuestIdsToEvaluate);
	}

	for (const FName QuestId : QuestIdsToEvaluate)
	{
		if (IsQuestActive(QuestId))
		{
			EvaluateQuestProgress(QuestId, &EventPayload);
		}
	}

	OnQuestEventProcessed.Broadcast(EventPayload);
}

void UQuestSubsystem::SubmitChoice(const FName QuestId, const FName ChoiceKey, const FName ChoiceResult)
{
	if (FQuestRuntimeState* RuntimeState = FindQuestRuntime(QuestId))
	{
		RuntimeState->SelectedChoices.Add(ChoiceKey, ChoiceResult);

		FQuestEventPayload Payload;
		Payload.EventType = EQuestEventType::ChoiceMade;
		Payload.QuestId = QuestId;
		Payload.ChoiceKey = ChoiceKey;
		Payload.ChoiceResult = ChoiceResult;
		Payload.EventName = ChoiceResult;

		UE_LOG(LogQuest, Log, TEXT("Quest '%s' recorded choice '%s' with result '%s'."),
			*QuestId.ToString(),
			*ChoiceKey.ToString(),
			*ChoiceResult.ToString());

		SubmitQuestEvent(Payload);
	}
}


void UQuestSubsystem::ExportPersistenceSnapshot(FQuestPersistenceSnapshot& OutSnapshot) const
{
	OutSnapshot.RuntimeQuestStates = RuntimeQuestStates;
	OutSnapshot.WorldStateBools = WorldStateBools;
	OutSnapshot.WorldStateInts = WorldStateInts;
	OutSnapshot.InventoryItemCounts = InventoryItemCounts;
	OutSnapshot.TargetEnabledStates = TargetEnabledStates;
	OutSnapshot.TimeOfDay = CurrentTimeOfDay;
}

void UQuestSubsystem::ImportPersistenceSnapshot(const FQuestPersistenceSnapshot& Snapshot)
{
	RuntimeQuestStates = Snapshot.RuntimeQuestStates;
	WorldStateBools = Snapshot.WorldStateBools;
	WorldStateInts = Snapshot.WorldStateInts;
	InventoryItemCounts = Snapshot.InventoryItemCounts;
	TargetEnabledStates = Snapshot.TargetEnabledStates;
	CurrentTimeOfDay = Snapshot.TimeOfDay;

	SyncAllRegisteredTargets();

	const bool bCanResolveQuestData = QuestDataSources.Num() > 0 || LoadedQuestBundles.Num() > 0;
	TArray<FName> RestoredQuestIds;
	RuntimeQuestStates.GenerateKeyArray(RestoredQuestIds);
	for (const FName QuestId : RestoredQuestIds)
	{
		if (bCanResolveQuestData)
		{
			const EQuestState SavedState = GetQuestState(QuestId);
			if (SavedState == EQuestState::Completed || SavedState == EQuestState::Failed)
			{
				UnloadQuestBundle(QuestId);
			}
			else
			{
				FString FailureReason;
				if (!EnsureQuestBundleLoaded(QuestId, &FailureReason))
				{
					UE_LOG(LogQuest, Warning, TEXT("Failed to reload quest '%s' while restoring: %s"), *QuestId.ToString(), *FailureReason);
					RuntimeQuestStates.Remove(QuestId);
					continue;
				}

				if (FQuestRuntimeState* RuntimeState = RuntimeQuestStates.Find(QuestId))
				{
					SanitizeRuntimeState(QuestId, *RuntimeState);
				}

				if (!RuntimeQuestStates.Contains(QuestId))
				{
					continue;
				}

				RefreshQuestObjective(QuestId);
			}
		}

		HandleQuestStateRestored(QuestId);
		OnQuestRestored.Broadcast(QuestId);
	}
}

void UQuestSubsystem::RegisterQuestTarget(const FName TargetId, UObject* TargetObject)
{
	if (TargetId.IsNone() || !IsValid(TargetObject))
	{
		return;
	}

	if (const TWeakObjectPtr<UObject>* ExistingTarget = RegisteredTargets.Find(TargetId))
	{
		if (ExistingTarget->IsValid() && ExistingTarget->Get() != TargetObject)
		{
			UE_LOG(LogQuest, Warning, TEXT("Quest target '%s' was re-registered from '%s' to '%s'."),
				*TargetId.ToString(),
				*GetNameSafe(ExistingTarget->Get()),
				*GetNameSafe(TargetObject));
		}
	}

	RegisteredTargets.Add(TargetId, TargetObject);
	UE_LOG(LogQuest, Verbose, TEXT("Registered quest target '%s'."), *TargetId.ToString());

	bool bEnabledState = true;
	if (GetTargetEnabledState(TargetId, bEnabledState))
	{
		FQuestEventPayload EmptyPayload;
		ApplyTargetEnabledState(TargetId, bEnabledState, EmptyPayload);
	}
}

void UQuestSubsystem::UnregisterQuestTarget(const FName TargetId, const UObject* TargetObject)
{
	const TWeakObjectPtr<UObject>* ExistingTarget = RegisteredTargets.Find(TargetId);
	if (ExistingTarget && ExistingTarget->Get() == TargetObject)
	{
		RegisteredTargets.Remove(TargetId);
	}
}

UObject* UQuestSubsystem::GetRegisteredQuestTarget(const FName TargetId) const
{
	if (const TWeakObjectPtr<UObject>* Target = RegisteredTargets.Find(TargetId))
	{
		return Target->Get();
	}

	return nullptr;
}

bool UQuestSubsystem::IsQuestActive(const FName QuestId) const
{
	return GetQuestState(QuestId) == EQuestState::Active;
}

bool UQuestSubsystem::HasQuest(const FName QuestId) const
{
	return RuntimeQuestStates.Contains(QuestId);
}

EQuestState UQuestSubsystem::GetQuestState(const FName QuestId) const
{
	if (const FQuestRuntimeState* RuntimeState = RuntimeQuestStates.Find(QuestId))
	{
		return RuntimeState->State;
	}

	return EQuestState::Inactive;
}

EQuestNodeState UQuestSubsystem::GetNodeState(const FName QuestId, const FName NodeId) const
{
	if (const FQuestRuntimeState* RuntimeState = RuntimeQuestStates.Find(QuestId))
	{
		return RuntimeState->GetNodeStateValue(NodeId);
	}

	return EQuestNodeState::NotStarted;
}

FText UQuestSubsystem::GetCurrentObjectiveText(const FName QuestId) const
{
	if (const FQuestRuntimeState* RuntimeState = RuntimeQuestStates.Find(QuestId))
	{
		return RuntimeState->CurrentObjectiveText;
	}

	return FText::GetEmpty();
}

void UQuestSubsystem::SetCurrentObjectiveText(const FName QuestId, const FText& ObjectiveText)
{
	if (FQuestRuntimeState* RuntimeState = FindQuestRuntime(QuestId))
	{
		RuntimeState->CurrentObjectiveText = ObjectiveText;
		OnQuestObjectiveUpdated.Broadcast(QuestId, ObjectiveText);
	}
}

void UQuestSubsystem::SetQuestFlag(const FName QuestId, const FName FlagId, const bool bValue)
{
	if (FQuestRuntimeState* RuntimeState = FindQuestRuntime(QuestId))
	{
		RuntimeState->Flags.Add(FlagId, bValue);
		UE_LOG(LogQuest, Verbose, TEXT("Quest '%s' flag '%s' set to %s."), *QuestId.ToString(), *FlagId.ToString(), bValue ? TEXT("true") : TEXT("false"));
		EvaluateQuestProgress(QuestId, nullptr);
	}
}

bool UQuestSubsystem::GetQuestFlag(const FName QuestId, const FName FlagId) const
{
	if (const FQuestRuntimeState* RuntimeState = RuntimeQuestStates.Find(QuestId))
	{
		if (const bool* bValue = RuntimeState->Flags.Find(FlagId))
		{
			return *bValue;
		}
	}

	return false;
}

void UQuestSubsystem::AddQuestCounter(const FName QuestId, const FName CounterId, const int32 Delta)
{
	if (FQuestRuntimeState* RuntimeState = FindQuestRuntime(QuestId))
	{
		const int32 NewValue = RuntimeState->Counters.FindRef(CounterId) + Delta;
		RuntimeState->Counters.Add(CounterId, NewValue);
		UE_LOG(LogQuest, Verbose, TEXT("Quest '%s' counter '%s' changed by %d. Current value: %d."),
			*QuestId.ToString(),
			*CounterId.ToString(),
			Delta,
			NewValue);
		EvaluateQuestProgress(QuestId, nullptr);
	}
}

void UQuestSubsystem::SetQuestCounter(const FName QuestId, const FName CounterId, const int32 Value)
{
	if (FQuestRuntimeState* RuntimeState = FindQuestRuntime(QuestId))
	{
		RuntimeState->Counters.Add(CounterId, Value);
		UE_LOG(LogQuest, Verbose, TEXT("Quest '%s' counter '%s' set to %d."),
			*QuestId.ToString(),
			*CounterId.ToString(),
			Value);
		EvaluateQuestProgress(QuestId, nullptr);
	}
}

int32 UQuestSubsystem::GetQuestCounter(const FName QuestId, const FName CounterId) const
{
	if (const FQuestRuntimeState* RuntimeState = RuntimeQuestStates.Find(QuestId))
	{
		if (const int32* Value = RuntimeState->Counters.Find(CounterId))
		{
			return *Value;
		}
	}

	return 0;
}

void UQuestSubsystem::SetInventoryItemCount(const FName ItemId, const int32 Value)
{
	InventoryItemCounts.Add(ItemId, Value);
	UE_LOG(LogQuest, Verbose, TEXT("Inventory item '%s' count set to %d."), *ItemId.ToString(), Value);

	TArray<FName> QuestIds;
	RuntimeQuestStates.GenerateKeyArray(QuestIds);
	for (const FName QuestId : QuestIds)
	{
		EvaluateQuestProgress(QuestId, nullptr);
	}
}

void UQuestSubsystem::AddInventoryItemCount(const FName ItemId, const int32 Delta)
{
	const int32 NewValue = InventoryItemCounts.FindRef(ItemId) + Delta;
	InventoryItemCounts.Add(ItemId, NewValue);
	UE_LOG(LogQuest, Verbose, TEXT("Inventory item '%s' changed by %d. Current value: %d."), *ItemId.ToString(), Delta, NewValue);

	TArray<FName> QuestIds;
	RuntimeQuestStates.GenerateKeyArray(QuestIds);
	for (const FName QuestId : QuestIds)
	{
		EvaluateQuestProgress(QuestId, nullptr);
	}
}

int32 UQuestSubsystem::GetInventoryItemCount(const FName ItemId) const
{
	return InventoryItemCounts.FindRef(ItemId);
}

void UQuestSubsystem::SetWorldStateBool(const FName Key, const bool bValue)
{
	WorldStateBools.Add(Key, bValue);
	UE_LOG(LogQuest, Verbose, TEXT("World state bool '%s' set to %s."), *Key.ToString(), bValue ? TEXT("true") : TEXT("false"));

	TArray<FName> QuestIds;
	RuntimeQuestStates.GenerateKeyArray(QuestIds);
	for (const FName QuestId : QuestIds)
	{
		EvaluateQuestProgress(QuestId, nullptr);
	}
}

bool UQuestSubsystem::GetWorldStateBool(const FName Key) const
{
	if (const bool* bValue = WorldStateBools.Find(Key))
	{
		return *bValue;
	}

	return false;
}

void UQuestSubsystem::SetWorldStateInt(const FName Key, const int32 Value)
{
	WorldStateInts.Add(Key, Value);
	UE_LOG(LogQuest, Verbose, TEXT("World state int '%s' set to %d."), *Key.ToString(), Value);

	TArray<FName> QuestIds;
	RuntimeQuestStates.GenerateKeyArray(QuestIds);
	for (const FName QuestId : QuestIds)
	{
		EvaluateQuestProgress(QuestId, nullptr);
	}
}

int32 UQuestSubsystem::GetWorldStateInt(const FName Key) const
{
	if (const int32* Value = WorldStateInts.Find(Key))
	{
		return *Value;
	}

	return 0;
}

void UQuestSubsystem::SetCurrentTimeOfDay(const EQuestTimeOfDay NewTimeOfDay)
{
	CurrentTimeOfDay = NewTimeOfDay;

	FQuestEventPayload Payload;
	Payload.EventType = EQuestEventType::TimeOfDayChanged;
	Payload.IntValue = static_cast<int32>(NewTimeOfDay);

	SubmitQuestEvent(Payload);
}

EQuestTimeOfDay UQuestSubsystem::GetCurrentTimeOfDay() const
{
	return CurrentTimeOfDay;
}

void UQuestSubsystem::SetTargetEnabledState(const FName TargetId, const bool bEnabled, const FQuestEventPayload& ContextPayload)
{
	TargetEnabledStates.Add(TargetId, bEnabled);
	ApplyTargetEnabledState(TargetId, bEnabled, ContextPayload);
}

bool UQuestSubsystem::GetTargetEnabledState(const FName TargetId, bool& bOutEnabled) const
{
	if (const bool* bValue = TargetEnabledStates.Find(TargetId))
	{
		bOutEnabled = *bValue;
		return true;
	}

	return false;
}

void UQuestSubsystem::CompleteQuestById(const FName QuestId)
{
	if (FQuestRuntimeState* RuntimeState = FindQuestRuntime(QuestId))
	{
		if (RuntimeState->State == EQuestState::Completed)
		{
			return;
		}

		RuntimeState->State = EQuestState::Completed;
		RuntimeState->ActiveNodeIds.Reset();
		RuntimeState->PendingNodeIds.Reset();
		ResetQuestObjective(QuestId);
		UE_LOG(LogQuest, Log, TEXT("Quest '%s' completed."), *QuestId.ToString());
		OnQuestStateChanged.Broadcast(QuestId, EQuestState::Completed);
		UnloadQuestBundle(QuestId);
	}
}

void UQuestSubsystem::FailQuestById(const FName QuestId)
{
	if (FQuestRuntimeState* RuntimeState = FindQuestRuntime(QuestId))
	{
		if (RuntimeState->State == EQuestState::Failed)
		{
			return;
		}

		RuntimeState->State = EQuestState::Failed;
		RuntimeState->ActiveNodeIds.Reset();
		RuntimeState->PendingNodeIds.Reset();
		ResetQuestObjective(QuestId);
		UE_LOG(LogQuest, Warning, TEXT("Quest '%s' failed."), *QuestId.ToString());
		OnQuestStateChanged.Broadcast(QuestId, EQuestState::Failed);
		UnloadQuestBundle(QuestId);
	}
}

FName UQuestSubsystem::GetSelectedChoice(const FName QuestId, const FName ChoiceKey) const
{
	if (const FQuestRuntimeState* RuntimeState = RuntimeQuestStates.Find(QuestId))
	{
		if (const FName* ChoiceValue = RuntimeState->SelectedChoices.Find(ChoiceKey))
		{
			return *ChoiceValue;
		}
	}

	return NAME_None;
}

void UQuestSubsystem::PrintQuestState(const FName QuestId) const
{
	const FQuestRuntimeState* RuntimeState = RuntimeQuestStates.Find(QuestId);
	if (!RuntimeState)
	{
		UE_LOG(LogQuest, Warning, TEXT("Quest '%s' is not present in runtime state."), *QuestId.ToString());
		return;
	}

	UE_LOG(LogQuest, Log, TEXT("Quest '%s' state=%s ActiveNodes=%s PendingNodes=%s Objective=%s"),
		*QuestId.ToString(),
		*StaticEnum<EQuestState>()->GetNameStringByValue(static_cast<int64>(RuntimeState->State)),
		*FString::JoinBy(RuntimeState->ActiveNodeIds, TEXT(","), [](const FName& Name) { return Name.ToString(); }),
		*FString::JoinBy(RuntimeState->PendingNodeIds, TEXT(","), [](const FName& Name) { return Name.ToString(); }),
		*RuntimeState->CurrentObjectiveText.ToString());
}

bool UQuestSubsystem::DebugJumpToNode(const FName QuestId, const FName NodeId)
{
	if (FQuestRuntimeState* RuntimeState = FindQuestRuntime(QuestId))
	{
		RuntimeState->ActiveNodeIds.Reset();
		RuntimeState->PendingNodeIds.Reset();
		QueueNodeForActivation(QuestId, NodeId);
		return TryActivateNode(QuestId, NodeId, nullptr);
	}

	return false;
}

const FQuestRow* UQuestSubsystem::FindQuestDefinition(const FName QuestId) const
{
	if (const FQuestDataBundle* Bundle = LoadedQuestBundles.Find(QuestId))
	{
		return Bundle->IsValid() ? &Bundle->QuestDefinition : nullptr;
	}

	return nullptr;
}

const FQuestNodeRow* UQuestSubsystem::FindNodeDefinition(const FName QuestId, const FName NodeId) const
{
	if (const FQuestDataBundle* Bundle = LoadedQuestBundles.Find(QuestId))
	{
		return Bundle->NodeDefinitions.Find(NodeId);
	}

	return nullptr;
}

const FQuestConditionRow* UQuestSubsystem::FindConditionDefinition(const FName QuestId, const FName ConditionId) const
{
	if (const FQuestDataBundle* Bundle = LoadedQuestBundles.Find(QuestId))
	{
		return Bundle->ConditionDefinitions.Find(ConditionId);
	}

	return nullptr;
}

const FQuestActionRow* UQuestSubsystem::FindActionDefinition(const FName QuestId, const FName ActionId) const
{
	if (const FQuestDataBundle* Bundle = LoadedQuestBundles.Find(QuestId))
	{
		return Bundle->ActionDefinitions.Find(ActionId);
	}

	return nullptr;
}

void UQuestSubsystem::HandlePlaySequenceAction_Implementation(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload)
{
	UE_LOG(LogQuest, Log, TEXT("HandlePlaySequenceAction called for action '%s'."), *ActionRow.ActionId.ToString());
}

void UQuestSubsystem::HandleStartDialogueAction_Implementation(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload)
{
	UE_LOG(LogQuest, Log, TEXT("HandleStartDialogueAction called for action '%s'."), *ActionRow.ActionId.ToString());
}

void UQuestSubsystem::HandleLoadOrUnlockAreaAction_Implementation(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload)
{
	UE_LOG(LogQuest, Log, TEXT("HandleLoadOrUnlockAreaAction called for action '%s'."), *ActionRow.ActionId.ToString());
}

void UQuestSubsystem::HandleSpawnActorAction_Implementation(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload)
{
	UE_LOG(LogQuest, Log, TEXT("HandleSpawnActorAction called for action '%s'."), *ActionRow.ActionId.ToString());
}

void UQuestSubsystem::HandleRemoveActorAction_Implementation(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload)
{
	UE_LOG(LogQuest, Log, TEXT("HandleRemoveActorAction called for action '%s'."), *ActionRow.ActionId.ToString());
}

void UQuestSubsystem::HandleGrantRewardAction_Implementation(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload)
{
	UE_LOG(LogQuest, Log, TEXT("HandleGrantRewardAction called for action '%s'."), *ActionRow.ActionId.ToString());
}

void UQuestSubsystem::HandleBroadcastQuestAction_Implementation(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload)
{
	UE_LOG(LogQuest, Log, TEXT("HandleBroadcastQuestAction called for action '%s'."), *ActionRow.ActionId.ToString());
}

void UQuestSubsystem::HandleQuestStateRestored_Implementation(const FName QuestId)
{
	UE_LOG(LogQuest, Verbose, TEXT("Quest '%s' runtime state restored."), *QuestId.ToString());
}

bool UQuestSubsystem::HandleExtendedConditionEvaluation_Implementation(const FQuestConditionRow& ConditionRow, const FQuestRuntimeState& RuntimeState, const FQuestEventPayload& ContextPayload, bool& bOutResult, FString& OutFailureReason) const
{
	return false;
}

bool UQuestSubsystem::HandleExtendedActionExecution_Implementation(const FName QuestId, const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload)
{
	return false;
}

bool UQuestSubsystem::LoadConfiguredDataTables()
{
	if (!QuestRegistryTableAsset.ToSoftObjectPath().IsValid())
	{
		return false;
	}
		
	// TSoftObjectPtr keeps a soft reference to the registry table asset path.
	//后面大概率是要立刻使用这张表
	// LoadSynchronous resolves the asset immediately because the registry is required for runtime quest lookups.
	QuestRegistryTable = QuestRegistryTableAsset.LoadSynchronous();
	if (!QuestRegistryTable)
	{
		UE_LOG(LogQuest, Warning, TEXT("Failed to load quest registry table."));
		return false;
	}

	InitializeQuestRegistryTable(QuestRegistryTable);
	return true;
}

void UQuestSubsystem::RebuildQuestSourceCache()
{
	QuestDataSources.Reset();

	if (!QuestRegistryTable)
	{
		return;
	}

	static const FString Context(TEXT("QuestRegistryTable"));
	for (const FName RowName : QuestRegistryTable->GetRowNames())
	{
		const FQuestDataTablesRow* Row = QuestRegistryTable->FindRow<FQuestDataTablesRow>(RowName, Context);
		if (!Row)
		{
			continue;
		}

		FQuestDataTablesRow Copy = *Row;
		const FName EffectiveQuestId = Copy.QuestId.IsNone() ? RowName : Copy.QuestId;
		if (EffectiveQuestId.IsNone())
		{
			UE_LOG(LogQuest, Warning, TEXT("Skipped a quest registry row because QuestId is empty. Row='%s'."), *RowName.ToString());
			continue;
		}

		Copy.QuestId = EffectiveQuestId;
		QuestDataSources.Add(EffectiveQuestId, Copy);
	}

	UE_LOG(LogQuest, Log, TEXT("Quest registry loaded with %d quest data sources."), QuestDataSources.Num());
}

void UQuestSubsystem::RefreshLoadedRuntimeQuests()
{
	TArray<FName> QuestIds;
	RuntimeQuestStates.GenerateKeyArray(QuestIds);

	for (const FName QuestId : QuestIds)
	{
		const EQuestState QuestState = GetQuestState(QuestId);
		if (QuestState == EQuestState::Completed || QuestState == EQuestState::Failed)
		{
			UnloadQuestBundle(QuestId);
			HandleQuestStateRestored(QuestId);
			OnQuestRestored.Broadcast(QuestId);
			continue;
		}

		FString FailureReason;
		if (!EnsureQuestBundleLoaded(QuestId, &FailureReason))
		{
			UE_LOG(LogQuest, Warning, TEXT("Failed to refresh quest '%s': %s"), *QuestId.ToString(), *FailureReason);
			RuntimeQuestStates.Remove(QuestId);
			continue;
		}

		if (FQuestRuntimeState* RuntimeState = RuntimeQuestStates.Find(QuestId))
		{
			SanitizeRuntimeState(QuestId, *RuntimeState);
		}

		if (!RuntimeQuestStates.Contains(QuestId))
		{
			continue;
		}

		RefreshQuestObjective(QuestId);
		HandleQuestStateRestored(QuestId);
		OnQuestRestored.Broadcast(QuestId);
	}
}

bool UQuestSubsystem::BuildQuestBundle(FName ExpectedQuestId, UDataTable* InQuestTable, UDataTable* InNodeTable, UDataTable* InConditionTable, UDataTable* InActionTable, FQuestDataBundle& OutBundle, FString* OutFailureReason) const
{
	const auto Fail = [OutFailureReason](const FString& Reason) -> bool
	{
		if (OutFailureReason)
		{
			*OutFailureReason = Reason;
		}
		return false;
	};

	if (!InQuestTable)
	{
		return Fail(TEXT("Quest table is null."));
	}

	if (!InNodeTable)
	{
		return Fail(TEXT("Node table is null."));
	}

	if (!InConditionTable)
	{
		return Fail(TEXT("Condition table is null."));
	}

	if (!InActionTable)
	{
		return Fail(TEXT("Action table is null."));
	}

	OutBundle = FQuestDataBundle();

	static const FString QuestContext(TEXT("QuestTable"));
	const TArray<FName> QuestRowNames = InQuestTable->GetRowNames();
	if (QuestRowNames.IsEmpty())
	{
		return Fail(TEXT("Quest table does not contain any rows."));
	}

	bool bFoundQuestDefinition = false;
	for (const FName RowName : QuestRowNames)
	{
		const FQuestRow* Row = InQuestTable->FindRow<FQuestRow>(RowName, QuestContext);
		if (!Row)
		{
			continue;
		}

		const FName EffectiveQuestId = Row->QuestId.IsNone() ? RowName : Row->QuestId;
		if (!ExpectedQuestId.IsNone() && EffectiveQuestId != ExpectedQuestId)
		{
			continue;
		}

		if (bFoundQuestDefinition)
		{
			return Fail(TEXT("Quest table contains multiple matching quest definitions. Expected exactly one."));
		}

		OutBundle.QuestDefinition = *Row;
		OutBundle.QuestDefinition.QuestId = EffectiveQuestId;
		bFoundQuestDefinition = true;
	}

	if (!bFoundQuestDefinition)
	{
		if (!ExpectedQuestId.IsNone())
		{
			return Fail(FString::Printf(TEXT("Quest table does not contain a definition for '%s'."), *ExpectedQuestId.ToString()));
		}

		return Fail(TEXT("Quest table does not contain a valid quest definition."));
	}

	const FName EffectiveQuestId = OutBundle.QuestDefinition.QuestId;

	static const FString NodeContext(TEXT("NodeTable"));
	for (const FName RowName : InNodeTable->GetRowNames())
	{
		const FQuestNodeRow* Row = InNodeTable->FindRow<FQuestNodeRow>(RowName, NodeContext);
		if (!Row)
		{
			continue;
		}

		FQuestNodeRow Copy = *Row;
		if (Copy.NodeId.IsNone())
		{
			Copy.NodeId = RowName;
		}

		if (Copy.QuestId.IsNone())
		{
			Copy.QuestId = EffectiveQuestId;
		}

		if (Copy.QuestId != EffectiveQuestId)
		{
			return Fail(FString::Printf(TEXT("Node row '%s' belongs to quest '%s', which does not match current quest '%s'."),
				*RowName.ToString(),
				*Copy.QuestId.ToString(),
				*EffectiveQuestId.ToString()));
		}

		if (Copy.NodeId.IsNone())
		{
			return Fail(FString::Printf(TEXT("Node row '%s' is missing NodeId."), *RowName.ToString()));
		}

		OutBundle.NodeDefinitions.Add(Copy.NodeId, Copy);
	}

	if (!OutBundle.QuestDefinition.RootNodeId.IsNone() && !OutBundle.NodeDefinitions.Contains(OutBundle.QuestDefinition.RootNodeId))
	{
		return Fail(FString::Printf(TEXT("Quest '%s' root node '%s' was not found in the node table."),
			*EffectiveQuestId.ToString(),
			*OutBundle.QuestDefinition.RootNodeId.ToString()));
	}

	static const FString ConditionContext(TEXT("ConditionTable"));
	for (const FName RowName : InConditionTable->GetRowNames())
	{
		const FQuestConditionRow* Row = InConditionTable->FindRow<FQuestConditionRow>(RowName, ConditionContext);
		if (!Row)
		{
			continue;
		}

		FQuestConditionRow Copy = *Row;
		if (Copy.ConditionId.IsNone())
		{
			Copy.ConditionId = RowName;
		}

		if (Copy.ConditionId.IsNone())
		{
			return Fail(FString::Printf(TEXT("Condition row '%s' is missing ConditionId."), *RowName.ToString()));
		}

		OutBundle.ConditionDefinitions.Add(Copy.ConditionId, Copy);
	}

	static const FString ActionContext(TEXT("ActionTable"));
	for (const FName RowName : InActionTable->GetRowNames())
	{
		const FQuestActionRow* Row = InActionTable->FindRow<FQuestActionRow>(RowName, ActionContext);
		if (!Row)
		{
			continue;
		}

		FQuestActionRow Copy = *Row;
		if (Copy.ActionId.IsNone())
		{
			Copy.ActionId = RowName;
		}

		if (Copy.ActionId.IsNone())
		{
			return Fail(FString::Printf(TEXT("Action row '%s' is missing ActionId."), *RowName.ToString()));
		}

		OutBundle.ActionDefinitions.Add(Copy.ActionId, Copy);
	}

	OutBundle.QuestTable = InQuestTable;
	OutBundle.NodeTable = InNodeTable;
	OutBundle.ConditionTable = InConditionTable;
	OutBundle.ActionTable = InActionTable;
	return ValidateQuestBundleReferences(OutBundle, OutFailureReason);
}

bool UQuestSubsystem::ValidateQuestBundleReferences(const FQuestDataBundle& Bundle, FString* OutFailureReason) const
{
	const auto Fail = [OutFailureReason](const FString& Reason) -> bool
	{
		if (OutFailureReason)
		{
			*OutFailureReason = Reason;
		}
		return false;
	};

	const auto ValidateConditionIds = [&Bundle, &Fail](const FName OwnerId, const TArray<FName>& ConditionIds) -> bool
	{
		for (const FName ConditionId : ConditionIds)
		{
			if (!Bundle.ConditionDefinitions.Contains(ConditionId))
			{
				return Fail(FString::Printf(TEXT("Quest '%s' references missing condition '%s'."), *OwnerId.ToString(), *ConditionId.ToString()));
			}
		}
		return true;
	};

	const auto ValidateActionIds = [&Bundle, &Fail](const FName OwnerId, const TArray<FName>& ActionIds) -> bool
	{
		for (const FName ActionId : ActionIds)
		{
			if (!Bundle.ActionDefinitions.Contains(ActionId))
			{
				return Fail(FString::Printf(TEXT("Quest '%s' references missing action '%s'."), *OwnerId.ToString(), *ActionId.ToString()));
			}
		}
		return true;
	};

	if (!ValidateConditionIds(Bundle.QuestDefinition.QuestId, Bundle.QuestDefinition.StartConditionIds))
	{
		return false;
	}

	for (const TPair<FName, FQuestNodeRow>& Pair : Bundle.NodeDefinitions)
	{
		const FQuestNodeRow& NodeRow = Pair.Value;
		if (!ValidateConditionIds(NodeRow.NodeId, NodeRow.StartConditionIds)
			|| !ValidateConditionIds(NodeRow.NodeId, NodeRow.CompleteConditionIds)
			|| !ValidateActionIds(NodeRow.NodeId, NodeRow.OnStartActionIds)
			|| !ValidateActionIds(NodeRow.NodeId, NodeRow.OnCompleteActionIds)
			|| !ValidateActionIds(NodeRow.NodeId, NodeRow.OnFailActionIds))
		{
			return false;
		}

		for (const FName NextNodeId : NodeRow.NextNodeIds)
		{
			if (!Bundle.NodeDefinitions.Contains(NextNodeId))
			{
				return Fail(FString::Printf(TEXT("Node '%s' references missing next node '%s'."), *NodeRow.NodeId.ToString(), *NextNodeId.ToString()));
			}
		}
	}

	return true;
}

bool UQuestSubsystem::EnsureQuestBundleLoaded(const FName QuestId, FString* OutFailureReason)
{
	const auto Fail = [OutFailureReason](const FString& Reason) -> bool
	{
		if (OutFailureReason)
		{
			*OutFailureReason = Reason;
		}
		return false;
	};

	if (QuestId.IsNone())
	{
		return Fail(TEXT("QuestId is empty."));
	}

	if (LoadedQuestBundles.Contains(QuestId))
	{
		return true;
	}

	const FQuestDataTablesRow* SourceRow = QuestDataSources.Find(QuestId);
	if (!SourceRow)
	{
		return Fail(FString::Printf(TEXT("Quest '%s' was not found in the quest registry."), *QuestId.ToString()));
	}

	UDataTable* LocalQuestTable = SourceRow->QuestTable.LoadSynchronous();
	if (!LocalQuestTable)
	{
		return Fail(FString::Printf(TEXT("Failed to load quest table for '%s'."), *QuestId.ToString()));
	}

	UDataTable* LocalNodeTable = SourceRow->NodeTable.LoadSynchronous();
	if (!LocalNodeTable)
	{
		return Fail(FString::Printf(TEXT("Failed to load node table for '%s'."), *QuestId.ToString()));
	}

	UDataTable* LocalConditionTable = SourceRow->ConditionTable.LoadSynchronous();
	if (!LocalConditionTable)
	{
		return Fail(FString::Printf(TEXT("Failed to load condition table for '%s'."), *QuestId.ToString()));
	}

	UDataTable* LocalActionTable = SourceRow->ActionTable.LoadSynchronous();
	if (!LocalActionTable)
	{
		return Fail(FString::Printf(TEXT("Failed to load action table for '%s'."), *QuestId.ToString()));
	}

	FQuestDataBundle Bundle;
	if (!BuildQuestBundle(QuestId, LocalQuestTable, LocalNodeTable, LocalConditionTable, LocalActionTable, Bundle, OutFailureReason))
	{
		return false;
	}

	LoadedQuestBundles.Add(QuestId, MoveTemp(Bundle));
	UE_LOG(LogQuest, Log, TEXT("Loaded quest data tables for '%s'."), *QuestId.ToString());
	return true;
}

void UQuestSubsystem::UnloadQuestBundle(const FName QuestId)
{
	if (LoadedQuestBundles.Remove(QuestId) > 0)
	{
		UE_LOG(LogQuest, Log, TEXT("Unloaded cached quest data tables for '%s'."), *QuestId.ToString());
	}
}

void UQuestSubsystem::ResetQuestObjective(const FName QuestId)
{
	if (FQuestRuntimeState* RuntimeState = FindQuestRuntime(QuestId))
	{
		if (!RuntimeState->CurrentObjectiveText.IsEmpty())
		{
			RuntimeState->CurrentObjectiveText = FText::GetEmpty();
			OnQuestObjectiveUpdated.Broadcast(QuestId, RuntimeState->CurrentObjectiveText);
		}
	}
}

void UQuestSubsystem::SyncAllRegisteredTargets()
{
	FQuestEventPayload EmptyPayload;
	for (const TPair<FName, bool>& Pair : TargetEnabledStates)
	{
		ApplyTargetEnabledState(Pair.Key, Pair.Value, EmptyPayload);
	}
}

void UQuestSubsystem::SanitizeRuntimeState(const FName QuestId, FQuestRuntimeState& RuntimeState)
{
	if (!FindQuestDefinition(QuestId))
	{
		UE_LOG(LogQuest, Warning, TEXT("Quest '%s' definition is missing. Removed stale runtime state."), *QuestId.ToString());
		RuntimeQuestStates.Remove(QuestId);
		return;
	}

	RuntimeState.QuestId = QuestId;
	RuntimeState.ActiveNodeIds = RuntimeState.ActiveNodeIds.FilterByPredicate([this, QuestId](const FName NodeId)
	{
		return FindNodeDefinition(QuestId, NodeId) != nullptr;
	});
	RuntimeState.PendingNodeIds = RuntimeState.PendingNodeIds.FilterByPredicate([this, QuestId](const FName NodeId)
	{
		return FindNodeDefinition(QuestId, NodeId) != nullptr;
	});

	for (auto It = RuntimeState.NodeStates.CreateIterator(); It; ++It)
	{
		if (!FindNodeDefinition(QuestId, It.Key()))
		{
			It.RemoveCurrent();
		}
	}

	if (const FQuestRow* QuestRow = FindQuestDefinition(QuestId))
	{
		const bool bHasRootTracked = RuntimeState.ActiveNodeIds.Contains(QuestRow->RootNodeId)
			|| RuntimeState.PendingNodeIds.Contains(QuestRow->RootNodeId)
			|| RuntimeState.GetNodeStateValue(QuestRow->RootNodeId) != EQuestNodeState::NotStarted;

		if (RuntimeState.State == EQuestState::Active && !QuestRow->RootNodeId.IsNone() && !bHasRootTracked)
		{
			RuntimeState.PendingNodeIds.AddUnique(QuestRow->RootNodeId);
		}
	}
}

void UQuestSubsystem::QueueNodeForActivation(const FName QuestId, const FName NodeId)
{
	FQuestRuntimeState* RuntimeState = FindQuestRuntime(QuestId);
	if (!RuntimeState || NodeId.IsNone())
	{
		return;
	}

	if (RuntimeState->GetNodeStateValue(NodeId) == EQuestNodeState::Completed)
	{
		return;
	}

	RuntimeState->PendingNodeIds.AddUnique(NodeId);
}

bool UQuestSubsystem::TryActivatePendingNodes(const FName QuestId, const FQuestEventPayload* EventPayload)
{
	FQuestRuntimeState* RuntimeState = FindQuestRuntime(QuestId);
	if (!RuntimeState || RuntimeState->State != EQuestState::Active)
	{
		return false;
	}

	bool bAnyStarted = false;
	bool bMadeProgress = true;

	while (bMadeProgress)
	{
		bMadeProgress = false;
		const TArray<FName> PendingNodes = RuntimeState->PendingNodeIds;

		for (const FName NodeId : PendingNodes)
		{
			const FQuestNodeRow* NodeRow = FindNodeDefinition(QuestId, NodeId);
			if (!NodeRow)
			{
				RuntimeState->PendingNodeIds.Remove(NodeId);
				continue;
			}

			if (!NodeRow->bAutoStart)
			{
				continue;
			}

			if (TryActivateNode(QuestId, NodeId, EventPayload))
			{
				RuntimeState = FindQuestRuntime(QuestId);
				if (!RuntimeState)
				{
					return bAnyStarted;
				}

				bAnyStarted = true;
				bMadeProgress = true;
			}
		}
	}

	return bAnyStarted;
}

void UQuestSubsystem::RefreshQuestObjective(const FName QuestId)
{
	FQuestRuntimeState* RuntimeState = FindQuestRuntime(QuestId);
	if (!RuntimeState)
	{
		return;
	}

	for (const FName ActiveNodeId : RuntimeState->ActiveNodeIds)
	{
		if (const FQuestNodeRow* NodeRow = FindNodeDefinition(QuestId, ActiveNodeId))
		{
			if (!NodeRow->bHidden && !NodeRow->ObjectiveText.IsEmpty())
			{
				if (!RuntimeState->CurrentObjectiveText.EqualTo(NodeRow->ObjectiveText))
				{
					RuntimeState->CurrentObjectiveText = NodeRow->ObjectiveText;
					OnQuestObjectiveUpdated.Broadcast(QuestId, RuntimeState->CurrentObjectiveText);
				}
				return;
			}
		}
	}

	if (!RuntimeState->CurrentObjectiveText.IsEmpty())
	{
		RuntimeState->CurrentObjectiveText = FText::GetEmpty();
		OnQuestObjectiveUpdated.Broadcast(QuestId, RuntimeState->CurrentObjectiveText);
	}
}

bool UQuestSubsystem::EvaluateQuestProgress(const FName QuestId, const FQuestEventPayload* EventPayload)
{
	FQuestRuntimeState* RuntimeState = FindQuestRuntime(QuestId);
	if (!RuntimeState || RuntimeState->State != EQuestState::Active)
	{
		return false;
	}

	if (!ConditionEvaluator)
	{
		return false;
	}

	bool bAnyChange = TryActivatePendingNodes(QuestId, EventPayload);
	const TArray<FName> ActiveNodes = RuntimeState->ActiveNodeIds;

	for (const FName NodeId : ActiveNodes)
	{
		const FQuestNodeRow* NodeRow = FindNodeDefinition(QuestId, NodeId);
		if (!NodeRow || NodeRow->CompleteConditionIds.IsEmpty())
		{
			continue;
		}

		FString FailureReason;
		if (ConditionEvaluator->EvaluateConditionList(NodeRow->CompleteConditionIds, *RuntimeState, EventPayload, &FailureReason))
		{
			FQuestEventPayload CompletionPayload;
			if (EventPayload)
			{
				CompletionPayload = *EventPayload;
			}
			CompletionPayload.QuestId = QuestId;
			CompletionPayload.NodeId = NodeId;

			CompleteNode(QuestId, NodeId, CompletionPayload);
			bAnyChange = true;
		}
		else if (!NodeRow->CompleteConditionIds.IsEmpty() && EventPayload && !FailureReason.IsEmpty())
		{
			UE_LOG(LogQuest, Verbose, TEXT("Quest '%s' node '%s' completion conditions failed: %s"),
				*QuestId.ToString(),
				*NodeId.ToString(),
				*FailureReason);
		}
	}

	if (bAnyChange)
	{
		RefreshQuestObjective(QuestId);
	}

	return bAnyChange;
}

bool UQuestSubsystem::TryActivateNode(const FName QuestId, const FName NodeId, const FQuestEventPayload* EventPayload)
{
	FQuestRuntimeState* RuntimeState = FindQuestRuntime(QuestId);
	if (!RuntimeState || RuntimeState->State != EQuestState::Active)
	{
		return false;
	}

	const FQuestNodeRow* NodeRow = FindNodeDefinition(QuestId, NodeId);
	if (!NodeRow)
	{
		UE_LOG(LogQuest, Warning, TEXT("Quest '%s' does not contain node '%s'."), *QuestId.ToString(), *NodeId.ToString());
		return false;
	}

	const EQuestNodeState ExistingState = RuntimeState->GetNodeStateValue(NodeId);
	if (ExistingState == EQuestNodeState::Active || ExistingState == EQuestNodeState::Completed)
	{
		RuntimeState->PendingNodeIds.Remove(NodeId);
		return false;
	}

	FString FailureReason;
	if (ConditionEvaluator && !ConditionEvaluator->EvaluateConditionList(NodeRow->StartConditionIds, *RuntimeState, EventPayload, &FailureReason))
	{
		if (!FailureReason.IsEmpty())
		{
			UE_LOG(LogQuest, Verbose, TEXT("Quest '%s' node '%s' cannot start yet: %s"),
				*QuestId.ToString(),
				*NodeId.ToString(),
				*FailureReason);
		}
		return false;
	}

	StartNode(QuestId, *NodeRow, EventPayload);
	return true;
}

void UQuestSubsystem::StartNode(const FName QuestId, const FQuestNodeRow& NodeRow, const FQuestEventPayload* EventPayload)
{
	FQuestRuntimeState* RuntimeState = FindQuestRuntime(QuestId);
	if (!RuntimeState)
	{
		return;
	}

	FQuestNodeRuntimeState& NodeState = RuntimeState->GetOrAddNodeState(NodeRow.NodeId);
	NodeState.State = EQuestNodeState::Active;
	RuntimeState->ActiveNodeIds.AddUnique(NodeRow.NodeId);
	RuntimeState->PendingNodeIds.Remove(NodeRow.NodeId);

	UE_LOG(LogQuest, Log, TEXT("Quest '%s' node '%s' started."), *QuestId.ToString(), *NodeRow.NodeId.ToString());
	OnQuestNodeStateChanged.Broadcast(QuestId, NodeRow.NodeId, EQuestNodeState::Active);

	if (ActionExecutor)
	{
		FQuestEventPayload StartPayload;
		if (EventPayload)
		{
			StartPayload = *EventPayload;
		}
		StartPayload.QuestId = QuestId;
		StartPayload.NodeId = NodeRow.NodeId;
		ActionExecutor->ExecuteActions(QuestId, NodeRow.OnStartActionIds, StartPayload);
	}

	RefreshQuestObjective(QuestId);

	if ((NodeRow.NodeType == EQuestNodeType::Internal || NodeRow.NodeType == EQuestNodeType::Complete) && NodeRow.CompleteConditionIds.IsEmpty())
	{
		FQuestEventPayload AutoPayload;
		AutoPayload.QuestId = QuestId;
		AutoPayload.NodeId = NodeRow.NodeId;
		AutoPayload.EventType = EQuestEventType::Custom;
		AutoPayload.EventName = TEXT("AutoComplete");
		CompleteNode(QuestId, NodeRow.NodeId, AutoPayload);
	}
}

void UQuestSubsystem::CompleteNode(const FName QuestId, const FName NodeId, const FQuestEventPayload& EventPayload)
{
	FQuestRuntimeState* RuntimeState = FindQuestRuntime(QuestId);
	const FQuestNodeRow* NodeRow = FindNodeDefinition(QuestId, NodeId);
	if (!RuntimeState || !NodeRow)
	{
		return;
	}

	FQuestNodeRuntimeState& NodeState = RuntimeState->GetOrAddNodeState(NodeId);
	if (NodeState.State == EQuestNodeState::Completed)
	{
		return;
	}

	NodeState.State = EQuestNodeState::Completed;
	RuntimeState->ActiveNodeIds.Remove(NodeId);
	RuntimeState->PendingNodeIds.Remove(NodeId);

	UE_LOG(LogQuest, Log, TEXT("Quest '%s' node '%s' completed."), *QuestId.ToString(), *NodeId.ToString());
	OnQuestNodeStateChanged.Broadcast(QuestId, NodeId, EQuestNodeState::Completed);

	if (ActionExecutor)
	{
		ActionExecutor->ExecuteActions(QuestId, NodeRow->OnCompleteActionIds, EventPayload);
	}

	ActivateNextNodes(QuestId, *NodeRow, EventPayload);
	RefreshQuestObjective(QuestId);

	if (NodeRow->NodeType == EQuestNodeType::Complete && GetQuestState(QuestId) == EQuestState::Active)
	{
		CompleteQuestById(QuestId);
	}
	else
	{
		EvaluateQuestProgress(QuestId, nullptr);
	}
}

void UQuestSubsystem::FailNode(const FName QuestId, const FName NodeId, const FQuestEventPayload& EventPayload)
{
	FQuestRuntimeState* RuntimeState = FindQuestRuntime(QuestId);
	const FQuestNodeRow* NodeRow = FindNodeDefinition(QuestId, NodeId);
	if (!RuntimeState || !NodeRow)
	{
		return;
	}

	FQuestNodeRuntimeState& NodeState = RuntimeState->GetOrAddNodeState(NodeId);
	NodeState.State = EQuestNodeState::Failed;
	RuntimeState->ActiveNodeIds.Remove(NodeId);
	RuntimeState->PendingNodeIds.Remove(NodeId);

	UE_LOG(LogQuest, Warning, TEXT("Quest '%s' node '%s' failed."), *QuestId.ToString(), *NodeId.ToString());
	OnQuestNodeStateChanged.Broadcast(QuestId, NodeId, EQuestNodeState::Failed);

	if (ActionExecutor)
	{
		ActionExecutor->ExecuteActions(QuestId, NodeRow->OnFailActionIds, EventPayload);
	}

}

void UQuestSubsystem::ActivateNextNodes(const FName QuestId, const FQuestNodeRow& CompletedNode, const FQuestEventPayload& EventPayload)
{
	if (CompletedNode.NextNodeIds.IsEmpty())
	{
		return;
	}

	switch (CompletedNode.BranchPolicy)
	{
	case EQuestBranchPolicy::FirstValid:
		for (const FName NextNodeId : CompletedNode.NextNodeIds)
		{
			QueueNodeForActivation(QuestId, NextNodeId);
			const FQuestNodeRow* NextNodeRow = FindNodeDefinition(QuestId, NextNodeId);
			if (NextNodeRow && NextNodeRow->bAutoStart && TryActivateNode(QuestId, NextNodeId, &EventPayload))
			{
				break;
			}
		}
		break;

	case EQuestBranchPolicy::SequentialAll:
	case EQuestBranchPolicy::AllValid:
	default:
		for (const FName NextNodeId : CompletedNode.NextNodeIds)
		{
			QueueNodeForActivation(QuestId, NextNodeId);
			const FQuestNodeRow* NextNodeRow = FindNodeDefinition(QuestId, NextNodeId);
			if (NextNodeRow && NextNodeRow->bAutoStart)
			{
				TryActivateNode(QuestId, NextNodeId, &EventPayload);
			}
		}
		break;
	}
}

void UQuestSubsystem::ApplyTargetEnabledState(const FName TargetId, const bool bEnabled, const FQuestEventPayload& ContextPayload)
{
	if (UObject* TargetObject = GetRegisteredQuestTarget(TargetId))
	{
		if (TargetObject->GetClass()->ImplementsInterface(UQuestActionTargetInterface::StaticClass()))
		{
			IQuestActionTargetInterface::Execute_SetQuestTargetEnabled(TargetObject, bEnabled);
		}
	}

	UE_LOG(LogQuest, Verbose, TEXT("Quest target '%s' enabled state set to %s."), *TargetId.ToString(), bEnabled ? TEXT("true") : TEXT("false"));
}

FQuestRuntimeState* UQuestSubsystem::FindQuestRuntime(const FName QuestId)
{
	return RuntimeQuestStates.Find(QuestId);
}

const FQuestRuntimeState* UQuestSubsystem::FindQuestRuntime(const FName QuestId) const
{
	return RuntimeQuestStates.Find(QuestId);
}
