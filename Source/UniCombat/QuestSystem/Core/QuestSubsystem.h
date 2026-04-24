// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "QuestSystem/QuestTypes.h"
#include "QuestSystem/QuestEventTypes.h"
#include "QuestSubsystem.generated.h"

class UDataTable;
class UQuestActionExecutor;
class UQuestConditionEvaluator;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestObjectiveUpdated, FName, QuestId, const FText&, ObjectiveText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStateChanged, FName, QuestId, EQuestState, QuestState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnQuestNodeStateChanged, FName, QuestId, FName, NodeId, EQuestNodeState, NodeState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestEventProcessed, const FQuestEventPayload&, Payload);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestRestored, FName, QuestId);

USTRUCT()
struct FQuestDataBundle
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> QuestTable = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> NodeTable = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> ConditionTable = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> ActionTable = nullptr;

	FQuestRow QuestDefinition;
	TMap<FName, FQuestNodeRow> NodeDefinitions;
	TMap<FName, FQuestConditionRow> ConditionDefinitions;
	TMap<FName, FQuestActionRow> ActionDefinitions;

	bool IsValid() const
	{
		return QuestDefinition.QuestId != NAME_None;
	}
};

/**
 * 
 */
UCLASS(Config = Game, DefaultConfig, BlueprintType)
class UNICOMBAT_API UQuestSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UQuestSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void InitializeQuestRegistryTable(UDataTable* InQuestRegistryTable);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void InitializeQuestDataTables(UDataTable* InQuestTable, UDataTable* InNodeTable, UDataTable* InConditionTable, UDataTable* InActionTable);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool AcceptQuest(FName QuestId);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool StartQuestNode(FName QuestId, FName NodeId);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SubmitQuestEvent(const FQuestEventPayload& EventPayload);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SubmitChoice(FName QuestId, FName ChoiceKey, FName ChoiceResult);

	void ExportPersistenceSnapshot(FQuestPersistenceSnapshot& OutSnapshot) const;
	void ImportPersistenceSnapshot(const FQuestPersistenceSnapshot& Snapshot);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void RegisterQuestTarget(FName TargetId, UObject* TargetObject);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void UnregisterQuestTarget(FName TargetId, const UObject* TargetObject);

	//获取已注册的任务目标
	UFUNCTION(BlueprintPure, Category = "Quest")
	UObject* GetRegisteredQuestTarget(FName TargetId) const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsQuestActive(FName QuestId) const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	bool HasQuest(FName QuestId) const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	EQuestState GetQuestState(FName QuestId) const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	EQuestNodeState GetNodeState(FName QuestId, FName NodeId) const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	FText GetCurrentObjectiveText(FName QuestId) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetCurrentObjectiveText(FName QuestId, const FText& ObjectiveText);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetQuestFlag(FName QuestId, FName FlagId, bool bValue);

	UFUNCTION(BlueprintPure, Category = "Quest")
	bool GetQuestFlag(FName QuestId, FName FlagId) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void AddQuestCounter(FName QuestId, FName CounterId, int32 Delta);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetQuestCounter(FName QuestId, FName CounterId, int32 Value);

	UFUNCTION(BlueprintPure, Category = "Quest")
	int32 GetQuestCounter(FName QuestId, FName CounterId) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetInventoryItemCount(FName ItemId, int32 Value);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void AddInventoryItemCount(FName ItemId, int32 Delta);

	UFUNCTION(BlueprintPure, Category = "Quest")
	int32 GetInventoryItemCount(FName ItemId) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetWorldStateBool(FName Key, bool bValue);

	UFUNCTION(BlueprintPure, Category = "Quest")
	bool GetWorldStateBool(FName Key) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetWorldStateInt(FName Key, int32 Value);

	UFUNCTION(BlueprintPure, Category = "Quest")
	int32 GetWorldStateInt(FName Key) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetCurrentTimeOfDay(EQuestTimeOfDay NewTimeOfDay);

	UFUNCTION(BlueprintPure, Category = "Quest")
	EQuestTimeOfDay GetCurrentTimeOfDay() const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetTargetEnabledState(FName TargetId, bool bEnabled, const FQuestEventPayload& ContextPayload);

	UFUNCTION(BlueprintPure, Category = "Quest")
	bool GetTargetEnabledState(FName TargetId, bool& bOutEnabled) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void CompleteQuestById(FName QuestId);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void FailQuestById(FName QuestId);

	UFUNCTION(BlueprintPure, Category = "Quest")
	FName GetSelectedChoice(FName QuestId, FName ChoiceKey) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void PrintQuestState(FName QuestId) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool DebugJumpToNode(FName QuestId, FName NodeId);

	const FQuestRow* FindQuestDefinition(FName QuestId) const;
	const FQuestNodeRow* FindNodeDefinition(FName QuestId, FName NodeId) const;
	const FQuestConditionRow* FindConditionDefinition(FName QuestId, FName ConditionId) const;
	const FQuestActionRow* FindActionDefinition(FName QuestId, FName ActionId) const;

	UFUNCTION(BlueprintNativeEvent, Category = "Quest|Blueprint")
	bool HandleExtendedConditionEvaluation(const FQuestConditionRow& ConditionRow, const FQuestRuntimeState& RuntimeState, const FQuestEventPayload& ContextPayload, bool& bOutResult, FString& OutFailureReason) const;
	virtual bool HandleExtendedConditionEvaluation_Implementation(const FQuestConditionRow& ConditionRow, const FQuestRuntimeState& RuntimeState, const FQuestEventPayload& ContextPayload, bool& bOutResult, FString& OutFailureReason) const;

	UFUNCTION(BlueprintNativeEvent, Category = "Quest|Blueprint")
	bool HandleExtendedActionExecution(FName QuestId, const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload);
	virtual bool HandleExtendedActionExecution_Implementation(FName QuestId, const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload);

	UFUNCTION(BlueprintNativeEvent, Category = "Quest|Blueprint")
	void HandlePlaySequenceAction(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload);
	virtual void HandlePlaySequenceAction_Implementation(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload);

	UFUNCTION(BlueprintNativeEvent, Category = "Quest|Blueprint")
	void HandleStartDialogueAction(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload);
	virtual void HandleStartDialogueAction_Implementation(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload);

	UFUNCTION(BlueprintNativeEvent, Category = "Quest|Blueprint")
	void HandleLoadOrUnlockAreaAction(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload);
	virtual void HandleLoadOrUnlockAreaAction_Implementation(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload);

	UFUNCTION(BlueprintNativeEvent, Category = "Quest|Blueprint")
	void HandleSpawnActorAction(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload);
	virtual void HandleSpawnActorAction_Implementation(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload);

	UFUNCTION(BlueprintNativeEvent, Category = "Quest|Blueprint")
	void HandleRemoveActorAction(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload);
	virtual void HandleRemoveActorAction_Implementation(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload);

	UFUNCTION(BlueprintNativeEvent, Category = "Quest|Blueprint")
	void HandleGrantRewardAction(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload);
	virtual void HandleGrantRewardAction_Implementation(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload);

	UFUNCTION(BlueprintNativeEvent, Category = "Quest|Blueprint")
	void HandleBroadcastQuestAction(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload);
	virtual void HandleBroadcastQuestAction_Implementation(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload);

	UFUNCTION(BlueprintNativeEvent, Category = "Quest|Blueprint")
	void HandleQuestStateRestored(FName QuestId);
	virtual void HandleQuestStateRestored_Implementation(FName QuestId);

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestObjectiveUpdated OnQuestObjectiveUpdated; //任务目标已更新
	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestStateChanged OnQuestStateChanged; //任务状态已改变
	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestNodeStateChanged OnQuestNodeStateChanged;//任务节点状态已更改
	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestEventProcessed OnQuestEventProcessed; //任务事件已处理
	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestRestored OnQuestRestored; //任务恢复

