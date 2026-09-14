// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "ConfiguratorHUD.generated.h"

class UTopBarWidget;
class UApartmentCardWidget;

/**
 * Главный виджет конфигуратора.
 *
 * Собирает верхнюю панель (TopBar) и всплывающую карточку квартиры
 * (ApartmentCard). TopBar виден всегда; ApartmentCard сама управляет
 * своей видимостью в зависимости от текущего режима.
 */
UCLASS(Abstract)
class FLOORCONFIGURATOR_API UConfiguratorHUD : public UUserWidget
{
	GENERATED_BODY()

protected:
	// ─────────────────────────────────────────────────────────────────
	//  Дочерние виджеты
	// ─────────────────────────────────────────────────────────────────

	// Верхняя панель: кнопки, крошки, панель этажей, чекбокс фильтра.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTopBarWidget> TopBar;

	// Всплывающая карточка выбранной квартиры.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UApartmentCardWidget> ApartmentCard;
};