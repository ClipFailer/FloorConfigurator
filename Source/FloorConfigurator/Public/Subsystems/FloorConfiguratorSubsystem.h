// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#pragma once

#include "CoreMinimal.h"
#include "Data/BuildingConfig.h"
#include "Data/ConfiguratorTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FloorConfiguratorSubsystem.generated.h"

/**
 *	Подсистема конфигуратора здания.
 *  При инициализации загружает конфиг здания.
 */
UCLASS()
class FLOORCONFIGURATOR_API UFloorConfiguratorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Configurator|Config")
	const FBuildingConfig& GetBuildingConfig() const { return BuildingConfig; }

	/** Обновляет состояние навигации на общий план (Genplan) */
	UFUNCTION(BlueprintCallable)
	void RequestGenplan();

	/**
	 * Обновляет состояние навигации на этаж (Floor)
	 *
	 * @param FloorIndex индекс этажа в floors.
	 */
	UFUNCTION(BlueprintCallable)
	void RequestFloor(int32 FloorIndex);

	/**
	 * Обновляет состояние навигации на квартиру (Apartment)
	 *
	 * @param ApartmentIndex индекс этажа в apartments.
	 */
	UFUNCTION(BlueprintCallable)
	void RequestApartment(int32 ApartmentId);

	/** Откатывает последнее навигационное действие. */
	UFUNCTION(BlueprintCallable, Category = "Configurator|Navigation")
	void GoBack();

	/** true, если есть что откатывать. */
	UFUNCTION(BlueprintPure, Category = "Configurator|Navigation")
	bool CanGoBack() const { return StateHistory.Num() > 0; }

	// Делегаты
	UPROPERTY(BlueprintAssignable, Category = "Configurator|Events")
	FOnViewModeChanged OnViewModeChanged; // Смена мода отображения.

	UPROPERTY(BlueprintAssignable, Category = "Configurator|Events")
	FOnFloorSelected OnFloorSelected; // Фокус на этаже.

	UPROPERTY(BlueprintAssignable, Category = "Configurator|Events")
	FOnApartmentSelected OnApartmentSelected; // Фокус на квартиру.

	/**
	 * Вовзращает кешированный FocusPoint здания.
	 *
	 * @return             Средняя точка.
	 */
	UFUNCTION(BlueprintCallable)
	FVector GetGenplanFocusPoint() const { return CachedGeneralFocusPoint; }

	/**
	 * Вовзращает кешированный FocusPoint этажа.
	 *
	 * @param FloorIndex   Индекс этажа.
	 * @return             Средняя точка.
	 */
	UFUNCTION(BlueprintCallable)
	FVector GetFloorFocusPoint(int32 FloorIndex) const;

protected:
	/**
	 *  Инициализатор подсистемы.
	 *
	 *	Загружает конфиг в BuildingConfig
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Находит квартиру по ID.
	 *
	 * @param ApartmentId     ID квартиры из JSON.
	 * @param OutFloorIndex   Индекс этажа, содержащего квартиру.
	 * @return                Указатель на квартиру или nullptr, если не найдена.
	 */
	const FApartmentData* FindApartmentById(int32 ApartmentId, int32& OutFloorIndex) const;

	/** Вычисляет и кеширует FocusPoint общего вида здания. */
	void CalculateGenplanFocusPoint();

	/** Вычисляет и кеширует FocusPoint`ы всех этажей. */
	void CalculateFloorFocusPoints();

	TArray<FViewState> StateHistory; // История состояний для возврата к предыдущему.

	FBuildingConfig BuildingConfig;
	FViewState		ViewState;

	FVector			CachedGeneralFocusPoint;
	TArray<FVector> CachedFloorFocusPoints;
};
