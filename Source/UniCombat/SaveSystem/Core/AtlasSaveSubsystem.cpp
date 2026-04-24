// Fill out your copyright notice in the Description page of Project Settings.


#include "AtlasSaveSubsystem.h"

#include "AtlasSaveGame.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "SaveSystem/Objects/AtlasSaveableObjectInterface.h"
#include "SaveSystem/Providers/AtlasSaveDataAdapterBase.h"
#include "SaveSystem/Setting/AtlasSaveSystemSetting.h"

DEFINE_LOG_CATEGORY_STATIC(LogMSDSaveSystem, Log, All);

void UAtlasSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	//加载
	const UAtlasSaveSystemSetting * Setting = GetDefault<UAtlasSaveSystemSetting>();
	if (Setting)
	{
		DefaultUserIndex = Setting->DefaultUserIndex;
		SlotIndexUserIndex = Setting->SlotIndexUserIndex;
		SlotIndexSlotName = Setting->SlotIndexSlotName;
		DefaultAutoSaveSlot = Setting->DefaultAutosaveSlot;
		CheckPointSlotPrefix = Setting->CheckpointSlotPrefix;
		
		VersionPolicyClass = Setting->VersionPolicyClass;
		DefaultProviderClasses = Setting->DefaultProviderClasses;
	}
	
	//如果未设置版本策略类则加载默认
	UClass* PolicyClass = UAtlasSaveVersionPolicy::StaticClass();
	if (!VersionPolicyClass.IsNull())
	{
		if (UClass* LoadedPolicyClass = VersionPolicyClass.TryLoadClass<UAtlasSaveVersionPolicy>())
		{
			PolicyClass = LoadedPolicyClass;
		}
	}
	VersionPolicy = NewObject<UAtlasSaveVersionPolicy>(this, PolicyClass);
	
	//
	RegisterConfiguredProviders();
	CachedSlotIndex = LoadOrCreateSlotIndex();
}

void UAtlasSaveSubsystem::Deinitialize()
{
	for (TPair<FName, TObjectPtr<UAtlasSaveDataAdapterBase>>& Pair : RegisteredProviders)
	{
		if (IsValid(Pair.Value))
		{
			Pair.Value->DeinitializeAdapter();
		}
	}
	
	RegisteredProviders.Reset();
	RegisteredSaveableObjects.Reset();
	ClearPendingRestoreState();
	PendingAsyncSaveGame = nullptr;
	PendingSlotIndexSaveGame = nullptr;
	CachedSlotIndex = nullptr;
	PendingAsyncSaveHeader = FAtlasSaveHeader();
	PendingSaveRequest = FAtlasSaveRequest();
	PendingLoadRequest = FAtlasLoadRequest();
	bSaveInProgress = false;
	bLoadInProgress = false;
	bSlotIndexSaveInProgress = false;
	bSlotIndexDirty = false;
	
	Super::Deinitialize();
}

bool UAtlasSaveSubsystem::SaveManual(const FString& SlotName, const FString& DisplayName)
{
	FAtlasSaveRequest Request;
	Request.SlotName = SlotName;
	Request.DisplayName = DisplayName;
	Request.UserIndex = DefaultUserIndex;
	Request.SaveType = EAtlasSaveType::Manual;
	return SaveToSlot(Request);
}

bool UAtlasSaveSubsystem::SaveAutosave(FName ContextId)
{
	FAtlasSaveRequest Request;
	Request.SlotName = ContextId.IsNone() ? 
		DefaultAutoSaveSlot : FString::Printf(TEXT("Auto_%s"), *ContextId.ToString());
	Request.DisplayName = Request.SlotName;
	Request.UserIndex = DefaultUserIndex;
	Request.SaveType = EAtlasSaveType::AutoSave;
	Request.ContextId = ContextId;
	return SaveToSlot(Request);
}

bool UAtlasSaveSubsystem::SaveCheckpoint(FName CheckpointId)
{
	FString CurrentSlotName = CheckpointId.IsNone()
		? FString::Printf(TEXT("%s_Default"), *CheckPointSlotPrefix)
		: FString::Printf(TEXT("%s_%s"), *CheckPointSlotPrefix, *CheckpointId.ToString());
	
	FAtlasSaveRequest Request;
	Request.SlotName = CurrentSlotName;
	Request.DisplayName = Request.SlotName;
	Request.UserIndex = DefaultUserIndex;
	Request.SaveType = EAtlasSaveType::CheckPoint;
	Request.ContextId = CheckpointId;
	return SaveToSlot(Request);
}

bool UAtlasSaveSubsystem::Load_SAV(const FAtlasLoadRequest& Request)
{
	return LoadFromSlot(Request);
}

