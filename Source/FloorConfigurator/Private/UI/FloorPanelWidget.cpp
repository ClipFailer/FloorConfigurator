// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#include "UI/FloorPanelWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Subsystems/FloorConfiguratorSubsystem.h"
#include "UI/FloorButtonWidget.h"

// ─────────────────────────────────────────────────────────────────────
//  Жизненный цикл
// ─────────────────────────────────────────────────────────────────────

void UFloorPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CachedSubsystem = GetGameInstance()->GetSubsystem<UFloorConfiguratorSubsystem>();
	if (!IsValid(CachedSubsystem))
	{
		UE_LOG(LogTemp, Error, TEXT("[FloorPanel] Подсистема не найдена."));
		return;
	}

	// Создаём кнопки этажей.
	BuildFloorButtons();

	// Подписка на смену режима — для обновления активной кнопки.
	CachedSubsystem->OnViewModeChanged.AddDynamic(this, &ThisClass::HandleViewModeChanged);

	// Первичная синхронизация активной кнопки.
	UpdateActiveButton();
}

void UFloorPanelWidget::NativeDestruct()
{
	if (IsValid(CachedSubsystem))
	{
		CachedSubsystem->OnViewModeChanged.RemoveDynamic(this, &ThisClass::HandleViewModeChanged);
	}

	Super::NativeDestruct();
}

// ─────────────────────────────────────────────────────────────────────
//  Обработчики
// ─────────────────────────────────────────────────────────────────────

void UFloorPanelWidget::HandleViewModeChanged(EConfiguratorViewMode /*NewMode*/)
{
	UpdateActiveButton();
}

// ─────────────────────────────────────────────────────────────────────
//  Внутренняя логика
// ─────────────────────────────────────────────────────────────────────

void UFloorPanelWidget::BuildFloorButtons()
{
	if (!FloorButtonsContainer)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FloorPanel] FloorButtonsContainer не задан."));
		return;
	}

	if (!IsValid(FloorButtonClass))
	{
		UE_LOG(LogTemp, Error, TEXT("[FloorPanel] FloorButtonClass не задан."));
		return;
	}

	if (!IsValid(CachedSubsystem))
	{
		return;
	}

	FloorButtonsContainer->ClearChildren();
	FloorButtons.Empty();

	const int32 FloorsCount = CachedSubsystem->GetFloorsCount();
	for (int32 i = 0; i < FloorsCount; ++i)
	{
		UFloorButtonWidget* Button = CreateWidget<UFloorButtonWidget>(this, FloorButtonClass);
		if (!IsValid(Button))
		{
			continue;
		}

		Button->Setup(i);

		if (UHorizontalBoxSlot* BoxSlot = FloorButtonsContainer->AddChildToHorizontalBox(Button))
		{
			BoxSlot->SetPadding(FMargin(4.f, 0.f));
		}

		FloorButtons.Add(Button);
	}
}

void UFloorPanelWidget::UpdateActiveButton()
{
	if (!IsValid(CachedSubsystem))
	{
		return;
	}

	const int32 ActiveFloor = CachedSubsystem->GetViewState().FloorIndex;

	for (int32 i = 0; i < FloorButtons.Num(); ++i)
	{
		if (IsValid(FloorButtons[i]))
		{
			FloorButtons[i]->SetActive(i == ActiveFloor);
		}
	}
}