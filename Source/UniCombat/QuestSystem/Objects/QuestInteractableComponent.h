// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QuestInterfaces.h"
#include "Components/ActorComponent.h"
#include "QuestInteractableComponent.generated.h"

class UQuestTargetComponent;
class UQuestSubsystem;

UCLASS(ClassGroup=(Quest), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class UNICOMBAT_API UQuestInteractableComponent : public UActorComponent, public IQuestEventSourceInterface
{
	GENERATED_BODY()

public:
	UQuestInteractableComponent();

	virtual void BeginPlay() override;

	//提交互动
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool SubmitInteraction(AActor* InstigatorActor, FName EventName = NAME_None, int32 IntValue = 1, bool bBoolValue = true);

	//可以互动
	UFUNCTION(BlueprintPure, Category = "Quest")
	bool CanInteract() const;

	//解析目标 ID
	UFUNCTION(BlueprintPure, Category = "Quest")
	FName ResolveTargetId() const;

	virtual FName GetQuestEventSourceId_Implementation() const override;
	virtual void BuildQuestEventPayload_Implementation(EQuestEventType EventType, FQuestEventPayload& OutPayload) const override;
	virtual void NotifyQuestEventSubmitted_Implementation(const FQuestEventPayload& Payload) override;

	UFUNCTION(BlueprintNativeEvent, Category = "Quest")
	bool CanSubmitInteraction(AActor* InstigatorActor) const;
	virtual bool CanSubmitInteraction_Implementation(AActor* InstigatorActor) const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest")
	void ReceiveInteractionSubmitted(const FQuestEventPayload& Payload);

protected:
	UQuestSubsystem* ResolveQuestSubsystem() const;
	UQuestTargetComponent* ResolveQuestTargetComponent() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FName TargetIdOverride = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	bool bSubmitOnlyWhenTargetEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	bool bOneShot = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
	bool bAlreadyUsed = false;
};
