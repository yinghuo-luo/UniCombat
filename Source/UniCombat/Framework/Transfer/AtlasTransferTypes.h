#pragma once

#include "CoreMinimal.h"
#include "SaveSystem/AtlasSaveTypes.h"
#include "AtlasTransferTypes.generated.h"

UENUM(BlueprintType)
enum class EAtlasTransferRestoreSource : uint8
{
	None,
	PendingSave,
	SaveSlot
};

UENUM(BlueprintType)
enum class EAtlasTransferState : uint8
{
	Idle,
	LoadingSave, //加载保存
	SameMapTeleport, //同地图传送
	WaitingForWorld, //等待世界
	ApplyingRestore //应用恢复
};

USTRUCT(BlueprintType)
struct FAtlasTransferRestoreRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transfer|Restore")
	EAtlasTransferRestoreSource Source = EAtlasTransferRestoreSource::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transfer|Restore")
	FString SaveSlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transfer|Restore")
	int32 SaveUserIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transfer|Restore")
	bool bUseLoadedSaveMap = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transfer|Restore")
	TArray<EAtlasSaveRestoreName> ProviderModules;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transfer|Restore")
	TArray<EAtlasSaveRestoreName> ObjectModules;
};

USTRUCT(BlueprintType)
struct FAtlasTransferActionRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transfer")
	FName TarMapName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transfer")
	FName CheckpointTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transfer")
	bool bUseSpawnTransform = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transfer")
	bool bShouldCreateNewSlot = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transfer")
	FTransform SpawnTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transfer")
	bool bShowLoadingScreen = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transfer")
	FString DebugName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transfer")
	FAtlasTransferRestoreRequest RestoreRequest;
};

USTRUCT(BlueprintType)
struct FAtlasTransferResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transfer")
	bool bSucceeded = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transfer")
	EAtlasTransferState FinalState = EAtlasTransferState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transfer")
	FName FinalMapName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transfer")
	FString FailureReason;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transfer")
	FAtlasTransferActionRequest Request;
};
