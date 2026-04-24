// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestConditionEvaluator.h"

#include "QuestSubsystem.h"

void UQuestConditionEvaluator::Initialize(UQuestSubsystem* InSubsystem)
{
	QuestSubsystem = InSubsystem;
}

bool UQuestConditionEvaluator::EvaluateConditionList(
	const TArray<FName>& ConditionIds,
	const FQuestRuntimeState& RuntimeState,
	const FQuestEventPayload* EventPayload,
	FString* OutFailureReason) const
{
	if (ConditionIds.IsEmpty())
	{
		return true;
	}

	const UQuestSubsystem* Subsystem = QuestSubsystem.Get();
	if (!Subsystem)
	{
		if (OutFailureReason)
		{
			*OutFailureReason = TEXT("任务子系统为空。");
		}
		return false;
	}

	bool bHasCombinedResult = false;
	bool bCombinedResult = false;

	for (const FName ConditionId : ConditionIds)
	{
		const FQuestConditionRow* Condition = 
			Subsystem->FindConditionDefinition(RuntimeState.QuestId, ConditionId);
		if (!Condition)
		{
			if (OutFailureReason)
			{
				*OutFailureReason = FString::Printf(TEXT("未找到条件“%s”。"), *ConditionId.ToString());
			}
			return false;
		}

		FString LocalFailureReason;
		const bool bConditionResult = EvaluateCondition(*Condition, RuntimeState, EventPayload, &LocalFailureReason);

		if (!bHasCombinedResult)
		{
			bCombinedResult = bConditionResult;
			bHasCombinedResult = true;
		}
		else if (Condition->GroupOp == EQuestConditionGroupOp::All)
		{
			bCombinedResult = bCombinedResult && bConditionResult;
		}
		else
		{
			bCombinedResult = bCombinedResult || bConditionResult;
		}

		if (!bConditionResult && OutFailureReason && OutFailureReason->IsEmpty())
		{
			*OutFailureReason = LocalFailureReason;
		}
	}

	return bCombinedResult;
}

