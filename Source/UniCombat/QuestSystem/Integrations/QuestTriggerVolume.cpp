// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestTriggerVolume.h"

#include "Components/BoxComponent.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Pawn.h"
#include "QuestSystem/Core/QuestSubsystem.h"
#include "QuestSystem/Objects/QuestTargetComponent.h"

AQuestTriggerVolume::AQuestTriggerVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);

	TriggerBox->InitBoxExtent(FVector(200.f, 200.f, 200.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
}

void AQuestTriggerVolume::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleTriggerBeginOverlap);
		TriggerBox->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}
}

bool AQuestTriggerVolume::SubmitEnteredRegionEvent(AActor* InstigatorActor)
{
	if (!CanTriggerActor(InstigatorActor))
	{
		return false;
	}

	UQuestSubsystem* QuestSubsystem = ResolveQuestSubsystem();
	if (!QuestSubsystem)
	{
		return false;
	}

	FQuestEventPayload Payload;
	BuildQuestEventPayload_Implementation(EQuestEventType::EnteredRegion, Payload);
	Payload.InstigatorActor = InstigatorActor;

	QuestSubsystem->SubmitQuestEvent(Payload);
	NotifyQuestEventSubmitted_Implementation(Payload);
	ReceiveTriggerActivated(Payload);

	if (bTriggerOnlyOnce)
	{
		bHasTriggered = true;
		if (TriggerBox)
		{
			TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

	return true;
}

void AQuestTriggerVolume::SetTriggerEnabled(const bool bInEnabled)
{
	bEnabled = bInEnabled;

	if (TriggerBox)
	{
		TriggerBox->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}
}

bool AQuestTriggerVolume::IsTriggerEnabled() const
{
	return bEnabled;
}

FName AQuestTriggerVolume::ResolveTargetId() const
{
	if (!TargetId.IsNone())
	{
		return TargetId;
	}

	if (const UQuestTargetComponent* QuestTarget = ResolveQuestTargetComponent())
	{
		return QuestTarget->GetTargetId();
	}

	return NAME_None;
}

FName AQuestTriggerVolume::GetQuestEventSourceId_Implementation() const
{
	return ResolveTargetId();
}

void AQuestTriggerVolume::BuildQuestEventPayload_Implementation(const EQuestEventType EventType, FQuestEventPayload& OutPayload) const
{
	OutPayload.EventType = EventType;
	OutPayload.QuestId = QuestId;
	OutPayload.TargetId = ResolveTargetId();
	OutPayload.EventName = EventName;
	OutPayload.IntValue = IntValue;
	OutPayload.bBoolValue = bBoolValue;
	OutPayload.SourceObject = const_cast<AQuestTriggerVolume*>(this);
}

void AQuestTriggerVolume::NotifyQuestEventSubmitted_Implementation(const FQuestEventPayload& Payload)
{
	UE_LOG(LogQuest, Verbose, TEXT("TriggerVolume '%s' submitted EnteredRegion for TargetId '%s'."),
		*GetName(),
		*Payload.TargetId.ToString());
}

bool AQuestTriggerVolume::CanTriggerActor_Implementation(AActor* OtherActor) const
{
	if (!bEnabled || (bTriggerOnlyOnce && bHasTriggered) || !IsValid(OtherActor))
	{
		return false;
	}

	if (!bOnlyPlayerPawn)
	{
		return true;
	}

	const APawn* Pawn = Cast<APawn>(OtherActor);
	return Pawn && Pawn->IsPlayerControlled();
}

void AQuestTriggerVolume::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	const int32 OtherBodyIndex,
	const bool bFromSweep,
	const FHitResult& SweepResult)
{
	SubmitEnteredRegionEvent(OtherActor);
}

UQuestSubsystem* AQuestTriggerVolume::ResolveQuestSubsystem() const
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSubsystem<UQuestSubsystem>();
		}
	}

	return nullptr;
}

UQuestTargetComponent* AQuestTriggerVolume::ResolveQuestTargetComponent() const
{
	return FindComponentByClass<UQuestTargetComponent>();
}

