// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#pragma once

#include "CoreMinimal.h"
#include "Data/ConfiguratorTypes.h"
#include "GameFramework/Actor.h"
#include "ConfiguratorCamera.generated.h"

class UCameraComponent;
class UFloorConfiguratorSubsystem;

/**
 * Камера конфигуратора.
 *
 * Отдельный актор, не привязанный к персонажу. Принимает команды
 * от PlayerController и Subsystem:
 *   - плавно летит к целевой точке (Genplan / Floor / Apartment);
 *   - вращается вокруг центра здания в режиме Genplan (орбита).
 *
 * Ввод напрямую не обрабатывает — методы вызываются извне.
 */
UCLASS()
class FLOORCONFIGURATOR_API AConfiguratorCamera : public AActor
{
	GENERATED_BODY()

public:
	// ─────────────────────────────────────────────────────────────────
	//  Жизненный цикл
	// ─────────────────────────────────────────────────────────────────

	AConfiguratorCamera();

	// ─────────────────────────────────────────────────────────────────
	//  Орбитальное вращение (режим Genplan)
	// ─────────────────────────────────────────────────────────────────

	/**
	 * Начинает вращение камеры вокруг центра здания.
	 * Работает только в режиме Genplan. Прерывает текущий перелёт.
	 */
	void StartOrbit();

	/** Завершает вращение. */
	void StopOrbit();

	/**
	 * Вращает камеру вокруг центра здания.
	 * Приращивает углы и пересчитывает позицию на сфере того же радиуса.
	 *
	 * @param DeltaYaw     Приращение угла поворота по горизонтали (градусы).
	 * @param DeltaPitch   Приращение угла наклона по вертикали (градусы).
	 */
	void AddOrbitInput(float DeltaYaw, float DeltaPitch);

	/** true, если пользователь сейчас вращает камеру (зажата ПКМ). */
	UFUNCTION(BlueprintPure)
	bool IsOrbiting() const { return bIsOrbiting; }

protected:
	// ─────────────────────────────────────────────────────────────────
	//  Жизненный цикл
	// ─────────────────────────────────────────────────────────────────

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ─────────────────────────────────────────────────────────────────
	//  Обработчики делегатов Subsystem
	// ─────────────────────────────────────────────────────────────────

	/**
	 * Реагирует на смену режима.
	 * Обновляет CurrentMode, сбрасывает орбиту, прерывает перелёт.
	 *
	 * @param NewMode   Новый режим навигации.
	 */
	UFUNCTION()
	void HandleViewModeChanged(EConfiguratorViewMode NewMode);

	/**
	 * Камера летит к этажу.
	 *
	 * @param FloorIndex   Индекс этажа.
	 * @param FocusPoint   Точка фокуса этажа.
	 */
	UFUNCTION()
	void HandleFloorSelected(int32 FloorIndex, FVector FocusPoint);

	/**
	 * Камера летит к квартире.
	 *
	 * @param ApartmentId   ID квартиры.
	 * @param FocusPoint    Точка фокуса квартиры.
	 */
	UFUNCTION()
	void HandleApartmentSelected(int32 ApartmentId, FVector FocusPoint);

	// ─────────────────────────────────────────────────────────────────
	//  Компоненты
	// ─────────────────────────────────────────────────────────────────

	// Корневой компонент — задаёт позицию и поворот актора.
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;

	// Сама камера. Прикреплена к Root, едет вместе с актором.
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> Camera;

	// ─────────────────────────────────────────────────────────────────
	//  Настройки перелёта
	// ─────────────────────────────────────────────────────────────────

	// Скорость интерполяции. Больше — резче, меньше — плавнее.
	UPROPERTY(EditAnywhere, Category = "Camera|Transition")
	float InterpSpeed = 3.f;

	// Частота обновления интерполяции (кадров в секунду).
	UPROPERTY(EditAnywhere, Category = "Camera|Transition", meta = (ClampMin = "1.0", ClampMax = "144"))
	float InterpFrequency = 60.f;

	// Порог «приехали» по расстоянию.
	UPROPERTY(EditAnywhere, Category = "Camera|Transition")
	float ArrivalDistanceThreshold = 1.f;

	// Порог «приехали» по углу (градусы).
	UPROPERTY(EditAnywhere, Category = "Camera|Transition")
	float ArrivalRotationThreshold = 0.5f;

	// ─────────────────────────────────────────────────────────────────
	//  Смещения для режимов
	// ─────────────────────────────────────────────────────────────────

	// Смещение камеры от центра здания в режиме Genplan.
	UPROPERTY(EditAnywhere, Category = "Camera|Offsets")
	FVector GenplanOffset = FVector(-2000.f, 0.f, 1500.f);

	// Смещение камеры от центра этажа в режиме Floor.
	UPROPERTY(EditAnywhere, Category = "Camera|Offsets")
	FVector FloorOffset = FVector(-1500.f, 0.f, 500.f);

