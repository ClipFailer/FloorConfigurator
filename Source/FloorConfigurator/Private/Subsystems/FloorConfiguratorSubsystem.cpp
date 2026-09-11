// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#include "Subsystems/FloorConfiguratorSubsystem.h"
#include "Data/ConfigLoader.h"

void UFloorConfiguratorSubsystem::RequestGenplan()
{
	// Если уже в Genplan — ничего не делаем.
	if (ViewState.Mode == EConfiguratorViewMode::Genplan)
	{
		return;
	}

	// Сохраняем текущее состояние в StateHistory.
	StateHistory.Add(ViewState);					

	// Обновляем состояние.
	ViewState.Mode = EConfiguratorViewMode::Genplan;
	ViewState.FloorIndex = INDEX_NONE;
	ViewState.ApartmentId = INDEX_NONE;

	// Сообщаем подписчикам о изменении мода.
	OnViewModeChanged.Broadcast(ViewState.Mode);

}

void UFloorConfiguratorSubsystem::RequestFloor(int32 FloorIndex)
{
	// Получаем и проверяем индекс этажа.
	if (!BuildingConfig.Floors.IsValidIndex(FloorIndex))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[FloorConfiguratorSubsystem] Передан невалидный индекс этажа при смене мода")
		);

		return;
	}

	// Если этаж пустой - ничего не делаем.
	const FFloorData& Floor = BuildingConfig.Floors[FloorIndex];
	if (Floor.Apartments.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FloorConfiguratorSubsystem] %d этаж пустой"), FloorIndex);
		return;
	}

	// Если уже выбран этаж — ничего не делаем.
	if (ViewState.Mode == EConfiguratorViewMode::Floor &&
		ViewState.FloorIndex == FloorIndex)
	{
		return;
	}

	// Сохраняем текущее состояние в StateHistory.
	StateHistory.Add(ViewState);

	// Обновляем состояние.
	ViewState.Mode = EConfiguratorViewMode::Floor;
	ViewState.FloorIndex = FloorIndex;
	ViewState.ApartmentId = INDEX_NONE;

	FVector FloorFocusPoint = GetFloorFocusPoint(FloorIndex);

	// Сообщаем подписчикам о изменениях.
	OnViewModeChanged.Broadcast(ViewState.Mode);
	OnFloorSelected.Broadcast(FloorIndex, FloorFocusPoint);
}

void UFloorConfiguratorSubsystem::RequestApartment(int32 ApartmentId)
{
	// Если квартира уже выбрана - ничего не делаем.
	if (ViewState.Mode == EConfiguratorViewMode::Apartment &&
		ViewState.ApartmentId == ApartmentId)
	{
		return;
	}

	// Получаем индекс этажа с нужной квартирой.
	int32 FoundFloorIndex = INDEX_NONE;
	const FApartmentData* FoundApartment = FindApartmentById(ApartmentId, FoundFloorIndex);

	// Если этаж или квартира не найдены - ничего не делаем.
	if (!FoundApartment)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FloorConfiguratorSubsystem] квартира %d не найдена."), ApartmentId);
		return;
	}

	// Сохраняем текущее состояние в StateHistory.
	StateHistory.Add(ViewState);

	// Обновляем состояние.
	ViewState.Mode = EConfiguratorViewMode::Apartment;
	ViewState.FloorIndex = FoundFloorIndex;
	ViewState.ApartmentId = ApartmentId;

	// Сообщаем подписчикам о изменениях.
	OnViewModeChanged.Broadcast(ViewState.Mode);
	OnApartmentSelected.Broadcast(ApartmentId, FoundApartment->FocusPoint);
}

void UFloorConfiguratorSubsystem::GoBack()
{
	// Если нет в истории состояний - ничего не делаем
	if (StateHistory.Num() == 0)
	{
		return;
	}

	// Снимаем последнее состояние с вершины стека.
	const FViewState PreviousState = StateHistory.Pop();

	// Применяем его — по режиму бросаем нужные делегаты.
	ViewState = PreviousState;
	OnViewModeChanged.Broadcast(ViewState.Mode);

	// Возвращаем состояние.
	switch (ViewState.Mode)
	{
	case EConfiguratorViewMode::Genplan:
	{
		// Ничего не делаем.
		break;
	}

	case EConfiguratorViewMode::Floor:
	{
		// Оповещаем об изменениях.
		OnFloorSelected.Broadcast(ViewState.FloorIndex, GetFloorFocusPoint(ViewState.FloorIndex));
		break;
	}

	case EConfiguratorViewMode::Apartment:
	{
		int32 FloorIndex = INDEX_NONE;
		const FApartmentData* Apartment = FindApartmentById(ViewState.ApartmentId, FloorIndex);
		if (Apartment)
		{
			OnApartmentSelected.Broadcast(ViewState.ApartmentId, Apartment->FocusPoint);
		}
		break;
	}
	}
}

void UFloorConfiguratorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Загружаем и проверяем конфиг.
	if (!FConfigLoader::LoadConfig(BuildingConfig))
	{
		UE_LOG(LogTemp, Error, TEXT("[UFloorConfiguratorSubsystem] Ошибка загрузки конфига"));
	}
}

const FApartmentData* UFloorConfiguratorSubsystem::FindApartmentById(
	int32 ApartmentId, int32& OutFloorIndex) const
{
	for (int32 FloorIndex = 0; FloorIndex < BuildingConfig.Floors.Num(); ++FloorIndex)
	{
		// Получаем этаж.
		const FFloorData& Floor = BuildingConfig.Floors[FloorIndex];

		// Ищем нужную квартиру.
		const FApartmentData* FoundApartment = Floor.Apartments.FindByPredicate(
			[ApartmentId](const FApartmentData& Apartment)
			{
				return Apartment.Id == ApartmentId;
			}
		);

		if (FoundApartment)
		{
			OutFloorIndex = FloorIndex;
			return FoundApartment;
		}
	}

	OutFloorIndex = INDEX_NONE;
	return nullptr;
}

FVector UFloorConfiguratorSubsystem::GetFloorFocusPoint(int32 FloorIndex) const
{
	// Если индекс этажа невалиден - ничего не делаем.
	if (!BuildingConfig.Floors.IsValidIndex(FloorIndex))
	{
		return FVector::ZeroVector;
	}

	const FFloorData& Floor = BuildingConfig.Floors[FloorIndex];
	if (Floor.Apartments.Num() == 0)
	{
		return FVector::ZeroVector;
	}

	// Сумируем все FocusPoint квартир этажа.
	FVector FocusPoint = FVector::ZeroVector;
	for (const FApartmentData& Apartment : Floor.Apartments)
	{
		FocusPoint += Apartment.FocusPoint;
	}
	// Получаем среднее арифметическое FocusPoint квартир - центр этажа.
	FocusPoint /= static_cast<float>(Floor.Apartments.Num());

	return FocusPoint;
}
