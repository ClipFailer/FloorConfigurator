// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "Data/ConfiguratorTypes.h"
#include "FloorPanelWidget.generated.h"

class UHorizontalBox;
class UFloorButtonWidget;
class UFloorConfiguratorSubsystem;

/**
 * Панель кнопок этажей.
 *
 * При NativeConstruct динамически создаёт по одной кнопке на каждый
 * этаж из конфига. Подписана на OnViewModeChanged и подсвечивает
 * активную кнопку согласно ViewState.FloorIndex. В режиме Genplan
 * ни одна кнопка не активна.
 */
UCLASS(Abstract)
class FLOORCONFIGURATOR_API UFloorPanelWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// ─────────────────────────────────────────────────────────────────
	//  Жизненный цикл
	// ─────────────────────────────────────────────────────────────────

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ─────────────────────────────────────────────────────────────────
	//  Дочерние виджеты
	// ─────────────────────────────────────────────────────────────────

	// Контейнер для кнопок этажей.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> FloorButtonsContainer;

	// ─────────────────────────────────────────────────────────────────
	//  Настройки
	// ─────────────────────────────────────────────────────────────────

	// BP-класс кнопки этажа (наследник UFloorButtonWidget).
	UPROPERTY(EditDefaultsOnly, Category = "Configurator|UI")
	TSubclassOf<UFloorButtonWidget> FloorButtonClass;

	// ─────────────────────────────────────────────────────────────────
	//  Обработчики
	// ─────────────────────────────────────────────────────────────────

	/**
	 * Смена режима — обновляем активную кнопку.
	 *
	 * @param NewMode   Новый режим.
	 */
	UFUNCTION()
	void HandleViewModeChanged(EConfiguratorViewMode NewMode);

	// ─────────────────────────────────────────────────────────────────
	//  Внутренняя логика
	// ─────────────────────────────────────────────────────────────────

	/** Создаёт по одной кнопке на каждый этаж из конфига. */
	void BuildFloorButtons();

	/** Подсвечивает кнопку, соответствующую ViewState.FloorIndex. */
	void UpdateActiveButton();

private:
	// ─────────────────────────────────────────────────────────────────
	//  Состояние
	// ─────────────────────────────────────────────────────────────────

	// Кешированная подсистема конфигуратора.
	UPROPERTY()
	TObjectPtr<UFloorConfiguratorSubsystem> CachedSubsystem;

	// Все созданные кнопки этажей.
	UPROPERTY()
	TArray<TObjectPtr<UFloorButtonWidget>> FloorButtons;
};