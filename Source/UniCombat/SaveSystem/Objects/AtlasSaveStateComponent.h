// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AtlasSaveableObjectInterface.h"
#include "Components/ActorComponent.h"
#include "AtlasSaveStateComponent.generated.h"


UCLASS(Blueprintable,ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UNICOMBAT_API UAtlasSaveStateComponent : public UActorComponent,public IAtlasSaveableObjectInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAtlasSaveStateComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void OnRegister() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
	UFUNCTION(BlueprintCallable, Category = "Save")
	void EnsurePersistentId();

	UFUNCTION(BlueprintCallable, Category = "Save")
	void MarkSaveDirty();

	UFUNCTION(BlueprintCallable, Category = "Save")
	void ClearSaveDirty();

	virtual FGuid GetSaveObjectId_Implementation() const override;
	virtual bool GetSaveIdentity_Implementation(FAtlasSaveObjectIdentity& OutIdentity) const override;
	virtual bool CaptureSaveData_Implementation(FAtlasSavePayload& OutPayload) const override;
	virtual void RestoreSaveData_Implementation(const FAtlasSavePayload& Payload) override;
	virtual bool ShouldSave_Implementation() const override;
	virtual bool IsSaveDirty_Implementation() const override;
	virtual EAtlasSaveRestoreName GetRestoreName_Implementation() const override;
	virtual bool ShouldCaptureTransform_Implementation() const override;

	UFUNCTION(BlueprintNativeEvent, Category = "Save")
	bool CaptureComponentSaveData(FAtlasSavePayload& OutPayload) const;
	virtual bool CaptureComponentSaveData_Implementation(FAtlasSavePayload& OutPayload) const;

	UFUNCTION(BlueprintNativeEvent, Category = "Save")
	void RestoreComponentSaveData(const FAtlasSavePayload& Payload);
	virtual void RestoreComponentSaveData_Implementation(const FAtlasSavePayload& Payload);

protected:
	void RegisterWithSaveSubsystem();
	void UnregisterFromSaveSubsystem();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save")
	FGuid PersistentObjectId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save")
	EAtlasSaveRestoreName RestoreName = EAtlasSaveRestoreName::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save")
	bool bCaptureOwnerTransform = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save")
	bool bEnabledForSaving = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save")
	FString DebugLabel;

	UPROPERTY(Transient)
	bool bDirty = true;
};
