// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#include "Base/ConfiguratorGameMode.h"
#include "Camera/ConfiguratorCamera.h"
#include "Interaction/ApartmentActor.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/FloorConfiguratorSubsystem.h"

// ─────────────────────────────────────────────────────────────────────
//  Жизненный цикл
// ─────────────────────────────────────────────────────────────────────

AConfiguratorGameMode::AConfiguratorGameMode()
{
	DefaultPawnClass = nullptr;
}

void AConfiguratorGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Ставим камеру как ViewTarget для первого игрока.
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		AConfiguratorCamera* Camera = Cast<AConfiguratorCamera>(
			UGameplayStatics::GetActorOfClass(GetWorld(), AConfiguratorCamera::StaticClass()));

		if (Camera)
		{
			PlayerController->SetViewTarget(Camera);
		}
	}

	SpawnApartmentActors();
}

// ─────────────────────────────────────────────────────────────────────
//  Спавн
// ─────────────────────────────────────────────────────────────────────

void AConfiguratorGameMode::SpawnApartmentActors()
{
	// Получаем подсистему конфигуратора.
	UFloorConfiguratorSubsystem* ConfiguratorSubsystem =
		GetGameInstance()->GetSubsystem<UFloorConfiguratorSubsystem>();
	if (!IsValid(ConfiguratorSubsystem))
	{
		UE_LOG(LogTemp, Error, TEXT("[GameMode] ConfiguratorSubsystem не найден."));
		return;
	}

	if (!IsValid(ApartmentActorClass))
	{
		UE_LOG(LogTemp, Error, TEXT("[GameMode] ApartmentActorClass не задан."));
		return;
	}

	// Параметры спавна.
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const float ApartmentSize = ConfiguratorSubsystem->GetApartmentSize();

	// Проходим по всем этажам и квартирам из конфига.
	const TArray<FFloorData>& Floors = ConfiguratorSubsystem->GetBuildingConfig().Floors;
	for (int32 FloorIndex = 0; FloorIndex < Floors.Num(); ++FloorIndex)
	{
		const float FloorHeight = ConfiguratorSubsystem->GetFloorHeight(FloorIndex);

		for (const FApartmentData& Apartment : Floors[FloorIndex].Apartments)
		{
			const FTransform SpawnTransform(Apartment.FocusPoint);

			AApartmentActor* ApartmentActor =
				GetWorld()->SpawnActor<AApartmentActor>(ApartmentActorClass, SpawnTransform, Params);

			if (ApartmentActor)
			{
				ApartmentActor->Init(Apartment, ApartmentSize, FloorHeight);
				SpawnedApartmentActors.Add(ApartmentActor);
			}
		}
	}
}