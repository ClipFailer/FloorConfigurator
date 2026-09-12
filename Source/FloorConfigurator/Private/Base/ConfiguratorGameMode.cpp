// Alexey Nikolaichik, 2026. Test assignment for PlayEstate.

#include "Base/ConfiguratorGameMode.h"
#include "Camera/ConfiguratorCamera.h"
#include "Kismet/GameplayStatics.h"

AConfiguratorGameMode::AConfiguratorGameMode()
{
	DefaultPawnClass = nullptr;
}

void AConfiguratorGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		AConfiguratorCamera* Camera = Cast<AConfiguratorCamera>(
			UGameplayStatics::GetActorOfClass(GetWorld(), AConfiguratorCamera::StaticClass()));

		if (Camera)
		{
			PlayerController->SetViewTarget(Camera);
		}
	}
}
