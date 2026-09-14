// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#pragma once

#include "CoreMinimal.h"
#include "ApartmentData.generated.h"

/**
 * Данные одной квартиры: ID, статус, площадь и точка фокуса камеры.
 * Заполняется из JSON-конфига через FConfigLoader.
 */
USTRUCT(BlueprintType)
struct FApartmentData
{
	GENERATED_BODY()

	// Уникальный ID квартиры.
	UPROPERTY(BlueprintReadOnly)
	int32 Id = 0;

	// Статус: "Продано" или "Свободно".
	UPROPERTY(BlueprintReadOnly)
	FString Status;

	// Площадь квартиры в квадратных метрах.
	UPROPERTY(BlueprintReadOnly)
	float Area = 0.f;

	// Точка фокуса камеры при выборе квартиры.
	UPROPERTY(BlueprintReadOnly)
	FVector FocusPoint = FVector::ZeroVector;

	// Возвращает true, если квартира продана.
	bool IsSold() const { return Status == TEXT("Продано"); }
};