// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ApartmentInterface.generated.h"

// Этот класс не требует изменений.
UINTERFACE(MinimalAPI)
class UApartmentInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Интерфейс для акторов, к которым можно навигировать камеру
 * как к квартире.
 */
class FLOORCONFIGURATOR_API IApartmentInterface
{
	GENERATED_BODY()

public:
	/**
	 * Возвращает ID квартиры для навигации.
	 *
	 * @return   ID квартиры или INDEX_NONE, если актор не навигируется.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Apartment")
	int32 GetApartmentId() const;
};