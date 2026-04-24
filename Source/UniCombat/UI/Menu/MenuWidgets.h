#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuWidgets.generated.h"

UCLASS(BlueprintType)
class UNICOMBAT_API UGameMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI|Menu")
	void SetMenuVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "UI|Menu")
	void RefreshMenu();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "UI|Menu")
	void ReceiveSetMenuVisible(bool bVisible);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI|Menu")
	void ReceiveRefreshMenu();
};
