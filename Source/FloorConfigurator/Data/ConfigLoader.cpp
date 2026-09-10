#include "ConfigLoader.h"
#include "BuildingConfig.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "JsonObjectConverter.h"

// анонимный namespace
namespace
{
	// Проверяет, что все обязательные поля квартиры заполнены корректно.
	bool IsApartmentValid(const FApartmentData& Apartment)
	{
		return Apartment.Id > 0
			&& Apartment.Area > 0.f
			&& !Apartment.Status.IsEmpty()
			&& !Apartment.FocusPoint.IsNearlyZero();
	}
} // namespace

bool FConfigLoader::LoadConfig(FBuildingConfig& OutConfig)
{
	// Получаем путь к конфигу config.json.
	const FString ConfigPath = FPaths::ProjectContentDir() / TEXT("Data/config.json");

	UE_LOG(LogTemp, Log, TEXT("[FConfigLoader] Поиск файла конфига: %s"), *ConfigPath);

	if (!FPaths::FileExists(ConfigPath))
	{
		UE_LOG(LogTemp, Error, TEXT("[FConfigLoader] Конфиг не найден: %s"), *ConfigPath);
		return false;
	}

	// Читаем содержимое в строку.
	FString ConfigString;
	if (!FFileHelper::LoadFileToString(ConfigString, *ConfigPath))
	{
		UE_LOG(LogTemp, Error, TEXT("[FConfigLoader] Ошибка чтения файла: %s"), *ConfigPath);
		return false;
	}

	// Парсим строку в структуру FBuildingConfig
	FBuildingConfig ParsedConfig;
	if (!FJsonObjectConverter::JsonObjectStringToUStruct(ConfigString, &ParsedConfig))
	{
		UE_LOG(LogTemp, Error, TEXT("[FConfigLoader] Ошибка парсинга JSON"));
		return false;
	}

	if (ParsedConfig.Floors.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[FConfigLoader] В конфиге нет этажей"));
		return false;
	}

	// Проверяем валидность конфига.
	// Проходим по всем этажам и проверяем каждую квартиру
	// и если она невалидна - удаляем из массива квартир.
	// Если этаж пустой - тоже удаляем.
	for (int32 FloorIndex = ParsedConfig.Floors.Num() - 1; FloorIndex >= 0; --FloorIndex)
	{
		FFloorData& Floor = ParsedConfig.Floors[FloorIndex];

		for (int32 ApartmentIndex = Floor.Apartments.Num() - 1; ApartmentIndex >= 0; --ApartmentIndex)
		{
			const FApartmentData& Apartment = Floor.Apartments[ApartmentIndex];

			// Проверка квартиры на валидность.
			const bool bValid = IsApartmentValid(Apartment);

			// Удаляем квартиру из массива, если она невалидна.
			if (!bValid)
			{
				UE_LOG(
					LogTemp,
					Warning,
					TEXT("[FConfigLoader] Пропускаем невалидную квартиру: ID=%d, Area=%.2f, Status='%s'"),
					Apartment.Id,
					Apartment.Area,
					*Apartment.Status);

				Floor.Apartments.RemoveAt(ApartmentIndex);
			}
		}

		if (Floor.Apartments.Num() == 0)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[FConfigLoader] Этаж %d хранит невалидные квартиры: удаление этажа"),
				FloorIndex);

			ParsedConfig.Floors.RemoveAt(FloorIndex);
		}
	}

	if (ParsedConfig.Floors.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[FConfigLoader] Нет валидных этажей после валидации"));
		return false;
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[FConfigLoader] %d этажей загружено из конфига"),
		ParsedConfig.Floors.Num());

	// Перемещаем готовый конфиг.
	OutConfig = MoveTemp(ParsedConfig);

	return true;
}