FAtlasRestoreBatchResult UAtlasSaveSubsystem::ApplyProviderRecords(EAtlasSaveRestoreName Name)
{
	FAtlasRestoreBatchResult Result;
	Result.RestoreName = Name;

	if (!PendingRestoreSaveGame)
	{
		Result.FailureReason = TEXT("No loaded save is pending restore.");
		return Result;
	}
	for (const FAtlasProviderRecord& Record : PendingRestoreSaveGame->ProviderRecords)
	{
		if (Record.RestoreName != Name)
		{
			continue;
		}

		++Result.MatchingRecordCount;
		if (AppliedProviderIds.Contains(Record.ProviderId))
		{
			++Result.AlreadyAppliedCount;
			continue;
		}

		if (TObjectPtr<UAtlasSaveDataAdapterBase>* FoundProvider = 
			RegisteredProviders.Find(Record.ProviderId))
		{
			UAtlasSaveDataAdapterBase* Provider = FoundProvider->Get();
			if (IsValid(Provider))
			{
				const FAtlasLoadExecutionContext Context = BuildLoadExecutionContext(PendingRestoreHeader.SlotName,
					PendingRestoreUserIndex, PendingRestoreSaveGame, Name);
				
				Provider->OnPreLoad(Context, &Record);
				if (Provider->ApplyLoadedData(Context, Record))
				{
					AppliedProviderIds.Add(Record.ProviderId);
					Provider->OnPostLoad(Context, &Record);
					++Result.AppliedCount;
				}
				else
				{
					Provider->OnPostLoad(Context, nullptr);
					++Result.FailedCount;
					if (Result.FailureReason.IsEmpty())
					{
						Result.FailureReason = FString::Printf(
							TEXT("Provider %s failed to apply restore module %d."),
							*Record.ProviderId.ToString(),
							static_cast<int32>(Name));
					}
					UE_LOG(LogMSDSaveSystem, Warning, TEXT("应用模块 %d 的提供程序记录 %s 失败。"),
						static_cast<int32>(Name),
						*Record.ProviderId.ToString());
				}
			}
			else
			{
				UE_LOG(LogMSDSaveSystem, Warning, TEXT("应用模块 %d 时，已注册的提供程序 %s 无效。"),
					static_cast<int32>(Name),
					*Record.ProviderId.ToString());
				++Result.FailedCount;
				if (Result.FailureReason.IsEmpty())
				{
					Result.FailureReason = FString::Printf(
						TEXT("Registered provider %s is invalid for restore module %d."),
						*Record.ProviderId.ToString(),
						static_cast<int32>(Name));
				}
			}
		}
		else
		{
			UE_LOG(LogMSDSaveSystem, Warning, TEXT("应用模块 %d 时，未找到记录 %s 对应的已注册提供程序。"),
				static_cast<int32>(Name),
				*Record.ProviderId.ToString());
			++Result.FailedCount;
			if (Result.FailureReason.IsEmpty())
			{
				Result.FailureReason = FString::Printf(
					TEXT("No registered provider was found for %s in restore module %d."),
					*Record.ProviderId.ToString(),
					static_cast<int32>(Name));
			}
		}
	}

	Result.bSuccess = Result.FailedCount == 0 && Result.FailureReason.IsEmpty();
	return Result;
}

