// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AtlasSaveTypes.generated.h"

class UGameInstance;
class UAtlasRunSaveGame;
class UWorld;

UENUM(BlueprintType)
enum class EAtlasSaveType : uint8
{
	Manual,
	AutoSave,
	CheckPoint
};

UENUM(BlueprintType)
enum class EAtlasSaveRestoreName : uint8
{
	None,
	GeneralParam,
	QuestSystem,
	CombatPlayer,
	Inventory,
	World_Data,
};

UENUM(BlueprintType)
enum class EAtlasSaveObjectIdentityKind : uint8
{
	Unknown, //未知
	StaticPlaced, //静态放置
	RuntimeSpawned, //运行时生成
	LogicalEntity //逻辑实体
};

USTRUCT(BlueprintType)
struct FAtlasSavePayload
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FName PayloadType = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 SchemaVersion = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TArray<uint8> Bytes; //实际数据存储的有效字节

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FString DebugSummary;
};

USTRUCT(BlueprintType)
struct FAtlasSaveHeader
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)
	FString SlotName;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)
	FString SaveDisplayName;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)
	FDateTime TimeStampUtc; //时间戳
	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)
	int32 SaveVersion = 1;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)
	FString BuildVersion;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)
	FName MapName = NAME_None;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)
	float PlayTimeSeconds = 0.f;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)
	EAtlasSaveType SaveType = EAtlasSaveType::Manual;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)
	FName ContextId = NAME_None;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)
	FGameplayTagContainer ContextTags;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)
	TArray<uint8> PreviewImageBytes;
};

USTRUCT(BlueprintType)
struct FAtlasProviderRecord
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FName ProviderId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 ProviderVersion = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FDateTime LastModified;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FString DebugName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	EAtlasSaveRestoreName  RestoreName= EAtlasSaveRestoreName::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FAtlasSavePayload Payload;
};
USTRUCT(BlueprintType)
struct FAtlasSaveObjectIdentity
{
	GENERATED_BODY()
	//身份类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	EAtlasSaveObjectIdentityKind IdentityKind = EAtlasSaveObjectIdentityKind::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FGuid StableId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FName OwningLevelName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FSoftObjectPath PlacedObjectPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FName SpawnSourceId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FString LogicalKey;
};


USTRUCT(BlueprintType)
struct FAtlasObjectSaveRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FGuid ObjectId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FSoftClassPath ClassPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FTransform Transform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	bool bHasTransform = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	EAtlasSaveRestoreName RestoreName = EAtlasSaveRestoreName::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FAtlasSavePayload Payload;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FAtlasSaveObjectIdentity Identity;
};

USTRUCT(BlueprintType)
struct FAtlasDynamicSpawnRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FAtlasSaveObjectIdentity Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FSoftClassPath SpawnClassPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FTransform SpawnTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FName LevelName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	EAtlasSaveRestoreName RestoreModuleName = EAtlasSaveRestoreName::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FAtlasSavePayload Payload;
};

USTRUCT(BlueprintType)
struct FAtlasDestroyedObjectRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FAtlasSaveObjectIdentity Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FName LevelName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	bool bDestroyed = true;
};


USTRUCT(BlueprintType)
struct FAtlasSaveSlotDescriptor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FString SlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 UserIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FAtlasSaveHeader Header;
};

USTRUCT(BlueprintType)
struct FAtlasSaveRequest
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FString SlotName;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FString DisplayName;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	int32 UserIndex = 0;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	EAtlasSaveType SaveType = EAtlasSaveType::Manual;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FName ContextId = NAME_None;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FGameplayTagContainer ContextTags;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool bSetActiveSlot = true;
};

USTRUCT(BlueprintType)
struct FAtlasLoadRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UserIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSetActiveSlot = true;
};

USTRUCT(BlueprintType)
struct FAtlasRestoreBatchResult
{
	GENERATED_BODY()

	/*字段含义：
	RestoreName 说明这次回执对应哪个恢复模块。
	MatchingRecordCount 当前待恢复存档里，RestoreName == Name 的记录总数。
	AlreadyAppliedCount 这些记录里，有多少在本次 pending restore 生命周期里已经应用过了。 来源是 AppliedProviderIds / AppliedObjectIds 去重。
	AppliedCount 本次调用里真正成功应用的记录数。
	FailedCount 本次调用里恢复失败的记录数。
	bSuccess 这批恢复是否整体成功。当前逻辑是“没有失败且没有失败原因”才算成功。
	FailureReason 第一条失败原因，给上层做提示或日志。*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EAtlasSaveRestoreName RestoreName = EAtlasSaveRestoreName::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MatchingRecordCount = 0; //匹配记录数

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 AlreadyAppliedCount = 0; //已申请计数

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 AppliedCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 FailedCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString FailureReason;
};

struct UNICOMBAT_API FAtlasSaveExecutionContext
{
	FString SlotName;
	int32 UserIndex = 0;
	EAtlasSaveType SaveType = EAtlasSaveType::Manual;
	FName ContextId = NAME_None;
	FGameplayTagContainer ContextTags;
	UWorld* World = nullptr;
	UGameInstance* GameInstance = nullptr;
	UAtlasRunSaveGame* SaveGame = nullptr;
};

struct UNICOMBAT_API FAtlasLoadExecutionContext
{
	FString SlotName;
	int32 UserIndex = 0;
	UWorld* World = nullptr;
	UGameInstance* GameInstance = nullptr;
	const UAtlasRunSaveGame* SaveGame = nullptr;
	EAtlasSaveRestoreName  RestoreName= EAtlasSaveRestoreName::None;
};
