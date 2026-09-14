// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#include "UI/ApartmentCardWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Data/ApartmentData.h"
#include "Subsystems/FloorConfiguratorSubsystem.h"

// ─────────────────────────────────────────────────────────────────────
//  Жизненный цикл
// ─────────────────────────────────────────────────────────────────────

void UApartmentCardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CachedSubsystem = GetGameInstance()->GetSubsystem<UFloorConfiguratorSubsystem>();
	if (!IsValid(CachedSubsystem))
	{
		UE_LOG(LogTemp, Error, TEXT("[ApartmentCard] Подсистема не найдена."));
		return;
	}

	// Подписка на делегаты подсистемы.
	CachedSubsystem->OnApartmentSelected.AddDynamic(this, &ThisClass::HandleApartmentSelected);
	CachedSubsystem->OnViewModeChanged.AddDynamic(this, &ThisClass::HandleViewModeChanged);
	CachedSubsystem->OnApartmentReserved.AddDynamic(this, &ThisClass::HandleApartmentReserved);

	// Кнопка "Забронировать".
	if (ReserveButton)
	{
		ReserveButton->OnClicked.AddDynamic(this, &ThisClass::HandleReserveClicked);
	}

	// Скрываем карточку до первого выбора квартиры.
	Refresh();
}

void UApartmentCardWidget::NativeDestruct()
{
	if (IsValid(CachedSubsystem))
	{
		CachedSubsystem->OnApartmentSelected.RemoveDynamic(this, &ThisClass::HandleApartmentSelected);
		CachedSubsystem->OnViewModeChanged.RemoveDynamic(this, &ThisClass::HandleViewModeChanged);
		CachedSubsystem->OnApartmentReserved.RemoveDynamic(this, &ThisClass::HandleApartmentReserved);
	}

	if (ReserveButton)
	{
		ReserveButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleReserveClicked);
	}

	Super::NativeDestruct();
}

// ─────────────────────────────────────────────────────────────────────
//  Обработчики
// ─────────────────────────────────────────────────────────────────────

void UApartmentCardWidget::HandleApartmentSelected(int32 InApartmentId, FVector InFocusPoint)
{
	CurrentApartmentId = InApartmentId;
	Refresh();
}

void UApartmentCardWidget::HandleViewModeChanged(EConfiguratorViewMode NewMode)
{
	Refresh();
}

void UApartmentCardWidget::HandleApartmentReserved(int32 InApartmentId)
{
	// Обновляем карточку, если забронировали именно её.
	if (InApartmentId == CurrentApartmentId)
	{
		Refresh();
	}
}

void UApartmentCardWidget::HandleReserveClicked()
{
	if (!IsValid(CachedSubsystem) || CurrentApartmentId == INDEX_NONE)
	{
		return;
	}

	CachedSubsystem->ReserveApartment(CurrentApartmentId);
}

// ─────────────────────────────────────────────────────────────────────
//  Внутренняя логика
// ─────────────────────────────────────────────────────────────────────

void UApartmentCardWidget::Refresh()
{
	if (!IsValid(CachedSubsystem))
	{
		return;
	}

	const FViewState& State = CachedSubsystem->GetViewState();

	// Показываем карточку только в режиме Apartment.
	if (State.Mode != EConfiguratorViewMode::Apartment)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// Восстанавливаем ID, если пришли в режим Apartment не через делегат
	// (например, через GoBack).
	if (CurrentApartmentId == INDEX_NONE)
	{
		CurrentApartmentId = State.ApartmentId;
	}

	int32				  FloorIndex = INDEX_NONE;
	const FApartmentData* Apartment = CachedSubsystem->FindApartmentById(
		CurrentApartmentId, FloorIndex);

	if (!Apartment)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	SetVisibility(ESlateVisibility::Visible);

	// Номер квартиры.
	if (ApartmentIdText)
	{
		ApartmentIdText->SetText(FText::FromString(
			FString::Printf(TEXT("Квартира %d"), Apartment->Id)));
	}

	// Этаж.
	if (FloorText)
	{
		FloorText->SetText(FText::FromString(
			FString::Printf(TEXT("Этаж %d"), FloorIndex + 1)));
	}

	// Площадь.
	if (AreaText)
	{
		AreaText->SetText(FText::FromString(
			FString::Printf(TEXT("%.1f м²"), Apartment->Area)));
	}

	// Статус — учитываем и данные из JSON, и локальные брони.
	const bool bSold = CachedSubsystem->IsApartmentSold(CurrentApartmentId);
	if (StatusText)
	{
		StatusText->SetText(FText::FromString(bSold ? TEXT("Продано") : TEXT("Свободно")));
		StatusText->SetColorAndOpacity(FSlateColor(
			bSold ? FLinearColor(0.8f, 0.3f, 0.3f, 1.f) : FLinearColor(0.3f, 0.8f, 0.3f, 1.f)));
	}

	// Кнопка "Забронировать" — только для свободных квартир.
	if (ReserveButton)
	{
		ReserveButton->SetVisibility(bSold
				? ESlateVisibility::Collapsed
				: ESlateVisibility::Visible);
		ReserveButton->SetIsEnabled(!bSold);
	}
}