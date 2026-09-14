// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#pragma once

#include "CoreMinimal.h"
#include "ConfiguratorTypes.generated.h"

/**
 * Состояние навигации камеры в конфигураторе.
 */
UENUM(BlueprintType)
enum class EConfiguratorViewMode : uint8
{
	Genplan	  UMETA(DisplayName = "Общий план"),
	Floor	  UMETA(DisplayName = "Этаж"),
	Apartment UMETA(DisplayName = "Квартира"),
};

/**
 * Снимок состояния навигации для стека Undo.
 */
USTRUCT()
struct FViewState
{
	GENERATED_BODY()

	// Текущий режим навигации.
	UPROPERTY()
	EConfiguratorViewMode Mode = EConfiguratorViewMode::Genplan;

	// Индекс этажа (INDEX_NONE, если не выбран).
	UPROPERTY()
	int32 FloorIndex = INDEX_NONE;

	// ID квартиры (INDEX_NONE, если не выбрана).
	UPROPERTY()
	int32 ApartmentId = INDEX_NONE;
};

// Смена режима камеры.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnViewModeChanged,
	EConfiguratorViewMode, NewMode);

// Выбран этаж: индекс + точка фокуса.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnFloorSelected,
	int32, FloorIndex,
	FVector, FocusPoint);

// Выбрана квартира: ID + точка фокуса.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnApartmentSelected,
	int32, ApartmentId,
	FVector, FocusPoint);

// Курсор наведён на карточку квартиры.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnApartmentHovered,
	int32, ApartmentId,
	bool, bHovered);

// Изменение глобального фильтра "скрыть проданные".
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnHideSoldChanged,
	bool, bHideSold);

// Квартира забронирована.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnApartmentReserved,
	int32, ApartmentId);