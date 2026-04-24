// Fill out your copyright notice in the Description page of Project Settings.


#include "AtlasSaveStateComponent.h"

#include "SaveSystem/Core/AtlasSaveSubsystem.h"


// Sets default values for this component's properties
UAtlasSaveStateComponent::UAtlasSaveStateComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UAtlasSaveStateComponent::BeginPlay()
{
	Super::BeginPlay();

	RegisterWithSaveSubsystem();
}

void UAtlasSaveStateComponent::OnRegister()
{
	Super::OnRegister();
	EnsurePersistentId();
}

void UAtlasSaveStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterFromSaveSubsystem();
	Super::EndPlay(EndPlayReason);
}

void UAtlasSaveStateComponent::EnsurePersistentId()
{
	if (!PersistentObjectId.IsValid() && !HasAnyFlags(RF_ClassDefaultObject))
	{
		PersistentObjectId = FGuid::NewGuid();
	}
}

void UAtlasSaveStateComponent::MarkSaveDirty()
{
	bDirty = true;
}

void UAtlasSaveStateComponent::ClearSaveDirty()
{
	bDirty = false;
}

FGuid UAtlasSaveStateComponent::GetSaveObjectId_Implementation() const
{
	return PersistentObjectId;
}

bool UAtlasSaveStateComponent::GetSaveIdentity_Implementation(FAtlasSaveObjectIdentity& OutIdentity) const
{
	OutIdentity = FAtlasSaveObjectIdentity();
	OutIdentity.StableId = PersistentObjectId;

	const AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		if (const ULevel* OwnerLevel = OwnerActor->GetLevel())
		{
			OutIdentity.OwningLevelName = OwnerLevel->GetOutermost()->GetFName();
		}
	}

	const bool bLooksLikeStaticPlacedObject =
		HasAnyFlags(RF_WasLoaded) || (OwnerActor && OwnerActor->HasAnyFlags(RF_WasLoaded));
	if (bLooksLikeStaticPlacedObject)
	{
		OutIdentity.IdentityKind = EAtlasSaveObjectIdentityKind::StaticPlaced;
		OutIdentity.PlacedObjectPath = FSoftObjectPath(this);
	}

	return OutIdentity.StableId.IsValid()
		|| OutIdentity.OwningLevelName != NAME_None
		|| !OutIdentity.PlacedObjectPath.IsNull();
}

bool UAtlasSaveStateComponent::CaptureSaveData_Implementation(FAtlasSavePayload& OutPayload) const
{
	return CaptureComponentSaveData(OutPayload);
}

void UAtlasSaveStateComponent::RestoreSaveData_Implementation(const FAtlasSavePayload& Payload)
{
	RestoreComponentSaveData(Payload);
	bDirty = false;
}

bool UAtlasSaveStateComponent::ShouldSave_Implementation() const
{
	return bEnabledForSaving;
}

bool UAtlasSaveStateComponent::IsSaveDirty_Implementation() const
{
	return bDirty;
}

EAtlasSaveRestoreName UAtlasSaveStateComponent::GetRestoreName_Implementation() const
{
	return RestoreName;
}

bool UAtlasSaveStateComponent::ShouldCaptureTransform_Implementation() const
{
	return bCaptureOwnerTransform;
}

bool UAtlasSaveStateComponent::CaptureComponentSaveData_Implementation(FAtlasSavePayload& OutPayload) const
{
	return false;
}

void UAtlasSaveStateComponent::RestoreComponentSaveData_Implementation(const FAtlasSavePayload& Payload)
{
}

void UAtlasSaveStateComponent::RegisterWithSaveSubsystem()
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UAtlasSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UAtlasSaveSubsystem>())
			{
				SaveSubsystem->RegisterSaveableObject(this);
			}
		}
	}
}

void UAtlasSaveStateComponent::UnregisterFromSaveSubsystem()
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UAtlasSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UAtlasSaveSubsystem>())
			{
				SaveSubsystem->UnregisterSaveableObject(this);
			}
		}
	}
}

