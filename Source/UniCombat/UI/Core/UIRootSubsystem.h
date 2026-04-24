#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UIRootSubsystem.generated.h"

class APlayerController;
class UGameHUDWidget;
class UGameMenuWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHUDReadyEvent, UGameHUDWidget*, HUD);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMenuReadyEvent, UGameMenuWidget*, Menu);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMenuVisibilityChangedEvent, bool, bMenuVisible);

UCLASS(Config = Game)
class UNICOMBAT_API UUIRootSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;

	UFUNCTION(BlueprintCallable, Category = "UI")
	bool EnsureHUD();

	UFUNCTION(BlueprintCallable, Category = "UI")
	bool EnsureMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	bool EnsureUI();

	UFUNCTION(BlueprintPure, Category = "UI")
	UGameHUDWidget* GetHUD() const { return HUDInstance; }

	UFUNCTION(BlueprintPure, Category = "UI")
	UGameMenuWidget* GetMenu() const { return MenuInstance; }

	UFUNCTION(BlueprintPure, Category = "UI")
	bool IsMenuVisible() const { return bMenuVisible; }

	UFUNCTION(BlueprintCallable, Category = "UI")
	bool ShowMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	bool ToggleMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void RefreshMenu();

	UPROPERTY(BlueprintAssignable, Category = "UI")
	FHUDReadyEvent OnHUDReady;

	//Ready 就绪
	UPROPERTY(BlueprintAssignable, Category = "UI")
	FMenuReadyEvent OnMenuReady;

	UPROPERTY(BlueprintAssignable, Category = "UI")
	FMenuVisibilityChangedEvent OnMenuVisibilityChanged;

protected:
	void UpdateMenuState(bool bShouldShow);
	void ApplyMenuInputMode(bool bShouldShow) const;
	APlayerController* ResolvePlayerController() const;
	void ResetHUDInstance();
	void ResetMenuInstance();

protected:
	TSoftClassPtr<UGameHUDWidget> HUDClass;
	TSoftClassPtr<UGameMenuWidget> MenuClass;
	int32 HUDZOrder = 0;
	int32 MenuZOrder = 50;

	UPROPERTY(Transient)
	TObjectPtr<UGameHUDWidget> HUDInstance = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UGameMenuWidget> MenuInstance = nullptr;

	UPROPERTY(Transient)
	bool bMenuVisible = false;

};
