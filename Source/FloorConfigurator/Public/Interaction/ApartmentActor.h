// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#pragma once

#include "CoreMinimal.h"
#include "Data/ConfiguratorTypes.h"
#include "GameFramework/Actor.h"
#include "Interfaces/ApartmentInterface.h"
#include "ApartmentActor.generated.h"

struct FApartmentData;

/**
 * 3D-представление квартиры на сцене.
 * Спавнится из AConfiguratorGameMode по данным BuildingConfig.
 * Поддерживает подсветку (выбор) и затемнение (фильтр "скрыть проданные").
 */
UCLASS()
class FLOORCONFIGURATOR_API AApartmentActor : public AActor, public IApartmentInterface
{
	GENERATED_BODY()

public:
	// ─────────────────────────────────────────────────────────────────
	//  Жизненный цикл
	// ─────────────────────────────────────────────────────────────────

	AApartmentActor();

	/**
	 * Инициализация данными из конфига. Вызывается сразу после спавна.
	 *
	 * @param Data           Данные квартиры из BuildingConfig.
	 * @param ApartmentSize  Размер квартиры (сторона квадрата) в UE-единицах.
	 * @param FloorHeight    Высота этажа в UE-единицах.
	 */
	void Init(const FApartmentData& Data, float ApartmentSize, float FloorHeight);

	// ─────────────────────────────────────────────────────────────────
	//  Геттеры
	// ─────────────────────────────────────────────────────────────────

	virtual int32 GetApartmentId_Implementation() const override { return ApartmentId; }

	UFUNCTION(BlueprintPure)
	FVector GetFocusPoint() const { return FocusPoint; }

	UFUNCTION(BlueprintPure)
	float GetArea() const { return Area; }

	UFUNCTION(BlueprintPure)
	bool IsSold() const { return bIsSold; }

	// ─────────────────────────────────────────────────────────────────
	//  Управление состоянием
	// ─────────────────────────────────────────────────────────────────

	/**
	 * Управляет подсветкой квартиры при выборе.
	 *
	 * @param bInHighlighted   true — включить подсветку.
	 */
	UFUNCTION(BlueprintCallable)
	void SetHighlighted(bool bInHighlighted);

	/**
	 * Управляет затемнением проданной квартиры при активном фильтре.
	 *
	 * @param bInFiltered   true — включить затемнение.
	 */
	UFUNCTION(BlueprintCallable)
	void SetFiltered(bool bInFiltered);

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
	 * Обработка выбора квартиры.
	 *
	 * @param InApartmentId   ID выбранной квартиры.
	 * @param InFocusPoint    Точка фокуса.
	 */
	UFUNCTION()
	void HandleApartmentSelected(int32 InApartmentId, FVector InFocusPoint);

	/**
	 * Смена режима — если ушли из Apartment, снимаем подсветку.
	 *
	 * @param NewMode   Новый режим.
	 */
	UFUNCTION()
	void HandleViewModeChanged(EConfiguratorViewMode NewMode);

	/**
	 * Изменение глобального фильтра "скрыть проданные".
	 *
	 * @param bInHideSold   Новое значение фильтра.
	 */
	UFUNCTION()
	void HandleHideSoldChanged(bool bInHideSold);

	// ─────────────────────────────────────────────────────────────────
	//  Компоненты
	// ─────────────────────────────────────────────────────────────────

	// Корневой компонент — задаёт позицию актора.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;

	// Меш квартиры.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> Mesh;

	// ─────────────────────────────────────────────────────────────────
	//  Материалы
	// ─────────────────────────────────────────────────────────────────

	// Материал подсветки для выделения квартиры при фокусе.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Apartment|Material")
	TObjectPtr<UMaterialInterface> HighlightOverlayMaterial;

	// Материал затемнения проданной квартиры (серый).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Apartment|Material")
	TObjectPtr<UMaterialInterface> FilteredOverlayMaterial;

	// Динамический материал подсветки.
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> HighlightMID;

	// Интенсивность подсветки.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Apartment|Material")
	float HighlightIntensity = 1.f;

	// Толщина выделения.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Apartment|Material")
	float HighlightThickness = 1.f;

	// Цвет подсветки.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Apartment|Material")
	FLinearColor HighlightColor = FLinearColor::Yellow;

private:
	// ─────────────────────────────────────────────────────────────────
	//  Хелперы
	// ─────────────────────────────────────────────────────────────────

	/** Применяет нужный оверлей в зависимости от bHighlighted и bFiltered. */
	void UpdateOverlay();

	// ─────────────────────────────────────────────────────────────────
	//  Данные квартиры
	// ─────────────────────────────────────────────────────────────────

	// ID квартиры.
	int32 ApartmentId = INDEX_NONE;

	// Точка фокуса камеры.
	FVector FocusPoint = FVector::ZeroVector;

	// Площадь квартиры.
	float Area = 49.f;

	// Продана ли квартира.
	bool bIsSold = false;

	// Текущее состояние подсветки.
	bool bHighlighted = false;

	// Текущее состояние фильтрации.
	bool bFiltered = false;
};