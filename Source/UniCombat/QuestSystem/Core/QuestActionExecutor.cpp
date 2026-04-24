// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestActionExecutor.h"

#include "QuestSystem/Objects/QuestInterfaces.h"
#include "QuestSubsystem.h"

void UQuestActionExecutor::Initialize(UQuestSubsystem* InSubsystem)
{
	QuestSubsystem = InSubsystem;
}

void UQuestActionExecutor::ExecuteActions(const FName QuestId, const TArray<FName>& ActionIds, const FQuestEventPayload& ContextPayload)
{
	const UQuestSubsystem* Subsystem = QuestSubsystem.Get();
	if (!Subsystem)
	{
		return;
	}

	TArray<FQuestActionRow> ResolvedActions;
	ResolvedActions.Reserve(ActionIds.Num());

	for (const FName ActionId : ActionIds)
	{
		const FQuestActionRow* Action = Subsystem->FindActionDefinition(QuestId, ActionId);
		if (!Action)
		{
			UE_LOG(LogQuest, Warning, TEXT("任务“%s”中未找到动作“%s”。"), *QuestId.ToString(), *ActionId.ToString());
			continue;
		}

		ResolvedActions.Add(*Action);
	}

	for (const FQuestActionRow& ActionRow : ResolvedActions)
	{
		ExecuteAction(QuestId, ActionRow, ContextPayload);
	}
}

void UQuestActionExecutor::ExecuteAction(const FName QuestId, const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload)
{
	UQuestSubsystem* Subsystem = QuestSubsystem.Get();
	if (!Subsystem)
	{
		return;
	}

	UE_LOG(LogQuest, Verbose, TEXT("执行任务“%s”的动作“%s”，类型=%s。"),
		*QuestId.ToString(),
		*ActionRow.ActionId.ToString(),
		*StaticEnum<EQuestActionType>()->GetNameStringByValue(static_cast<int64>(ActionRow.ActionType)));

	switch (ActionRow.ActionType)
	{
	case EQuestActionType::UpdateObjective:
		Subsystem->SetCurrentObjectiveText(QuestId, FText::FromString(ActionRow.NameParam.ToString()));
		break;

	case EQuestActionType::SetQuestFlag:
		Subsystem->SetQuestFlag(QuestId, ActionRow.TargetId, ActionRow.bBoolParam);
		break;

	case EQuestActionType::AddQuestCounter:
		Subsystem->AddQuestCounter(QuestId, ActionRow.TargetId, ActionRow.IntParam);
		break;

	case EQuestActionType::SetQuestCounter:
		Subsystem->SetQuestCounter(QuestId, ActionRow.TargetId, ActionRow.IntParam);
		break;

	case EQuestActionType::SetWorldStateBool:
		Subsystem->SetWorldStateBool(ActionRow.TargetId, ActionRow.bBoolParam);
		break;

	case EQuestActionType::SetWorldStateInt:
		Subsystem->SetWorldStateInt(ActionRow.TargetId, ActionRow.IntParam);
		break;

	case EQuestActionType::EnableTarget:
		Subsystem->SetTargetEnabledState(ActionRow.TargetId, true, ContextPayload);
		break;

	case EQuestActionType::DisableTarget:
		Subsystem->SetTargetEnabledState(ActionRow.TargetId, false, ContextPayload);
		break;

	case EQuestActionType::PlaySequence:
		Subsystem->HandlePlaySequenceAction(ActionRow, ContextPayload);
		break;

	case EQuestActionType::StartDialogue:
		Subsystem->HandleStartDialogueAction(ActionRow, ContextPayload);
		break;

	case EQuestActionType::SetTimeOfDay:
		Subsystem->SetCurrentTimeOfDay(static_cast<EQuestTimeOfDay>(ActionRow.IntParam));
		break;

	case EQuestActionType::LoadOrUnlockArea:
		Subsystem->HandleLoadOrUnlockAreaAction(ActionRow, ContextPayload);
		break;

	case EQuestActionType::SpawnActor:
		Subsystem->HandleSpawnActorAction(ActionRow, ContextPayload);
		break;

	case EQuestActionType::RemoveActor:
		Subsystem->HandleRemoveActorAction(ActionRow, ContextPayload);
		break;

	case EQuestActionType::GrantReward:
		Subsystem->HandleGrantRewardAction(ActionRow, ContextPayload);
		break;

	case EQuestActionType::CompleteQuest:
		Subsystem->CompleteQuestById(ActionRow.NameParam.IsNone() ? QuestId : ActionRow.NameParam);
		break;

	case EQuestActionType::FailQuest:
		Subsystem->FailQuestById(ActionRow.NameParam.IsNone() ? QuestId : ActionRow.NameParam);
		break;

	case EQuestActionType::BroadcastEvent:
		Subsystem->HandleBroadcastQuestAction(ActionRow, ContextPayload);
		break;

	default:
		if (!Subsystem->HandleExtendedActionExecution(QuestId, ActionRow, ContextPayload))
		{
			UE_LOG(LogQuest, Warning, TEXT("任务“%s”的动作“%s”未被处理，类型=%s。"),
				*QuestId.ToString(),
				*ActionRow.ActionId.ToString(),
				*StaticEnum<EQuestActionType>()->GetNameStringByValue(static_cast<int64>(ActionRow.ActionType)));
		}
		break;
	}

	if (ShouldDispatchActionToTarget(ActionRow) && !ActionRow.TargetId.IsNone())
	{
		if (UObject* TargetObject = Subsystem->GetRegisteredQuestTarget(ActionRow.TargetId))
		{
			if (TargetObject->GetClass()->ImplementsInterface(UQuestActionTargetInterface::StaticClass()))
			{
				IQuestActionTargetInterface::Execute_ApplyQuestAction(TargetObject, ActionRow, ContextPayload);
			}
		}
	}
}

bool UQuestActionExecutor::ShouldDispatchActionToTarget(const FQuestActionRow& ActionRow) const
{
	switch (ActionRow.ActionType)
	{
	case EQuestActionType::EnableTarget:
	case EQuestActionType::DisableTarget:
	case EQuestActionType::PlaySequence:
	case EQuestActionType::StartDialogue:
	case EQuestActionType::LoadOrUnlockArea:
	case EQuestActionType::SpawnActor:
	case EQuestActionType::RemoveActor:
	case EQuestActionType::BroadcastEvent:
		return true;

	default:
		return ActionRow.bDispatchToTarget;
	}
}
