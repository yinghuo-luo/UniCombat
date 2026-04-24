#pragma once

#include "CoreMinimal.h"
#include "Framework/Transfer/AtlasTransferTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AtlasTransferSubsystem.generated.h"

class AActor;
class APlayerController;
class APawn;
class SWidget;
class UAtlasSaveSubsystem;
class UUserWidget;
class UWorld;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAtlasTransferStartedSignature, FAtlasTransferActionRequest, Request);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAtlasTransferFinishedSignature, FAtlasTransferResult, Result);

UCLASS(BlueprintType)
class UNICOMBAT_API UAtlasTransferSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Transfer")
	bool StartTransfer(const FAtlasTransferActionRequest& Request);

	UFUNCTION(BlueprintPure, Category = "Transfer")
	bool IsTransferRunning() const { return bTransferActive; }

	UFUNCTION(BlueprintPure, Category = "Transfer")
	EAtlasTransferState GetTransferState() const { return TransferState; }

	UFUNCTION(BlueprintPure, Category = "Transfer")
	FName GetResolvedTargetMapName() const { return ResolvedTargetMapName; }

	UPROPERTY(BlueprintAssignable, Category = "Transfer")
	FAtlasTransferStartedSignature OnTransferStarted;

	UPROPERTY(BlueprintAssignable, Category = "Transfer")
	FAtlasTransferFinishedSignature OnTransferFinished;

protected:
	UFUNCTION()
	void HandleSaveLoadFinished(FAtlasSaveHeader Header, bool bSuccess);

	void HandlePostWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS);
	void HandleObservedWorldBeginPlay(UWorld* World);
	void ContinuePendingTransfer(FName LoadedSaveMapName);
	bool BeginSameMapTransfer(UWorld* World);
	void HandleSameMapTeleport();
	bool FinalizeTransferInWorld(UWorld* World);
	bool ApplySaveRestore();
	bool ApplyDestination(UWorld* World);
	bool ResolveDestinationTransform(UWorld* World, FTransform& OutTransform);
	AActor* FindCheckpointActor(UWorld* World, FName CheckpointTag) const;
	APlayerController* ResolvePrimaryPlayerController(UWorld* World) const;
	APawn* ResolvePlayerPawn(UWorld* World) const;
	FName ResolveCurrentMapName(UWorld* World) const;
	void BindObservedWorldBeginPlay(UWorld* World);
	void UnbindObservedWorldBeginPlay();
	void ShowLoadingScreen();
	void HideLoadingScreen();
	void CompleteTransfer();
	void FailTransfer(const FString& FailureReason);
	void ResetTransferState();
	UAtlasSaveSubsystem* ResolveSaveSubsystem() const;

protected:
	UPROPERTY(Transient)
	FAtlasTransferActionRequest PendingRequest;

	UPROPERTY(Transient)
	EAtlasTransferState TransferState = EAtlasTransferState::Idle;

	UPROPERTY(Transient)
	FName ResolvedTargetMapName = NAME_None;

	UPROPERTY(Transient)
	bool bTransferActive = false;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> LoadingWidgetInstance = nullptr;

	TSharedPtr<SWidget> LoadingScreenContent;
	TWeakObjectPtr<UWorld> ObservedTargetWorld;
	FDelegateHandle PostWorldInitializationHandle;
	FDelegateHandle WorldBeginPlayHandle;
	FTimerHandle SameMapTeleportTimerHandle;
};
