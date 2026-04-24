// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "QuestEventTypes.generated.h"

class AActor;
class UObject;

UENUM(BlueprintType)
enum class EQuestEventType : uint8
{
	None,
	EnteredRegion, //已经进入区域
	InteractionCompleted, //已完成交互
	DialogueCompleted, //已完成对话
	EnemyKilled, //已击杀敌人
	BossDefeated, //已击败首领
	PuzzleSolved, //已解密
	RitualStepCompleted, //已完成仪式步骤
	ChoiceMade, //已做出选择
	ItemAcquired, //已获得物品
	TimeOfDayChanged, //已更改时间
	Custom 
};

/*
 * 任务事件有效载荷
 */
USTRUCT(BlueprintType)
struct FQuestEventPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EQuestEventType EventType = EQuestEventType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName QuestId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName NodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TargetId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EventName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ChoiceKey = NAME_None; //选择键

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ChoiceResult = NAME_None; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 IntValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBoolValue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> InstigatorActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UObject> SourceObject = nullptr;
};

