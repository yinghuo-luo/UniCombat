#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Combat/AtlasCombatMode.h"
#include "AtlasPlayerAnimInstance.generated.h"

class AAtlasPlayerCombatCharacter;
class UCharacterMovementComponent;

UCLASS()
class UNICOMBAT_API UAtlasPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Owner")
	TObjectPtr<AAtlasPlayerCombatCharacter> PlayerCharacter = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Owner")
	TObjectPtr<UCharacterMovementComponent> MovementComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector Velocity = FVector::ZeroVector; //速度

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Speed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Direction = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float VerticalSpeed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bShouldMove = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsFalling = false; //空中

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsJumping = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsAccelerating = false; //加速

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bHasMovementInput = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsCrouching = false; //蹲伏

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting = false; //冲刺

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsDead = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsStunned = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	EAtlasCombatMode CombatMode = EAtlasCombatMode::Knife;
	
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bWeaponEquipped = false; //已装备武器

	UPROPERTY(BlueprintReadOnly, Category = "Aiming")
	bool bUseControllerYaw = false;

	UPROPERTY(BlueprintReadOnly, Category = "Aiming")
	float RootYawOffset = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Aiming")
	float AimYaw = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Aiming")
	float AimPitch = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundDistance = 0.0f; //地面距离

private:
	//刷新拥有者参考信息
	void RefreshOwnerReferences();

	//是落下的最后一帧
	bool bWasFallingLastFrame = false;
};
