// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#include "UI/FloorButtonWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Subsystems/FloorConfiguratorSubsystem.h"

// ─────────────────────────────────────────────────────────────────────
//  Жизненный цикл
// ─────────────────────────────────────────────────────────────────────

void UFloorButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (FloorButton)
	{
		FloorButton->OnClicked.AddDynamic(this, &ThisClass::HandleClicked);
	}

	// Применяем текущее состояние стиля.
	SetActive(bIsActive);
}

// ─────────────────────────────────────────────────────────────────────
//  Инициализация
// ─────────────────────────────────────────────────────────────────────

void UFloorButtonWidget::Setup(int32 InFloorIndex)
{
	FloorIndex = InFloorIndex;

	if (FloorText)
	{
		FloorText->SetText(FText::AsNumber(FloorIndex + 1));
	}
}

void UFloorButtonWidget::SetActive(bool bInActive)
{
	bIsActive = bInActive;

	if (FloorText)
	{
		FloorText->SetColorAndOpacity(
			FSlateColor(bInActive ? ActiveColor : InactiveColor));
	}
}

// ─────────────────────────────────────────────────────────────────────
//  Обработчики
// ─────────────────────────────────────────────────────────────────────

void UFloorButtonWidget::HandleClicked()
{
	if (FloorIndex == INDEX_NONE)
	{
		return;
	}

	UFloorConfiguratorSubsystem* Subsystem =
		GetGameInstance()->GetSubsystem<UFloorConfiguratorSubsystem>();

	if (IsValid(Subsystem))
	{
		Subsystem->RequestFloor(FloorIndex);
	}
}