FAtlasRestoreBatchResult UAtlasSaveSubsystem::ApplyObjectRecords(EAtlasSaveRestoreName Name)
{
	FAtlasRestoreBatchResult Result;
	Result.RestoreName = Name;

	if (!PendingRestoreSaveGame)
	{
		Result.FailureReason = TEXT("No loaded save is pending restore.");
		return Result;
	}

	CleanupInvalidSaveables();
	DiscoverSaveablesInWorld();

	for (const FAtlasObjectSaveRecord& Record : PendingRestoreSaveGame->ObjectRecords)
	{
		if (Record.RestoreName != Name)
		{
			continue;
		}

		++Result.MatchingRecordCount;
		if (AppliedObjectIds.Contains(Record.ObjectId))
		{
			++Result.AlreadyAppliedCount;
			continue;
		}

		UObject* SaveableObject = ResolveSaveableObject(Record.ObjectId);
		
		if (!IsValid(SaveableObject))
		{
			++Result.FailedCount;
			if (Result.FailureReason.IsEmpty())
			{
				Result.FailureReason = FString::Printf(
					TEXT("Failed to resolve saveable object %s for restore module %d."),
					*Record.ObjectId.ToString(EGuidFormats::DigitsWithHyphens),
					static_cast<int32>(Name));
			}
			UE_LOG(LogMSDSaveSystem, Warning, TEXT("未找到保存的Object"));
			continue;
		}

		if (Record.bHasTransform && !TryApplyTransformToObject(SaveableObject, Record.Transform))
		{
			++Result.FailedCount;
			if (Result.FailureReason.IsEmpty())
			{
				Result.FailureReason = FString::Printf(
					TEXT("Failed to apply transform for object %s in restore module %d."),
					*Record.ObjectId.ToString(EGuidFormats::DigitsWithHyphens),
					static_cast<int32>(Name));
			}
			UE_LOG(LogMSDSaveSystem, Warning, TEXT("Failed to apply transform for object %s in restore module %d."),
				*Record.ObjectId.ToString(EGuidFormats::DigitsWithHyphens),
				static_cast<int32>(Name));
			continue;
		}
		//恢复数据
		if (SaveableObject->GetClass()->ImplementsInterface(UAtlasSaveableObjectInterface::StaticClass()))
		{
			IAtlasSaveableObjectInterface::Execute_RestoreSaveData(SaveableObject, Record.Payload);
			AppliedObjectIds.Add(Record.ObjectId);
			++Result.AppliedCount;
		}
		else
		{
			++Result.FailedCount;
			if (Result.FailureReason.IsEmpty())
			{
				Result.FailureReason = FString::Printf(
					TEXT("Resolved object %s no longer implements the saveable interface."),
					*Record.ObjectId.ToString(EGuidFormats::DigitsWithHyphens));
			}
			UE_LOG(LogMSDSaveSystem, Warning, TEXT("Resolved object %s no longer implements the saveable interface."),
				*Record.ObjectId.ToString(EGuidFormats::DigitsWithHyphens));
		}
	}

	Result.bSuccess = Result.FailedCount == 0 && Result.FailureReason.IsEmpty();
	return Result;
}

bool UAtlasSaveSubsystem::HasSlotDescriptors()
{
	return CachedSlotIndex->SlotDescriptors.Num()>0 ? true : false;
}

FString UAtlasSaveSubsystem::GetLatestAutoSaveParam(FAtlasSaveHeader& OutHeader)
{
	for (FAtlasSaveSlotDescriptor Item : CachedSlotIndex->SlotDescriptors)
	{
		if (Item.Header.SaveType == EAtlasSaveType::AutoSave)
		{
			OutHeader = Item.Header;
			return Item.SlotName;
		}
	}
	OutHeader = FAtlasSaveHeader();
	return FString("");
}

void UAtlasSaveSubsystem::BuildSaveHeader(const FAtlasSaveRequest& Request, FAtlasSaveHeader& OutHeader) const
{
	OutHeader.SlotName = Request.SlotName;
	OutHeader.SaveDisplayName = Request.DisplayName.IsEmpty()?Request.SlotName:Request.DisplayName;
	OutHeader.TimeStampUtc = FDateTime::UtcNow();
	OutHeader.SaveVersion = FMath::Max(VersionPolicy? VersionPolicy->GetCurrentSaveVersion() : 1, 1);
	
	const TCHAR* BuildVersion = FApp::GetBuildVersion();
	OutHeader.BuildVersion = (BuildVersion && BuildVersion[0] !=TEXT('\0')) ? 
		BuildVersion : FEngineVersion::Current().ToString();
	
	OutHeader.MapName = GetWorld() 
		? FName(*UWorld::RemovePIEPrefix(GetWorld()->GetMapName())) 
		: NAME_None;

	OutHeader.PlayTimeSeconds = 0.f;
	OutHeader.SaveType = Request.SaveType;
	OutHeader.ContextId = Request.ContextId;
	OutHeader.ContextTags = Request.ContextTags;
}

FAtlasSaveExecutionContext UAtlasSaveSubsystem::BuildSaveExecutionContext(const FAtlasSaveRequest& Request,
	UAtlasRunSaveGame* SaveGameObject) const
{
	FAtlasSaveExecutionContext Context;
	Context.SlotName = Request.SlotName;
	Context.UserIndex = Request.UserIndex;
	Context.SaveType = Request.SaveType;
	Context.ContextId = Request.ContextId;
	Context.ContextTags = Request.ContextTags;
	Context.World = GetWorld();
	Context.GameInstance = GetGameInstance();
	Context.SaveGame = SaveGameObject;
	return Context;
}

