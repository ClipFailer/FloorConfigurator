// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#pragma once

#include "ApartmentData.h"
#include "CoreMinimal.h"
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