// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "Data/ConfiguratorTypes.h"
#include "ApartmentCardWidget.generated.h"

class UButton;
class UTextBlock;
class UFloorConfiguratorSubsystem;

/**
 * Всплывающая карточка выбранной квартиры.
 *
 * Появляется при выборе квартиры (клик по 3D-объекту), отображает ID,
 * этаж, площадь и статус. Для свободных квартир показывает кнопку
 * "Забронировать". Скрывается при выходе из режима Apartment.
 */
UCLASS(Abstract)
class FLOORCONFIGURATOR_API UApartmentCardWidget : public UUserWidget
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

	// Номер квартиры.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ApartmentIdText;

	// Номер этажа.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> FloorText;

	// Площадь квартиры.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AreaText;

	// Статус: Продано / Свободно.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusText;

	// Кнопка "Забронировать". Видна только для свободных квартир.
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ReserveButton;

	// ─────────────────────────────────────────────────────────────────
	//  Обработчики
	// ─────────────────────────────────────────────────────────────────

	/**
	 * Реакция на выбор квартиры: сохраняем ID и обновляем карточку.
	 *
	 * @param InApartmentId   ID выбранной квартиры.
	 * @param InFocusPoint    Точка фокуса квартиры.
	 */
	UFUNCTION()
	void HandleApartmentSelected(int32 InApartmentId, FVector InFocusPoint);

	/**
	 * Смена режима — скрыть карточку при выходе из Apartment.
	 *
	 * @param NewMode   Новый режим навигации.
	 */
	UFUNCTION()
	void HandleViewModeChanged(EConfiguratorViewMode NewMode);

	/**
	 * Реакция на бронирование квартиры — обновляем карточку.
	 *
	 * @param InApartmentId   ID забронированной квартиры.
	 */
	UFUNCTION()
	void HandleApartmentReserved(int32 InApartmentId);

	/** Реакция на клик по кнопке "Забронировать". */
	UFUNCTION()
	void HandleReserveClicked();

	// ─────────────────────────────────────────────────────────────────
	//  Внутренняя логика
	// ─────────────────────────────────────────────────────────────────

	/** Перечитывает выбранную квартиру из подсистемы и обновляет UI. */
	void Refresh();

private:
	// ─────────────────────────────────────────────────────────────────
	//  Состояние
	// ─────────────────────────────────────────────────────────────────

	// Кешированная подсистема конфигуратора.
	UPROPERTY()
	TObjectPtr<UFloorConfiguratorSubsystem> CachedSubsystem;

	// ID текущей отображаемой квартиры.
	int32 CurrentApartmentId = INDEX_NONE;
};