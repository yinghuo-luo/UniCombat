#include "Combat/Animation/AtlasPlayerAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "KismetAnimationLibrary.h"
#include "Combat/Characters/AtlasPlayerCombatCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UAtlasPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	RefreshOwnerReferences();
}

void UAtlasPlayerAnimInstance::NativeUpdateAnimation(const float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	RefreshOwnerReferences();
	if (PlayerCharacter == nullptr || MovementComponent == nullptr)
	{
		return;
	}

	Velocity = PlayerCharacter->GetVelocity();
	const FVector HorizontalVelocity(Velocity.X, Velocity.Y, 0.0f);

	Speed = HorizontalVelocity.Size();
	Direction = Speed > 3.0f ? UKismetAnimationLibrary::CalculateDirection(HorizontalVelocity, PlayerCharacter->GetActorRotation()) : 0.0f;
	VerticalSpeed = Velocity.Z;

	bHasMovementInput = !PlayerCharacter->GetLastMovementInputVector().IsNearlyZero(0.1f);
	bIsAccelerating = !MovementComponent->GetCurrentAcceleration().IsNearlyZero(0.1f);

	const bool bPreviousFalling = bIsFalling;
	bWasFallingLastFrame = bPreviousFalling;
	bIsFalling = MovementComponent->IsFalling();
	bIsJumping = !bPreviousFalling && bIsFalling && VerticalSpeed > 10.0f;

	bShouldMove = Speed > 5.0f && (bHasMovementInput || bIsAccelerating);

	bIsCrouching = MovementComponent->IsCrouching();
	bIsSprinting = PlayerCharacter->IsSprinting();

	bIsDead = PlayerCharacter->IsDead();
	bIsStunned = PlayerCharacter->IsCombatStunned();

	CombatMode = PlayerCharacter->GetCombatMode();
	bWeaponEquipped = PlayerCharacter->IsWeaponEquipped();

	bUseControllerYaw = PlayerCharacter->UsesControllerRotationYawForAnim();

	const FRotator AimDelta = UKismetMathLibrary::NormalizedDeltaRotator(
		PlayerCharacter->GetBaseAimRotation(),
		PlayerCharacter->GetActorRotation());
	AimYaw = AimDelta.Yaw;
	AimPitch = AimDelta.Pitch;

	if (!bShouldMove && !bIsFalling)
	{
		RootYawOffset = FMath::FInterpTo(RootYawOffset, AimYaw, DeltaSeconds, 8.0f);
	}
	else
	{
		RootYawOffset = FMath::FInterpTo(RootYawOffset, 0.0f, DeltaSeconds, 12.0f);
	}

	GroundDistance = 0.0f;
}

void UAtlasPlayerAnimInstance::RefreshOwnerReferences()
{
	if (PlayerCharacter == nullptr)
	{
		PlayerCharacter = Cast<AAtlasPlayerCombatCharacter>(TryGetPawnOwner());
	}

	if (MovementComponent == nullptr && PlayerCharacter != nullptr)
	{
		MovementComponent = PlayerCharacter->GetCharacterMovement();
	}
}
