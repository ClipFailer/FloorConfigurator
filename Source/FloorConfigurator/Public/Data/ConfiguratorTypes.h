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

	UPROPERTY()
	EConfiguratorViewMode Mode = EConfiguratorViewMode::Genplan;

	UPROPERTY()
	int32 FloorIndex = INDEX_NONE;

	UPROPERTY()
	int32 ApartmentId = INDEX_NONE;
};

/** Смена режима камеры. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnViewModeChanged,
	EConfiguratorViewMode, NewMode);

/** Выбран этаж: индекс + точка фокуса. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnFloorSelected,
	int32, FloorIndex,
	FVector, FocusPoint);

/** Выбрана квартира: ID + точка фокуса. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnApartmentSelected,
	int32, ApartmentId,
	FVector, FocusPoint);