// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#include "Camera/ConfiguratorCamera.h"
#include "Camera/CameraComponent.h"
#include "Subsystems/FloorConfiguratorSubsystem.h"

// ─────────────────────────────────────────────────────────────────────
//  Жизненный цикл
// ─────────────────────────────────────────────────────────────────────

AConfiguratorCamera::AConfiguratorCamera()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(Root);
}

void AConfiguratorCamera::BeginPlay()
{
	Super::BeginPlay();

	// Получаем подсистему конфигуратора.
	ConfiguratorSubsystem = GetGameInstance()->GetSubsystem<UFloorConfiguratorSubsystem>();

	if (IsValid(ConfiguratorSubsystem))
	{
		ConfiguratorSubsystem->OnViewModeChanged.AddDynamic(this, &ThisClass::HandleViewModeChanged);
		ConfiguratorSubsystem->OnFloorSelected.AddDynamic(this, &ThisClass::HandleFloorSelected);
		ConfiguratorSubsystem->OnApartmentSelected.AddDynamic(this, &ThisClass::HandleApartmentSelected);

		UpdateTargetForGenplan();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[ConfiguratorCamera] ConfiguratorSubsystem не найден."));
	}
}

void AConfiguratorCamera::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Отписываемся от делегатов.
	if (IsValid(ConfiguratorSubsystem))
	{
		ConfiguratorSubsystem->OnViewModeChanged.RemoveDynamic(this, &ThisClass::HandleViewModeChanged);
		ConfiguratorSubsystem->OnFloorSelected.RemoveDynamic(this, &ThisClass::HandleFloorSelected);
		ConfiguratorSubsystem->OnApartmentSelected.RemoveDynamic(this, &ThisClass::HandleApartmentSelected);
	}

	// Останавливаем таймер интерполяции, если активен.
	GetWorld()->GetTimerManager().ClearTimer(InterpTimer);

	Super::EndPlay(EndPlayReason);
}

// ─────────────────────────────────────────────────────────────────────
//  Орбита
// ─────────────────────────────────────────────────────────────────────

void AConfiguratorCamera::StartOrbit()
{
	// Орбита доступна только в режиме Genplan.
	if (CurrentMode != EConfiguratorViewMode::Genplan)
	{
		return;
	}

	if (!IsValid(ConfiguratorSubsystem))
	{
		UE_LOG(LogTemp, Error, TEXT("[ConfiguratorCamera] ConfiguratorSubsystem не найден."));
		return;
	}

	// Прерываем текущий перелёт, если он идёт.
	if (bIsTransitioning)
	{
		StopInterp();
	}

	bIsOrbiting = true;

	// Центр вращения — фокус здания.
	const FVector FocusPoint = ConfiguratorSubsystem->GetGenplanFocusPoint();

	// Позиция камеры относительно центра.
	const FVector ToCamera = GetActorLocation() - FocusPoint;

	// Радиус орбиты = расстояние от камеры до центра.
	OrbitDistance = ToCamera.Size();

	// Начальные углы.
	const FRotator DirRotation = ToCamera.Rotation();
	OrbitYaw = DirRotation.Yaw;
	OrbitPitch = DirRotation.Pitch;
}

void AConfiguratorCamera::StopOrbit()
{
	bIsOrbiting = false;
}

void AConfiguratorCamera::AddOrbitInput(float DeltaYaw, float DeltaPitch)
{
	if (!bIsOrbiting)
	{
		return;
	}
	if (!IsValid(ConfiguratorSubsystem))
	{
		return;
	}

	// Смещаем углы. Pitch ограничен, чтобы камера не перевернулась.
	OrbitYaw += DeltaYaw * OrbitSensitivity;
	OrbitPitch = FMath::Clamp(
		OrbitPitch + DeltaPitch * OrbitSensitivity,
		OrbitMinPitch,
		OrbitMaxPitch);

	const FVector FocusPoint = ConfiguratorSubsystem->GetGenplanFocusPoint();

	// Новая позиция камеры на сфере того же радиуса.
	const FRotator OrbitRotation(OrbitPitch, OrbitYaw, 0.f);
	const FVector  RotationDirection = OrbitRotation.Vector();
	const FVector  NewLocation = FocusPoint + RotationDirection * OrbitDistance;
	SetActorLocation(NewLocation);

	// Поворачиваем камеру на центр.
	const FRotator FocusRotation = (FocusPoint - NewLocation).Rotation();
	SetActorRotation(FocusRotation);
}

// ─────────────────────────────────────────────────────────────────────
//  Обработчики делегатов Subsystem
// ─────────────────────────────────────────────────────────────────────

void AConfiguratorCamera::HandleViewModeChanged(EConfiguratorViewMode NewMode)
{
	CurrentMode = NewMode;

	// Сбрасываем орбиту.
	bIsOrbiting = false;

	// Прерываем текущий перелёт, чтобы начать новый.
	if (bIsTransitioning)
	{
		StopInterp();
	}

	if (NewMode == EConfiguratorViewMode::Genplan)
	{
		UpdateTargetForGenplan();
	}
}

void AConfiguratorCamera::HandleFloorSelected(int32 FloorIndex, FVector FocusPoint)
{
	UpdateTargetForFloor(FloorIndex, FocusPoint);
}

