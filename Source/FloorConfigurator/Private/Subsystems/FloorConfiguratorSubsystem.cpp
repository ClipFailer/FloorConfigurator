// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#include "Subsystems/FloorConfiguratorSubsystem.h"
#include "Data/ConfigLoader.h"

// ─────────────────────────────────────────────────────────────────────
//  Жизненный цикл
// ─────────────────────────────────────────────────────────────────────

void UFloorConfiguratorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Загружаем конфиг.
	if (!FConfigLoader::LoadConfig(BuildingConfig))
	{
		UE_LOG(LogTemp, Error, TEXT("[Subsystem] Ошибка загрузки конфига."));
		return;
	}

	// Кешируем вычисляемые значения.
	CalculateGenplanFocusPoint();
	CalculateFloorFocusPoints();
	CalculateApartmentSize();
}

// ─────────────────────────────────────────────────────────────────────
//  Команды навигации
// ─────────────────────────────────────────────────────────────────────

void UFloorConfiguratorSubsystem::RequestGenplan()
{
	// Если уже в Genplan — ничего не делаем.
	if (ViewState.Mode == EConfiguratorViewMode::Genplan)
	{
		return;
	}

	StateHistory.Add(ViewState);

	ViewState.Mode = EConfiguratorViewMode::Genplan;
	ViewState.FloorIndex = INDEX_NONE;
	ViewState.ApartmentId = INDEX_NONE;

	OnViewModeChanged.Broadcast(ViewState.Mode);
}

void UFloorConfiguratorSubsystem::RequestFloor(int32 FloorIndex)
{
	if (!BuildingConfig.Floors.IsValidIndex(FloorIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Subsystem] Невалидный индекс этажа: %d"), FloorIndex);
		return;
	}

	const FFloorData& Floor = BuildingConfig.Floors[FloorIndex];
	if (Floor.Apartments.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Subsystem] Этаж %d пуст."), FloorIndex);
		return;
	}

	if (ViewState.Mode == EConfiguratorViewMode::Floor &&
		ViewState.FloorIndex == FloorIndex)
	{
		return;
	}

	StateHistory.Add(ViewState);

	ViewState.Mode = EConfiguratorViewMode::Floor;
	ViewState.FloorIndex = FloorIndex;
	ViewState.ApartmentId = INDEX_NONE;

	const FVector FloorFocusPoint = GetFloorFocusPoint(FloorIndex);

	OnViewModeChanged.Broadcast(ViewState.Mode);
	OnFloorSelected.Broadcast(FloorIndex, FloorFocusPoint);
}

void UFloorConfiguratorSubsystem::RequestApartment(int32 ApartmentId)
{
	if (ViewState.Mode == EConfiguratorViewMode::Apartment &&
		ViewState.ApartmentId == ApartmentId)
	{
		return;
	}

	int32				  FloorIndex = INDEX_NONE;
	const FApartmentData* FoundApartment = FindApartmentById(ApartmentId, FloorIndex);

	if (!FoundApartment)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Subsystem] Квартира %d не найдена."), ApartmentId);
		return;
	}

	StateHistory.Add(ViewState);

	ViewState.Mode = EConfiguratorViewMode::Apartment;
	ViewState.FloorIndex = FloorIndex;
	ViewState.ApartmentId = ApartmentId;

	OnViewModeChanged.Broadcast(ViewState.Mode);
	OnApartmentSelected.Broadcast(ApartmentId, FoundApartment->FocusPoint);
}

void UFloorConfiguratorSubsystem::GoBack()
{
	if (StateHistory.Num() == 0)
	{
		return;
	}

	ViewState = StateHistory.Pop();
	OnViewModeChanged.Broadcast(ViewState.Mode);

	switch (ViewState.Mode)
	{
		case EConfiguratorViewMode::Genplan:
		{
			break;
		}

		case EConfiguratorViewMode::Floor:
		{
			OnFloorSelected.Broadcast(
				ViewState.FloorIndex, GetFloorFocusPoint(ViewState.FloorIndex));
			break;
		}

		case EConfiguratorViewMode::Apartment:
		{
			int32				  FloorIndex = INDEX_NONE;
			const FApartmentData* Apartment = FindApartmentById(ViewState.ApartmentId, FloorIndex);
			if (Apartment)
			{
				OnApartmentSelected.Broadcast(ViewState.ApartmentId, Apartment->FocusPoint);
			}
			break;
		}
	}
}

// ─────────────────────────────────────────────────────────────────────
//  Геттеры
// ─────────────────────────────────────────────────────────────────────

FVector UFloorConfiguratorSubsystem::GetFloorFocusPoint(int32 FloorIndex) const
{
	if (!CachedFloorFocusPoints.IsValidIndex(FloorIndex))
	{
		return FVector::ZeroVector;
	}

	return CachedFloorFocusPoints[FloorIndex];
}

float UFloorConfiguratorSubsystem::GetFloorHeight(int32 FloorIndex) const
{
	if (!BuildingConfig.Floors.IsValidIndex(FloorIndex))
	{
		return DefaultFloorHeight;
	}

	if (BuildingConfig.Floors.IsValidIndex(FloorIndex + 1))
	{
		const float CurrentZ = GetFloorLevelZ(FloorIndex);
		const float NextZ = GetFloorLevelZ(FloorIndex + 1);
		const float Delta = NextZ - CurrentZ;

		if (Delta > 0)
		{
			return Delta;
		}
	}

	if (BuildingConfig.Floors.IsValidIndex(FloorIndex - 1))
	{
		return GetFloorHeight(FloorIndex - 1);
	}

	return DefaultFloorHeight;
}

