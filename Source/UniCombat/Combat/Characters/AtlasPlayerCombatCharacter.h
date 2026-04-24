// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Combat/AtlasCombatMode.h"
#include "Combat/Characters/AtlasCombatCharacterBase.h"
#include "AtlasPlayerCombatCharacter.generated.h"

class UInputAction;
class UInputMappingContext;

UCLASS()
class UNICOMBAT_API AAtlasPlayerCombatCharacter : public AAtlasCombatCharacterBase
{
	GENERATED_BODY()

public:
	AAtlasPlayerCombatCharacter();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	void RestoreSaveData_AttributeSet(struct FPlayerAttributeSaveData InData) const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	EAtlasCombatMode GetCombatMode() const { return CombatMode; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsWeaponEquipped() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsCombatStunned() const;

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsSprinting() const { return bIsSprinting; }

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool UsesControllerRotationYawForAnim() const { return bUseControllerRotationYaw; }
	
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetSprintingState(bool bNewIsSprinting);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void CommitCombatMode(EAtlasCombatMode NewMode);

	UFUNCTION(Server, Reliable)
	void ServerCommitCombatMode(EAtlasCombatMode NewMode);

	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void SetCurrentCombatTarget(AActor* NewTarget);

	UFUNCTION(BlueprintPure, Category = "Targeting")
	AActor* GetCurrentCombatTarget() const { return CurrentCombatTarget.Get(); }

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void BP_OnCombatModeChanged(EAtlasCombatMode PreviousMode, EAtlasCombatMode NewMode);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Targeting")
	TWeakObjectPtr<AActor> CurrentCombatTarget;

	UPROPERTY(ReplicatedUsing = OnRep_CombatMode, VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	EAtlasCombatMode CombatMode = EAtlasCombatMode::Knife;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting = false;

	UFUNCTION()
	void OnRep_CombatMode(EAtlasCombatMode PreviousMode);

private:
	bool CanCommitCombatMode(EAtlasCombatMode NewMode) const;
	void ApplyCombatModeTags();
	void HandleCombatModeChanged(EAtlasCombatMode PreviousMode);

	//处理move speed
	void HandleMoveSpeedChanged(const FOnAttributeChangeData& Data) const;
	FDelegateHandle MoveSpeedChangedDelegateHandle;
	
	void Input_LightAttack();
	void Input_HeavyAttack();
	void Input_SwitchCombatMode();
	void Input_Dodge();
};