void UAtlasSaveSubsystem::GatherProviderRecords(const FAtlasSaveExecutionContext& Context,
	UAtlasRunSaveGame& SaveGameObject)
{
	for (TPair<FName,TObjectPtr<UAtlasSaveDataAdapterBase>>& Pair : RegisteredProviders)
	{
		UAtlasSaveDataAdapterBase* Provider= Pair.Value;
		if (!IsValid(Provider) || !Provider->SupportsSaveType(Context.SaveType))
			continue;
		Provider->OnPreSave(Context);
		
		FAtlasProviderRecord Record;
		if (Provider->GatherSaveData(Context, Record))
		{
			if (Record.ProviderId.IsNone())
			{
				Record.ProviderId = Provider->GetProviderId();
			}
			if (Record.ProviderVersion <= 0)
			{
				Record.ProviderVersion = Provider->GetProviderVersion();
			}
			if (Record.DebugName.IsEmpty())
			{
				Record.DebugName = Provider->GetDebugName();
			}
			
			SaveGameObject.ProviderRecords.Add(Record);
			Provider->OnPostSave(Context,&Record);
		}
		else
		{
			Provider->OnPostSave(Context, nullptr);
		}
	}
}

void UAtlasSaveSubsystem::GatherObjectRecords(UAtlasRunSaveGame& SaveGameObject)
{
	DiscoverSaveablesInWorld();
	CleanupInvalidSaveables();
	
	auto HasMeaningfulIdentity = [](const FAtlasSaveObjectIdentity& Identity) -> bool
	{
		return Identity.IdentityKind != EAtlasSaveObjectIdentityKind::Unknown
			|| Identity.StableId.IsValid()
			|| Identity.OwningLevelName != NAME_None
			|| !Identity.PlacedObjectPath.IsNull()
			|| Identity.SpawnSourceId != NAME_None
			|| !Identity.LogicalKey.IsEmpty();
	};
	for (const TWeakObjectPtr<UObject>& SaveablePtr : RegisteredSaveableObjects)
	{
		UObject* SaveableObject = SaveablePtr.Get();
		if (!IsValid(SaveableObject) || !SaveableObject->GetClass()->
			ImplementsInterface(UAtlasSaveableObjectInterface::StaticClass()))
		{
			continue;
		}
		if (!IAtlasSaveableObjectInterface::Execute_ShouldSave(SaveableObject))
		{
			continue;
		}
		const FGuid ObjectId = IAtlasSaveableObjectInterface::Execute_GetSaveObjectId(SaveableObject);
		if (!ObjectId.IsValid())
		{
			continue;
		}
		FAtlasSavePayload Payload;
		if (!IAtlasSaveableObjectInterface::Execute_CaptureSaveData(SaveableObject, Payload))
		{
			continue;
		}
		
		FAtlasObjectSaveRecord Record;
		Record.ObjectId = ObjectId;
		Record.ClassPath = FSoftClassPath(SaveableObject->GetClass());
		Record.RestoreName = IAtlasSaveableObjectInterface::Execute_GetRestoreName(SaveableObject);
		Record.Payload = MoveTemp(Payload);
		
		FAtlasSaveObjectIdentity Identity;
		if (IAtlasSaveableObjectInterface::Execute_GetSaveIdentity(SaveableObject, Identity) && HasMeaningfulIdentity(Identity))
		{
			Record.Identity = MoveTemp(Identity);
		}
		
		if (IAtlasSaveableObjectInterface::Execute_ShouldCaptureTransform(SaveableObject))
		{
			Record.bHasTransform = TryGetObjectTransform(SaveableObject, Record.Transform);
		}

		SaveGameObject.ObjectRecords.Add(MoveTemp(Record));
	}
}

bool UAtlasSaveSubsystem::SaveToSlot(const FAtlasSaveRequest& Request)
{
	if (Request.SlotName.IsEmpty() || bSaveInProgress || bLoadInProgress)
	{
		if (bSaveInProgress || bLoadInProgress)
		{
			UE_LOG(LogMSDSaveSystem, Warning, TEXT("拒绝保存槽位 %s，因为另一个保存或加载操作正在进行中。"),
				*Request.SlotName);
		}
		return false;
	}
	
	UAtlasRunSaveGame*SaveGameObject = Cast<UAtlasRunSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UAtlasRunSaveGame::StaticClass()));
	if (!SaveGameObject)
		return false;
	
	BuildSaveHeader(Request,SaveGameObject->Header);
	OnSaveStarted.Broadcast(SaveGameObject->Header); //广播开始保存
	
	const FAtlasSaveExecutionContext Context = BuildSaveExecutionContext(Request, SaveGameObject);
	GatherProviderRecords(Context,*SaveGameObject);
	GatherObjectRecords(*SaveGameObject);

	PendingAsyncSaveGame = SaveGameObject;
	PendingSaveRequest = Request;
	PendingAsyncSaveHeader = SaveGameObject->Header;
	bSaveInProgress = true;
	
	FAsyncSaveGameToSlotDelegate SaveDelegate;
	SaveDelegate.BindUObject(this,&ThisClass::HandleAsyncSaveFinished);
	UGameplayStatics::AsyncSaveGameToSlot(SaveGameObject,Request.SlotName,Request.UserIndex,SaveDelegate);
	return true;
	
}

