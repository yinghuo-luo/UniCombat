#include "Framework/Transfer/AtlasTransferSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Framework/Transfer/AtlasTransferSettings.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Framework/AtlasGameInstance.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#include "SaveSystem/Core/AtlasSaveSubsystem.h"


namespace AtlasTransferPrivate
{
	//解析地图名字从世界
	FName ResolveMapNameFromWorld(UWorld* World)
	{
		return World ? FName(*UGameplayStatics::GetCurrentLevelName(World, true)) : NAME_None;
	}
}

void UAtlasTransferSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!PostWorldInitializationHandle.IsValid())
	{
		PostWorldInitializationHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(
			this,
			&ThisClass::HandlePostWorldInitialization);
	}

	if (UAtlasSaveSubsystem* SaveSubsystem = ResolveSaveSubsystem())
	{
		SaveSubsystem->OnLoadFinished.AddDynamic(this, &ThisClass::HandleSaveLoadFinished);
	}
}

void UAtlasTransferSubsystem::Deinitialize()
{
	if (UAtlasSaveSubsystem* SaveSubsystem = ResolveSaveSubsystem())
	{
		SaveSubsystem->OnLoadFinished.RemoveDynamic(this, &ThisClass::HandleSaveLoadFinished);
	}

	if (PostWorldInitializationHandle.IsValid())
	{
		FWorldDelegates::OnPostWorldInitialization.Remove(PostWorldInitializationHandle);
		PostWorldInitializationHandle.Reset();
	}

	UnbindObservedWorldBeginPlay();
	HideLoadingScreen();
	ResetTransferState();

	Super::Deinitialize();
}

bool UAtlasTransferSubsystem::StartTransfer(const FAtlasTransferActionRequest& Request)
{
	if (bTransferActive)
	{
		return false;
	}

	PendingRequest = Request;
	TransferState = EAtlasTransferState::Idle;
	ResolvedTargetMapName = NAME_None;
	bTransferActive = true;

	OnTransferStarted.Broadcast(PendingRequest);

	//RestoreRequest
	if (PendingRequest.RestoreRequest.Source == EAtlasTransferRestoreSource::SaveSlot)
	{
		UAtlasSaveSubsystem* SaveSubsystem = ResolveSaveSubsystem();
		if (!SaveSubsystem)
		{
			FailTransfer(TEXT("Save subsystem is unavailable."));
			return false;
		}

		if (PendingRequest.bShowLoadingScreen)
		{
			ShowLoadingScreen();
		}
		
		FAtlasLoadRequest LoadRequest;
		LoadRequest.SlotName = PendingRequest.RestoreRequest.SaveSlotName;
		LoadRequest.UserIndex = PendingRequest.RestoreRequest.SaveUserIndex;

		if (!SaveSubsystem->Load_SAV(LoadRequest))
		{
			FailTransfer(TEXT("Failed to start loading the save slot."));
			return false;
		}

		TransferState = EAtlasTransferState::LoadingSave;
		return true;
	}

	ContinuePendingTransfer(NAME_None);
	return true;
}

void UAtlasTransferSubsystem::HandleSaveLoadFinished(const FAtlasSaveHeader Header, const bool bSuccess)
{
	if (!bTransferActive || TransferState != EAtlasTransferState::LoadingSave)
	{
		return;
	}

	if (!bSuccess)
	{
		FailTransfer(FString::Printf(TEXT("Failed to load save slot %s."), *Header.SlotName));
		return;
	}

	ContinuePendingTransfer(Header.MapName);
}

void UAtlasTransferSubsystem::HandlePostWorldInitialization(UWorld* World, const UWorld::InitializationValues /*IVS*/)
{
	if (!bTransferActive || TransferState != EAtlasTransferState::WaitingForWorld || !World)
	{
		return;
	}

	if (World->GetGameInstance() != GetGameInstance())
	{
		return;
	}

	if (AtlasTransferPrivate::ResolveMapNameFromWorld(World) != ResolvedTargetMapName)
	{
		return;
	}

	BindObservedWorldBeginPlay(World);

	if (World->HasBegunPlay())
	{
		HandleObservedWorldBeginPlay(World);
	}
}