	// Сдвиг точки интереса вверх — чтобы смотреть в центр куба квартиры.
	UPROPERTY(EditAnywhere, Category = "Camera|Apartment")
	float ApartmentViewHeight = 150.f;

	// Во сколько раз дистанция обзора больше размера квартиры.
	UPROPERTY(EditAnywhere, Category = "Camera|Apartment")
	float ApartmentViewDistanceMultiplier = 1.8f;

	// ─────────────────────────────────────────────────────────────────
	//  Настройки орбиты
	// ─────────────────────────────────────────────────────────────────

	// Чувствительность мыши при вращении орбиты.
	UPROPERTY(EditAnywhere, Category = "Camera|Orbit")
	float OrbitSensitivity = 1.f;

	// Минимальный угол наклона (защита от переворота камеры).
	UPROPERTY(EditAnywhere, Category = "Camera|Orbit")
	float OrbitMinPitch = -80.f;

	// Максимальный угол наклона.
	UPROPERTY(EditAnywhere, Category = "Camera|Orbit")
	float OrbitMaxPitch = 80.f;

private:
	// ─────────────────────────────────────────────────────────────────
	//  Хелперы: установка целей перелёта
	// ─────────────────────────────────────────────────────────────────

	/** Пересчитывает цель для режима Genplan. */
	void UpdateTargetForGenplan();

	/**
	 * Пересчитывает цель для режима Floor.
	 *
	 * @param FloorIndex   Индекс этажа.
	 * @param FocusPoint   Точка фокуса этажа.
	 */
	void UpdateTargetForFloor(int32 FloorIndex, const FVector& FocusPoint);

	/**
	 * Пересчитывает цель для режима Apartment.
	 *
	 * @param ApartmentId   ID квартиры.
	 * @param FocusPoint    Точка фокуса квартиры.
	 */
	void UpdateTargetForApartment(int32 ApartmentId, const FVector& FocusPoint);

	/**
	 * Устанавливает цель перелёта: фокус + смещение.
	 *
	 * @param FocusPoint   Точка интереса.
	 * @param Offset       Смещение камеры относительно фокуса.
	 */
	void SetTargetFromFocus(const FVector& FocusPoint, const FVector& Offset);

	/**
	 * Устанавливает цель для квартиры.
	 *
	 * Камера встаёт снаружи здания — направление смещения считается
	 * от центра здания к квартире. Это гарантирует, что даже квартира
	 * на дальней стороне будет видна после перелёта.
	 *
	 * @param ApartmentFocus   Точка фокуса квартиры.
	 */
	void SetTargetFromApartment(const FVector& ApartmentFocus);

	// ─────────────────────────────────────────────────────────────────
	//  Хелперы: интерполяция
	// ─────────────────────────────────────────────────────────────────

	/** Запускает таймер интерполяции. */
	void StartInterp();

	/** Останавливает таймер интерполяции. */
	void StopInterp();

	/** Один шаг интерполяции. Вызывается таймером. */
	void InterpStep();

	/** Расстояние от текущей позиции до TargetLocation. */
	float GetDistanceToTarget() const;

	/** Угловое расхождение между текущим поворотом и TargetRotation. */
	float GetRotationDeltaToTarget() const;

	// ─────────────────────────────────────────────────────────────────
	//  Состояние: перелёт
	// ─────────────────────────────────────────────────────────────────

	// Куда летим.
	FVector TargetLocation = FVector::ZeroVector;

	// Куда поворачиваемся.
	FRotator TargetRotation = FRotator::ZeroRotator;

	// true, пока идёт перелёт.
	bool bIsTransitioning = false;

	// Таймер интерполяции.
	FTimerHandle InterpTimer;

	// ─────────────────────────────────────────────────────────────────
	//  Состояние: орбита
	// ─────────────────────────────────────────────────────────────────

	// true, если пользователь зажал ПКМ.
	bool bIsOrbiting = false;

	// Радиус орбиты.
	float OrbitDistance = 0.f;

	// Горизонтальный угол.
	float OrbitYaw = 0.f;

	// Вертикальный угол (ограничен OrbitMinPitch/OrbitMaxPitch).
	float OrbitPitch = 0.f;

	// ─────────────────────────────────────────────────────────────────
	//  Состояние: режим
	// ─────────────────────────────────────────────────────────────────

	// Текущий режим (кэш из Subsystem).
	EConfiguratorViewMode CurrentMode = EConfiguratorViewMode::Genplan;

	// ─────────────────────────────────────────────────────────────────
	//  Кэш ссылок
	// ─────────────────────────────────────────────────────────────────

	// Подсистема конфигуратора.
	UPROPERTY()
	TObjectPtr<UFloorConfiguratorSubsystem> ConfiguratorSubsystem = nullptr;
};