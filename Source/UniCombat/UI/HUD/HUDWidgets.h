#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDWidgets.generated.h"

UCLASS(BlueprintType)
class UNICOMBAT_API UGameHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI|HUD")
	void SetMenuOpen(bool bMenuOpen);

	UFUNCTION(BlueprintCallable, Category = "UI|HUD")
	void RefreshHUD();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "UI|HUD")
	void ReceiveSetMenuOpen(bool bMenuOpen);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI|HUD")
	void ReceiveRefreshHUD();
};
