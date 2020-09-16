
#include "ElevatorButton.h"
#include "ElevatingActionSecretAgent.h"
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

    static ConstructorHelpers::FObjectFinder<USoundWave> ElevatorButtonClickSoundWave
    (TEXT("SoundWave'/Game/ElevatingActionAudio/GameMasterAudio/ElevatorButtonSignal/switch_button_push_on_off_10.switch_button_push_on_off_10'"));
    if (ElevatorButtonClickSoundWave.Succeeded())
        ElevatorButtonClickSound = ElevatorButtonClickSoundWave.Object;

    static ConstructorHelpers::FObjectFinder<USoundWave> Alarm
    (TEXT("SoundWave'/Game/ElevatingActionAudio/GameMasterAudio/ElevatorButtonSignal/alarm_siren_loop_04.alarm_siren_loop_04'"));
    if (Alarm.Succeeded())
        ElevatorArrivedAlarm = Alarm.Object;

    ElevatorButtonAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ElevatorButtonAudio"));
    ElevatorButtonAudioComponent->SetAutoActivate(false);
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

    if (GetOwner())
        CurrentFloorNumber = 30 + FMath::FloorToInt(GetOwner()->GetActorLocation().Z / 300);
}

AElevator* UElevatorButton::GetElevator() const
{
    return Elevator;
}

USoundWave* UElevatorButton::GetElevatorArrivedAlarm() const
{
    return ElevatorArrivedAlarm;
}

int32 UElevatorButton::GetCurrentFloorNumber() const
{
    return CurrentFloorNumber;
}

void UElevatorButton::CallElevatorTo(int32 FloorNumber)
{
    Elevator->SetTargetFloorNumber(FloorNumber);
    Elevator->SetElevatorButtonCaller(this);

    AElevatingActionSecretAgent* SecretAgentOtto = Cast<AElevatingActionSecretAgent>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (CurrentFloorNumber == SecretAgentOtto->GetCurrentFloorNumber())
    {
        ElevatorButtonAudioComponent->SetSound(ElevatorButtonClickSound);
        ElevatorButtonAudioComponent->Play();
    }
}

void UElevatorButton::SetButtonBrightness(float Brightness)
{
    if (ButtonMaterial)
    {
        if (ButtonBrightness == Brightness)
            return;
        
        ButtonBrightness = Brightness;
        ButtonMaterial->SetScalarParameterValue(TEXT("Base_EmissiveMultiplier"), ButtonBrightness);
        SetMaterial(0, ButtonMaterial);
    }
}

float UElevatorButton::GetButtonBrightness() const
{
    return ButtonBrightness;
}
