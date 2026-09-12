// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#include "Camera/ConfiguratorCamera.h"
#include "Camera/CameraComponent.h"
#include "Subsystems/FloorConfiguratorSubsystem.h"

AConfiguratorCamera::AConfiguratorCamera()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(Root);
}

void AConfiguratorCamera::HandleViewModeChanged(EConfiguratorViewMode NewMode)
{
	UE_LOG(LogTemp, Log, TEXT("[ConfiguratorCamera] Смена режима вида."));
}

void AConfiguratorCamera::HandleFloorSelected(int32 FloorIndex, FVector FocusPoint)
{
	UpdateTargetForFloor(FloorIndex, FocusPoint);
}

void AConfiguratorCamera::HandleApartmentSelected(int32 ApartmentId, FVector FocusPoint)
{
	UpdateTargetForApartment(ApartmentId, FocusPoint);
}

void AConfiguratorCamera::BeginPlay()
{
	Super::BeginPlay();

	// Получаем сабсистему конфигуратора.
	ConfiguratorSubsystem = GetGameInstance()->GetSubsystem<UFloorConfiguratorSubsystem>();

	// Если сабсистема валидна - подключаемся к делегатам.
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

void AConfiguratorCamera::UpdateTargetForGenplan()
{
	if (!IsValid(ConfiguratorSubsystem))
	{
		return;
	}

	FVector GenplanFocusPoint = ConfiguratorSubsystem->GetGenplanFocusPoint();

	SetTargetFromFocus(GenplanFocusPoint, GenplanOffset);

	StartInterp();
}

void AConfiguratorCamera::UpdateTargetForFloor(int32 FloorIndex, const FVector& FocusPoint)
{
	SetTargetFromFocus(FocusPoint, FloorOffset);

	StartInterp();

	UE_LOG(LogTemp, Log, TEXT("[ConfiguratorCamera] Фокус камеры на этаже %d."), FloorIndex);
}

void AConfiguratorCamera::UpdateTargetForApartment(int32 ApartmentId, const FVector& FocusPoint)
{
	SetTargetFromFocus(FocusPoint, ApartmentOffset);

	StartInterp();

	UE_LOG(LogTemp, Log, TEXT("[ConfiguratorCamera] Фокус камеры на квартире %d."), ApartmentId);
}

void AConfiguratorCamera::SetTargetFromFocus(const FVector& FocusPoint, const FVector& Offset)
{
	TargetLocation = FocusPoint + Offset;

	// Получаем точку, куда нужно смотреть камере с позиции цели.
	TargetRotation = (FocusPoint - TargetLocation).Rotation();
}

void AConfiguratorCamera::StartInterp()
{
	// Если таймер уже активен - ничего не делаем.
	if (bIsTransitioning)
		return;

	bIsTransitioning = true;

	// Запускаем таймер, который вызывает InterpStep каждую 1.f / InterpFrequency секунду.
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

	// Останавливаем таймер.
	GetWorld()->GetTimerManager().ClearTimer(InterpTimer);
}

void AConfiguratorCamera::InterpStep()
{
	// Фиксированная частота таймера.
	const float DeltaTime = 1.f / InterpFrequency;

	const float Distance = GetDistanceToTarget();
	const float RotationDelta = GetRotationDeltaToTarget();

	// Если расстояние меньше порога прибытия - останавливаем движение.
	if (Distance <= ArrivalThreshold && RotationDelta <= ArrivalThreshold)
	{
		// Доводим камеру до цели.
		SetActorLocation(TargetLocation);
		SetActorRotation(TargetRotation);

		// Останавливаем таймер.
		StopInterp();

		return;
	}

	if (Distance > ArrivalThreshold)
	{
		const FVector CurrentLocation = GetActorLocation();

		// Интерполированное перемещение камеры к цели за DeltaTime.
		const FVector NewLocation = FMath::VInterpTo(
			CurrentLocation,
			TargetLocation,
			DeltaTime,
			InterpSpeed);
		SetActorLocation(NewLocation);
	}

	if (RotationDelta > ArrivalThreshold)
	{
		const FRotator CurrentRotation = GetActorRotation();

		// Интерполированное вращение камеры к цели за DeltaTime.
		const FRotator NewRotation = FMath::RInterpTo(
			CurrentRotation,
			TargetRotation,
			DeltaTime,
			InterpSpeed);
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
	return FMath::Sqrt(
		FMath::Square(CurrentRotation.Yaw - TargetRotation.Yaw) +
		FMath::Square(CurrentRotation.Pitch - TargetRotation.Pitch));
}