// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/Characters/AtlasPlayerCombatCharacter.h"

#include "Combat/AbilitySystem/AtlasAbilitySystemComponent.h"
#include "Combat/AtlasGameplayTags.h"
#include "Combat/Weapons/AtlasWeaponComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "Combat/AbilitySystem/AtlasCombatAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "SaveSystem/Integrations/Components/PlayerSaveStateComponent.h"

AAtlasPlayerCombatCharacter::AAtlasPlayerCombatCharacter()
{
	bUseControllerRotationYaw = false;
}

void AAtlasPlayerCombatCharacter::BeginPlay()
{
	Super::BeginPlay();
	//恢复存档数据
	
	if (CombatAttributes != nullptr && GetCharacterMovement() != nullptr)
	{
		GetCharacterMovement()->MaxWalkSpeed = CombatAttributes->GetMoveSpeed();
	}

	if (HasAuthority())
	{
		ApplyCombatModeTags();
	}

	HandleCombatModeChanged(CombatMode);
	
	//绑定速度值变化
	MoveSpeedChangedDelegateHandle = AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(UAtlasCombatAttributeSet::GetMoveSpeedAttribute())
		.AddUObject(this, &ThisClass::HandleMoveSpeedChanged);
}

void AAtlasPlayerCombatCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAtlasPlayerCombatCharacter, CombatMode);
	DOREPLIFETIME(AAtlasPlayerCombatCharacter, bIsSprinting);
}

bool AAtlasPlayerCombatCharacter::IsCombatStunned() const
{
	const UAtlasAbilitySystemComponent* AtlasASC = GetAtlasAbilitySystemComponent();
	return AtlasASC != nullptr && AtlasASC->HasMatchingGameplayTag(TAG_State_Stunned);
}

bool AAtlasPlayerCombatCharacter::IsWeaponEquipped() const
{
	return WeaponComponent != nullptr && WeaponComponent->HasEquippedWeapon();
}


void AAtlasPlayerCombatCharacter::SetSprintingState(const bool bNewIsSprinting)
{
	if (bIsSprinting == bNewIsSprinting)
	{
		return;
	}

	bIsSprinting = bNewIsSprinting;

	if (HasAuthority())
	{
		ForceNetUpdate();
	}
}

void AAtlasPlayerCombatCharacter::CommitCombatMode(const EAtlasCombatMode NewMode)
{
	if (!CanCommitCombatMode(NewMode))
	{
		return;
	}

	if (!HasAuthority())
	{
		ServerCommitCombatMode(NewMode);
		return;
	}

	const EAtlasCombatMode PreviousMode = CombatMode;
	CombatMode = NewMode;

	ApplyCombatModeTags();
	if (WeaponComponent != nullptr)
	{
		WeaponComponent->EquipWeaponForCombatMode(NewMode);
	}
	HandleCombatModeChanged(PreviousMode);
	ForceNetUpdate();
}

void AAtlasPlayerCombatCharacter::ServerCommitCombatMode_Implementation(const EAtlasCombatMode NewMode)
{
	CommitCombatMode(NewMode);
}

void AAtlasPlayerCombatCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AAtlasPlayerCombatCharacter::RestoreSaveData_AttributeSet(struct FPlayerAttributeSaveData InData) const
{
	CombatAttributes->SetHealth(InData.Health);
	CombatAttributes->SetMaxHealth(InData.MaxHealth);
	CombatAttributes->SetStamina(InData.Stamina);
	CombatAttributes->SetMaxStamina(InData.MaxStamina);
	CombatAttributes->SetSpirit(InData.Spirit);
	CombatAttributes->SetMaxSpirit(InData.MaxSpirit);
	CombatAttributes->SetAttackPower(InData.AttackPower);
	CombatAttributes->SetAbilityPower(InData.AbilityPower);
	CombatAttributes->SetDefense(InData.Defense);
	CombatAttributes->SetPoise(InData.Poise);
	CombatAttributes->SetMaxPoise(InData.MaxPoise);
	CombatAttributes->SetMoveSpeed(InData.MoveSpeed);
}

void AAtlasPlayerCombatCharacter::SetCurrentCombatTarget(AActor* NewTarget)
{
	CurrentCombatTarget = NewTarget;
}

void AAtlasPlayerCombatCharacter::Input_LightAttack()
{
	TryActivateAbilityByInputTag(TAG_Input_Attack_Light);
}

void AAtlasPlayerCombatCharacter::Input_HeavyAttack()
{
	TryActivateAbilityByInputTag(TAG_Input_Attack_Heavy);
}

void AAtlasPlayerCombatCharacter::Input_SwitchCombatMode()
{
	TryActivateAbilityByInputTag(TAG_Input_Combat_Switch);
}

void AAtlasPlayerCombatCharacter::Input_Dodge()
{
	TryActivateAbilityByInputTag(TAG_Input_Move_Dodge);
}


void AAtlasPlayerCombatCharacter::OnRep_CombatMode(const EAtlasCombatMode PreviousMode)
{
	HandleCombatModeChanged(PreviousMode);
}

bool AAtlasPlayerCombatCharacter::CanCommitCombatMode(const EAtlasCombatMode NewMode) const
{
	return !IsDead() && CombatMode != NewMode;
}

void AAtlasPlayerCombatCharacter::ApplyCombatModeTags()
{
	if (!HasAuthority() || AbilitySystemComponent == nullptr)
	{
		return;
	}

	AbilitySystemComponent->RemoveLooseGameplayTag(TAG_State_Combat_Unarmed);
	AbilitySystemComponent->RemoveLooseGameplayTag(TAG_State_Combat_Knife);

	if (CombatMode == EAtlasCombatMode::Knife)
	{
		AbilitySystemComponent->AddLooseGameplayTag(TAG_State_Combat_Knife);
	}
	else
	{
		AbilitySystemComponent->AddLooseGameplayTag(TAG_State_Combat_Unarmed);
	}
}

void AAtlasPlayerCombatCharacter::HandleCombatModeChanged(const EAtlasCombatMode PreviousMode)
{
	BP_OnCombatModeChanged(PreviousMode, CombatMode);
}

void AAtlasPlayerCombatCharacter::HandleMoveSpeedChanged(const FOnAttributeChangeData& Data) const
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed = FMath::Max(Data.NewValue, 0.0f);
	}
}
