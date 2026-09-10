// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#pragma once

#include "CoreMinimal.h"
#include "ApartmentData.generated.h"

/**
 * Хранит данные одной квартиры: ID, статус, площадь и точку фокуса камеры.
 * Заполняется из JSON-конфига через UConfigLoader.
 */
USTRUCT(BlueprintType)
struct FApartmentData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 Id = 0;

	UPROPERTY(BlueprintReadOnly)
	FString Status;

	UPROPERTY(BlueprintReadOnly)
	float Area = 0.f;

	UPROPERTY(BlueprintReadOnly)
	FVector FocusPoint = FVector::ZeroVector;

	bool IsSold() const { return Status == TEXT("Продано"); }
};