void UAtlasTransferSubsystem::HandleObservedWorldBeginPlay(UWorld* World)
{
	if (!bTransferActive || TransferState != EAtlasTransferState::WaitingForWorld || !World)
	{
		return;
	}

	FTimerDelegate DeferredFinalizeDelegate;
	DeferredFinalizeDelegate.BindLambda([this, WeakWorld = TWeakObjectPtr<UWorld>(World)]()
	{
		UnbindObservedWorldBeginPlay();

		if (UWorld* FinalWorld = WeakWorld.Get())
		{
			if (FinalizeTransferInWorld(FinalWorld))
			{
				CompleteTransfer();
			}
		}
	});

	World->GetTimerManager().SetTimerForNextTick(DeferredFinalizeDelegate);
}

void UAtlasTransferSubsystem::ContinuePendingTransfer(const FName LoadedSaveMapName)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		FailTransfer(TEXT("Current world is unavailable."));
		return;
	}

	ResolvedTargetMapName = PendingRequest.TarMapName;
	if (ResolvedTargetMapName.IsNone() && PendingRequest.RestoreRequest.bUseLoadedSaveMap)
	{
		if (!LoadedSaveMapName.IsNone())
		{
			ResolvedTargetMapName = LoadedSaveMapName;
		}
		else if (PendingRequest.RestoreRequest.Source == EAtlasTransferRestoreSource::PendingSave)
		{
			if (UAtlasSaveSubsystem* SaveSubsystem = ResolveSaveSubsystem(); SaveSubsystem && SaveSubsystem->HasPendingRestoreData())
			{
				ResolvedTargetMapName = SaveSubsystem->GetPendingRestoreHeader().MapName;
			}
		}
	}

	if (ResolvedTargetMapName.IsNone())
	{
		ResolvedTargetMapName = ResolveCurrentMapName(World);
	}

	if (ResolvedTargetMapName.IsNone())
	{
		FailTransfer(TEXT("Target map could not be resolved."));
		return;
	}

	const FName CurrentMapName = ResolveCurrentMapName(World);
	if (ResolvedTargetMapName == CurrentMapName)
	{
		if (!BeginSameMapTransfer(World))
		{
			FailTransfer(TEXT("Failed to start same-map transfer."));
		}
		return;
	}

	if (PendingRequest.bShowLoadingScreen)
	{
		ShowLoadingScreen();
	}

	if (UAtlasGameInstance* GI = Cast<UAtlasGameInstance>(GetGameInstance()))
	{
		GI->bShouldCreateNewSlot= PendingRequest.bShouldCreateNewSlot;
	}
	
	TransferState = EAtlasTransferState::WaitingForWorld;
	UGameplayStatics::OpenLevel(World, ResolvedTargetMapName);
}

bool UAtlasTransferSubsystem::BeginSameMapTransfer(UWorld* World)
{
	if (!World)
	{
		return false;
	}

	HideLoadingScreen();
	TransferState = EAtlasTransferState::SameMapTeleport;

	const UAtlasTransferSettings* Settings = GetDefault<UAtlasTransferSettings>();
	const float FadeOutDuration = Settings ? FMath::Max(Settings->SameMapFadeOutDuration, 0.0f) : 0.0f;
	const float HoldDuration = Settings ? FMath::Max(Settings->SameMapBlackHoldDuration, 0.0f) : 0.0f;

	if (APlayerController* PlayerController = ResolvePrimaryPlayerController(World))
	{
		if (PlayerController->PlayerCameraManager)
		{
			PlayerController->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, FadeOutDuration, FLinearColor::Black, false, true);
		}
	}

	const float Delay = FadeOutDuration + HoldDuration;
	FTimerDelegate TeleportDelegate;
	TeleportDelegate.BindUObject(this, &ThisClass::HandleSameMapTeleport);
	World->GetTimerManager().SetTimer(SameMapTeleportTimerHandle, TeleportDelegate, Delay, false);
	return true;
}

void UAtlasTransferSubsystem::HandleSameMapTeleport()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		FailTransfer(TEXT("Current world is unavailable."));
		return;
	}

	if (!FinalizeTransferInWorld(World))
	{
		return;
	}

	if (APlayerController* PlayerController = ResolvePrimaryPlayerController(World))
	{
		if (PlayerController->PlayerCameraManager)
		{
			const UAtlasTransferSettings* Settings = GetDefault<UAtlasTransferSettings>();
			const float FadeInDuration = Settings ? FMath::Max(Settings->SameMapFadeInDuration, 0.0f) : 0.0f;
			PlayerController->PlayerCameraManager->StartCameraFade(1.0f, 0.0f, FadeInDuration, FLinearColor::Black, false, true);
		}
	}

	CompleteTransfer();
}

