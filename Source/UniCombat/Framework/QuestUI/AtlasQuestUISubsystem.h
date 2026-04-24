#pragma once

#include "CoreMinimal.h"
#include "Framework/QuestUI/AtlasQuestUITypes.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "AtlasQuestUISubsystem.generated.h"

class UGameHUDWidget;
class UGameMenuWidget;
class UQuestSubsystem;
class UUIRootSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAtlasTrackedQuestChangedEvent, FName, QuestId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAtlasQuestUIDataChangedEvent);

UCLASS(BlueprintType)
class UNICOMBAT_API UAtlasQuestUISubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "Framework|QuestUI")
	FName GetTrackedQuestId() const
	{
		return TrackedQuestId;
	}

	UFUNCTION(BlueprintPure, Category = "Framework|QuestUI")
	bool HasTrackedQuest() const
	{
		return TrackedQuestId != NAME_None;
	}

	UFUNCTION(BlueprintPure, Category = "Framework|QuestUI")
	bool IsQuestTracked(FName QuestId) const;

	UFUNCTION(BlueprintPure, Category = "Framework|QuestUI")
	FAtlasQuestUIEntry GetTrackedQuestEntry() const;

	UFUNCTION(BlueprintPure, Category = "Framework|QuestUI")
	TArray<FAtlasQuestUIEntry> GetQuestEntriesCopy() const
	{
		return QuestEntries;
	}

	const TArray<FAtlasQuestUIEntry>& GetQuestEntries() const
	{
		return QuestEntries;
	}

	UFUNCTION(BlueprintCallable, Category = "Framework|QuestUI")
	bool SetTrackedQuest(FName QuestId);

	UFUNCTION(BlueprintCallable, Category = "Framework|QuestUI")
	bool CycleTrackedQuest(int32 Direction = 1);

	UFUNCTION(BlueprintCallable, Category = "Framework|QuestUI")
	void RefreshQuestEntries();

	UFUNCTION(BlueprintPure, Category = "Framework|QuestUI")
	bool IsQuestTrackable(FName QuestId) const;

	UPROPERTY(BlueprintAssignable, Category = "Framework|QuestUI")
	FAtlasTrackedQuestChangedEvent OnTrackedQuestChanged;

	UPROPERTY(BlueprintAssignable, Category = "Framework|QuestUI")
	FAtlasQuestUIDataChangedEvent OnQuestUIDataChanged;

protected:
	UFUNCTION()
	void HandleQuestObjectiveUpdated(FName QuestId, const FText& ObjectiveText);

	UFUNCTION()
	void HandleQuestStateChanged(FName QuestId, EQuestState QuestState);

	UFUNCTION()
	void HandleQuestRestored(FName QuestId);

	UFUNCTION()
	void HandleHUDReady(UGameHUDWidget* HUD);

	UFUNCTION()
	void HandleMenuReady(UGameMenuWidget* Menu);

	UQuestSubsystem* GetQuestSubsystem() const;
	UUIRootSubsystem* GetUIRootSubsystem() const;

	FAtlasQuestUIEntry BuildQuestEntry(FName QuestId) const;
	TArray<FName> GetRuntimeQuestIds() const;
	TArray<FName> GetActiveQuestIds() const;
	void CacheQuestTitle(FName QuestId);
	void RefreshWidgets() const;
	bool UpdateTrackedQuestFromEntries();
	void UpdateTrackedFlags();

protected:
	UPROPERTY(Transient)
	TMap<FName, FText> CachedQuestTitles;

	UPROPERTY(Transient)
	TArray<FAtlasQuestUIEntry> QuestEntries;

	UPROPERTY(Transient)
	FName TrackedQuestId = NAME_None;

	bool bAutoTrackFirstActiveQuestWhenUnset = true;
	bool bAutoTrackNextActiveQuestWhenTrackedQuestStops = true;
};
