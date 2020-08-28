
#include "ElevatorButton.h"
#include "Kismet/GameplayStatics.h"

UElevatorButton::UElevatorButton()
{
    PrimaryComponentTick.bCanEverTick = false;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> ElevatorButtonMesh(TEXT("StaticMesh'/Game/ElevatingActionOffice/Meshes/SM_Bld_ElevatorButton.SM_Bld_ElevatorButton'"));
    if (ElevatorButtonMesh.Succeeded())
    {
        UStaticMeshComponent::SetStaticMesh(ElevatorButtonMesh.Object);

        ConstructorHelpers::FObjectFinder<UMaterialInstance> WallMaterial(TEXT("/Game/ElevatingActionOffice/Materials/M_ElevatingActionOffice_Wall"));
        if (WallMaterial.Succeeded())
            ButtonMaterialInstance = WallMaterial.Object;
    }
}

void UElevatorButton::BeginPlay()
{
    Super::BeginPlay();

    if (ButtonMaterialInstance)
    {
        ButtonMaterial = UMaterialInstanceDynamic::Create(ButtonMaterialInstance, this, TEXT("M_ElevatingActionOffice_ElevatorButton"));
        ButtonMaterial->GetScalarParameterValue(TEXT("Base_EmissiveMultiplier"), ButtonBrightness);
        SetMaterial(0, ButtonMaterial);
    }
}

AElevator* UElevatorButton::GetElevator() const
{
    return Elevator;
}

void UElevatorButton::CallElevatorTo(int32 FloorNumber)
{
    Elevator->SetTargetFloorNumber(FloorNumber);
}

void UElevatorButton::SetButtonBrightness(float Brightness)
{
    if (ButtonMaterial)
    {
        ButtonBrightness = Brightness;

        ButtonMaterial->SetScalarParameterValue(TEXT("Base_EmissiveMultiplier"), ButtonBrightness);
        SetMaterial(0, ButtonMaterial);
    }
}

float UElevatorButton::GetButtonBrightness() const
{
    return ButtonBrightness;
}