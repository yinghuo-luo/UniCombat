#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AtlasTalentTreeData.generated.h"

class UGameplayEffect;
class UAtlasAbilitySet;

USTRUCT(BlueprintType)
struct FAtlasTalentRankDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Talent")
	int32 Cost = 1; //成本

	//已经赋予的能力集
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Talent")
	TObjectPtr<UAtlasAbilitySet> GrantedAbilitySet = nullptr;  

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Talent")
	TArray<TSubclassOf<UGameplayEffect>> GrantedEffects;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Talent")
	float EffectLevel = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Talent")
	FGameplayTagContainer GrantedTags;
};

USTRUCT(BlueprintType)
struct FAtlasTalentNodeDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Talent")
	FGameplayTag TalentTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Talent")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Talent", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Talent")
	FGameplayTagContainer PrerequisiteTalentTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Talent")
	bool bRequireAllPrerequisites = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Talent")
	FGameplayTagContainer RequiredOwnerTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Talent")
	TArray<FAtlasTalentRankDefinition> RankDefinitions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Talent")
	FVector2D GraphPosition = FVector2D::ZeroVector;
};

UCLASS(BlueprintType, Const)
class UNICOMBAT_API UAtlasTalentTreeData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Talent")
	TArray<FAtlasTalentNodeDefinition> TalentNodes;

	const FAtlasTalentNodeDefinition* FindTalentDefinition(FGameplayTag TalentTag) const;
};