bool UAtlasSaveSubsystem::LoadFromSlot(const FAtlasLoadRequest& Request)
{
	if (Request.SlotName.IsEmpty() || bSaveInProgress || bLoadInProgress)
	{
		if (bSaveInProgress || bLoadInProgress)
		{
			UE_LOG(LogMSDSaveSystem, Warning, TEXT("拒绝加载槽位 %s，因为另一个保存或加载操作正在进行中。"),
				*Request.SlotName);
		}
		return false;
	}
	ClearPendingRestoreState();
	
	if (!UGameplayStatics::DoesSaveGameExist(Request.SlotName, Request.UserIndex))
	{
		return false;
	}
	
	PendingLoadRequest = Request;
	bLoadInProgress = true;

	FAtlasSaveHeader LoadingHeader;
	LoadingHeader.SlotName = Request.SlotName;
	OnLoadStarted.Broadcast(LoadingHeader);
	
	FAsyncLoadGameFromSlotDelegate LoadDelegate;
	LoadDelegate.BindUObject(this, &ThisClass::HandleAsyncLoadFinished);

	UGameplayStatics::AsyncLoadGameFromSlot(Request.SlotName, Request.UserIndex, LoadDelegate);

	//数据已经读入，可以按模块加载
	return true;
}

bool UAtlasSaveSubsystem::DeleteSlot(const FString& SlotName, int32 UserIndex)
{
	if (SlotName.IsEmpty())
	{
		return false;
	}

	const bool bDeleted = UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);
	if (bDeleted)
	{
		if (UAtlasSaveSlotIndexSaveGame* SlotIndex = LoadOrCreateSlotIndex())
		{
			SlotIndex->SlotDescriptors.RemoveAll([&SlotName, UserIndex](const FAtlasSaveSlotDescriptor& Descriptor)
			{
				return Descriptor.SlotName == SlotName && Descriptor.UserIndex == UserIndex;
			});

			QueueSlotIndexSave();
		}
		if (ActiveSlotName == SlotName)
		{
			ActiveSlotName.Reset();
		}
	}

	return bDeleted;
}

TArray<FAtlasSaveSlotDescriptor> UAtlasSaveSubsystem::EnumerateSlots() const
{
	TArray<FAtlasSaveSlotDescriptor> OutDescriptors;
	if (!CachedSlotIndex)
	{
		return OutDescriptors;
	}

	OutDescriptors = CachedSlotIndex->SlotDescriptors;
	OutDescriptors.Sort([](const FAtlasSaveSlotDescriptor& A, const FAtlasSaveSlotDescriptor& B)
	{
		return A.Header.TimeStampUtc > B.Header.TimeStampUtc;
	});

	return OutDescriptors;
}

void UAtlasSaveSubsystem::RegisterConfiguredProviders()
{
	for (const FSoftClassPath& ProviderClassPath : DefaultProviderClasses)
	{
		if (ProviderClassPath.IsNull())
		{
			UE_LOG(LogMSDSaveSystem, Warning, TEXT("Skipping null save provider class entry in AtlasSaveSystemSetting.DefaultProviderClasses."));
			continue;
		}

		if (UClass* ProviderClass = ProviderClassPath.TryLoadClass<UAtlasSaveDataAdapterBase>())
		{
			UAtlasSaveDataAdapterBase* Provider = NewObject<UAtlasSaveDataAdapterBase>(this, ProviderClass);
			//将生成的Provider放入Registered Providers数组
			if (!RegisterProvider(Provider))
			{
				UE_LOG(LogMSDSaveSystem, Warning, TEXT("Failed to register configured save provider %s."), *ProviderClassPath.ToString());
			}
		}
		else
		{
			UE_LOG(LogMSDSaveSystem, Warning, TEXT("Failed to load configured save provider class %s."), *ProviderClassPath.ToString());
		}
	}
}

void UAtlasSaveSubsystem::UnregisterProviderById(FName ProviderId)
{
	if (TObjectPtr<UAtlasSaveDataAdapterBase>* ExistingProvider = RegisteredProviders.Find(ProviderId))
	{
		if (IsValid(*ExistingProvider))
		{
			(*ExistingProvider)->DeinitializeAdapter();
		}

		RegisteredProviders.Remove(ProviderId);
	}
}

void UAtlasSaveSubsystem::ClearPendingRestoreState()
{
	PendingRestoreSaveGame = nullptr;
	PendingRestoreHeader = FAtlasSaveHeader();
	PendingRestoreUserIndex = DefaultUserIndex;
	AppliedProviderIds.Reset();
	AppliedObjectIds.Reset();
	
}

