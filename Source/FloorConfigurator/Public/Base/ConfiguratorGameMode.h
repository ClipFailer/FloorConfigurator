// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ConfiguratorGameMode.generated.h"

/**
 * GameMode конфигуратора.
 */
UCLASS()
class FLOORCONFIGURATOR_API AConfiguratorGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AConfiguratorGameMode();

protected:
	virtual void BeginPlay() override;
};
