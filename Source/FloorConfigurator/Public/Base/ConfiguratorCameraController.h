// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ConfiguratorCameraController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class AConfiguratorCamera;
class UFloorConfiguratorSubsystem;
class UConfiguratorHUD;

/**
 * PlayerController конфигуратора.
 *
 * Обрабатывает ввод игрока:
 *   - ЛКМ — клик по 3D-объекту квартиры;
 *   - ПКМ (зажать) — вращение камеры вокруг здания (только в Genplan);
 *   - движение мыши — приращение углов орбиты.
 *
 * Не содержит игровой логики — передаёт ввод в камеру или Subsystem.
 * Ввод привязывается через Enhanced Input в BP-наследнике.
 */
UCLASS()
class FLOORCONFIGURATOR_API AConfiguratorCameraController : public APlayerController
{
	GENERATED_BODY()

protected:
	// ─────────────────────────────────────────────────────────────────
	//  Жизненный цикл
	// ─────────────────────────────────────────────────────────────────

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// ─────────────────────────────────────────────────────────────────
	//  Обработчики ввода
	// ─────────────────────────────────────────────────────────────────

	/** Нажатие ПКМ — начало орбиты. */
	void OnOrbitStarted();

	/** Отпускание ПКМ — конец орбиты. */
	void OnOrbitCompleted();

	/**
	 * Движение мыши — вращение камеры по орбите.
	 *
	 * @param Value   Значение оси мыши (Axis2D).
	 */
	void OnLook(const FInputActionValue& Value);

	/**
	 * ЛКМ — клик по 3D-объекту квартиры.
	 *
	 * @param Value   Значение действия (не используется).
	 */
	void OnClick(const FInputActionValue& Value);

	// ─────────────────────────────────────────────────────────────────
	//  Input-ассеты (назначаются в BP-наследнике)
	// ─────────────────────────────────────────────────────────────────

	// Контекст маппинга Enhanced Input.
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputContext;

	// ПКМ — старт/стоп орбиты.
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Orbit;

	// Движение мыши — вращение камеры вокруг фокуса.
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Look;

	// ЛКМ — клик по квартире.
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Click;

	// ─────────────────────────────────────────────────────────────────
	//  UI
	// ─────────────────────────────────────────────────────────────────

	// Класс главного виджета UI. Задаётся в BP-наследнике контроллера.
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UConfiguratorHUD> HUDWidgetClass;

private:
	// ─────────────────────────────────────────────────────────────────
	//  Хелперы
	// ─────────────────────────────────────────────────────────────────

	/** Находит и кэширует камеру на уровне. */
	void FindAndCacheCamera();

	/** Находит и кэширует подсистему конфигуратора. */
	void FindAndCacheSubsystem();

	/** Подключает Input Mapping Context для управления. */
	void RegisterInputContext();

	/** Пытается выбрать квартиру под курсором и передать запрос в Subsystem. */
	void TrySelectApartmentUnderCursor();

	// ─────────────────────────────────────────────────────────────────
	//  Состояние
	// ─────────────────────────────────────────────────────────────────

	// Созданный главный виджет UI.
	UPROPERTY()
	TObjectPtr<UConfiguratorHUD> HUDWidget;

	// Кэш камеры.
	UPROPERTY()
	TObjectPtr<AConfiguratorCamera> CachedCamera = nullptr;

	// Кэш подсистемы.
	UPROPERTY()
	TObjectPtr<UFloorConfiguratorSubsystem> Subsystem = nullptr;
};