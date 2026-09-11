// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#pragma once

#include "CoreMinimal.h"
#include "Data/ApartmentData.h"
#include "FloorData.generated.h"

/**
 * Данные одного этажа здания.
 */
USTRUCT(BlueprintType)
struct FFloorData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<FApartmentData> Apartments;
};