bool UAtlasTransferSubsystem::FinalizeTransferInWorld(UWorld* World)
{
	if (!World)
	{
		FailTransfer(TEXT("Target world is unavailable."));
		return false;
	}

	TransferState = EAtlasTransferState::ApplyingRestore;

	if (!ApplySaveRestore())
	{
		return false;
	}

	if (!ApplyDestination(World))
	{
		return false;
	}

	return true;
}

bool UAtlasTransferSubsystem::ApplySaveRestore()
{
	const FAtlasTransferRestoreRequest& RestoreRequest = PendingRequest.RestoreRequest;
	if (RestoreRequest.Source == EAtlasTransferRestoreSource::None)
	{
		return true;
	}

	UAtlasSaveSubsystem* SaveSubsystem = ResolveSaveSubsystem();
	if (!SaveSubsystem)
	{
		FailTransfer(TEXT("Save subsystem is unavailable."));
		return false;
	}

	if ((RestoreRequest.Source == EAtlasTransferRestoreSource::PendingSave
			|| RestoreRequest.Source == EAtlasTransferRestoreSource::SaveSlot)
		&& !SaveSubsystem->HasPendingRestoreData())
	{
		FailTransfer(TEXT("No pending save restore data is available."));
		return false;
	}

	for (const EAtlasSaveRestoreName ModuleName : RestoreRequest.ProviderModules)
	{
		if (ModuleName == EAtlasSaveRestoreName::None)
		{
			continue;
		}

		const FAtlasRestoreBatchResult Result = SaveSubsystem->ApplyProviderRecords(ModuleName);
		if (!Result.bSuccess)
		{
			FailTransfer(Result.FailureReason.IsEmpty()
				? FString::Printf(TEXT("Failed to apply provider restore module %d."), static_cast<int32>(ModuleName))
				: Result.FailureReason);
			return false;
		}
	}

	for (const EAtlasSaveRestoreName ModuleName : RestoreRequest.ObjectModules)
	{
		if (ModuleName == EAtlasSaveRestoreName::None)
		{
			continue;
		}

		const FAtlasRestoreBatchResult Result = SaveSubsystem->ApplyObjectRecords(ModuleName);
		if (!Result.bSuccess)
		{
			FailTransfer(Result.FailureReason.IsEmpty()
				? FString::Printf(TEXT("Failed to apply object restore module %d."), static_cast<int32>(ModuleName))
				: Result.FailureReason);
			return false;
		}
	}

	return true;
}

bool UAtlasTransferSubsystem::ApplyDestination(UWorld* World)
{
	FTransform DestinationTransform = FTransform::Identity;
	if (!ResolveDestinationTransform(World, DestinationTransform))
	{
		return false;
	}

	if (PendingRequest.CheckpointTag.IsNone() && !PendingRequest.bUseSpawnTransform)
	{
		return true;
	}

	APawn* PlayerPawn = ResolvePlayerPawn(World);
	if (!PlayerPawn)
	{
		FailTransfer(TEXT("Player pawn was not found in the target world."));
		return false;
	}

	PlayerPawn->SetActorTransform(DestinationTransform);
	return true;
}

bool UAtlasTransferSubsystem::ResolveDestinationTransform(UWorld* World, FTransform& OutTransform)
{
	if (!World)
	{
		return false;
	}

	if (!PendingRequest.CheckpointTag.IsNone())
	{
		if (const AActor* CheckpointActor = FindCheckpointActor(World, PendingRequest.CheckpointTag))
		{
			OutTransform = CheckpointActor->GetActorTransform();
			return true;
		}

		FailTransfer(FString::Printf(TEXT("Checkpoint tag %s was not found."), *PendingRequest.CheckpointTag.ToString()));
		return false;
	}

	if (PendingRequest.bUseSpawnTransform)
	{
		OutTransform = PendingRequest.SpawnTransform;
		return true;
	}

	return true;
}

AActor* UAtlasTransferSubsystem::FindCheckpointActor(UWorld* World, const FName CheckpointTag) const
{
	if (!World || CheckpointTag.IsNone())
	{
		return nullptr;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Candidate = *It;
		if (IsValid(Candidate) && Candidate->Tags.Contains(CheckpointTag))
		{
			return Candidate;
		}
	}

	return nullptr;
}

APlayerController* UAtlasTransferSubsystem::ResolvePrimaryPlayerController(UWorld* World) const
{
	return World ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;
}

APawn* UAtlasTransferSubsystem::ResolvePlayerPawn(UWorld* World) const
{
	if (APlayerController* PlayerController = ResolvePrimaryPlayerController(World))
	{
		return PlayerController->GetPawn();
	}

	return nullptr;
}

