// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SaveSystem/Objects/AtlasSaveStateComponent.h"
#include "PlayerSaveStateComponent.generated.h"

USTRUCT(BlueprintType)
struct FPlayerAttributeSaveData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)
	float Health;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)
	float MaxHealth;

	//@stamina 耐力
	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)
	float Stamina;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)
	float MaxStamina;

	//@sporit 灵力/法力
	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)
	float Spirit;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)
	float MaxSpirit;

	//攻击力
	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)
	float AttackPower;
	
	//技能力量
	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)
	float AbilityPower;

	//防御
	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)
	float Defense;

	//韧性
	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)
	float Poise;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)
	float MaxPoise;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)
	float MoveSpeed;

	//抗性（火抗等用一个）
	UPROPERTY(EditAnywhere,BlueprintReadWrite,SaveGame)
	float StatusResistance;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UNICOMBAT_API UPlayerSaveStateComponent : public UAtlasSaveStateComponent
{
	GENERATED_BODY()
	
public:
	// Sets default values for this component's properties
	UPlayerSaveStateComponent();

	void SetAttributeParam(TObjectPtr<class UAtlasCombatAttributeSet> InAttribute);

	//捕获/恢复
	virtual bool CaptureComponentSaveData_Implementation(FAtlasSavePayload& OutPayload) const override;
	virtual void RestoreComponentSaveData_Implementation(const FAtlasSavePayload& Payload) override;
	
private:
	FPlayerAttributeSaveData MarkData;
};
