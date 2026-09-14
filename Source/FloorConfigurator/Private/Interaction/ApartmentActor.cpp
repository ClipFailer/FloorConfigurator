// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#include "Interaction/ApartmentActor.h"
#include "Data/ApartmentData.h"
#include "Subsystems/FloorConfiguratorSubsystem.h"

// ─────────────────────────────────────────────────────────────────────
//  Жизненный цикл
// ─────────────────────────────────────────────────────────────────────

AApartmentActor::AApartmentActor()
{
	// Root — пустой SceneComponent, задаёт позицию актора.
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void AApartmentActor::BeginPlay()
{
	Super::BeginPlay();

	// Подписка на делегаты подсистемы.
	UFloorConfiguratorSubsystem* Subsystem =
		GetGameInstance()->GetSubsystem<UFloorConfiguratorSubsystem>();
	if (Subsystem)
	{
		Subsystem->OnApartmentSelected.AddDynamic(this, &ThisClass::HandleApartmentSelected);
		Subsystem->OnViewModeChanged.AddDynamic(this, &ThisClass::HandleViewModeChanged);
		Subsystem->OnHideSoldChanged.AddDynamic(this, &ThisClass::HandleHideSoldChanged);
	}

	// Создаём динамический материал подсветки.
	if (IsValid(HighlightOverlayMaterial))
	{
		HighlightMID = UMaterialInstanceDynamic::Create(HighlightOverlayMaterial, this);
		if (IsValid(HighlightMID))
		{
			HighlightMID->SetScalarParameterValue(TEXT("HighlightIntensity"), HighlightIntensity);
			HighlightMID->SetScalarParameterValue(TEXT("Thickness"), HighlightThickness);
			HighlightMID->SetVectorParameterValue(TEXT("HighlightColor"), HighlightColor);
		}
	}
}

void AApartmentActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Отписываемся от делегатов.
	UFloorConfiguratorSubsystem* Subsystem =
		GetGameInstance()->GetSubsystem<UFloorConfiguratorSubsystem>();
	if (IsValid(Subsystem))
	{
		Subsystem->OnApartmentSelected.RemoveDynamic(this, &ThisClass::HandleApartmentSelected);
		Subsystem->OnViewModeChanged.RemoveDynamic(this, &ThisClass::HandleViewModeChanged);
		Subsystem->OnHideSoldChanged.RemoveDynamic(this, &ThisClass::HandleHideSoldChanged);
	}

	Super::EndPlay(EndPlayReason);
}

// ─────────────────────────────────────────────────────────────────────
//  Управление состоянием
// ─────────────────────────────────────────────────────────────────────

void AApartmentActor::SetHighlighted(bool bInHighlighted)
{
	if (bHighlighted == bInHighlighted)
	{
		return;
	}

	bHighlighted = bInHighlighted;
	UpdateOverlay();
}

void AApartmentActor::SetFiltered(bool bInFiltered)
{
	if (bFiltered == bInFiltered)
	{
		return;
	}

	bFiltered = bInFiltered;
	UpdateOverlay();
}

// ─────────────────────────────────────────────────────────────────────
//  Обработчики делегатов Subsystem
// ─────────────────────────────────────────────────────────────────────

void AApartmentActor::HandleApartmentSelected(int32 InApartmentId, FVector InFocusPoint)
{
	if (InApartmentId != ApartmentId)
	{
		if (bHighlighted)
		{
			SetHighlighted(false);
		}
		return;
	}

	SetHighlighted(true);
}

void AApartmentActor::HandleViewModeChanged(EConfiguratorViewMode NewMode)
{
	// Снимаем подсветку при выходе из режима Apartment.
	if (NewMode != EConfiguratorViewMode::Apartment && bHighlighted)
	{
		SetHighlighted(false);
	}
}

void AApartmentActor::HandleHideSoldChanged(bool bInHideSold)
{
	// Затемняем только проданные квартиры.
	SetFiltered(bInHideSold && bIsSold);
}

// ─────────────────────────────────────────────────────────────────────
//  Внутренняя логика
// ─────────────────────────────────────────────────────────────────────

void AApartmentActor::UpdateOverlay()
{
	if (!Mesh)
	{
		return;
	}

	// Приоритет: подсветка > фильтр > ничего.
	if (bHighlighted && IsValid(HighlightOverlayMaterial))
	{
		Mesh->SetOverlayMaterial(HighlightOverlayMaterial);
	}
	else if (bFiltered && IsValid(FilteredOverlayMaterial))
	{
		Mesh->SetOverlayMaterial(FilteredOverlayMaterial);
	}
	else
	{
		Mesh->SetOverlayMaterial(nullptr);
	}
}

void AApartmentActor::Init(const FApartmentData& Data, float ApartmentSize, float FloorHeight)
{
	ApartmentId = Data.Id;
	FocusPoint = Data.FocusPoint;
	Area = Data.Area;
	bIsSold = Data.IsSold();

	// Масштаб меша под размер квартиры и высоту этажа.
	const float ScaleXY = ApartmentSize / 100.f;
	const float ScaleZ = FloorHeight / 100.f;

	Mesh->SetRelativeScale3D(FVector(ScaleXY, ScaleXY, ScaleZ));

	// Сдвигаем меш так, чтобы нижняя грань совпала с FocusPoint.Z.
	// Куб имеет центр в 0, поэтому смещаем на половину высоты.
	Mesh->SetRelativeLocation(FVector(0.f, 0.f, FloorHeight / 2.f));
}