bool UAtlasSaveSubsystem::RegisterProvider(UAtlasSaveDataAdapterBase* Provider)
{
	if (!IsValid(Provider))
	{
		return false;
	}

	const FName ProviderId = Provider->GetProviderId();
	
	if (ProviderId.IsNone())
	{
		UE_LOG(LogMSDSaveSystem, Warning, TEXT("保存提供程序时，ProviderId 为空，因此被拒绝: %s"), *GetNameSafe(Provider));
		return false;
	}

	if (TObjectPtr<UAtlasSaveDataAdapterBase>* ExistingProvider = RegisteredProviders.Find(ProviderId))
	{
		if (*ExistingProvider == Provider)
		{
			return true; //已有
		}

		if (IsValid(*ExistingProvider))
		{
			(*ExistingProvider)->DeinitializeAdapter();
		}
	}

	Provider->InitializeAdapter(this);
	RegisteredProviders.Add(ProviderId, Provider);
	return true;
}

void UAtlasSaveSubsystem::HandleAsyncSaveFinished(const FString& SlotName, int32 UserIndex, bool bSuccess)
{
	const FAtlasSaveRequest CompletedRequest = PendingSaveRequest;
	const FAtlasSaveHeader CompletedHeader = PendingAsyncSaveHeader;
	
	bSaveInProgress = false;
	PendingAsyncSaveGame = nullptr;
	PendingSaveRequest = FAtlasSaveRequest();
	PendingAsyncSaveHeader = FAtlasSaveHeader();
	
	if (bSuccess)
	{
		if (CompletedRequest.bSetActiveSlot)
		{
			ActiveSlotName = SlotName;
		}
		CurrentLoadedHeader = CompletedHeader;
		UpdateSlotIndex(CompletedHeader, UserIndex);
	}
	OnSaveFinished.Broadcast(CompletedHeader, bSuccess);
}

void UAtlasSaveSubsystem::UpdateSlotIndex(const FAtlasSaveHeader& SaveHeader, int32 UserIndex)
{
	if (UAtlasSaveSlotIndexSaveGame* SlotIndex = LoadOrCreateSlotIndex())
	{
		/** FindByPredicate
		 * 查找与谓词函子匹配的元素。
		* @param Pred 要应用于每个元素的函子。如果未找到匹配项，则返回 true 或 nullptr。
		* @see FilterByPredicate, ContainsByPredicate
		*/
		FAtlasSaveSlotDescriptor* ExistingDescriptor = SlotIndex->SlotDescriptors.
			FindByPredicate([&SaveHeader, UserIndex](const FAtlasSaveSlotDescriptor& Descriptor)
		{
			return Descriptor.SlotName == SaveHeader.SlotName && Descriptor.UserIndex == UserIndex;
		});
		if (!ExistingDescriptor)
		{
			/**
			* 在数组末尾添加一个新元素，可能会重新分配整个数组空间。
			* 新元素将使用默认构造函数创建。
			* @return 对新插入元素的引用。
			* @see Add_GetRef、AddZeroed_GetRef、AddUnique_GetRef、Insert_GetRef
			*/
			ExistingDescriptor = &SlotIndex->SlotDescriptors.AddDefaulted_GetRef();
		}
		ExistingDescriptor->SlotName = SaveHeader.SlotName;
		ExistingDescriptor->UserIndex = UserIndex;
		ExistingDescriptor->Header = SaveHeader;
		QueueSlotIndexSave();
	}
}

UAtlasSaveSlotIndexSaveGame* UAtlasSaveSubsystem::LoadOrCreateSlotIndex()
{
	if (CachedSlotIndex)
		return CachedSlotIndex;
	
	if (UGameplayStatics::DoesSaveGameExist(SlotIndexSlotName, SlotIndexUserIndex))
	{
		/**
		* 从指定的存档槽加载内容。
		* @param SlotName 要加载的存档槽的名称。
		* @param UserIndex 用于标识执行保存操作的用户的平台用户索引，在某些平台上会被忽略。
		* @return 包含已加载游戏状态的对象（如果加载失败则返回 nullptr）。
		*/
		if (UAtlasSaveSlotIndexSaveGame* SlotIndex = Cast<UAtlasSaveSlotIndexSaveGame>(
			UGameplayStatics::LoadGameFromSlot(SlotIndexSlotName, SlotIndexUserIndex)))
		{
			CachedSlotIndex = SlotIndex;
			return CachedSlotIndex;
		}
	}

	CachedSlotIndex = Cast<UAtlasSaveSlotIndexSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UAtlasSaveSlotIndexSaveGame::StaticClass()));
	return CachedSlotIndex;
}

