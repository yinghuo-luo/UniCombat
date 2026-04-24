// Fill out your copyright notice in the Description page of Project Settings.


#include "AtlasSaveVersionPolicy.h"
#include "SaveSystem/AtlasSaveTypes.h"
#include "SaveSystem/Setting/AtlasSaveSystemSetting.h"

namespace 
{
	static constexpr int32 GAtlasDefaultSaveVersion = 1;
	//清理已配置版本
	static int32 SanitizeConfiguredVersion(const int32 InVersion,
		const int32 FallbackVersion = GAtlasDefaultSaveVersion)
	{
		return InVersion > 0 ? InVersion : FallbackVersion;
	}
}

int32 UAtlasSaveVersionPolicy::GetCurrentSaveVersion_Implementation() const
{
	return ResolveCurrentSaveVersion();
}

bool UAtlasSaveVersionPolicy::CanLoadHeader_Implementation(const FAtlasSaveHeader& Header,
	FString& OutFailureReason) const
{
	const int32 CurrentSaveVersion = ResolveCurrentSaveVersion();
	const int32 MinimumLoadableSaveVersion = ResolveMinimumLoadableSaveVersion();

	if (Header.SaveVersion <= 0)
	{
		OutFailureReason = FString::Printf(TEXT("Save header has an invalid SaveVersion: %d."), Header.SaveVersion);
		return false;
	}

	if (Header.SaveVersion < MinimumLoadableSaveVersion)
	{
		OutFailureReason = FString::Printf(
			TEXT("Save version %d is older than the minimum supported version %d."),
			Header.SaveVersion,
			MinimumLoadableSaveVersion);
		return false;
	}

	if (Header.SaveVersion > CurrentSaveVersion)
	{
		OutFailureReason = FString::Printf(
			TEXT("Save version %d is newer than the current supported version %d."),
			Header.SaveVersion,
			CurrentSaveVersion);
		return false;
	}

	return true;
}

bool UAtlasSaveVersionPolicy::CanLoadProviderRecord_Implementation(const FAtlasProviderRecord& Record,
	FString& OutFailureReason) const
{
	if (Record.ProviderId.IsNone())
	{
		OutFailureReason = TEXT("Provider record is missing ProviderId.");
		return false;
	}

	if (Record.ProviderVersion <= 0)
	{
		OutFailureReason = FString::Printf(
			TEXT("Provider %s has an invalid ProviderVersion: %d."),
			*Record.ProviderId.ToString(),
			Record.ProviderVersion);
		return false;
	}

	if (Record.Payload.SchemaVersion <= 0)
	{
		OutFailureReason = FString::Printf(
			TEXT("Provider %s has an invalid payload schema version: %d."),
			*Record.ProviderId.ToString(),
			Record.Payload.SchemaVersion);
		return false;
	}

	if (Record.Payload.PayloadType.IsNone())
	{
		OutFailureReason = FString::Printf(
			TEXT("Provider %s is missing payload type information."),
			*Record.ProviderId.ToString());
		return false;
	}

	if (Record.Payload.Bytes.IsEmpty())
	{
		OutFailureReason = FString::Printf(
			TEXT("Provider %s has an empty payload."),
			*Record.ProviderId.ToString());
		return false;
	}

	return true;
}

int32 UAtlasSaveVersionPolicy::ResolveCurrentSaveVersion() const
{
	if (CurrentSaveVersionOverride >0)
		return CurrentSaveVersionOverride;
	if (const UAtlasSaveSystemSetting * Setting = GetDefault<UAtlasSaveSystemSetting>())
	{
		return SanitizeConfiguredVersion(Setting->CurrentSaveVersion);
	}
	return GAtlasDefaultSaveVersion;
}

int32 UAtlasSaveVersionPolicy::ResolveMinimumLoadableSaveVersion() const
{
	const int32 CurrentSaveVersion = ResolveCurrentSaveVersion();

	if (MinimumLoadableSaveVersionOverride > 0)
	{
		return FMath::Clamp(MinimumLoadableSaveVersionOverride, GAtlasDefaultSaveVersion, CurrentSaveVersion);
	}

	if (const UAtlasSaveSystemSetting* Settings = GetDefault<UAtlasSaveSystemSetting>())
	{
		return FMath::Clamp(Settings->MinimumLoadableSaveVersion, GAtlasDefaultSaveVersion, CurrentSaveVersion);
	}

	return GAtlasDefaultSaveVersion;
}