protected:
	//加载已配置的数据表
	bool LoadConfiguredDataTables();
	void RebuildQuestSourceCache();
	void RefreshLoadedRuntimeQuests();
	bool BuildQuestBundle(FName ExpectedQuestId, UDataTable* InQuestTable, UDataTable* InNodeTable, UDataTable* InConditionTable, UDataTable* InActionTable, FQuestDataBundle& OutBundle, FString* OutFailureReason = nullptr) const;
	bool ValidateQuestBundleReferences(const FQuestDataBundle& Bundle, FString* OutFailureReason = nullptr) const;
	//确保任务包已加载
	bool EnsureQuestBundleLoaded(FName QuestId, FString* OutFailureReason = nullptr);
	void UnloadQuestBundle(FName QuestId);
	void ResetQuestObjective(FName QuestId);
	//同步所有已注册目标
	void SyncAllRegisteredTargets();
	void SanitizeRuntimeState(FName QuestId, FQuestRuntimeState& RuntimeState);

	void QueueNodeForActivation(FName QuestId, FName NodeId);
	bool TryActivatePendingNodes(FName QuestId, const FQuestEventPayload* EventPayload);
	void RefreshQuestObjective(FName QuestId);
	bool EvaluateQuestProgress(FName QuestId, const FQuestEventPayload* EventPayload);
	bool TryActivateNode(FName QuestId, FName NodeId, const FQuestEventPayload* EventPayload);
	void StartNode(FName QuestId, const FQuestNodeRow& NodeRow, const FQuestEventPayload* EventPayload);
	void CompleteNode(FName QuestId, FName NodeId, const FQuestEventPayload& EventPayload);
	void FailNode(FName QuestId, FName NodeId, const FQuestEventPayload& EventPayload);
	void ActivateNextNodes(FName QuestId, const FQuestNodeRow& CompletedNode, const FQuestEventPayload& EventPayload);
	void ApplyTargetEnabledState(FName TargetId, bool bEnabled, const FQuestEventPayload& ContextPayload);
	FQuestRuntimeState* FindQuestRuntime(FName QuestId);
	const FQuestRuntimeState* FindQuestRuntime(FName QuestId) const;

protected:
	//由setting配置
	TSoftObjectPtr<UDataTable> QuestRegistryTableAsset;
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> QuestRegistryTable = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UQuestConditionEvaluator> ConditionEvaluator = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UQuestActionExecutor> ActionExecutor = nullptr;

	UPROPERTY(Transient)
	TMap<FName, FQuestRuntimeState> RuntimeQuestStates;

	UPROPERTY(Transient)
	TMap<FName, bool> WorldStateBools;

	UPROPERTY(Transient)
	TMap<FName, int32> WorldStateInts;

	UPROPERTY(Transient)
	TMap<FName, int32> InventoryItemCounts;

	UPROPERTY(Transient)
	TMap<FName, bool> TargetEnabledStates;

	UPROPERTY(Transient)
	EQuestTimeOfDay CurrentTimeOfDay = EQuestTimeOfDay::Day;

	UPROPERTY(Transient)
	TMap<FName, FQuestDataBundle> LoadedQuestBundles;

	TMap<FName, FQuestDataTablesRow> QuestDataSources;
	TMap<FName, TWeakObjectPtr<UObject>> RegisteredTargets;
};
