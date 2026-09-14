// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#pragma once

#include "CoreMinimal.h"
#include "Data/BuildingConfig.h"
#include "Data/ConfiguratorTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FloorConfiguratorSubsystem.generated.h"

/**
 * Подсистема конфигуратора здания.
 * При инициализации загружает конфиг здания.
 */
UCLASS()
class FLOORCONFIGURATOR_API UFloorConfiguratorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ─────────────────────────────────────────────────────────────────
	//  Геттеры
	// ─────────────────────────────────────────────────────────────────

	/** Возвращает конфиг здания. */
	UFUNCTION(BlueprintPure, Category = "Configurator|Config")
	const FBuildingConfig& GetBuildingConfig() const { return BuildingConfig; }

	/** Возвращает кешированный FocusPoint здания (средняя точка). */
	UFUNCTION(BlueprintPure)
	FVector GetGenplanFocusPoint() const { return CachedGeneralFocusPoint; }

	/**
	 * Возвращает кешированный FocusPoint этажа.
	 *
	 * @param FloorIndex   Индекс этажа.
	 * @return             Средняя точка.
	 */
	UFUNCTION(BlueprintPure)
	FVector GetFloorFocusPoint(int32 FloorIndex) const;

	/**
	 * Возвращает высоту этажа в UE-единицах.
	 * Считается как разница Z между этажом и следующим.
	 * Для последнего этажа — высота предыдущего.
	 *
	 * @param FloorIndex   Индекс этажа.
	 */
	UFUNCTION(BlueprintPure, Category = "Configurator|Cache")
	float GetFloorHeight(int32 FloorIndex) const;

	/** Возвращает кэшированный размер квартиры (сторона квадрата). */
	UFUNCTION(BlueprintPure, Category = "Configurator|Cache")
	float GetApartmentSize() const { return CachedApartmentSize; }

	/** Возвращает текущее состояние навигации. */
	const FViewState& GetViewState() const { return ViewState; }

	/** Возвращает количество этажей. */
	UFUNCTION(BlueprintPure, Category = "Configurator|Cache")
	int32 GetFloorsCount() const { return BuildingConfig.Floors.Num(); }

	/**
	 * Возвращает этаж по индексу.
	 *
	 * @param FloorIndex   Индекс этажа. Должен быть валидным.
	 */
	UFUNCTION(BlueprintPure, Category = "Configurator|Cache")
	const FFloorData& GetFloor(int32 FloorIndex) const { return BuildingConfig.Floors[FloorIndex]; }

	/** Возвращает true, если есть что откатывать. */
	bool CanGoBack() const { return StateHistory.Num() > 0; }

	/**
	 * Находит квартиру по ID.
	 *
	 * @param ApartmentId     ID квартиры из JSON.
	 * @param OutFloorIndex   Индекс этажа, содержащего квартиру.
	 * @return                Указатель на квартиру или nullptr, если не найдена.
	 */
	const FApartmentData* FindApartmentById(int32 ApartmentId, int32& OutFloorIndex) const;

	/**
	 * Проверяет, забронирована ли квартира локально.
	 *
	 * @param ApartmentId   ID квартиры.
	 * @return              true, если квартира в списке забронированных.
	 */
	bool IsApartmentReserved(int32 ApartmentId) const;

	/**
	 * Возвращает true, если квартира недоступна для брони.
	 * Учитывает и статус из JSON, и локальные брони.
	 *
	 * @param ApartmentId   ID квартиры.
	 */
	bool IsApartmentSold(int32 ApartmentId) const;

	/**
	 * Бронирует квартиру. По ТЗ достаточно лога.
	 * Дополнительно сохраняет ID в локальном списке, чтобы состояние
	 * не сбрасывалось при повторном открытии карточки.
	 *
	 * @param ApartmentId   ID квартиры.
	 */
	void ReserveApartment(int32 ApartmentId);

	// ─────────────────────────────────────────────────────────────────
	//  Команды навигации
	// ─────────────────────────────────────────────────────────────────

	/** Обновляет состояние навигации на общий план (Genplan). */
	UFUNCTION(BlueprintCallable)
	void RequestGenplan();

	/**
	 * Обновляет состояние навигации на этаж (Floor).
	 *
	 * @param FloorIndex   Индекс этажа в Floors.
	 */
	UFUNCTION(BlueprintCallable)
	void RequestFloor(int32 FloorIndex);

	/**
	 * Обновляет состояние навигации на квартиру (Apartment).
	 *
	 * @param ApartmentId   ID квартиры из конфига.
	 */
	UFUNCTION(BlueprintCallable)
	void RequestApartment(int32 ApartmentId);

	/** Откатывает последнее навигационное действие. */
	UFUNCTION(BlueprintCallable, Category = "Configurator|Navigation")
	void GoBack();

	// ─────────────────────────────────────────────────────────────────
	//  Делегаты
	// ─────────────────────────────────────────────────────────────────

	// Смена режима отображения.
	UPROPERTY(BlueprintAssignable, Category = "Configurator|Events")
	FOnViewModeChanged OnViewModeChanged;

	// Фокус на этаже.
	UPROPERTY(BlueprintAssignable, Category = "Configurator|Events")
	FOnFloorSelected OnFloorSelected;

	// Фокус на квартиру.
	UPROPERTY(BlueprintAssignable, Category = "Configurator|Events")
	FOnApartmentSelected OnApartmentSelected;

	// Курсор на карточке квартиры.
	UPROPERTY(BlueprintAssignable, Category = "Configurator|Events")
	FOnApartmentHovered OnApartmentHovered;

	// Изменение фильтра "скрыть проданные".
	UPROPERTY(BlueprintAssignable, Category = "Configurator|Events")
	FOnHideSoldChanged OnHideSoldChanged;

	// Квартира забронирована.
	UPROPERTY(BlueprintAssignable, Category = "Configurator|Events")
	FOnApartmentReserved OnApartmentReserved;

