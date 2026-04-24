#include "UI/Menu/MenuWidgets.h"

void UGameMenuWidget::SetMenuVisible(const bool bVisible)
{
	ReceiveSetMenuVisible(bVisible);
}

void UGameMenuWidget::RefreshMenu()
{
	ReceiveRefreshMenu();
}
