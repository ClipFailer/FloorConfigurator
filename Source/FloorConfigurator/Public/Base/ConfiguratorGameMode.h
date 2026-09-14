// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ConfiguratorGameMode.generated.h"

class AApartmentActor;

/**
 * GameMode конфигуратора.
 *
 * Спавнит акторы квартир по данным BuildingConfig при старте уровня.
 */
UCLASS()
class FLOORCONFIGURATOR_API AConfiguratorGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// ─────────────────────────────────────────────────────────────────
	//  Жизненный цикл
	// ─────────────────────────────────────────────────────────────────

	AConfiguratorGameMode();

protected:
	// ─────────────────────────────────────────────────────────────────
	//  Жизненный цикл
	// ─────────────────────────────────────────────────────────────────

	virtual void BeginPlay() override;

	// ─────────────────────────────────────────────────────────────────
	//  Спавн
	// ─────────────────────────────────────────────────────────────────

	/** Спавнит акторы квартир по данным BuildingConfig. */
	void SpawnApartmentActors();

	// ─────────────────────────────────────────────────────────────────
	//  Настройки
	// ─────────────────────────────────────────────────────────────────

	// BP-класс актора квартиры.
	UPROPERTY(EditDefaultsOnly, Category = "Configurator")
	TSubclassOf<AApartmentActor> ApartmentActorClass;

private:
	// ─────────────────────────────────────────────────────────────────
	//  Состояние
	// ─────────────────────────────────────────────────────────────────

	// Спавненные акторы квартир.
	UPROPERTY()
	TArray<TObjectPtr<AApartmentActor>> SpawnedApartmentActors;
};