// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#include "Data/ConfigLoader.h"
#include "Data/BuildingConfig.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
	// Проверяет, что обязательные поля квартиры заполнены корректно.
	bool IsApartmentValid(const FApartmentData& Apartment)
	{
		return Apartment.Id > 0 &&
			Apartment.Area > 0.f &&
			!Apartment.Status.IsEmpty();
	}
} // namespace

bool FConfigLoader::LoadConfig(FBuildingConfig& OutConfig)
{
	// Путь к конфигу: Config/config.json.
	const FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("config.json");

	if (!FPaths::FileExists(ConfigPath))
	{
		UE_LOG(LogTemp, Error, TEXT("[FConfigLoader] Конфиг не найден: %s"), *ConfigPath);
		return false;
	}

	// Читаем файл в строку.
	FString ConfigString;
	if (!FFileHelper::LoadFileToString(ConfigString, *ConfigPath))
	{
		UE_LOG(LogTemp, Error, TEXT("[FConfigLoader] Ошибка чтения файла: %s"), *ConfigPath);
		return false;
	}

	// Парсим JSON в структуру.
	FBuildingConfig ParsedConfig;
	if (!FJsonObjectConverter::JsonObjectStringToUStruct(ConfigString, &ParsedConfig))
	{
		UE_LOG(LogTemp, Error, TEXT("[FConfigLoader] Ошибка парсинга JSON."));
		return false;
	}

	if (ParsedConfig.Floors.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[FConfigLoader] В конфиге нет этажей."));
		return false;
	}

	// Валидация: удаляем невалидные квартиры и пустые этажи.
	// Проходим с конца, чтобы безопасно удалять по индексу.
	for (int32 FloorIndex = ParsedConfig.Floors.Num() - 1; FloorIndex >= 0; --FloorIndex)
	{
		FFloorData& Floor = ParsedConfig.Floors[FloorIndex];

		for (int32 ApartmentIndex = Floor.Apartments.Num() - 1; ApartmentIndex >= 0; --ApartmentIndex)
		{
			const FApartmentData& Apartment = Floor.Apartments[ApartmentIndex];

			if (!IsApartmentValid(Apartment))
			{
				UE_LOG(LogTemp, Warning,
					TEXT("[FConfigLoader] Пропускаем невалидную квартиру: ID=%d, Area=%.2f, Status='%s'"),
					Apartment.Id, Apartment.Area, *Apartment.Status);

				Floor.Apartments.RemoveAt(ApartmentIndex);
			}
		}

		if (Floor.Apartments.Num() == 0)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[FConfigLoader] Этаж %d пуст после валидации — удаляем."),
				FloorIndex);

			ParsedConfig.Floors.RemoveAt(FloorIndex);
		}
	}

	if (ParsedConfig.Floors.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[FConfigLoader] Нет валидных этажей после валидации."));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[FConfigLoader] Загружено этажей: %d"), ParsedConfig.Floors.Num());

	OutConfig = MoveTemp(ParsedConfig);
	return true;
}