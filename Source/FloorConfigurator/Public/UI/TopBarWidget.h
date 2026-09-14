// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "Data/ConfiguratorTypes.h"
#include "TopBarWidget.generated.h"

class UButton;
class UCheckBox;
class UTextBlock;
class UFloorConfiguratorSubsystem;
class UFloorPanelWidget;

/**
 * Верхняя панель конфигуратора.
 *
 * Содержит кнопки "Назад" и "Общий вид", текст текущего состояния,
 * панель кнопок этажей и чекбокс "Скрыть проданные". Обновляет UI
 * при смене режима и при выборе этажа/квартиры.
 */
UCLASS(Abstract)
class FLOORCONFIGURATOR_API UTopBarWidget : public UUserWidget
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

	// Кнопка "Назад". Откатывает последнее действие.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BackButton;

	// Кнопка "Общий вид". Возвращает в Genplan.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> GenplanButton;

	// Чекбокс "Скрыть проданные".
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> HideSoldCheckbox;

	// Текст текущего состояния навигации.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrentStateText;

	// Панель выбора этажей.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UFloorPanelWidget> FloorPanelWidget;

	// ─────────────────────────────────────────────────────────────────
	//  Обработчики
	// ─────────────────────────────────────────────────────────────────

	/** Реакция на клик по кнопке "Назад". */
	UFUNCTION()
	void HandleBackClicked();

	/** Реакция на клик по кнопке "Общий вид". */
	UFUNCTION()
	void HandleGenplanClicked();

	/**
	 * Реакция на переключение чекбокса "Скрыть проданные".
	 *
	 * @param bInHideSold   Новое состояние фильтра.
	 */
	UFUNCTION()
	void HandleHideSoldChanged(bool bInHideSold);

	/**
	 * Смена режима — обновляем крошки и активность кнопок.
	 *
	 * @param NewMode   Новый режим.
	 */
	UFUNCTION()
	void HandleViewModeChanged(EConfiguratorViewMode NewMode);

	/**
	 * Выбор этажа — обновляем крошки.
	 *
	 * @param FloorIndex   Индекс этажа.
	 * @param FocusPoint   Точка фокуса.
	 */
	UFUNCTION()
	void HandleFloorSelected(int32 FloorIndex, FVector FocusPoint);

	/**
	 * Выбор квартиры — обновляем крошки.
	 *
	 * @param ApartmentId   ID квартиры.
	 * @param FocusPoint    Точка фокуса.
	 */
	UFUNCTION()
	void HandleApartmentSelected(int32 ApartmentId, FVector FocusPoint);

	// ─────────────────────────────────────────────────────────────────
	//  Внутренняя логика
	// ─────────────────────────────────────────────────────────────────

	/** Перечитывает состояние подсистемы и обновляет UI. */
	void Refresh();

private:
	// ─────────────────────────────────────────────────────────────────
	//  Состояние
	// ─────────────────────────────────────────────────────────────────

	// Кешированная подсистема конфигуратора.
	UPROPERTY()
	TObjectPtr<UFloorConfiguratorSubsystem> CachedSubsystem;
};