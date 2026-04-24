#pragma once

#include "CoreMinimal.h"
#include "Combat/Data/AtlasAbilitySet.h"
#include "Combat/Weapons/AtlasWeaponTypes.h"
#include "Components/ActorComponent.h"
#include "AtlasWeaponComponent.generated.h"

class AAtlasCombatCharacterBase;
class AAtlasWeaponBase;
class UAtlasAbilitySystemComponent;

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class UNICOMBAT_API UAtlasWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAtlasWeaponComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeaponForCombatMode(EAtlasCombatMode CombatMode);

	UFUNCTION(Server, Reliable)
	void ServerEquipWeaponForCombatMode(EAtlasCombatMode CombatMode);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeaponClass(TSubclassOf<AAtlasWeaponBase> WeaponClass, FName AttachSocketName = NAME_None);

	UFUNCTION(Server, Reliable)
	void ServerEquipWeaponClass(TSubclassOf<AAtlasWeaponBase> WeaponClass, FName AttachSocketName);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void UnequipCurrentWeapon(bool bDestroyWeapon = true);

	UFUNCTION(Server, Reliable)
	void ServerUnequipCurrentWeapon(bool bDestroyWeapon);

	UFUNCTION(BlueprintPure, Category = "Weapon")
	AAtlasWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool HasEquippedWeapon() const { return CurrentWeapon != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	EAtlasCombatMode GetEquippedCombatMode() const { return EquippedCombatMode; }

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool BuildCurrentAttackParameters(FAtlasWeaponAttackParameters& OutAttackParameters) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bAutoEquipOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bSyncWithOwnerCombatMode = true;//同步拥有者战斗模式

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (EditCondition = "!bSyncWithOwnerCombatMode"))
	EAtlasCombatMode InitialEquippedCombatMode = EAtlasCombatMode::Knife;

	//可以配置的武器（刀/棍）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TArray<FAtlasWeaponLoadoutEntry> WeaponLoadout;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<AAtlasWeaponBase> CurrentWeapon = nullptr;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon")
	EAtlasCombatMode EquippedCombatMode = EAtlasCombatMode::Knife;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon")
	FName EquippedAttachSocketName = NAME_None;

	UFUNCTION()
	void OnRep_CurrentWeapon(AAtlasWeaponBase* PreviousWeapon);

private:
	const FAtlasWeaponLoadoutEntry* FindLoadoutEntry(EAtlasCombatMode CombatMode) const;
	EAtlasCombatMode ResolveAutoEquipMode() const;
	AAtlasCombatCharacterBase* GetCombatCharacterOwner() const;
	UAtlasAbilitySystemComponent* GetOwnerAbilitySystemComponent() const;
	void AttachWeaponToOwner(AAtlasWeaponBase* WeaponActor, FName OverrideSocketName) const;
	void HandleCurrentWeaponChanged(AAtlasWeaponBase* PreviousWeapon);
	void ApplyWeaponGrants(AAtlasWeaponBase* WeaponActor);
	void RemoveWeaponGrants();
	void SetCurrentWeaponInternal(AAtlasWeaponBase* NewWeapon, EAtlasCombatMode NewCombatMode, FName AttachSocketName);

	FAtlasAbilitySet_GrantedHandles EquippedWeaponGrantedHandles;
	FGameplayTagContainer GrantedWeaponTags;
};
