// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "FloorButtonWidget.generated.h"

class UButton;
class UTextBlock;

/**
 * Кнопка одного этажа.
 *
 * Создаётся динамически панелью этажей и хранит индекс своего этажа.
 * При клике вызывает UFloorConfiguratorSubsystem::RequestFloor().
 * Умеет подсвечиваться как "активная", когда этаж выбран.
 */
UCLASS(Abstract)
class FLOORCONFIGURATOR_API UFloorButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ─────────────────────────────────────────────────────────────────
	//  Инициализация
	// ─────────────────────────────────────────────────────────────────

	/**
	 * Задаёт индекс этажа и обновляет текст кнопки.
	 * Должен быть вызван сразу после CreateWidget.
	 *
	 * @param InFloorIndex   Индекс этажа в конфиге здания.
	 */
	void Setup(int32 InFloorIndex);

	/**
	 * Устанавливает/снимает визуальное выделение кнопки.
	 *
	 * @param bInActive   true — этаж выбран, false — нет.
	 */
	void SetActive(bool bInActive);

protected:
	// ─────────────────────────────────────────────────────────────────
	//  Жизненный цикл
	// ─────────────────────────────────────────────────────────────────

	virtual void NativeConstruct() override;

	// ─────────────────────────────────────────────────────────────────
	//  Дочерние виджеты
	// ─────────────────────────────────────────────────────────────────

	// Кликабельная область кнопки.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> FloorButton;

	// Текст на кнопке (номер этажа).
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> FloorText;

	// ─────────────────────────────────────────────────────────────────
	//  Стили
	// ─────────────────────────────────────────────────────────────────

	// Цвет текста в неактивном состоянии.
	UPROPERTY(EditDefaultsOnly, Category = "Configurator|UI")
	FLinearColor InactiveColor = FLinearColor(1.f, 1.f, 1.f, 0.6f);

	// Цвет текста в активном состоянии.
	UPROPERTY(EditDefaultsOnly, Category = "Configurator|UI")
	FLinearColor ActiveColor = FLinearColor(1.f, 1.f, 1.f, 1.f);

	// ─────────────────────────────────────────────────────────────────
	//  Обработчики
	// ─────────────────────────────────────────────────────────────────

	/** Реакция на клик: запрос фокуса на этаж. */
	UFUNCTION()
	void HandleClicked();

private:
	// ─────────────────────────────────────────────────────────────────
	//  Состояние
	// ─────────────────────────────────────────────────────────────────

	// Индекс этажа, за который отвечает эта кнопка.
	int32 FloorIndex = INDEX_NONE;

	// Текущее состояние выделения.
	bool bIsActive = false;
};