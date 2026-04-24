#pragma once

#include "CoreMinimal.h"
#include "Framework/QuestUI/AtlasQuestUITypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AtlasQuestUIBlueprintLibrary.generated.h"

class UAtlasQuestUISubsystem;

UCLASS()
class UNICOMBAT_API UAtlasQuestUIBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Framework|QuestUI", meta = (WorldContext = "WorldContextObject"))
	static UAtlasQuestUISubsystem* GetQuestUISubsystem(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "Framework|QuestUI", meta = (WorldContext = "WorldContextObject"))
	static FAtlasQuestUIEntry GetTrackedQuestEntry(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "Framework|QuestUI", meta = (WorldContext = "WorldContextObject"))
	static TArray<FAtlasQuestUIEntry> GetQuestEntries(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Framework|QuestUI", meta = (WorldContext = "WorldContextObject"))
	static bool SetTrackedQuest(const UObject* WorldContextObject, FName QuestId);

	UFUNCTION(BlueprintCallable, Category = "Framework|QuestUI", meta = (WorldContext = "WorldContextObject"))
	static bool CycleTrackedQuest(const UObject* WorldContextObject, int32 Direction = 1);

	UFUNCTION(BlueprintCallable, Category = "Framework|QuestUI", meta = (WorldContext = "WorldContextObject"))
	static void RefreshQuestEntries(const UObject* WorldContextObject);
};
