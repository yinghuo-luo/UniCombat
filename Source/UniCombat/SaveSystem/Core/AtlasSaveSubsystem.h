// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AtlasSaveVersionPolicy.h"
#include "SaveSystem/AtlasSaveTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AtlasSaveSubsystem.generated.h"

class UAtlasSaveDataAdapterBase;
class UAtlasSaveSlotIndexSaveGame;
class USaveGame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAtlasSaveHeaderEvent, FAtlasSaveHeader, Header);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAtlasSaveResultEvent, FAtlasSaveHeader, Header, bool, bSuccess);


/**
 * 
 */
UCLASS()
class UNICOMBAT_API UAtlasSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	bool SaveManual(const FString& SlotName,const FString& DisplayName);
	bool SaveAutosave(FName ContextId);
	bool SaveCheckpoint(FName CheckpointId);
	
	bool Load_SAV(const FAtlasLoadRequest& Request);
	bool DeleteSlot(const FString& SlotName, int32 UserIndex);
	
	TArray<FAtlasSaveSlotDescriptor> EnumerateSlots() const;
	FAtlasRestoreBatchResult ApplyProviderRecords(EAtlasSaveRestoreName Name);
	FAtlasRestoreBatchResult ApplyObjectRecords(EAtlasSaveRestoreName Name);

	UFUNCTION(BlueprintPure, Category = "Save")
	bool HasPendingRestoreData() const { return PendingRestoreSaveGame != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Save")
	FAtlasSaveHeader GetPendingRestoreHeader() const { return PendingRestoreHeader; }
	
	UPROPERTY(BlueprintAssignable, Category = "Save")
	FAtlasSaveHeaderEvent OnSaveStarted;
	UPROPERTY(BlueprintAssignable, Category = "Save")
	FAtlasSaveResultEvent OnSaveFinished;
	UPROPERTY(BlueprintAssignable, Category = "Save")
	FAtlasSaveHeaderEvent OnLoadStarted;
	UPROPERTY(BlueprintAssignable, Category = "Save")
	FAtlasSaveResultEvent OnLoadFinished;

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool HasSlotDescriptors();
	UFUNCTION(BlueprintCallable, Category = "Save")
	FString GetLatestAutoSaveParam(FAtlasSaveHeader& OutHeader);
	
	//component
	void RegisterSaveableObject(UObject* SaveableObject);
	void UnregisterSaveableObject(UObject* SaveableObject);
protected:
	void BuildSaveHeader(const FAtlasSaveRequest&Request , FAtlasSaveHeader& OutHeader) const;
	FAtlasSaveExecutionContext BuildSaveExecutionContext(const FAtlasSaveRequest& Request
		,UAtlasRunSaveGame* SaveGameObject) const;
	
	//对数据提供来源的处理
	void GatherProviderRecords(const FAtlasSaveExecutionContext& Context,UAtlasRunSaveGame& SaveGameObject);
	void GatherObjectRecords(UAtlasRunSaveGame& SaveGameObject);
	
	
	int32 DefaultUserIndex = 0;
	int32 SlotIndexUserIndex = 0;
	FString SlotIndexSlotName = TEXT("MSD_SlotIndex");
	FString DefaultAutoSaveSlot = TEXT("Auto_Default");
	FString CheckPointSlotPrefix = TEXT("Checkpoint");
	FSoftClassPath VersionPolicyClass;
	TArray<FSoftClassPath> DefaultProviderClasses;
	
	UPROPERTY(Transient)
	TObjectPtr<UAtlasSaveVersionPolicy> VersionPolicy = nullptr;
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UAtlasSaveDataAdapterBase>> RegisteredProviders;
	
	UPROPERTY(Transient)
	TObjectPtr<UAtlasRunSaveGame> PendingRestoreSaveGame = nullptr;
	UPROPERTY(Transient)
	TObjectPtr<UAtlasRunSaveGame> PendingAsyncSaveGame = nullptr;
	UPROPERTY(Transient)
	FAtlasSaveHeader PendingAsyncSaveHeader;
	
	UPROPERTY(Transient)
	FAtlasSaveHeader CurrentLoadedHeader;
	UPROPERTY(Transient)
	FString ActiveSlotName;
	
	UPROPERTY(Transient)
	TObjectPtr<UAtlasSaveSlotIndexSaveGame> CachedSlotIndex = nullptr;
	UPROPERTY(Transient)
	TObjectPtr<UAtlasSaveSlotIndexSaveGame> PendingSlotIndexSaveGame = nullptr;

	/*----加载----*/
	UPROPERTY(Transient)
	FAtlasSaveHeader PendingRestoreHeader;
private:
	bool SaveToSlot(const FAtlasSaveRequest& Request);
	bool LoadFromSlot(const FAtlasLoadRequest& Request);
	
	void RegisterConfiguredProviders();
	void UnregisterProviderById(FName ProviderId);
	void ClearPendingRestoreState();
	bool RegisterProvider(UAtlasSaveDataAdapterBase* Provider);
	
	////Adapter
	void HandleAsyncSaveFinished(const FString& SlotName, int32 UserIndex, bool bSuccess);
	void UpdateSlotIndex(const FAtlasSaveHeader& SaveHeader, int32 UserIndex);
	UAtlasSaveSlotIndexSaveGame* LoadOrCreateSlotIndex();
	
	void QueueSlotIndexSave();
	void StartSlotIndexSave();
	void HandleAsyncSlotIndexSaveFinished(const FString& SlotName, int32 UserIndex, bool bSuccess);
	
	//component类型
	void DiscoverSaveablesInWorld();
	void CleanupInvalidSaveables();
	bool TryGetObjectTransform(UObject* SaveableObject, FTransform& OutTransform) const;
	
	/*----加载----*/
	void HandleAsyncLoadFinished(const FString& SlotName, int32 UserIndex, USaveGame* LoadedGameData);
	bool ValidateLoadedSave(const UAtlasRunSaveGame* SaveGameObject, FString& OutFailureReason) const;
	FAtlasLoadExecutionContext BuildLoadExecutionContext(
		const FString& SlotName,int32 UserIndex,const UAtlasRunSaveGame* SaveGameObject,
		EAtlasSaveRestoreName Name) const;
	
	UObject* ResolveSaveableObject(const FGuid& ObjectId);
	bool TryApplyTransformToObject(UObject* SaveableObject, const FTransform& Transform) const;
	/*----保存----*/
	//Adapter
	bool bSaveInProgress = false;
	bool bLoadInProgress = false;
	FAtlasSaveRequest PendingSaveRequest;
	
	bool bSlotIndexDirty = false;
	bool bSlotIndexSaveInProgress = false;
	
	//component 
	TArray<TWeakObjectPtr<UObject>> RegisteredSaveableObjects;
	TSet<FGuid> AppliedObjectIds;
	
	/*----加载----*/
	FAtlasLoadRequest PendingLoadRequest;
	TSet<FName> AppliedProviderIds;
	int32 PendingRestoreUserIndex = 0;
};