void AConfiguratorCamera::HandleApartmentSelected(int32 ApartmentId, FVector FocusPoint)
{
	UpdateTargetForApartment(ApartmentId, FocusPoint);
}

// ─────────────────────────────────────────────────────────────────────
//  Установка целей перелёта
// ─────────────────────────────────────────────────────────────────────

void AConfiguratorCamera::UpdateTargetForGenplan()
{
	if (!IsValid(ConfiguratorSubsystem))
	{
		return;
	}

	const FVector GenplanFocusPoint = ConfiguratorSubsystem->GetGenplanFocusPoint();
	SetTargetFromFocus(GenplanFocusPoint, GenplanOffset);
}

void AConfiguratorCamera::UpdateTargetForFloor(int32 FloorIndex, const FVector& FocusPoint)
{
	SetTargetFromFocus(FocusPoint, FloorOffset);
}

void AConfiguratorCamera::UpdateTargetForApartment(int32 ApartmentId, const FVector& FocusPoint)
{
	SetTargetFromApartment(FocusPoint);
}

void AConfiguratorCamera::SetTargetFromFocus(const FVector& FocusPoint, const FVector& Offset)
{
	TargetLocation = FocusPoint + Offset;
	TargetRotation = (FocusPoint - TargetLocation).Rotation();

	StartInterp();
}

void AConfiguratorCamera::SetTargetFromApartment(const FVector& ApartmentFocus)
{
	if (!IsValid(ConfiguratorSubsystem))
	{
		return;
	}

	// Смотрим в центр квартиры, а не в её основание.
	const FVector ApartmentCenter = ApartmentFocus + FVector(0.f, 0.f, ApartmentViewHeight);

	const FVector BuildingCenter = ConfiguratorSubsystem->GetGenplanFocusPoint();

	// Направление отхода — от центра здания к квартире, в плоскости XY.
	FVector Direction = ApartmentCenter - BuildingCenter;
	Direction.Z = 0.f;
	if (Direction.IsNearlyZero())
	{
		Direction = FVector(-1.f, 0.f, 0.f);
	}
	Direction.Normalize();

	// Дистанция обзора пропорциональна размеру квартиры —
	// камера автоматически подстраивается под масштаб данных из JSON.
	const float ApartmentSize = ConfiguratorSubsystem->GetApartmentSize();
	const float ViewDistance = ApartmentSize * ApartmentViewDistanceMultiplier;

	const FVector Offset = Direction * ViewDistance;

	TargetLocation = ApartmentCenter + Offset;
	TargetRotation = (ApartmentCenter - TargetLocation).Rotation();

	StartInterp();
}

// ─────────────────────────────────────────────────────────────────────
//  Интерполяция
// ─────────────────────────────────────────────────────────────────────

void AConfiguratorCamera::StartInterp()
{
	// Если таймер уже активен — ничего не делаем.
	if (bIsTransitioning)
	{
		return;
	}

	bIsTransitioning = true;

	// Запускаем таймер: InterpStep каждые 1.f / InterpFrequency секунд.
	GetWorld()->GetTimerManager().SetTimer(
		InterpTimer,
		this,
		&ThisClass::InterpStep,
		1.f / InterpFrequency,
		true);
}

void AConfiguratorCamera::StopInterp()
{
	bIsTransitioning = false;
	GetWorld()->GetTimerManager().ClearTimer(InterpTimer);
}

void AConfiguratorCamera::InterpStep()
{
	const float DeltaTime = 1.f / InterpFrequency;

	const float Distance = GetDistanceToTarget();
	const float RotationDelta = GetRotationDeltaToTarget();

	// Дошли до цели — доводим точно и останавливаемся.
	if (Distance <= ArrivalDistanceThreshold && RotationDelta <= ArrivalRotationThreshold)
	{
		SetActorLocation(TargetLocation);
		SetActorRotation(TargetRotation);
		StopInterp();
		return;
	}

	if (Distance > ArrivalDistanceThreshold)
	{
		const FVector CurrentLocation = GetActorLocation();
		const FVector NewLocation = FMath::VInterpTo(
			CurrentLocation, TargetLocation, DeltaTime, InterpSpeed);
		SetActorLocation(NewLocation);
	}

	if (RotationDelta > ArrivalRotationThreshold)
	{
		const FRotator CurrentRotation = GetActorRotation();
		const FRotator NewRotation = FMath::RInterpTo(
			CurrentRotation, TargetRotation, DeltaTime, InterpSpeed);
		SetActorRotation(NewRotation);
	}
}

float AConfiguratorCamera::GetDistanceToTarget() const
{
	return FVector::Dist(GetActorLocation(), TargetLocation);
}

float AConfiguratorCamera::GetRotationDeltaToTarget() const
{
	const FRotator CurrentRotation = GetActorRotation();

	const float YawDelta = FMath::Abs(
		FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, TargetRotation.Yaw));
	const float PitchDelta = FMath::Abs(
		FMath::FindDeltaAngleDegrees(CurrentRotation.Pitch, TargetRotation.Pitch));

	return FMath::Sqrt(YawDelta * YawDelta + PitchDelta * PitchDelta);
}