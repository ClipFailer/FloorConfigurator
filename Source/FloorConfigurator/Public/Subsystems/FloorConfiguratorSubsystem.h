// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/BuildingConfig.h"
#include "FloorConfiguratorSubsystem.generated.h"

/**
 *	Сабсистема конфигуратора здания.
 *  При инициализации загружает конфиг здания.
 */
UCLASS()
class FLOORCONFIGURATOR_API UFloorConfiguratorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Configurator|Config")
	const FBuildingConfig& GetBuildingConfig() const { return BuildingConfig; };

protected:
	/**
	 *  Инициализатор сабсистемы.
	 *
	 *	Загружает конфиг в BuildingConfig
	 *
	 * @param Collection
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	FBuildingConfig BuildingConfig;
};
