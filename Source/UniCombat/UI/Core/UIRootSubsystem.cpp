#include "UI/Core/UIRootSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/PlayerController.h"
#include "UI/HUD/HUDWidgets.h"
#include "UI/Menu/MenuWidgets.h"
#include "UI/Setting/AtlasUISetting.h"

namespace
{
	//Reusable可使用的
	template <typename TWidgetType>
	bool IsWidgetReusable(TWidgetType* Widget, const APlayerController* PlayerController)
	{
		return IsValid(Widget)
			&& IsValid(PlayerController)
			&& Widget->GetOwningPlayer() == PlayerController
			&& Widget->GetWorld() == PlayerController->GetWorld();
	}
}

void UUIRootSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UAtlasUISetting* Setting = GetDefault<UAtlasUISetting>();
	if (Setting)
	{
		HUDClass = Setting->HUDClass;
		MenuClass = Setting->MenuClass;
		HUDZOrder = Setting->HUDZOrder;
		MenuZOrder = Setting->MenuZOrder;
	}

}

void UUIRootSubsystem::Deinitialize()
{
	ResetMenuInstance();
	ResetHUDInstance();
	bMenuVisible = false;
	Super::Deinitialize();
}

void UUIRootSubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	Super::PlayerControllerChanged(NewPlayerController);

	ResetMenuInstance();
	ResetHUDInstance();
	bMenuVisible = false;

	if (NewPlayerController)
	{
		ApplyMenuInputMode(false);
	}
}

bool UUIRootSubsystem::EnsureHUD()
{
	APlayerController* PlayerController = ResolvePlayerController();
	if (!PlayerController)
	{
		return false;
	}

	if (!IsWidgetReusable(HUDInstance.Get(), PlayerController))
	{
		ResetHUDInstance();
	}

	if (HUDInstance)
	{
		if (!HUDInstance->IsInViewport())
		{
			HUDInstance->AddToViewport(HUDZOrder);
		}

		HUDInstance->SetMenuOpen(bMenuVisible);
		return true;
	}

	UClass* WidgetClass = HUDClass.LoadSynchronous();
	if (!WidgetClass)
	{
		return false;
	}

	HUDInstance = CreateWidget<UGameHUDWidget>(PlayerController, WidgetClass);
	if (!HUDInstance)
	{
		return false;
	}

	HUDInstance->AddToViewport(HUDZOrder);
	HUDInstance->SetMenuOpen(false);
	OnHUDReady.Broadcast(HUDInstance);
	return true;
}

bool UUIRootSubsystem::EnsureMenu()
{
	APlayerController* PlayerController = ResolvePlayerController();
	if (!PlayerController)
	{
		return false;
	}

	if (!IsWidgetReusable(MenuInstance.Get(), PlayerController))
	{
		ResetMenuInstance();
	}

	if (MenuInstance)
	{
		if (!MenuInstance->IsInViewport())
		{
			MenuInstance->AddToViewport(MenuZOrder);
		}

		MenuInstance->SetVisibility(bMenuVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		MenuInstance->SetMenuVisible(bMenuVisible);
		return true;
	}

	UClass* WidgetClass = MenuClass.LoadSynchronous();
	if (!WidgetClass)
	{
		return false;
	}

	MenuInstance = CreateWidget<UGameMenuWidget>(PlayerController, WidgetClass);
	if (!MenuInstance)
	{
		return false;
	}

	MenuInstance->AddToViewport(MenuZOrder);
	MenuInstance->SetVisibility(ESlateVisibility::Collapsed);
	MenuInstance->SetMenuVisible(false);
	OnMenuReady.Broadcast(MenuInstance);
	return true;
}

bool UUIRootSubsystem::EnsureUI()
{
	return EnsureHUD() && EnsureMenu();
}

bool UUIRootSubsystem::ShowMenu()
{
	if (!EnsureUI())
	{
		return false;
	}

	RefreshMenu();
	UpdateMenuState(true);
	return true;
}

void UUIRootSubsystem::HideMenu()
{
	UpdateMenuState(false);
}

bool UUIRootSubsystem::ToggleMenu()
{
	if (bMenuVisible)
	{
		HideMenu();
		return false;
	}

	return ShowMenu();
}

void UUIRootSubsystem::RefreshMenu()
{
	if (!EnsureMenu() || !MenuInstance)
	{
		return;
	}

	MenuInstance->RefreshMenu();
}

void UUIRootSubsystem::UpdateMenuState(const bool bShouldShow)
{
	const bool bNextVisible = bShouldShow && MenuInstance;
	if (bMenuVisible == bNextVisible)
	{
		return;
	}

	bMenuVisible = bNextVisible;
	if (MenuInstance)
	{
		MenuInstance->SetVisibility(bMenuVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		MenuInstance->SetMenuVisible(bMenuVisible);

		if (bMenuVisible && MenuInstance->IsFocusable())
		{
			MenuInstance->SetKeyboardFocus();
		}
	}

	if (HUDInstance)
	{
		HUDInstance->SetMenuOpen(bMenuVisible);
	}

	ApplyMenuInputMode(bMenuVisible);
	OnMenuVisibilityChanged.Broadcast(bMenuVisible);
}

void UUIRootSubsystem::ApplyMenuInputMode(const bool bShouldShow) const
{
	APlayerController* PlayerController = ResolvePlayerController();
	if (!PlayerController)
	{
		return;
	}

	PlayerController->bShowMouseCursor = bShouldShow;
	PlayerController->SetIgnoreLookInput(bShouldShow);
	PlayerController->SetIgnoreMoveInput(bShouldShow);

	if (bShouldShow && MenuInstance)
	{
		UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(PlayerController, MenuInstance, EMouseLockMode::DoNotLock, false);
		return;
	}

	UWidgetBlueprintLibrary::SetInputMode_GameOnly(PlayerController);
	UWidgetBlueprintLibrary::SetFocusToGameViewport();
}

APlayerController* UUIRootSubsystem::ResolvePlayerController() const
{
	return GetLocalPlayer() ? GetLocalPlayer()->GetPlayerController(GetWorld()) : nullptr;
}

void UUIRootSubsystem::ResetHUDInstance()
{
	if (IsValid(HUDInstance))
	{
		HUDInstance->RemoveFromParent();
	}

	HUDInstance = nullptr;
}

void UUIRootSubsystem::ResetMenuInstance()
{
	if (IsValid(MenuInstance))
	{
		MenuInstance->RemoveFromParent();
	}

	MenuInstance = nullptr;
}
