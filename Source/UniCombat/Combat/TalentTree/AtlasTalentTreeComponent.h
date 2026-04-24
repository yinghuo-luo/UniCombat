#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Combat/Data/AtlasAbilitySet.h"
#include "Combat/TalentTree/AtlasTalentTreeData.h"
#include "Components/ActorComponent.h"
#include "AtlasTalentTreeComponent.generated.h"

class UAtlasAbilitySystemComponent;
class UGameplayEffect;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAtlasTalentTreeChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAtlasTalentNodeRankChangedSignature, FGameplayTag, TalentTag, int32, NewRank);

USTRUCT(BlueprintType)
struct FAtlasTalentNodeState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talent")
	FGameplayTag TalentTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talent")
	int32 CurrentRank = 0;
};

struct FAtlasAppliedTalentRankState
{
	FAtlasAbilitySet_GrantedHandles GrantedHandles;
	TArray<FActiveGameplayEffectHandle> GrantedEffectHandles;
	FGameplayTagContainer GrantedTags;
};

struct FAtlasAppliedTalentNodeState
{
	TArray<FAtlasAppliedTalentRankState> AppliedRanks;
	bool bGrantedTalentTag = false;
};

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class UNICOMBAT_API UAtlasTalentTreeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAtlasTalentTreeComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Talent")
	UAtlasTalentTreeData* GetTalentTreeData() const { return TalentTreeData; }

	UFUNCTION(BlueprintPure, Category = "Talent")
	int32 GetUnspentTalentPoints() const { return UnspentTalentPoints; }

	UFUNCTION(BlueprintPure, Category = "Talent")
	int32 GetTalentRank(FGameplayTag TalentTag) const;

	UFUNCTION(BlueprintPure, Category = "Talent")
	bool IsTalentUnlocked(FGameplayTag TalentTag) const;

	UFUNCTION(BlueprintPure, Category = "Talent")
	bool HasRequiredOwnerTags(const FGameplayTagContainer& RequiredTags) const;

	UFUNCTION(BlueprintCallable, Category = "Talent")
	void AddTalentPoints(int32 PointsToAdd);

	UFUNCTION(Server, Reliable)
	void ServerAddTalentPoints(int32 PointsToAdd);

	UFUNCTION(BlueprintCallable, Category = "Talent")
	bool CanUpgradeTalent(FGameplayTag TalentTag, FText& OutFailureReason) const;

	UFUNCTION(BlueprintCallable, Category = "Talent")
	bool UpgradeTalent(FGameplayTag TalentTag);

	UFUNCTION(Server, Reliable)
	void ServerUpgradeTalent(FGameplayTag TalentTag);

	UFUNCTION(BlueprintCallable, Category = "Talent")
	void ResetTalents();

	UFUNCTION(Server, Reliable)
	void ServerResetTalents();

	UPROPERTY(BlueprintAssignable, Category = "Talent")
	FAtlasTalentTreeChangedSignature OnTalentTreeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Talent")
	FAtlasTalentNodeRankChangedSignature OnTalentNodeRankChanged;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Talent")
	TObjectPtr<UAtlasTalentTreeData> TalentTreeData = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_UnspentTalentPoints, BlueprintReadOnly, Category = "Talent")
	int32 UnspentTalentPoints = 0;

	UPROPERTY(ReplicatedUsing = OnRep_TalentStates, BlueprintReadOnly, Category = "Talent")
	TArray<FAtlasTalentNodeState> TalentStates;

	UFUNCTION()
	void OnRep_UnspentTalentPoints();

	UFUNCTION()
	void OnRep_TalentStates();

private:
	const FAtlasTalentNodeDefinition* FindTalentDefinition(FGameplayTag TalentTag) const;
	int32 FindTalentStateIndex(FGameplayTag TalentTag) const;
	UAtlasAbilitySystemComponent* GetOwnerAbilitySystemComponent() const;
	bool SatisfiesPrerequisites(const FAtlasTalentNodeDefinition& TalentDefinition) const;
	void RebuildGrantedTalents();
	void ClearGrantedTalents();
	void ApplyTalentRank(const FAtlasTalentNodeDefinition& TalentDefinition, int32 RankIndex);
	void BroadcastTalentStateChanged(FGameplayTag TalentTag, int32 NewRank);

	TMap<FGameplayTag, FAtlasAppliedTalentNodeState> AppliedTalentStates;
};
