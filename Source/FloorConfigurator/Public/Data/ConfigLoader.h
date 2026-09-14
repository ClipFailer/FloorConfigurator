// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#pragma once

#include "CoreMinimal.h"

struct FBuildingConfig;

/**
 * Утилита для загрузки и парсинга конфигурации здания из JSON.
 *
 * Не хранит состояние: единственный метод читает файл и заполняет
 * переданную структуру. Используется в UFloorConfiguratorSubsystem
 * при инициализации.
 */
class FConfigLoader
{
public:
	/**
	 * Читает Config/config.json и парсит его в FBuildingConfig.
	 *
	 * При отсутствии файла, ошибке чтения или парсинга возвращает false
	 * и пишет причину в лог. Структура OutConfig в этом случае не изменяется.
	 *
	 * В готовом билде файл копируется в папку Config/ рядом с .exe.
	 *
	 * @param OutConfig   Заполненная структура конфигурации при успехе.
	 * @return            true, если данные успешно загружены и распарсены.
	 */
	static bool LoadConfig(FBuildingConfig& OutConfig);
};