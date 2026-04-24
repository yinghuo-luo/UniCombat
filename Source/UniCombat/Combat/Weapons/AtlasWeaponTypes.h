#pragma once

#include "CoreMinimal.h"
#include "Combat/AtlasCombatMode.h"
#include "Engine/EngineTypes.h"
#include "GameplayTagContainer.h"
#include "AtlasWeaponTypes.generated.h"

class AActor;
class AAtlasWeaponBase;

UENUM(BlueprintType)
enum class EAtlasWeaponState : uint8
{
	Upper UMETA(DisplayName = "Upper"),
	Middle UMETA(DisplayName = "Middle"),
	Lower UMETA(DisplayName = "Lower")
};


/*
 * 武器攻击参数
 */
USTRUCT(BlueprintType)
struct FAtlasWeaponAttackParameters
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	FVector TraceStart = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	FVector TraceEnd = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	float TraceRadius = 25.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	float DamageMultiplier = 1.0f; //伤害倍率

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	float PoiseMultiplier = 1.0f; //韧性倍率

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	FGameplayTagContainer DamageEffectTags;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<AActor> EffectCauser = nullptr;
};

/*
 * 武器装载项
 */
USTRUCT(BlueprintType)
struct FAtlasWeaponLoadoutEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	EAtlasCombatMode CombatMode = EAtlasCombatMode::Knife;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AAtlasWeaponBase> WeaponClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FName AttachSocketName = NAME_None;
};
