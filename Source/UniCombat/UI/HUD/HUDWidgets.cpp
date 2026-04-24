#include "UI/HUD/HUDWidgets.h"

void UGameHUDWidget::SetMenuOpen(const bool bMenuOpen)
{
	ReceiveSetMenuOpen(bMenuOpen);
}

void UGameHUDWidget::RefreshHUD()
{
	ReceiveRefreshHUD();
}
