// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuestSystem/Objects/QuestInterfaces.h"
#include "QuestTriggerVolume.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class UQuestSubsystem;
class UQuestTargetComponent;
struct FHitResult;

UCLASS()
class UNICOMBAT_API AQuestTriggerVolume : public AActor, public IQuestEventSourceInterface
{
	GENERATED_BODY()

public:
	AQuestTriggerVolume();

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool SubmitEnteredRegionEvent(AActor* InstigatorActor);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetTriggerEnabled(bool bInEnabled);

	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsTriggerEnabled() const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	FName ResolveTargetId() const;

	virtual FName GetQuestEventSourceId_Implementation() const override;
	virtual void BuildQuestEventPayload_Implementation(EQuestEventType EventType, FQuestEventPayload& OutPayload) const override;
	virtual void NotifyQuestEventSubmitted_Implementation(const FQuestEventPayload& Payload) override;

	UFUNCTION(BlueprintNativeEvent, Category = "Quest")
	bool CanTriggerActor(AActor* OtherActor) const;
	virtual bool CanTriggerActor_Implementation(AActor* OtherActor) const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest")
	void ReceiveTriggerActivated(const FQuestEventPayload& Payload);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UQuestSubsystem* ResolveQuestSubsystem() const;
	UQuestTargetComponent* ResolveQuestTargetComponent() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
	TObjectPtr<UBoxComponent> TriggerBox = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FName TargetId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FName QuestId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FName EventName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	int32 IntValue = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	bool bBoolValue = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	bool bTriggerOnlyOnce = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	bool bOnlyPlayerPawn = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	bool bEnabled = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
	bool bHasTriggered = false;
};
