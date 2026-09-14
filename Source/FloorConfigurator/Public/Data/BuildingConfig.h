// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#pragma once

#include "CoreMinimal.h"
#include "Data/FloorData.h"
#include "BuildingConfig.generated.h"

/**
 * Корневая структура конфигурации здания.
 * Содержит список этажей, загружаемый из JSON через FConfigLoader.
 * Используется как хранилище данных в UFloorConfiguratorSubsystem.
 */
USTRUCT(BlueprintType)
struct FBuildingConfig
{
	GENERATED_BODY()

	// Список этажей здания.
	UPROPERTY(BlueprintReadOnly)
	TArray<FFloorData> Floors;
};