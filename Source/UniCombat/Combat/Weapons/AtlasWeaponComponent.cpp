#include "Combat/Weapons/AtlasWeaponComponent.h"

#include "Combat/AbilitySystem/AtlasAbilitySystemComponent.h"
#include "Combat/Characters/AtlasCombatCharacterBase.h"
#include "Combat/Characters/AtlasPlayerCombatCharacter.h"
#include "Combat/Weapons/AtlasWeaponBase.h"
#include "Combat/Weapons/AtlasWeaponData.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

UAtlasWeaponComponent::UAtlasWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	/**
	* 设置 bReplicates 的值，而不会对该实例产生其他副作用。
	* 此方法仅应在组件构造期间调用。
	* 此方法仅用于允许代码直接修改 bReplicates 以保持现有行为。
	*/
	SetIsReplicatedByDefault(true);
}

void UAtlasWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() != nullptr && GetOwner()->HasAuthority() && bAutoEquipOnBeginPlay)
	{
		EquipWeaponForCombatMode(ResolveAutoEquipMode());
	}
}

void UAtlasWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetOwner() != nullptr && GetOwner()->HasAuthority())
	{
		RemoveWeaponGrants();
	}

	Super::EndPlay(EndPlayReason);
}

void UAtlasWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAtlasWeaponComponent, CurrentWeapon);
	DOREPLIFETIME(UAtlasWeaponComponent, EquippedCombatMode);
	DOREPLIFETIME(UAtlasWeaponComponent, EquippedAttachSocketName);
}

