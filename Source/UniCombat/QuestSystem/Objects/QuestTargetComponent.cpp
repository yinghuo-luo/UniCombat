// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestTargetComponent.h"

#include "GameFramework/Actor.h"
#include "Engine/GameInstance.h"
#include "QuestSystem/Core/QuestSubsystem.h"

UQuestTargetComponent::UQuestTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;
	bIsEnabled = bStartEnabled;
}

void UQuestTargetComponent::BeginPlay()
{
	Super::BeginPlay();

	bIsEnabled = bStartEnabled;
	SyncWithQuestSubsystem();
}

void UQuestTargetComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UQuestSubsystem* QuestSubsystem = ResolveQuestSubsystem())
	{
		QuestSubsystem->UnregisterQuestTarget(RegisteredTargetId.IsNone() ? TargetId : RegisteredTargetId, this);
	}

	RegisteredTargetId = NAME_None;

	Super::EndPlay(EndPlayReason);
}

FName UQuestTargetComponent::GetTargetId() const
{
	return TargetId;
}

void UQuestTargetComponent::SetTargetId(const FName InTargetId)
{
	if (TargetId == InTargetId)
	{
		return;
	}

	if (UQuestSubsystem* QuestSubsystem = ResolveQuestSubsystem())
	{
		if (!RegisteredTargetId.IsNone())
		{
			QuestSubsystem->UnregisterQuestTarget(RegisteredTargetId, this);
			RegisteredTargetId = NAME_None;
		}
	}

	TargetId = InTargetId;

	if (HasBegunPlay())
	{
		SyncWithQuestSubsystem();
	}
}

void UQuestTargetComponent::SyncWithQuestSubsystem()
{
	if (TargetId.IsNone())
	{
		if (UQuestSubsystem* QuestSubsystem = ResolveQuestSubsystem())
		{
			if (!RegisteredTargetId.IsNone())
			{
				QuestSubsystem->UnregisterQuestTarget(RegisteredTargetId, this);
				RegisteredTargetId = NAME_None;
			}
		}

		UE_LOG(LogQuest, Warning, TEXT("QuestTargetComponent on '%s' has no TargetId."), *GetNameSafe(GetOwner()));
		return;
	}

	if (UQuestSubsystem* QuestSubsystem = ResolveQuestSubsystem())
	{
		if (!RegisteredTargetId.IsNone() && RegisteredTargetId != TargetId)
		{
			QuestSubsystem->UnregisterQuestTarget(RegisteredTargetId, this);
			RegisteredTargetId = NAME_None;
		}

		QuestSubsystem->RegisterQuestTarget(TargetId, this);
		RegisteredTargetId = TargetId;
		bool bResolvedEnabled = bIsEnabled;
		if (QuestSubsystem->GetTargetEnabledState(TargetId, bResolvedEnabled))
		{
			SetQuestTargetEnabled_Implementation(bResolvedEnabled);
		}
		else
		{
			QuestSubsystem->SetTargetEnabledState(TargetId, bIsEnabled, FQuestEventPayload());
		}
	}
}

FName UQuestTargetComponent::GetQuestTargetId_Implementation() const
{
	return TargetId;
}

void UQuestTargetComponent::SetQuestTargetEnabled_Implementation(const bool bEnabled)
{
	bIsEnabled = bEnabled;
	ApplyEnabledStateToOwner(bEnabled);
	ReceiveQuestTargetEnabledChanged(bEnabled);
}

bool UQuestTargetComponent::IsQuestTargetEnabled_Implementation() const
{
	return bIsEnabled;
}

void UQuestTargetComponent::ApplyQuestAction_Implementation(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload)
{
	ReceiveQuestActionApplied(ActionRow, ContextPayload);
}

UQuestSubsystem* UQuestTargetComponent::ResolveQuestSubsystem() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	if (UGameInstance* GameInstance = World->GetGameInstance())
	{
		return GameInstance->GetSubsystem<UQuestSubsystem>();
	}

	return nullptr;
}

void UQuestTargetComponent::ApplyEnabledStateToOwner(const bool bEnabled)
{
	if (AActor* OwnerActor = GetOwner())
	{
		OwnerActor->SetActorHiddenInGame(!bEnabled);
		OwnerActor->SetActorEnableCollision(bEnabled);
		OwnerActor->SetActorTickEnabled(bEnabled);
	}
}