protected:
	// ─────────────────────────────────────────────────────────────────
	//  Жизненный цикл
	// ─────────────────────────────────────────────────────────────────

	/**
	 * Инициализатор подсистемы.
	 * Загружает конфиг в BuildingConfig.
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	// ─────────────────────────────────────────────────────────────────
	//  Хелперы для вычислений
	// ─────────────────────────────────────────────────────────────────

	/** Вычисляет и кеширует FocusPoint общего вида здания. */
	void CalculateGenplanFocusPoint();

	/** Вычисляет и кеширует FocusPoint'ы всех этажей. */
	void CalculateFloorFocusPoints();

	/** Вычисляет размер квартиры из расстояний между ними. */
	void CalculateApartmentSize();

	/**
	 * Возвращает Z-уровень этажа (по первой квартире).
	 *
	 * @param FloorIndex   Индекс этажа.
	 */
	float GetFloorLevelZ(int32 FloorIndex) const;

	// ─────────────────────────────────────────────────────────────────
	//  Состояние
	// ─────────────────────────────────────────────────────────────────

	// Загруженный конфиг здания.
	FBuildingConfig BuildingConfig;

	// Текущее состояние навигации.
	FViewState ViewState;

	// История действий для возврата к предыдущему.
	TArray<FViewState> StateHistory;

	// ID локально забронированных квартир.
	TSet<int32> ReservedApartmentIds;

	// ─────────────────────────────────────────────────────────────────
	//  Кэши (вычисляются один раз при инициализации)
	// ─────────────────────────────────────────────────────────────────

	// Кеш FocusPoint всего здания.
	FVector CachedGeneralFocusPoint;

	// Кеш FocusPoint'ов всех этажей.
	TArray<FVector> CachedFloorFocusPoints;

	// Кэшированный размер квартиры в UE-единицах.
	float CachedApartmentSize = 300.f;

	// ─────────────────────────────────────────────────────────────────
	//  Константы
	// ─────────────────────────────────────────────────────────────────

	// Дефолтная высота, если данных не хватает.
	static constexpr float DefaultFloorHeight = 300.f;
};