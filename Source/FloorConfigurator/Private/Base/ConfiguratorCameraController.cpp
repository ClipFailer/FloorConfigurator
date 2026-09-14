// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#include "Base/ConfiguratorCameraController.h"
#include "Camera/ConfiguratorCamera.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Interfaces/ApartmentInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/FloorConfiguratorSubsystem.h"
#include "UI/ConfiguratorHUD.h"

// ─────────────────────────────────────────────────────────────────────
//  Жизненный цикл
// ─────────────────────────────────────────────────────────────────────

void AConfiguratorCameraController::BeginPlay()
{
	Super::BeginPlay();

	// Показываем мышь и задаём игровой ввод без захвата курсора.
	bShowMouseCursor = true;
	SetInputMode(FInputModeGameAndUI());

	FindAndCacheCamera();
	FindAndCacheSubsystem();
	RegisterInputContext();

	// Создаём главный виджет UI.
	if (IsValid(HUDWidgetClass))
	{
		HUDWidget = CreateWidget<UConfiguratorHUD>(this, HUDWidgetClass);
		if (IsValid(HUDWidget))
		{
			HUDWidget->AddToViewport();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[Controller] Не удалось создать HUD-виджет."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Controller] HUDWidgetClass не задан."));
	}
}

void AConfiguratorCameraController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInputComp)
	{
		UE_LOG(LogTemp, Error, TEXT("[Controller] EnhancedInputComponent не найден."));
		return;
	}

	// Режим орбиты: зажали — включён, отжали — выключен.
	if (IsValid(IA_Orbit))
	{
		EnhancedInputComp->BindAction(IA_Orbit, ETriggerEvent::Started, this, &ThisClass::OnOrbitStarted);
		EnhancedInputComp->BindAction(IA_Orbit, ETriggerEvent::Completed, this, &ThisClass::OnOrbitCompleted);
	}

	// Вращение камеры в режиме орбиты.
	if (IsValid(IA_Look))
	{
		EnhancedInputComp->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ThisClass::OnLook);
	}

	// Клик по квартире.
	if (IsValid(IA_Click))
	{
		EnhancedInputComp->BindAction(IA_Click, ETriggerEvent::Started, this, &ThisClass::OnClick);
	}
}

// ─────────────────────────────────────────────────────────────────────
//  Обработчики ввода
// ─────────────────────────────────────────────────────────────────────

void AConfiguratorCameraController::OnOrbitStarted()
{
	if (!IsValid(CachedCamera))
	{
		return;
	}

	CachedCamera->StartOrbit();
}

void AConfiguratorCameraController::OnOrbitCompleted()
{
	if (!IsValid(CachedCamera))
	{
		return;
	}

	CachedCamera->StopOrbit();
}

void AConfiguratorCameraController::OnLook(const FInputActionValue& Value)
{
	if (!IsValid(CachedCamera))
	{
		return;
	}

	// Передаём приращение осей в камеру.
	const FVector2D Axis = Value.Get<FVector2D>();
	CachedCamera->AddOrbitInput(Axis.X, -Axis.Y);
}

void AConfiguratorCameraController::OnClick(const FInputActionValue& Value)
{
	TrySelectApartmentUnderCursor();
}

// ─────────────────────────────────────────────────────────────────────
//  Хелперы
// ─────────────────────────────────────────────────────────────────────

void AConfiguratorCameraController::FindAndCacheCamera()
{
	CachedCamera = Cast<AConfiguratorCamera>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AConfiguratorCamera::StaticClass()));

	if (!CachedCamera)
	{
		UE_LOG(LogTemp, Error, TEXT("[Controller] Камера не найдена."));
	}
}

void AConfiguratorCameraController::FindAndCacheSubsystem()
{
	Subsystem = GetGameInstance()->GetSubsystem<UFloorConfiguratorSubsystem>();

	if (!Subsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[Controller] Подсистема не найдена."));
	}
}

void AConfiguratorCameraController::RegisterInputContext()
{
	if (!InputContext)
	{
		UE_LOG(LogTemp, Error, TEXT("[Controller] InputMappingContext не задан."));
		return;
	}

	// Получаем подсистему ввода локального игрока.
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

	if (InputSubsystem)
	{
		InputSubsystem->AddMappingContext(InputContext, 0);
	}
}

void AConfiguratorCameraController::TrySelectApartmentUnderCursor()
{
	if (!IsValid(Subsystem))
	{
		return;
	}

	// Трассировка из камеры через позицию курсора.
	FHitResult Hit;
	const bool bHit = GetHitResultUnderCursorByChannel(
		UEngineTypes::ConvertToTraceType(ECC_Visibility), false, Hit);

	if (!bHit)
	{
		return;
	}

	// Проверяем, что под курсором актор, реализующий интерфейс квартиры.
	AActor* Apartment = Hit.GetActor();
	if (!IsValid(Apartment) || !Apartment->Implements<UApartmentInterface>())
	{
		return;
	}

	// Передаём подсистеме запрос на фокус на квартиру.
	const int32 ApartmentId = IApartmentInterface::Execute_GetApartmentId(Apartment);
	Subsystem->RequestApartment(ApartmentId);
}