void UAtlasSaveSubsystem::QueueSlotIndexSave()
{
	if (!LoadOrCreateSlotIndex())
		return;
	bSlotIndexDirty = true;
	if (!bSlotIndexSaveInProgress)
	{
		StartSlotIndexSave();
	}
}

void UAtlasSaveSubsystem::StartSlotIndexSave()
{
	if (!CachedSlotIndex || !bSlotIndexDirty)
		return;
	
	UAtlasSaveSlotIndexSaveGame* SlotIndexSnapshot = Cast<UAtlasSaveSlotIndexSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UAtlasSaveSlotIndexSaveGame::StaticClass()));
	if (!SlotIndexSnapshot)
		return;
	
	SlotIndexSnapshot->SlotDescriptors = CachedSlotIndex->SlotDescriptors;
	PendingSlotIndexSaveGame = SlotIndexSnapshot;
	bSlotIndexDirty = false;
	bSlotIndexSaveInProgress = true;
	
	FAsyncSaveGameToSlotDelegate SaveDelegate;
	SaveDelegate.BindUObject(this, &ThisClass::HandleAsyncSlotIndexSaveFinished);
	UGameplayStatics::AsyncSaveGameToSlot(SlotIndexSnapshot, SlotIndexSlotName, SlotIndexUserIndex, SaveDelegate);
	
}

void UAtlasSaveSubsystem::HandleAsyncSlotIndexSaveFinished(const FString& SlotName, int32 UserIndex, bool bSuccess)
{
	bSlotIndexSaveInProgress = false;
	PendingSlotIndexSaveGame = nullptr;
	
	if (!bSuccess)
	{
		UE_LOG(LogMSDSaveSystem, Warning, TEXT("保存槽位索引 %s [%d] 失败。"), *SlotName, UserIndex);
	}
	
	//
	if (bSlotIndexDirty)
	{
		StartSlotIndexSave();
	}
}

void UAtlasSaveSubsystem::DiscoverSaveablesInWorld()
{
	//遍历世界中查找那些带有数据存储组件
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!IsValid(Actor))
			{
				continue;
			}

			if (Actor->GetClass()->ImplementsInterface(UAtlasSaveableObjectInterface::StaticClass()))
			{
				RegisterSaveableObject(Actor);
			}

			TInlineComponentArray<UActorComponent*> Components(Actor);
			for (UActorComponent* Component : Components)
			{
				if (IsValid(Component) && Component->GetClass()->ImplementsInterface(UAtlasSaveableObjectInterface::StaticClass()))
				{
					RegisterSaveableObject(Component);
				}
			}
		}
	}
}

void UAtlasSaveSubsystem::RegisterSaveableObject(UObject* SaveableObject)
{
	/**
	* ImplementsInterface 此函数将返回该类是否实现了传入的类/接口
	* @param SomeClass - 要检查的接口，并判断该类是否实现了它
	*/
	if (!IsValid(SaveableObject) || !SaveableObject->GetClass()->
		ImplementsInterface(UAtlasSaveableObjectInterface::StaticClass()))
	{
		return;
	}

	RegisteredSaveableObjects.AddUnique(TWeakObjectPtr<UObject>(SaveableObject));
}

void UAtlasSaveSubsystem::UnregisterSaveableObject(UObject* SaveableObject)
{
	/*这个函数的逻辑就是：遍历 RegisteredSaveableObjects找到里面指向 SaveableObject 的那些元素全部删掉
	* RemoveAll 是 TArray 的一个接口，作用是：删除所有满足条件的元素，
	* 它会遍历数组，把 lambda 返回 true 的元素全部移除
	 */
	RegisteredSaveableObjects.RemoveAll([SaveableObject](const TWeakObjectPtr<UObject>& Candidate)
	{
		return Candidate.Get() == SaveableObject;
	});
}

void UAtlasSaveSubsystem::CleanupInvalidSaveables()
{
	RegisteredSaveableObjects.RemoveAll([](const TWeakObjectPtr<UObject>& Candidate)
	{
		return !Candidate.IsValid();
	});
}

bool UAtlasSaveSubsystem::TryGetObjectTransform(UObject* SaveableObject, FTransform& OutTransform) const
{
	if (const AActor* Actor = Cast<AActor>(SaveableObject))
	{
		OutTransform = Actor->GetActorTransform();
		return true;
	}

	if (const USceneComponent* SceneComponent = Cast<USceneComponent>(SaveableObject))
	{
		OutTransform = SceneComponent->GetComponentTransform();
		return true;
	}

	if (const UActorComponent* ActorComponent = Cast<UActorComponent>(SaveableObject))
	{
		if (const AActor* OwnerActor = ActorComponent->GetOwner())
		{
			OutTransform = OwnerActor->GetActorTransform();
			return true;
		}
	}

	return false;
}

