#include "Combat/Weapons/AtlasWeaponBase.h"

#include "Combat/Characters/AtlasCombatCharacterBase.h"
#include "Combat/Weapons/AtlasWeaponData.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"

AAtlasWeaponBase::AAtlasWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	/** 如果 Actor 具有有效的 Owner，则调用 Owner 的 IsNetRelevantFor 和 GetNetPriority 方法 */
	bNetUseOwnerRelevancy = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(SceneRoot);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("TraceStart"));
	TraceStart->SetupAttachment(SceneRoot);

	TraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("TraceEnd"));
	TraceEnd->SetupAttachment(SceneRoot);

	SpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnPoint"));
	SpawnPoint->SetupAttachment(SceneRoot);
}

AAtlasCombatCharacterBase* AAtlasWeaponBase::GetOwningCombatCharacter() const
{
	return Cast<AAtlasCombatCharacterBase>(GetOwner());
}

void AAtlasWeaponBase::AttachToCharacter(AAtlasCombatCharacterBase* NewOwnerCharacter, const FName OverrideSocketName)
{
	if (NewOwnerCharacter == nullptr)
	{
		return;
	}

	SetOwner(NewOwnerCharacter);
	/** 设置 Instigator 的值，而不会对该实例造成其他副作用。 */
	SetInstigator(Cast<APawn>(NewOwnerCharacter));

	USkeletalMeshComponent* OwnerMesh = NewOwnerCharacter->GetMesh();
	if (OwnerMesh == nullptr)
	{
		return;
	}

	AttachToComponent(
		OwnerMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		ResolveAttachSocket(OverrideSocketName));
}

void AAtlasWeaponBase::DetachFromCharacter()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetOwner(nullptr);
	SetInstigator(nullptr);
}

bool AAtlasWeaponBase::BuildAttackParameters(FAtlasWeaponAttackParameters& OutAttackParameters) const
{
	//构建攻击参数
	OutAttackParameters = FAtlasWeaponAttackParameters();
	OutAttackParameters.EffectCauser = const_cast<AAtlasWeaponBase*>(this);

	if (WeaponData != nullptr)
	{
		OutAttackParameters.TraceRadius = FMath::Max(WeaponData->TraceRadius, 1.0f);
		OutAttackParameters.TraceChannel = WeaponData->TraceChannel;
		OutAttackParameters.DamageMultiplier = FMath::Max(WeaponData->DamageMultiplier, 0.0f);
		OutAttackParameters.PoiseMultiplier = FMath::Max(WeaponData->PoiseMultiplier, 0.0f);
		OutAttackParameters.DamageEffectTags = WeaponData->DamageEffectTags;
	}

	OutAttackParameters.TraceStart = TraceStart != nullptr
		? TraceStart->GetComponentLocation()
		: GetActorLocation();

	OutAttackParameters.TraceEnd = TraceEnd != nullptr
		? TraceEnd->GetComponentLocation()
		: OutAttackParameters.TraceStart;

	if (OutAttackParameters.TraceStart.Equals(OutAttackParameters.TraceEnd))
	{
		const float TraceDistance = WeaponData != nullptr
			? FMath::Max(WeaponData->TraceDistance, 100.0f)
			: 150.0f;
		const FVector ForwardVector = WeaponMesh != nullptr
			? WeaponMesh->GetForwardVector()
			: GetActorForwardVector();
		OutAttackParameters.TraceEnd = OutAttackParameters.TraceStart + (ForwardVector * TraceDistance);
	}

	return true;
}

FTransform AAtlasWeaponBase::GetSpawnTransform() const
{
	if (SpawnPoint != nullptr)
	{
		return SpawnPoint->GetComponentTransform();
	}

	return GetActorTransform();
}

FVector AAtlasWeaponBase::GetSpawnLocation() const
{
	return GetSpawnTransform().GetLocation();
}

void AAtlasWeaponBase::OnEquipped_Implementation(AAtlasCombatCharacterBase* NewOwnerCharacter)
{
}

void AAtlasWeaponBase::OnUnequipped_Implementation(AAtlasCombatCharacterBase* PreviousOwnerCharacter)
{
}

FName AAtlasWeaponBase::ResolveAttachSocket(const FName OverrideSocketName) const
{
	if (OverrideSocketName != NAME_None)
	{
		return OverrideSocketName;
	}

	return WeaponData != nullptr ? WeaponData->AttachSocketName : NAME_None;
}
