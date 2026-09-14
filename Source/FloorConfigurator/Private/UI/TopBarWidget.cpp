// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#include "UI/TopBarWidget.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "Subsystems/FloorConfiguratorSubsystem.h"
#include "UI/FloorPanelWidget.h"

// ─────────────────────────────────────────────────────────────────────
//  Жизненный цикл
// ─────────────────────────────────────────────────────────────────────

void UTopBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CachedSubsystem = GetGameInstance()->GetSubsystem<UFloorConfiguratorSubsystem>();
	if (!IsValid(CachedSubsystem))
	{
		UE_LOG(LogTemp, Error, TEXT("[TopBar] Подсистема не найдена."));
		return;
	}

	// Кнопка "Назад".
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &ThisClass::HandleBackClicked);
	}

	// Кнопка "Общий вид".
	if (GenplanButton)
	{
		GenplanButton->OnClicked.AddDynamic(this, &ThisClass::HandleGenplanClicked);
	}

	// Чекбокс "Скрыть проданные".
	if (HideSoldCheckbox)
	{
		HideSoldCheckbox->OnCheckStateChanged.AddDynamic(this, &ThisClass::HandleHideSoldChanged);
	}

	// Подписка на делегаты подсистемы.
	CachedSubsystem->OnViewModeChanged.AddDynamic(this, &ThisClass::HandleViewModeChanged);
	CachedSubsystem->OnFloorSelected.AddDynamic(this, &ThisClass::HandleFloorSelected);
	CachedSubsystem->OnApartmentSelected.AddDynamic(this, &ThisClass::HandleApartmentSelected);

	// Первичное состояние.
	Refresh();
}

void UTopBarWidget::NativeDestruct()
{
	if (IsValid(CachedSubsystem))
	{
		CachedSubsystem->OnViewModeChanged.RemoveDynamic(this, &ThisClass::HandleViewModeChanged);
		CachedSubsystem->OnFloorSelected.RemoveDynamic(this, &ThisClass::HandleFloorSelected);
		CachedSubsystem->OnApartmentSelected.RemoveDynamic(this, &ThisClass::HandleApartmentSelected);
	}

	// Отписка от чекбокса.
	if (HideSoldCheckbox)
	{
		HideSoldCheckbox->OnCheckStateChanged.RemoveDynamic(this, &ThisClass::HandleHideSoldChanged);
	}

	Super::NativeDestruct();
}

// ─────────────────────────────────────────────────────────────────────
//  Обработчики
// ─────────────────────────────────────────────────────────────────────

void UTopBarWidget::HandleHideSoldChanged(bool bInHideSold)
{
	if (IsValid(CachedSubsystem))
	{
		CachedSubsystem->OnHideSoldChanged.Broadcast(bInHideSold);
	}
}

void UTopBarWidget::HandleBackClicked()
{
	if (IsValid(CachedSubsystem))
	{
		CachedSubsystem->GoBack();
	}
}

void UTopBarWidget::HandleGenplanClicked()
{
	if (IsValid(CachedSubsystem))
	{
		CachedSubsystem->RequestGenplan();
	}
}

void UTopBarWidget::HandleViewModeChanged(EConfiguratorViewMode /*NewMode*/)
{
	Refresh();
}

void UTopBarWidget::HandleFloorSelected(int32 /*FloorIndex*/, FVector /*FocusPoint*/)
{
	Refresh();
}

void UTopBarWidget::HandleApartmentSelected(int32 /*ApartmentId*/, FVector /*FocusPoint*/)
{
	Refresh();
}

// ─────────────────────────────────────────────────────────────────────
//  Внутренняя логика
// ─────────────────────────────────────────────────────────────────────

void UTopBarWidget::Refresh()
{
	if (!IsValid(CachedSubsystem))
	{
		return;
	}

	const FViewState& State = CachedSubsystem->GetViewState();

	// Текст текущего состояния навигации.
	if (CurrentStateText)
	{
		FString Text;
		switch (State.Mode)
		{
			case EConfiguratorViewMode::Genplan:
				Text = TEXT("Общий план");
				break;

			case EConfiguratorViewMode::Floor:
				Text = FString::Printf(TEXT("Этаж %d"), State.FloorIndex + 1);
				break;

			case EConfiguratorViewMode::Apartment:
				Text = FString::Printf(TEXT("Этаж %d / Квартира %d"),
					State.FloorIndex + 1, State.ApartmentId);
				break;
		}
		CurrentStateText->SetText(FText::FromString(Text));
	}

	// Кнопка "Назад" активна только при наличии истории.
	if (BackButton)
	{
		BackButton->SetIsEnabled(CachedSubsystem->CanGoBack());
	}

	// Кнопка "Общий вид" активна, только если мы не в Genplan.
	if (GenplanButton)
	{
		GenplanButton->SetIsEnabled(State.Mode != EConfiguratorViewMode::Genplan);
	}
}