FName UAtlasTransferSubsystem::ResolveCurrentMapName(UWorld* World) const
{
	return AtlasTransferPrivate::ResolveMapNameFromWorld(World);
}

void UAtlasTransferSubsystem::BindObservedWorldBeginPlay(UWorld* World)
{
	if (!World)
	{
		return;
	}

	if (ObservedTargetWorld.Get() == World && WorldBeginPlayHandle.IsValid())
	{
		return;
	}

	UnbindObservedWorldBeginPlay();
	ObservedTargetWorld = World;
	WorldBeginPlayHandle = World->OnWorldBeginPlay.AddUObject(this, &ThisClass::HandleObservedWorldBeginPlay, World);
}

void UAtlasTransferSubsystem::UnbindObservedWorldBeginPlay()
{
	//
	if (ObservedTargetWorld.IsValid() && WorldBeginPlayHandle.IsValid())
	{
		ObservedTargetWorld->OnWorldBeginPlay.Remove(WorldBeginPlayHandle);
	}

	ObservedTargetWorld.Reset();
	WorldBeginPlayHandle.Reset();
}

void UAtlasTransferSubsystem::ShowLoadingScreen()
{
	//显示加载界面
	if (LoadingScreenContent.IsValid() || !GEngine ||
		!GEngine->GameViewport)
	{
		return;
	}

	const UAtlasTransferSettings* Settings = GetDefault<UAtlasTransferSettings>();
	const int32 ZOrder = Settings ? Settings->LoadingWidgetZOrder : 10000;

	if (Settings && !Settings->LoadingWidgetClass.IsNull())
	{
		if (TSubclassOf<UUserWidget> LoadingWidgetClass = Settings->LoadingWidgetClass.LoadSynchronous())
		{
			if (APlayerController* PlayerController = ResolvePrimaryPlayerController(GetWorld()))
			{
				LoadingWidgetInstance = CreateWidget<UUserWidget>(PlayerController, LoadingWidgetClass);
			}
			else
			{
				LoadingWidgetInstance = CreateWidget<UUserWidget>(GetGameInstance(), LoadingWidgetClass);
			}

			if (LoadingWidgetInstance)
			{
				LoadingWidgetInstance->SetVisibility(ESlateVisibility::Visible);
				LoadingWidgetInstance->AddToViewport(ZOrder);
				return;
			}
		}
	}
	
	if (!LoadingScreenContent.IsValid())
	{
		LoadingScreenContent =
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor::Black)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("AtlasTransfer", "LoadingText", "Loading..."))
			];
	}

	GEngine->GameViewport->AddViewportWidgetContent(LoadingScreenContent.ToSharedRef(), ZOrder);
}

void UAtlasTransferSubsystem::HideLoadingScreen()
{
	//隐藏加载界面
	if (LoadingWidgetInstance)
	{
		LoadingWidgetInstance->RemoveFromParent();
		LoadingWidgetInstance = nullptr;
	}

	if (LoadingScreenContent.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(LoadingScreenContent.ToSharedRef());
	}

	LoadingScreenContent.Reset();
}

void UAtlasTransferSubsystem::CompleteTransfer()
{
	FAtlasTransferResult Result;
	Result.bSucceeded = true;
	Result.FinalState = TransferState;
	Result.FinalMapName = ResolvedTargetMapName;
	Result.Request = PendingRequest;

	HideLoadingScreen();
	ResetTransferState();
	OnTransferFinished.Broadcast(Result);
}

void UAtlasTransferSubsystem::FailTransfer(const FString& FailureReason)
{
	if (!bTransferActive)
	{
		return;
	}

	FAtlasTransferResult Result;
	Result.bSucceeded = false;
	Result.FinalState = TransferState;
	Result.FinalMapName = ResolvedTargetMapName;
	Result.FailureReason = FailureReason;
	Result.Request = PendingRequest;

	HideLoadingScreen();
	ResetTransferState();
	OnTransferFinished.Broadcast(Result);
}

void UAtlasTransferSubsystem::ResetTransferState()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SameMapTeleportTimerHandle);
	}

	UnbindObservedWorldBeginPlay();
	PendingRequest = FAtlasTransferActionRequest();
	TransferState = EAtlasTransferState::Idle;
	ResolvedTargetMapName = NAME_None;
	bTransferActive = false;
}

UAtlasSaveSubsystem* UAtlasTransferSubsystem::ResolveSaveSubsystem() const
{
	//解析保存系统
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UAtlasSaveSubsystem>();
	}

	return nullptr;
}
