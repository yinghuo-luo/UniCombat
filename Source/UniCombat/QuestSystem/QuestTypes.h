// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "QuestTypes.generated.h"

class AActor;

DECLARE_LOG_CATEGORY_EXTERN(LogQuest, Log, All);

UENUM(BlueprintType)
enum class EQuestState : uint8
{
	Inactive,
	Active,
	Completed,
	Failed
};

UENUM(BlueprintType)
enum class EQuestNodeState : uint8
{
	NotStarted,
	Active,
	Completed,
	Failed
};

UENUM(BlueprintType)
enum class EQuestCategory : uint8
{
	Main,
	Side,
	Investigation,
	Safehouse,
	Ritual,
	Dungeon,
	World
};

UENUM(BlueprintType)
enum class EQuestNodeType : uint8
{
	Objective,
	Branch,
	Optional,
	Internal,
	Choice,
	Complete
};

UENUM(BlueprintType)
enum class EQuestBranchPolicy : uint8
{
	SequentialAll,
	FirstValid,
	AllValid
};

UENUM(BlueprintType)
enum class EQuestConditionType : uint8
{
	PreviousNodeCompleted,
	QuestFlag,
	QuestCounter,
	WorldStateBool,
	WorldStateInt,
	EnteredRegion,
	InteractionCompleted,
	DialogueCompleted,
	EnemyKilled,
	BossDefeated,
	PuzzleSolved,
	HasItem,
	TimeOfDay,
	ChoiceResult,
	RitualStepCompleted,
	QuestState,
	CustomEvent
};

UENUM(BlueprintType)
enum class EQuestCompareOp : uint8
{
	Equal,
	NotEqual,
	Greater,
	GreaterOrEqual,
	Less,
	LessOrEqual
};

UENUM(BlueprintType)
enum class EQuestConditionGroupOp : uint8
{
	All,
	Any
};

UENUM(BlueprintType)
enum class EQuestActionType : uint8
{
	UpdateObjective,
	SetQuestFlag,
	AddQuestCounter,
	SetQuestCounter,
	SetWorldStateBool,
	SetWorldStateInt,
	EnableTarget,
	DisableTarget,
	PlaySequence,
	StartDialogue,
	SetTimeOfDay,
	LoadOrUnlockArea,
	SpawnActor,
	RemoveActor,
	GrantReward,
	CompleteQuest,
	FailQuest,
	BroadcastEvent
};

UENUM(BlueprintType)
enum class EQuestTimeOfDay : uint8
{
	Any,
	Day,
	Night
};

USTRUCT(BlueprintType)
struct FQuestWorldStateBoolEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FName Key = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	bool bValue = false;
};

USTRUCT(BlueprintType)
struct FQuestWorldStateIntEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FName Key = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 Value = 0;
};

USTRUCT(BlueprintType)
struct FQuestRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName QuestId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EQuestCategory Category = EQuestCategory::Main;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> StartConditionIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName RootNodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName RewardId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer QuestTags;
};

USTRUCT(BlueprintType)
struct FQuestNodeRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName QuestId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName NodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EQuestNodeType NodeType = EQuestNodeType::Objective;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ObjectiveText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAutoStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bHidden = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> StartConditionIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> CompleteConditionIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> OnStartActionIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> OnCompleteActionIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> OnFailActionIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> NextNodeIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EQuestBranchPolicy BranchPolicy = EQuestBranchPolicy::SequentialAll;
};

USTRUCT(BlueprintType)
struct FQuestConditionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ConditionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EQuestConditionType ConditionType = EQuestConditionType::CustomEvent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ParamName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ParamId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 IntValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bBoolValue = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EQuestCompareOp CompareOp = EQuestCompareOp::Equal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EQuestConditionGroupOp GroupOp = EQuestConditionGroupOp::All;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Comment;
};

USTRUCT(BlueprintType)
struct FQuestActionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ActionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EQuestActionType ActionType = EQuestActionType::UpdateObjective;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName TargetId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName NameParam = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 IntParam = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bBoolParam = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bDispatchToTarget = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSoftObjectPath SoftPath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<AActor> SoftClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Comment;
};

USTRUCT(BlueprintType)
struct FQuestDataTablesRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName QuestId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UDataTable> QuestTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UDataTable> NodeTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UDataTable> ConditionTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UDataTable> ActionTable;
};

USTRUCT(BlueprintType)
struct FQuestNodeRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame)
	FName NodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame)
	EQuestNodeState State = EQuestNodeState::NotStarted;
};

USTRUCT(BlueprintType)
struct FQuestRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame)
	FName QuestId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame)
	EQuestState State = EQuestState::Inactive;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame)
	TArray<FName> ActiveNodeIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame)
	TArray<FName> PendingNodeIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame)
	TMap<FName, FQuestNodeRuntimeState> NodeStates;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame)
	TMap<FName, bool> Flags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame)
	TMap<FName, int32> Counters;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame)
	TMap<FName, FName> SelectedChoices;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame)
	FText CurrentObjectiveText;

	FQuestNodeRuntimeState& GetOrAddNodeState(const FName NodeId)
	{
		FQuestNodeRuntimeState& NodeState = NodeStates.FindOrAdd(NodeId);
		NodeState.NodeId = NodeId;
		return NodeState;
	}

	const FQuestNodeRuntimeState* FindNodeState(const FName NodeId) const
	{
		return NodeStates.Find(NodeId);
	}

	EQuestNodeState GetNodeStateValue(const FName NodeId) const
	{
		if (const FQuestNodeRuntimeState* NodeState = NodeStates.Find(NodeId))
		{
			return NodeState->State;
		}

		return EQuestNodeState::NotStarted;
	}
};

USTRUCT(BlueprintType)
struct FQuestPersistenceSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame)
	TMap<FName, FQuestRuntimeState> RuntimeQuestStates;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame)
	TMap<FName, bool> WorldStateBools;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame)
	TMap<FName, int32> WorldStateInts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame)
	TMap<FName, int32> InventoryItemCounts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame)
	TMap<FName, bool> TargetEnabledStates;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame)
	EQuestTimeOfDay TimeOfDay = EQuestTimeOfDay::Day;
};