bool UQuestConditionEvaluator::EvaluateCondition(
	const FQuestConditionRow& Condition,
	const FQuestRuntimeState& RuntimeState,
	const FQuestEventPayload* EventPayload,
	FString* OutFailureReason) const
{
	const UQuestSubsystem* Subsystem = QuestSubsystem.Get();
	if (!Subsystem)
	{
		if (OutFailureReason)
		{
			*OutFailureReason = TEXT("任务子系统为空。");
		}
		return false;
	}

	const auto Fail = [OutFailureReason](const FString& Reason) -> bool
	{
		if (OutFailureReason)
		{
			*OutFailureReason = Reason;
		}
		return false;
	};

	switch (Condition.ConditionType)
	{
	case EQuestConditionType::PreviousNodeCompleted:
		return EvaluateNameComparison(
			Subsystem->GetNodeState(RuntimeState.QuestId, Condition.ParamId) == EQuestNodeState::Completed ? Condition.ParamId : NAME_None,
			Condition.CompareOp,
			Condition.ParamId)
			? true
			: Fail(FString::Printf(TEXT("前置节点“%s”尚未完成。"), *Condition.ParamId.ToString()));

	case EQuestConditionType::QuestFlag:
	{
		const bool bFlagValue = Subsystem->GetQuestFlag(RuntimeState.QuestId, Condition.ParamId);
		return EvaluateBoolComparison(bFlagValue, Condition.CompareOp, Condition.bBoolValue)
			? true
			: Fail(FString::Printf(TEXT("任务标记“%s”比较失败。"), *Condition.ParamId.ToString()));
	}

	case EQuestConditionType::QuestCounter:
	{
		const int32 CounterValue = Subsystem->GetQuestCounter(RuntimeState.QuestId, Condition.ParamId);
		return EvaluateIntComparison(CounterValue, Condition.CompareOp, Condition.IntValue)
			? true
			: Fail(FString::Printf(TEXT("任务计数器“%s”比较失败。"), *Condition.ParamId.ToString()));
	}

	case EQuestConditionType::WorldStateBool:
	{
		const bool bValue = Subsystem->GetWorldStateBool(Condition.ParamId);
		return EvaluateBoolComparison(bValue, Condition.CompareOp, Condition.bBoolValue)
			? true
			: Fail(FString::Printf(TEXT("世界布尔状态“%s”比较失败。"), *Condition.ParamId.ToString()));
	}

	case EQuestConditionType::WorldStateInt:
	{
		const int32 Value = Subsystem->GetWorldStateInt(Condition.ParamId);
		return EvaluateIntComparison(Value, Condition.CompareOp, Condition.IntValue)
			? true
			: Fail(FString::Printf(TEXT("世界整数状态“%s”比较失败。"), *Condition.ParamId.ToString()));
	}

	case EQuestConditionType::EnteredRegion:
		if (!EventPayload || EventPayload->EventType != EQuestEventType::EnteredRegion)
		{
			return Fail(TEXT("缺少进入区域事件。"));
		}
		return Condition.ParamId.IsNone() || EventPayload->TargetId == Condition.ParamId
			? true
			: Fail(FString::Printf(TEXT("进入区域目标不匹配，期望“%s”。"), *Condition.ParamId.ToString()));

	case EQuestConditionType::InteractionCompleted:
		if (!EventPayload || EventPayload->EventType != EQuestEventType::InteractionCompleted)
		{
			return Fail(TEXT("缺少交互完成事件。"));
		}
		return Condition.ParamId.IsNone() || EventPayload->TargetId == Condition.ParamId
			? true
			: Fail(FString::Printf(TEXT("交互目标不匹配，期望“%s”。"), *Condition.ParamId.ToString()));

	case EQuestConditionType::DialogueCompleted:
		if (!EventPayload || EventPayload->EventType != EQuestEventType::DialogueCompleted)
		{
			return Fail(TEXT("缺少对话完成事件。"));
		}
		return Condition.ParamId.IsNone() || EventPayload->TargetId == Condition.ParamId
			? true
			: Fail(FString::Printf(TEXT("对话目标不匹配，期望“%s”。"), *Condition.ParamId.ToString()));

	case EQuestConditionType::EnemyKilled:
		if (!EventPayload || EventPayload->EventType != EQuestEventType::EnemyKilled)
		{
			return Fail(TEXT("缺少击杀敌人事件。"));
		}
		if (!Condition.ParamId.IsNone() && EventPayload->TargetId != Condition.ParamId)
		{
			return Fail(FString::Printf(TEXT("敌人目标不匹配，期望“%s”。"), *Condition.ParamId.ToString()));
		}
		return EvaluateIntComparison(EventPayload->IntValue, Condition.CompareOp, FMath::Max(Condition.IntValue, 1))
			? true
			: Fail(TEXT("击杀数量比较失败。"));

	case EQuestConditionType::BossDefeated:
		if (!EventPayload || EventPayload->EventType != EQuestEventType::BossDefeated)
		{
			return Fail(TEXT("缺少 Boss 击败事件。"));
		}
		return Condition.ParamId.IsNone() || EventPayload->TargetId == Condition.ParamId
			? true
			: Fail(FString::Printf(TEXT("Boss 目标不匹配，期望“%s”。"), *Condition.ParamId.ToString()));

	case EQuestConditionType::PuzzleSolved:
		if (!EventPayload || EventPayload->EventType != EQuestEventType::PuzzleSolved)
		{
			return Fail(TEXT("缺少谜题完成事件。"));
		}
		return Condition.ParamId.IsNone() || EventPayload->TargetId == Condition.ParamId
			? true
			: Fail(FString::Printf(TEXT("谜题目标不匹配，期望“%s”。"), *Condition.ParamId.ToString()));

	case EQuestConditionType::HasItem:
	{
		const int32 ItemCount = Subsystem->GetInventoryItemCount(Condition.ParamId);
		return EvaluateIntComparison(ItemCount, Condition.CompareOp, FMath::Max(Condition.IntValue, 1))
			? true
			: Fail(FString::Printf(TEXT("物品“%s”数量比较失败。"), *Condition.ParamId.ToString()));
	}

	case EQuestConditionType::TimeOfDay:
		return EvaluateIntComparison(
			static_cast<int32>(Subsystem->GetCurrentTimeOfDay()),
			Condition.CompareOp,
			Condition.IntValue)
			? true
			: Fail(TEXT("时间条件比较失败。"));

	case EQuestConditionType::ChoiceResult:
	{
		const FName ChoiceValue = Subsystem->GetSelectedChoice(RuntimeState.QuestId, Condition.ParamId);
		return EvaluateNameComparison(ChoiceValue, Condition.CompareOp, Condition.ParamName)
			? true
			: Fail(FString::Printf(TEXT("选择“%s”与期望结果“%s”不匹配。"), *Condition.ParamId.ToString(), *Condition.ParamName.ToString()));
	}

	case EQuestConditionType::RitualStepCompleted:
		if (!EventPayload || EventPayload->EventType != EQuestEventType::RitualStepCompleted)
		{
			return Fail(TEXT("缺少仪式步骤完成事件。"));
		}
		return Condition.ParamId.IsNone() || EventPayload->TargetId == Condition.ParamId
			? true
			: Fail(FString::Printf(TEXT("仪式步骤目标不匹配，期望“%s”。"), *Condition.ParamId.ToString()));

	case EQuestConditionType::QuestState:
		return EvaluateIntComparison(static_cast<int32>(RuntimeState.State), Condition.CompareOp, Condition.IntValue)
			? true
			: Fail(TEXT("任务状态比较失败。"));

	case EQuestConditionType::CustomEvent:
		if (!EventPayload || EventPayload->EventType != EQuestEventType::Custom)
		{
			return Fail(TEXT("缺少自定义事件。"));
		}
		if (!Condition.ParamId.IsNone() && EventPayload->TargetId != Condition.ParamId)
		{
			return Fail(FString::Printf(TEXT("自定义事件目标不匹配，期望“%s”。"), *Condition.ParamId.ToString()));
		}
		if (!Condition.ParamName.IsNone() && EventPayload->EventName != Condition.ParamName)
		{
			return Fail(FString::Printf(TEXT("自定义事件名称不匹配，期望“%s”。"), *Condition.ParamName.ToString()));
		}
		return true;

	default:
		{
			FQuestEventPayload EmptyPayload;
			const FQuestEventPayload& ContextPayload = EventPayload ? *EventPayload : EmptyPayload;
			bool bConditionResult = false;
			FString LocalFailureReason;
			if (Subsystem->HandleExtendedConditionEvaluation(Condition, RuntimeState, ContextPayload, bConditionResult, LocalFailureReason))
			{
				if (!bConditionResult && OutFailureReason && !LocalFailureReason.IsEmpty())
				{
					*OutFailureReason = LocalFailureReason;
				}
				return bConditionResult;
			}

			return Fail(TEXT("不支持的条件类型。"));
		}
	}
}

