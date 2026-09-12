// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ConfiguratorCamera.generated.h"

class UCameraComponent;
class UFloorConfiguratorSubsystem;
enum class EConfiguratorViewMode : uint8;

UCLASS()
class FLOORCONFIGURATOR_API AConfiguratorCamera : public AActor
{
	GENERATED_BODY()

public:
	AConfiguratorCamera();

protected:
	virtual void BeginPlay() override;

	/**
	 * Обрабатывает смену режима.
	 * @param NewMode	Новый режим.
	 */
	UFUNCTION()
	void HandleViewModeChanged(EConfiguratorViewMode NewMode);

	/**
	 * Обрабатывает выбор этажа для фокуса.
	 *
	 * @param FloorIndex	Индекс этажа.
	 * @param FocusPoint	Координаты для фокуса.
	 */
	UFUNCTION()
	void HandleFloorSelected(int32 FloorIndex, FVector FocusPoint);

	/**
	 * Обрабатывает выбор квартиры для фокуса.
	 *
	 * @param FloorIndex	Номер квартиры.
	 * @param FocusPoint	Координаты для фокуса.
	 */
	UFUNCTION()
	void HandleApartmentSelected(int32 ApartmentId, FVector FocusPoint);

	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* Camera;

	/** Скорость интерполяции камеры при перелёте. Больше — резче, меньше — плавнее. */
	UPROPERTY(EditAnywhere, Category = "Camera")
	float InterpSpeed = 3.f;

	/** Смещение камеры от центра здания в режиме Genplan. Задаёт общий ракурс. */
	UPROPERTY(EditAnywhere, Category = "Camera")
	FVector GenplanOffset = FVector(-2000.f, 0.f, 1500.f);

	/** Смещение камеры от FocusPoint этажа в режиме Floor. */
	UPROPERTY(EditAnywhere, Category = "Camera")
	FVector FloorOffset = FVector(-1500.f, 0.f, 500.f);

	/** Смещение камеры от FocusPoint квартиры в режиме Apartment. */
	UPROPERTY(EditAnywhere, Category = "Camera")
	FVector ApartmentOffset = FVector(-800.f, 0.f, 200.f);

	/** Частота обновления интерполяции (кадров в секунду) */
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (ClampMin = "1.0", ClampMax = "144"))
	float InterpFrequency = 60.f;

	/** Порог для остановки движения камеры */
	UPROPERTY(EditAnywhere, Category = "Camera")
	float ArrivalThreshold = 1.f;

private:
	/** Пересчитывает позицию камеры для режима Genplan (фокус на общем плане). */
	void UpdateTargetForGenplan();

	/** Пересчитывает позицию камеры для режима Floor (фокус на этаже). */
	void UpdateTargetForFloor(int32 FloorIndex, const FVector& FocusPoint);

	/** Пересчитывает позицию камеры для режима Apartment (фокус на квартире). */
	void UpdateTargetForApartment(int32 ApartmentId, const FVector& FocusPoint);

	/** Устанавливает позицию камеры на точку с смещением. */
	void SetTargetFromFocus(const FVector& FocusPoint, const FVector& Offset);

	/** Запускает таймер интерполяции. */
	void StartInterp();

	/** Останавливает таймер интерполяции. */
	void StopInterp();

	/** Один шаг интерполяции. Вызывается таймером с фиксированной частотой (InterpFrequence). */
	void InterpStep();

	/** Возвращает расстояние от текущей позиции камеры до TargetLocation. */
	float GetDistanceToTarget() const;

	/** Возвращает угловое расхождение между текущим поворотом и TargetRotation (Yaw + Pitch). */
	float GetRotationDeltaToTarget() const;

	FVector	 TargetLocation;
	FRotator TargetRotation;
	bool	 bIsTransitioning = false;

	// Таймер интерполяции.
	FTimerHandle InterpTimer;

	// Кеш конфигуратор.
	UPROPERTY()
	UFloorConfiguratorSubsystem* ConfiguratorSubsystem;
};