float UFloorConfiguratorSubsystem::GetFloorLevelZ(int32 FloorIndex) const
{
	if (!BuildingConfig.Floors.IsValidIndex(FloorIndex))
	{
		return 0.f;
	}

	const FFloorData& Floor = BuildingConfig.Floors[FloorIndex];
	if (Floor.Apartments.Num() == 0)
	{
		return 0.f;
	}

	return Floor.Apartments[0].FocusPoint.Z;
}

const FApartmentData* UFloorConfiguratorSubsystem::FindApartmentById(
	int32 ApartmentId, int32& OutFloorIndex) const
{
	for (int32 FloorIndex = 0; FloorIndex < BuildingConfig.Floors.Num(); ++FloorIndex)
	{
		const FFloorData& Floor = BuildingConfig.Floors[FloorIndex];

		const FApartmentData* FoundApartment = Floor.Apartments.FindByPredicate(
			[ApartmentId](const FApartmentData& Apartment) {
				return Apartment.Id == ApartmentId;
			});

		if (FoundApartment)
		{
			OutFloorIndex = FloorIndex;
			return FoundApartment;
		}
	}

	OutFloorIndex = INDEX_NONE;
	return nullptr;
}

bool UFloorConfiguratorSubsystem::IsApartmentReserved(int32 ApartmentId) const
{
	return ReservedApartmentIds.Contains(ApartmentId);
}

bool UFloorConfiguratorSubsystem::IsApartmentSold(int32 ApartmentId) const
{
	if (ReservedApartmentIds.Contains(ApartmentId))
	{
		return true;
	}

	int32				  FloorIndex = INDEX_NONE;
	const FApartmentData* Apartment = FindApartmentById(ApartmentId, FloorIndex);
	return Apartment && Apartment->IsSold();
}

// ─────────────────────────────────────────────────────────────────────
//  Бронирование
// ─────────────────────────────────────────────────────────────────────

void UFloorConfiguratorSubsystem::ReserveApartment(int32 ApartmentId)
{
	int32				  FloorIndex = INDEX_NONE;
	const FApartmentData* Apartment = FindApartmentById(ApartmentId, FloorIndex);

	if (!Apartment)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Subsystem] ReserveApartment: квартира %d не найдена."), ApartmentId);
		return;
	}

	if (IsApartmentSold(ApartmentId))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Subsystem] Квартира %d уже забронирована."), ApartmentId);
		return;
	}

	ReservedApartmentIds.Add(ApartmentId);

	UE_LOG(LogTemp, Warning,
		TEXT("[Subsystem] Квартира %d (этаж %d) забронирована."), ApartmentId, FloorIndex);

	OnApartmentReserved.Broadcast(ApartmentId);
}

// ─────────────────────────────────────────────────────────────────────
//  Вычисление кешей
// ─────────────────────────────────────────────────────────────────────

void UFloorConfiguratorSubsystem::CalculateGenplanFocusPoint()
{
	FVector FocusPointSum = FVector::ZeroVector;
	int32	ApartmentsCount = 0;

	for (const FFloorData& Floor : BuildingConfig.Floors)
	{
		for (const FApartmentData& Apartment : Floor.Apartments)
		{
			FocusPointSum += Apartment.FocusPoint;
			++ApartmentsCount;
		}
	}

	CachedGeneralFocusPoint = (ApartmentsCount > 0)
		? FocusPointSum / static_cast<float>(ApartmentsCount)
		: FVector::ZeroVector;
}

void UFloorConfiguratorSubsystem::CalculateFloorFocusPoints()
{
	CachedFloorFocusPoints.Empty(BuildingConfig.Floors.Num());

	for (const FFloorData& Floor : BuildingConfig.Floors)
	{
		if (Floor.Apartments.Num() == 0)
		{
			CachedFloorFocusPoints.Add(FVector::ZeroVector);
			continue;
		}

		FVector FocusPoint = FVector::ZeroVector;
		for (const FApartmentData& Apartment : Floor.Apartments)
		{
			FocusPoint += Apartment.FocusPoint;
		}
		FocusPoint /= static_cast<float>(Floor.Apartments.Num());

		CachedFloorFocusPoints.Add(FocusPoint);
	}
}

void UFloorConfiguratorSubsystem::CalculateApartmentSize()
{
	float MinDistanceSq = FLT_MAX;

	for (const FFloorData& Floor : BuildingConfig.Floors)
	{
		const int32 Num = Floor.Apartments.Num();

		if (Num < 2)
		{
			continue;
		}

		for (int32 i = 0; i < Num; ++i)
		{
			for (int32 j = i + 1; j < Num; ++j)
			{
				const float DistSq = FVector::DistSquared2D(
					Floor.Apartments[i].FocusPoint,
					Floor.Apartments[j].FocusPoint);

				if (DistSq > KINDA_SMALL_NUMBER && DistSq < MinDistanceSq)
				{
					MinDistanceSq = DistSq;
				}
			}
		}
	}

	if (MinDistanceSq == FLT_MAX)
	{
		return;
	}

	CachedApartmentSize = FMath::Sqrt(MinDistanceSq);
}