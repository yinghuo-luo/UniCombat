// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestInteractableComponent.h"

#include "Engine/GameInstance.h"
#include "GameFramework/Actor.h"
#include "QuestSystem/Core/QuestSubsystem.h"
#include "QuestTargetComponent.h"

UQuestInteractableComponent::UQuestInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UQuestInteractableComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UQuestInteractableComponent::SubmitInteraction(AActor* InstigatorActor, const FName EventName, const int32 IntValue, const bool bBoolValue)
{
	if (bOneShot && bAlreadyUsed)
	{
		return false;
	}

	if (!CanInteract() || !CanSubmitInteraction(InstigatorActor))
	{
		return false;
	}

	UQuestSubsystem* QuestSubsystem = ResolveQuestSubsystem();
	if (!QuestSubsystem)
	{
		return false;
	}

	FQuestEventPayload Payload;
	Payload.EventType = EQuestEventType::InteractionCompleted;
	Payload.TargetId = ResolveTargetId();
	Payload.EventName = EventName;
	Payload.IntValue = IntValue;
	Payload.bBoolValue = bBoolValue;
	Payload.InstigatorActor = InstigatorActor;
	Payload.SourceObject = this;

	QuestSubsystem->SubmitQuestEvent(Payload);
	NotifyQuestEventSubmitted_Implementation(Payload);
	ReceiveInteractionSubmitted(Payload);

	if (bOneShot)
	{
		bAlreadyUsed = true;
	}

	return true;
}

bool UQuestInteractableComponent::CanInteract() const
{
	if (bOneShot && bAlreadyUsed)
	{
		return false;
	}

	if (!bSubmitOnlyWhenTargetEnabled)
	{
		return true;
	}

	if (const UQuestTargetComponent* QuestTarget = ResolveQuestTargetComponent())
	{
		return IQuestActionTargetInterface::Execute_IsQuestTargetEnabled(const_cast<UQuestTargetComponent*>(QuestTarget));
	}

	return true;
}

FName UQuestInteractableComponent::ResolveTargetId() const
{
	if (!TargetIdOverride.IsNone())
	{
		return TargetIdOverride;
	}

	if (const UQuestTargetComponent* QuestTarget = ResolveQuestTargetComponent())
	{
		return QuestTarget->GetTargetId();
	}

	return NAME_None;
}

FName UQuestInteractableComponent::GetQuestEventSourceId_Implementation() const
{
	return ResolveTargetId();
}

void UQuestInteractableComponent::BuildQuestEventPayload_Implementation(const EQuestEventType EventType, FQuestEventPayload& OutPayload) const
{
	OutPayload.EventType = EventType;
	OutPayload.TargetId = ResolveTargetId();
	OutPayload.SourceObject = const_cast<UQuestInteractableComponent*>(this);
}

void UQuestInteractableComponent::NotifyQuestEventSubmitted_Implementation(const FQuestEventPayload& Payload)
{
	UE_LOG(LogQuest, Verbose, TEXT("Interactable '%s' submitted interaction event for TargetId '%s'."),
		*GetNameSafe(GetOwner()),
		*Payload.TargetId.ToString());
}

bool UQuestInteractableComponent::CanSubmitInteraction_Implementation(AActor* InstigatorActor) const
{
	return true;
}

UQuestSubsystem* UQuestInteractableComponent::ResolveQuestSubsystem() const
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

UQuestTargetComponent* UQuestInteractableComponent::ResolveQuestTargetComponent() const
{
	if (AActor* OwnerActor = GetOwner())
	{
		return OwnerActor->FindComponentByClass<UQuestTargetComponent>();
	}

	return nullptr;
}