bool UQuestConditionEvaluator::EvaluateIntComparison(const int32 LeftValue, const EQuestCompareOp CompareOp, const int32 RightValue) const
{
	switch (CompareOp)
	{
	case EQuestCompareOp::Equal:
		return LeftValue == RightValue;
	case EQuestCompareOp::NotEqual:
		return LeftValue != RightValue;
	case EQuestCompareOp::Greater:
		return LeftValue > RightValue;
	case EQuestCompareOp::GreaterOrEqual:
		return LeftValue >= RightValue;
	case EQuestCompareOp::Less:
		return LeftValue < RightValue;
	case EQuestCompareOp::LessOrEqual:
		return LeftValue <= RightValue;
	default:
		return false;
	}
}

bool UQuestConditionEvaluator::EvaluateBoolComparison(const bool bLeftValue, const EQuestCompareOp CompareOp, const bool bRightValue) const
{
	switch (CompareOp)
	{
	case EQuestCompareOp::Equal:
		return bLeftValue == bRightValue;
	case EQuestCompareOp::NotEqual:
		return bLeftValue != bRightValue;
	default:
		return false;
	}
}

bool UQuestConditionEvaluator::EvaluateNameComparison(const FName LeftValue, const EQuestCompareOp CompareOp, const FName RightValue) const
{
	switch (CompareOp)
	{
	case EQuestCompareOp::Equal:
		return LeftValue == RightValue;
	case EQuestCompareOp::NotEqual:
		return LeftValue != RightValue;
	default:
		return false;
	}
}