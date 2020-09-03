
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

    static ConstructorHelpers::FObjectFinder<USoundCue> ElevatorButtonClickSound
    (TEXT("SoundCue'/Game/ElevatingActionAudio/GameMasterAudio/ElevatorButtonSignal/S_ElevatorButtonClick_Cue.S_ElevatorButtonClick_Cue'"));
    if (ElevatorButtonClickSound.Succeeded())
    {
        ElevatorButtonClickCue = ElevatorButtonClickSound.Object;
    }

    static ConstructorHelpers::FObjectFinder<USoundWave> Alarm
    (TEXT("SoundWave'/Game/ElevatingActionAudio/GameMasterAudio/ElevatorButtonSignal/alarm_siren_loop_04.alarm_siren_loop_04'"));
    if (Alarm.Succeeded())
        ElevatorArrivedAlarm = Alarm.Object;

    static ConstructorHelpers::FObjectFinder<USoundWave> Beep(TEXT("SoundWave'/Game/ElevatingActionAudio/GameMasterAudio/ElevatorButtonSignal/beep_08.beep_08'"));
    if (Beep.Succeeded())
        ElevatorButtonHighlightSound = Beep.Object;
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
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), ElevatorButtonClickCue, GetRelativeLocation());
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

        AElevatingActionSecretAgent* SecretAgentOtto = Cast<AElevatingActionSecretAgent>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
        if (CurrentFloorNumber == SecretAgentOtto->GetCurrentFloorNumber() && Brightness == 2.0f)
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), ElevatorButtonHighlightSound, GetRelativeLocation());
    }
}

float UElevatorButton::GetButtonBrightness() const
{
    return ButtonBrightness;
}
