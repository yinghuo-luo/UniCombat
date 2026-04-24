// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QuestInterfaces.h"
#include "Components/ActorComponent.h"
#include "QuestTargetComponent.generated.h"


class UQuestSubsystem;

UCLASS(ClassGroup=(Quest), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class UNICOMBAT_API UQuestTargetComponent : public UActorComponent, public IQuestActionTargetInterface
{
	GENERATED_BODY()

public:
	UQuestTargetComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintPure, Category = "Quest")
	FName GetTargetId() const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetTargetId(FName InTargetId);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SyncWithQuestSubsystem();

	virtual FName GetQuestTargetId_Implementation() const override;
	virtual void SetQuestTargetEnabled_Implementation(bool bEnabled) override;
	virtual bool IsQuestTargetEnabled_Implementation() const override;
	virtual void ApplyQuestAction_Implementation(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest")
	void ReceiveQuestTargetEnabledChanged(bool bEnabled);

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest")
	void ReceiveQuestActionApplied(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload);

protected:
	UQuestSubsystem* ResolveQuestSubsystem() const;
	void ApplyEnabledStateToOwner(bool bEnabled);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FName TargetId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	bool bStartEnabled = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
	bool bIsEnabled = true;

	UPROPERTY(Transient)
	FName RegisteredTargetId = NAME_None;
};
