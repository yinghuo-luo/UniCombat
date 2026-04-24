// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerSaveStateComponent.h"

#include "Combat/AbilitySystem/AtlasCombatAttributeSet.h"
#include "Combat/Characters/AtlasPlayerCombatCharacter.h"
#include "SaveSystem/Core/AtlasSaveSerialization.h"


// Sets default values for this component's properties
UPlayerSaveStateComponent::UPlayerSaveStateComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	bCaptureOwnerTransform = false;
	
	RestoreName = EAtlasSaveRestoreName::CombatPlayer;
}

void UPlayerSaveStateComponent::SetAttributeParam(TObjectPtr<UAtlasCombatAttributeSet> InAttribute)
{
	MarkData.Health = InAttribute->GetHealth();
	MarkData.MaxHealth = InAttribute->GetMaxHealth();
	MarkData.Stamina = InAttribute->GetStamina();
	MarkData.MaxStamina = InAttribute->GetMaxStamina();
	MarkData.Spirit = InAttribute->GetSpirit();
	MarkData.MaxSpirit = InAttribute->GetMaxSpirit();
	MarkData.AttackPower = InAttribute->GetAttackPower();
	MarkData.AbilityPower = InAttribute->GetAbilityPower();
	MarkData.Defense = InAttribute->GetDefense();
	MarkData.Poise = InAttribute->GetPoise();
	MarkData.MaxPoise = InAttribute->GetMaxPoise();
	MarkData.MoveSpeed =InAttribute->GetMoveSpeed();
	MarkData.StatusResistance =InAttribute->GetStatusResistance();
	
	MarkSaveDirty();
}

bool UPlayerSaveStateComponent::CaptureComponentSaveData_Implementation(FAtlasSavePayload& OutPayload) const
{
	OutPayload.PayloadType = FPlayerAttributeSaveData::StaticStruct()->GetFName();
	OutPayload.SchemaVersion =1;
	OutPayload.DebugSummary = FString::Printf(
		TEXT("Health=%f Stamina=%f"),MarkData.Health,MarkData.Stamina);
	return FAtlasSaveSerialization::SerializeUStruct(MarkData,OutPayload.Bytes);
}

void UPlayerSaveStateComponent::RestoreComponentSaveData_Implementation(const FAtlasSavePayload& Payload)
{
	if(Payload.PayloadType != FPlayerAttributeSaveData::StaticStruct()->GetFName())
	{
		return;
	}
	if (!FAtlasSaveSerialization::DeserializeUStruct(Payload.Bytes,MarkData))
	{
		return;
	}
	AAtlasPlayerCombatCharacter* Player = Cast<AAtlasPlayerCombatCharacter>(GetOwner());
	if (!Player)
	{
		return;
	}
	
	Player->RestoreSaveData_AttributeSet(MarkData);
}
