#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Combat/Weapons/AtlasWeaponTypes.h"
#include "AtlasWeaponBase.generated.h"

class AAtlasCombatCharacterBase;
class UAtlasWeaponData;
class USceneComponent;
class USkeletalMeshComponent;

UCLASS(Abstract)
class UNICOMBAT_API AAtlasWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AAtlasWeaponBase();

	UFUNCTION(BlueprintPure, Category = "Weapon")
	UAtlasWeaponData* GetWeaponData() const { return WeaponData; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	AAtlasCombatCharacterBase* GetOwningCombatCharacter() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void AttachToCharacter(AAtlasCombatCharacterBase* NewOwnerCharacter, FName OverrideSocketName = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void DetachFromCharacter();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool BuildAttackParameters(FAtlasWeaponAttackParameters& OutAttackParameters) const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FTransform GetSpawnTransform() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FVector GetSpawnLocation() const;

	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void OnEquipped(AAtlasCombatCharacterBase* NewOwnerCharacter);
	virtual void OnEquipped_Implementation(AAtlasCombatCharacterBase* NewOwnerCharacter);

	UFUNCTION(BlueprintNativeEvent, Category = "Weapon")
	void OnUnequipped(AAtlasCombatCharacterBase* PreviousOwnerCharacter);
	virtual void OnUnequipped_Implementation(AAtlasCombatCharacterBase* PreviousOwnerCharacter);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USceneComponent> TraceStart;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USceneComponent> TraceEnd;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USceneComponent> SpawnPoint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UAtlasWeaponData> WeaponData = nullptr;

private:
	FName ResolveAttachSocket(FName OverrideSocketName) const;
};