void UAtlasWeaponComponent::EquipWeaponForCombatMode(const EAtlasCombatMode CombatMode)
{
	if (GetOwner() == nullptr)
	{
		return;
	}

	if (!GetOwner()->HasAuthority())
	{
		ServerEquipWeaponForCombatMode(CombatMode);
		return;
	}

	const FAtlasWeaponLoadoutEntry* LoadoutEntry = FindLoadoutEntry(CombatMode);
	if (LoadoutEntry == nullptr || LoadoutEntry->WeaponClass == nullptr)
	{
		UnequipCurrentWeapon();
		EquippedCombatMode = CombatMode;
		return;
	}

	if (CurrentWeapon != nullptr && CurrentWeapon->GetClass() == LoadoutEntry->WeaponClass)
	{
		EquippedCombatMode = CombatMode;
		EquippedAttachSocketName = LoadoutEntry->AttachSocketName;
		AttachWeaponToOwner(CurrentWeapon, EquippedAttachSocketName);
		GetOwner()->ForceNetUpdate();
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.Instigator = Cast<APawn>(GetOwner());
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AAtlasWeaponBase* SpawnedWeapon = GetWorld() != nullptr
		? GetWorld()->SpawnActor<AAtlasWeaponBase>(LoadoutEntry->WeaponClass, FTransform::Identity, SpawnParameters)
		: nullptr;
	if (SpawnedWeapon == nullptr)
	{
		return;
	}

	SetCurrentWeaponInternal(SpawnedWeapon, CombatMode, LoadoutEntry->AttachSocketName);
}

void UAtlasWeaponComponent::ServerEquipWeaponForCombatMode_Implementation(const EAtlasCombatMode CombatMode)
{
	EquipWeaponForCombatMode(CombatMode);
}

void UAtlasWeaponComponent::EquipWeaponClass(TSubclassOf<AAtlasWeaponBase> WeaponClass, const FName AttachSocketName)
{
	if (GetOwner() == nullptr || WeaponClass == nullptr)
	{
		return;
	}

	if (!GetOwner()->HasAuthority())
	{
		ServerEquipWeaponClass(WeaponClass, AttachSocketName);
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.Instigator = Cast<APawn>(GetOwner());
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AAtlasWeaponBase* SpawnedWeapon = GetWorld() != nullptr
		? GetWorld()->SpawnActor<AAtlasWeaponBase>(WeaponClass, FTransform::Identity, SpawnParameters)
		: nullptr;
	if (SpawnedWeapon == nullptr)
	{
		return;
	}

	const UAtlasWeaponData* WeaponData = SpawnedWeapon->GetWeaponData();
	const EAtlasCombatMode TargetMode = WeaponData != nullptr
		? WeaponData->CombatMode
		: EquippedCombatMode;
	SetCurrentWeaponInternal(SpawnedWeapon, TargetMode, AttachSocketName);
}

void UAtlasWeaponComponent::ServerEquipWeaponClass_Implementation(TSubclassOf<AAtlasWeaponBase> WeaponClass,
	const FName AttachSocketName)
{
	EquipWeaponClass(WeaponClass, AttachSocketName);
}

void UAtlasWeaponComponent::UnequipCurrentWeapon(const bool bDestroyWeapon)
{
	if (GetOwner() == nullptr)
	{
		return;
	}

	if (!GetOwner()->HasAuthority())
	{
		ServerUnequipCurrentWeapon(bDestroyWeapon);
		return;
	}

	AAtlasWeaponBase* PreviousWeapon = CurrentWeapon;
	if (PreviousWeapon == nullptr)
	{
		EquippedCombatMode = EAtlasCombatMode::Knife;
		EquippedAttachSocketName = NAME_None;
		return;
	}

	RemoveWeaponGrants();

	CurrentWeapon = nullptr;
	EquippedCombatMode = EAtlasCombatMode::Knife;
	EquippedAttachSocketName = NAME_None;

	HandleCurrentWeaponChanged(PreviousWeapon);

	if (bDestroyWeapon)
	{
		PreviousWeapon->Destroy();
	}
	else
	{
		PreviousWeapon->DetachFromCharacter();
	}

	GetOwner()->ForceNetUpdate();
}

void UAtlasWeaponComponent::ServerUnequipCurrentWeapon_Implementation(const bool bDestroyWeapon)
{
	UnequipCurrentWeapon(bDestroyWeapon);
}

bool UAtlasWeaponComponent::BuildCurrentAttackParameters(FAtlasWeaponAttackParameters& OutAttackParameters) const
{
	if (CurrentWeapon == nullptr)
	{
		return false;
	}

	return CurrentWeapon->BuildAttackParameters(OutAttackParameters);
}

void UAtlasWeaponComponent::OnRep_CurrentWeapon(AAtlasWeaponBase* PreviousWeapon)
{
	HandleCurrentWeaponChanged(PreviousWeapon);
}

const FAtlasWeaponLoadoutEntry* UAtlasWeaponComponent::FindLoadoutEntry(const EAtlasCombatMode CombatMode) const
{
	/**
	* 查找与谓词函子匹配的元素。
	* @param Pred 要应用于每个元素的函子。
	* @returns 指向谓词返回 true 的第一个元素的指针，如果没有找到匹配项，则返回 nullptr。
	* @see FilterByPredicate, ContainsByPredicate
	*/
	return WeaponLoadout.FindByPredicate(
		[CombatMode](const FAtlasWeaponLoadoutEntry& Entry)
		{
			return Entry.CombatMode == CombatMode;
		});
}

EAtlasCombatMode UAtlasWeaponComponent::ResolveAutoEquipMode() const
{
	//解决自动装备模式
	if (bSyncWithOwnerCombatMode)
	{
		if (const AAtlasPlayerCombatCharacter* PlayerCharacter = Cast<AAtlasPlayerCombatCharacter>(GetOwner()))
		{
			return PlayerCharacter->GetCombatMode();
		}
	}

	//如果不自动加载就使用配置的
	if (FindLoadoutEntry(InitialEquippedCombatMode) != nullptr)
	{
		return InitialEquippedCombatMode;
	}

	return WeaponLoadout.Num() > 0
		? WeaponLoadout[0].CombatMode
		: EAtlasCombatMode::Knife;
}

AAtlasCombatCharacterBase* UAtlasWeaponComponent::GetCombatCharacterOwner() const
{
	return Cast<AAtlasCombatCharacterBase>(GetOwner());
}

UAtlasAbilitySystemComponent* UAtlasWeaponComponent::GetOwnerAbilitySystemComponent() const
{
	if (const AAtlasCombatCharacterBase* CombatCharacter = GetCombatCharacterOwner())
	{
		return CombatCharacter->GetAtlasAbilitySystemComponent();
	}

	return nullptr;
}

void UAtlasWeaponComponent::AttachWeaponToOwner(AAtlasWeaponBase* WeaponActor, const FName OverrideSocketName) const
{
	if (WeaponActor == nullptr)
	{
		return;
	}

	if (AAtlasCombatCharacterBase* CombatCharacter = GetCombatCharacterOwner())
	{
		WeaponActor->AttachToCharacter(CombatCharacter, OverrideSocketName);
	}
}

void UAtlasWeaponComponent::HandleCurrentWeaponChanged(AAtlasWeaponBase* PreviousWeapon)
{
	AAtlasCombatCharacterBase* OwnerCharacter = GetCombatCharacterOwner();

	if (PreviousWeapon != nullptr && PreviousWeapon != CurrentWeapon)
	{
		PreviousWeapon->OnUnequipped(OwnerCharacter);
	}

	if (CurrentWeapon != nullptr)
	{
		AttachWeaponToOwner(CurrentWeapon, EquippedAttachSocketName);
		CurrentWeapon->OnEquipped(OwnerCharacter);
	}
}

void UAtlasWeaponComponent::ApplyWeaponGrants(AAtlasWeaponBase* WeaponActor)
{
	UAtlasAbilitySystemComponent* AbilitySystemComponent = GetOwnerAbilitySystemComponent();
	const UAtlasWeaponData* WeaponData = WeaponActor != nullptr ? WeaponActor->GetWeaponData() : nullptr;
	if (AbilitySystemComponent == nullptr || WeaponData == nullptr)
	{
		return;
	}

	for (const FGameplayTag& GameplayTag : WeaponData->GrantedOwnerTags)
	{
		AbilitySystemComponent->AddLooseGameplayTag(GameplayTag);
		GrantedWeaponTags.AddTag(GameplayTag);
	}

	if (WeaponData->AbilitySetOnEquip != nullptr)
	{
		AbilitySystemComponent->GrantAbilitySet(
			WeaponData->AbilitySetOnEquip,
			WeaponActor,
			&EquippedWeaponGrantedHandles);
	}
}

void UAtlasWeaponComponent::RemoveWeaponGrants()
{
	UAtlasAbilitySystemComponent* AbilitySystemComponent = GetOwnerAbilitySystemComponent();
	if (AbilitySystemComponent == nullptr)
	{
		GrantedWeaponTags.Reset();
		return;
	}

	EquippedWeaponGrantedHandles.TakeFromAbilitySystem(AbilitySystemComponent);

	for (const FGameplayTag& GameplayTag : GrantedWeaponTags)
	{
		if (AbilitySystemComponent->HasMatchingGameplayTag(GameplayTag))
		{
			AbilitySystemComponent->RemoveLooseGameplayTag(GameplayTag);
		}
	}

	GrantedWeaponTags.Reset();
}

void UAtlasWeaponComponent::SetCurrentWeaponInternal(AAtlasWeaponBase* NewWeapon, const EAtlasCombatMode NewCombatMode,
	const FName AttachSocketName)
{
	AAtlasWeaponBase* PreviousWeapon = CurrentWeapon;
	if (PreviousWeapon == NewWeapon)
	{
		EquippedCombatMode = NewCombatMode;
		EquippedAttachSocketName = AttachSocketName;

		if (CurrentWeapon != nullptr)
		{
			AttachWeaponToOwner(CurrentWeapon, EquippedAttachSocketName);
		}

		return;
	}

	if (PreviousWeapon != nullptr)
	{
		RemoveWeaponGrants();
	}

	CurrentWeapon = NewWeapon;
	EquippedCombatMode = NewCombatMode;
	EquippedAttachSocketName = AttachSocketName;

	if (CurrentWeapon != nullptr)
	{
		CurrentWeapon->SetOwner(GetOwner());
		CurrentWeapon->SetInstigator(Cast<APawn>(GetOwner()));
		ApplyWeaponGrants(CurrentWeapon);
	}

	HandleCurrentWeaponChanged(PreviousWeapon);

	if (PreviousWeapon != nullptr)
	{
		PreviousWeapon->Destroy();
	}

	if (GetOwner() != nullptr)
	{
		GetOwner()->ForceNetUpdate();
	}
}