void UAtlasSaveSubsystem::HandleAsyncLoadFinished(const FString& SlotName, int32 UserIndex, USaveGame* LoadedGameData)
{
	const FAtlasLoadRequest CompletedRequest = PendingLoadRequest;
	PendingLoadRequest = FAtlasLoadRequest();
	bLoadInProgress = false;

	UAtlasRunSaveGame* SaveGameObject = Cast<UAtlasRunSaveGame>(LoadedGameData);
	if (!SaveGameObject)
	{
		FAtlasSaveHeader FailedHeader;
		FailedHeader.SlotName = SlotName;
		OnLoadFinished.Broadcast(FailedHeader, false);
		return;
	}
	
	FString FailureReason;
	if (!ValidateLoadedSave(SaveGameObject, FailureReason))
	{
		UE_LOG(LogMSDSaveSystem, Warning, TEXT("校验存档槽位 %s 失败：%s"), *SlotName, *FailureReason);
		OnLoadFinished.Broadcast(SaveGameObject->Header, false);
		return;
	}
	
	if (CompletedRequest.bSetActiveSlot)
	{
		ActiveSlotName = SlotName;
	}
	
	PendingRestoreSaveGame = SaveGameObject;
	PendingRestoreHeader = SaveGameObject->Header;
	PendingRestoreUserIndex = UserIndex;
	CurrentLoadedHeader = SaveGameObject->Header;

	OnLoadFinished.Broadcast(PendingRestoreHeader, true);
}

bool UAtlasSaveSubsystem::ValidateLoadedSave(const UAtlasRunSaveGame* SaveGameObject, FString& OutFailureReason) const
{
	//通过版本策略，判断是否能加载这份数据
	if (!SaveGameObject)
	{
		OutFailureReason = TEXT("Loaded save object is null.");
		return false;
	}

	if (VersionPolicy && !VersionPolicy->CanLoadHeader(SaveGameObject->Header, 
		OutFailureReason))
	{
		return false;
	}

	if (VersionPolicy)
	{
		for (const FAtlasProviderRecord& Record : SaveGameObject->ProviderRecords)
		{
			if (!VersionPolicy->CanLoadProviderRecord(Record, OutFailureReason))
			{
				return false;
			}
		}
	}

	return true;
}

FAtlasLoadExecutionContext UAtlasSaveSubsystem::BuildLoadExecutionContext(const FString& SlotName, int32 UserIndex,
	const UAtlasRunSaveGame* SaveGameObject, EAtlasSaveRestoreName Name) const
{
	FAtlasLoadExecutionContext Context;
	Context.SlotName = SlotName;
	Context.UserIndex = UserIndex;
	Context.World = GetWorld();
	Context.GameInstance = GetGameInstance();
	Context.SaveGame = SaveGameObject;
	Context.RestoreName = Name;
	return Context;
}

UObject* UAtlasSaveSubsystem::ResolveSaveableObject(const FGuid& ObjectId)
{
	//解析可保存对象
	//清楚无效对象
	CleanupInvalidSaveables();

	auto FindById = [&ObjectId](const TArray<TWeakObjectPtr<UObject>>& Candidates) -> UObject*
	{
		for (const TWeakObjectPtr<UObject>& Candidate : Candidates)
		{
			UObject* CandidateObject = Candidate.Get();
			if (!IsValid(CandidateObject) || !CandidateObject->GetClass()
				->ImplementsInterface(UAtlasSaveableObjectInterface::StaticClass()))
			{
				continue;
			}

			if (IAtlasSaveableObjectInterface::Execute_GetSaveObjectId(CandidateObject) == ObjectId)
			{
				return CandidateObject;
			}
		}

		return nullptr;
	};

	if (UObject* ExistingObject = FindById(RegisteredSaveableObjects))
	{
		return ExistingObject;
	}

	//如果数组没有就查找一遍再找一次
	DiscoverSaveablesInWorld();
	return FindById(RegisteredSaveableObjects);
}

bool UAtlasSaveSubsystem::TryApplyTransformToObject(UObject* SaveableObject, const FTransform& Transform) const
{
	//恢复位置
	if (AActor* Actor = Cast<AActor>(SaveableObject))
	{
		Actor->SetActorTransform(Transform);
		return true;
	}

	if (USceneComponent* SceneComponent = Cast<USceneComponent>(SaveableObject))
	{
		SceneComponent->SetWorldTransform(Transform);
		return true;
	}

	if (UActorComponent* ActorComponent = Cast<UActorComponent>(SaveableObject))
	{
		if (AActor* OwnerActor = ActorComponent->GetOwner())
		{
			OwnerActor->SetActorTransform(Transform);
			return true;
		}
	}

	return false;
}
	
