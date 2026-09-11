// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#include "Subsystems/FloorConfiguratorSubsystem.h"
#include "Data/ConfigLoader.h"

void UFloorConfiguratorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Загружаем и проверяем конфиг.
	if (!FConfigLoader::LoadConfig(BuildingConfig))
	{
		UE_LOG(LogTemp, Error, TEXT("[UFloorConfiguratorSubsystem] Ошибка загрузки конфига"));
	}
}