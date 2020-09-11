
#include "Elevator.h"
#include "ElevatingActionSecretAgent.h"
#include "Kismet/GameplayStatics.h"

AElevator::AElevator()
{
    PrimaryActorTick.bCanEverTick = true;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> ElevatorMesh(TEXT("/Game/PolygonOffice/Meshes/Buildings/SM_Bld_Elevator_01"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> ElevatorDoorMesh(TEXT("/Game/PolygonOffice/Meshes/Buildings/SM_Bld_Elevator_01_Door"));

	if (ElevatorMesh.Succeeded())
	{
		GetStaticMeshComponent()->SetStaticMesh(ElevatorMesh.Object);
		GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
		GetStaticMeshComponent()->SetGenerateOverlapEvents(true);
	}

    ElevatorDirection = EDirectionState::Down;
    ElevatorSpeed = 150.0f;
    ElevatorStoppedTime = 0.0f;
    ElevatorStoppedDuration = 3.0f;

    ElevatorDoorsState = EDoorsState::Closed;
    ElevatorDoorsSpeed = 75.0f;
    LeftDoorTargetLocation = FVector(0.0f, -5.0f, 0.0f);
    RightDoorTargetLocation = FVector(250.0f, -10.0f, 0.0f);
    OpenDoorOffset = 70.0f;

    MinFloorNumber = 1;
    NextTargetFloorNumber = -1;
	
    LeftDoor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftDoor"));
    LeftDoor->SetCollisionProfileName(TEXT("BlockAll"));
    LeftDoor->SetupAttachment(RootComponent);
    LeftDoor->SetRelativeLocation(LeftDoorTargetLocation);
    if (ElevatorDoorMesh.Succeeded())
		LeftDoor->SetStaticMesh(ElevatorDoorMesh.Object);

    RightDoor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightDoor"));
    RightDoor->SetCollisionProfileName(TEXT("BlockAll"));
    RightDoor->SetupAttachment(RootComponent);
    RightDoor->SetRelativeLocationAndRotation(RightDoorTargetLocation, FRotator(0.0f, 180.0f, 0.0f));
    if (ElevatorDoorMesh.Succeeded())
		RightDoor->SetStaticMesh(ElevatorDoorMesh.Object);

	ElevatorMovingAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MovingAudio"));
	ElevatorMovingAudioComponent->SetupAttachment(RootComponent);
	ElevatorMovingAudioComponent->SetRelativeLocation(FVector(125.0f, -125.0f, 90.15f));
	ElevatorMovingAudioComponent->SetAutoActivate(false);

	InsideElevatorArea = CreateDefaultSubobject<UBoxComponent>(TEXT("InsideElevatorArea"));
	InsideElevatorArea->SetupAttachment(RootComponent);
	InsideElevatorArea->InitBoxExtent(FVector::OneVector * 75.0f);
	InsideElevatorArea->SetRelativeLocation(FVector(125.0f, -125.0f, 125.0f));
	InsideElevatorArea->SetGenerateOverlapEvents(true);
	InsideElevatorArea->SetCollisionProfileName(TEXT("OverlapAll"));

	static ConstructorHelpers::FObjectFinder<USoundWave> MovingUp(TEXT("SoundWave'/Game/ElevatingActionAudio/GameMasterAudio/ElevatorMoving/elevator_loop_02.elevator_loop_02'"));
	if (MovingUp.Succeeded())
		ElevatorMovingUpSoundWave = MovingUp.Object;

	static ConstructorHelpers::FObjectFinder<USoundWave> MovingDown(TEXT("SoundWave'/Game/ElevatingActionAudio/GameMasterAudio/ElevatorMoving/elevator_loop_01.elevator_loop_01'"));
	if (MovingDown.Succeeded())
		ElevatorMovingDownSoundWave = MovingDown.Object;

	static ConstructorHelpers::FObjectFinder<USoundWave> SlideDoorsClosed
    (TEXT("SoundWave'/Game/ElevatingActionAudio/GameMasterAudio/ElevatorMoving/door_metal_draw_slide_close_02.door_metal_draw_slide_close_02'"));
	if (SlideDoorsClosed.Succeeded())
		ElevatorDoorsClosingSoundWave = SlideDoorsClosed.Object;

	static ConstructorHelpers::FObjectFinder<USoundWave> SlideDoorsOpen
	(TEXT("SoundWave'/Game/ElevatingActionAudio/GameMasterAudio/ElevatorMoving/door_metal_draw_slide_open_01.door_metal_draw_slide_open_01'"));
	if (SlideDoorsOpen.Succeeded())
		ElevatorDoorsOpeningSoundWave = SlideDoorsOpen.Object;
}

void AElevator::BeginPlay()
{
    Super::BeginPlay();

    CurrentFloorNumber = 30 + FMath::FloorToInt(GetActorLocation().Z / 300);
    CurrentTargetFloorNumber = CurrentFloorNumber;

    if (CurrentFloorNumber >= 30)
        MaxFloorNumber = 30;
    else
	    MaxFloorNumber = CurrentFloorNumber;
}

void AElevator::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//Door Movement
	FVector LeftDoorLocation = FMath::VInterpConstantTo(LeftDoor->GetRelativeLocation(), LeftDoorTargetLocation, DeltaSeconds, ElevatorDoorsSpeed);
	FVector RightDoorLocation = FMath::VInterpConstantTo(RightDoor->GetRelativeLocation(), RightDoorTargetLocation, DeltaSeconds, ElevatorDoorsSpeed);

	LeftDoor->SetRelativeLocation(LeftDoorLocation);
	RightDoor->SetRelativeLocation(RightDoorLocation);

	if (!AreDoorsMoving())
	{
		if (LeftDoorLocation.X >= 0.0f && RightDoorLocation.X >= 250.0f)
			ElevatorDoorsState = EDoorsState::Closed;
		else
			ElevatorDoorsState = EDoorsState::Open;
	}

	//Elevator Movement
	ElevatorTargetLocation = FVector(GetActorLocation().X, GetActorLocation().Y, 300 * (CurrentTargetFloorNumber - 30));
	FVector ElevatorLocation = FMath::VInterpConstantTo(GetActorLocation(), ElevatorTargetLocation, DeltaSeconds, ElevatorSpeed);
	SetActorLocation(ElevatorLocation);
	
	if (IsElevatorMoving())
	{
		if (IsTargetFloorNumberSet())
		{
			if (ElevatorDirection == EDirectionState::Up 	&& NextTargetFloorNumber > CurrentTargetFloorNumber ||
                ElevatorDirection == EDirectionState::Down 	&& NextTargetFloorNumber < CurrentTargetFloorNumber)
                	GoToFloor(NextTargetFloorNumber);
		}
	}
	else
	{
		ElevatorStoppedTime += DeltaSeconds;
		
		if (!HasElevatorPassedStopTime())
		{
			CurrentFloorNumber = CurrentTargetFloorNumber;
			
			if (ElevatorStoppedTime > 0.009f)
			{
				if (ElevatorMovingAudioComponent->IsPlaying())
				{
					if (ElevatorMovingAudioComponent->Sound == ElevatorMovingUpSoundWave || ElevatorMovingAudioComponent->Sound == ElevatorMovingDownSoundWave)
						ElevatorMovingAudioComponent->Stop();
				}

				if (ElevatorButtonCaller && CurrentFloorNumber == ElevatorButtonCaller->GetCurrentFloorNumber())
				{
					AElevatingActionSecretAgent* PlayerSecretAgent = Cast<AElevatingActionSecretAgent>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
					if (IsValid(PlayerSecretAgent) && CurrentFloorNumber == PlayerSecretAgent->GetCurrentFloorNumber())
					{
						if (PlayerSecretAgent->GetCurrentLocation() == ELocationState::Hallway)
						{
							UGameplayStatics::PlaySoundAtLocation(GetWorld(), ElevatorButtonCaller->GetElevatorArrivedAlarm(), ElevatorButtonCaller->GetRelativeLocation());
							ElevatorButtonCaller = nullptr;
						}
					}
				}
			}
			
			if (!GetOwner())
				OpenDoors();
		}
		else
		{
			if (!GetOwner())
				CloseDoors();
			
			if (ElevatorDoorsState == EDoorsState::Closed)
			{
				if (!IsTargetFloorNumberSet())
				{
					if (CurrentFloorNumber == MinFloorNumber)
						ElevatorDirection = EDirectionState::Up;
					else if (CurrentFloorNumber == MaxFloorNumber)
						ElevatorDirection = EDirectionState::Down;

					GoToNextFloor(ElevatorDirection);
				}
				else
				{
					if (CurrentFloorNumber != NextTargetFloorNumber)
						GoToFloor(NextTargetFloorNumber);
					else
						ElevatorStoppedTime = 0.0f;
				}
			}
			else
			{
				if (GetOwner() || (IsTargetFloorNumberSet() && CurrentTargetFloorNumber == NextTargetFloorNumber))
				{
					NextTargetFloorNumber = -1;
				}
			}
		}
	}
}

void AElevator::CloseDoors()
{
	LeftDoorTargetLocation = FVector(0.0f, -5.0f, 0.0f);
	RightDoorTargetLocation = FVector(250.0f, -10.0f, 0.0f);

	if (GetOwner())
	{
		AElevatingActionSecretAgent* PlayerSecretAgent = Cast<AElevatingActionSecretAgent>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		if (IsValid(PlayerSecretAgent))
		{
			if (CurrentFloorNumber == PlayerSecretAgent->GetCurrentFloorNumber())
			{
				ElevatorMovingAudioComponent->SetSound(ElevatorDoorsClosingSoundWave);
				ElevatorMovingAudioComponent->Play();
			}
		}
	}
}

void AElevator::OpenDoors()
{
	LeftDoorTargetLocation = FVector(-OpenDoorOffset, -5.0f, 0.0f);
	RightDoorTargetLocation = FVector(250.0f + OpenDoorOffset, -10.0f, 0.0f);

	if (GetOwner())
	{
		AElevatingActionSecretAgent* PlayerSecretAgent = Cast<AElevatingActionSecretAgent>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		if (IsValid(PlayerSecretAgent))
		{
			if (CurrentFloorNumber == PlayerSecretAgent->GetCurrentFloorNumber())
			{
				ElevatorMovingAudioComponent->SetSound(ElevatorDoorsOpeningSoundWave);
				ElevatorMovingAudioComponent->Play();
			}
		}
	}
}

bool AElevator::AreDoorsMoving() const
{
	return !(LeftDoor->GetRelativeLocation().Equals(LeftDoorTargetLocation) && RightDoor->GetRelativeLocation().Equals(RightDoorTargetLocation));
}

bool AElevator::AreDoorsClosed() const
{
	return ElevatorDoorsState == EDoorsState::Closed;
}

bool AElevator::IsElevatorMoving() const
{
	return !GetActorLocation().Equals(ElevatorTargetLocation);
}

bool AElevator::HasElevatorPassedStopTime() const
{
	return ElevatorStoppedTime >= ElevatorStoppedDuration;
}

void AElevator::ResetStopTime()
{
	ElevatorStoppedTime = 0.0f;
}

bool AElevator::IsTargetFloorNumberSet() const
{
	return NextTargetFloorNumber != -1;
}

void AElevator::GoToNextFloor(EDirectionState Direction)
{
	if (ElevatorMovingAudioComponent->IsPlaying() && ElevatorMovingAudioComponent->Sound == ElevatorDoorsClosingSoundWave)
		return;
	
	NextTargetFloorNumber = -1;
	
	ElevatorDirection = Direction;
	if (ElevatorDirection == EDirectionState::Up)
	{
		if (CurrentFloorNumber + 1 < MinFloorNumber || CurrentFloorNumber + 1 > MaxFloorNumber)
			return;
		
		CurrentTargetFloorNumber = CurrentFloorNumber + 1;

		if (GetOwner())
		{
			AElevatingActionSecretAgent* ThisSecretAgent = Cast<AElevatingActionSecretAgent>(GetOwner());
			
			if (Cast<APlayerController>(ThisSecretAgent->GetController()))
			{
				if (!ElevatorMovingAudioComponent->IsPlaying() ||
                    (ElevatorMovingAudioComponent->IsPlaying() && ElevatorMovingAudioComponent->Sound == ElevatorMovingDownSoundWave))
				{
					ElevatorMovingAudioComponent->SetSound(ElevatorMovingUpSoundWave);
					ElevatorMovingAudioComponent->Play();
				}
			}
		}
	}
	else if (ElevatorDirection == EDirectionState::Down)
	{
		if (CurrentFloorNumber - 1 < MinFloorNumber || CurrentFloorNumber - 1 > MaxFloorNumber)
			return;
		
		CurrentTargetFloorNumber = CurrentFloorNumber - 1;

		if (GetOwner())
		{
			AElevatingActionSecretAgent* ThisSecretAgent = Cast<AElevatingActionSecretAgent>(GetOwner());
			
			if (Cast<APlayerController>(ThisSecretAgent->GetController()))
			{
				if (!ElevatorMovingAudioComponent->IsPlaying() ||
					(ElevatorMovingAudioComponent->IsPlaying() && ElevatorMovingAudioComponent->Sound == ElevatorMovingUpSoundWave))
				{
					ElevatorMovingAudioComponent->SetSound(ElevatorMovingDownSoundWave);
					ElevatorMovingAudioComponent->Play();
				}
			}
		}
	}

	ElevatorStoppedTime = 0.0f;
}

void AElevator::GoToFloor(int32 FloorNumber)
{
	if (FloorNumber > CurrentFloorNumber)
		ElevatorDirection = EDirectionState::Up;
	else if (FloorNumber < CurrentFloorNumber)
		ElevatorDirection = EDirectionState::Down;
	
	CurrentTargetFloorNumber = FMath::Clamp(FloorNumber, MinFloorNumber, MaxFloorNumber);
	ElevatorStoppedTime = 0.0f;
}

void AElevator::SetTargetFloorNumber(int32 FloorNumber)
{
	NextTargetFloorNumber = FMath::Clamp(FloorNumber, MinFloorNumber, MaxFloorNumber);
}

UElevatorButton* AElevator::GetElevatorButtonCaller() const
{
	return ElevatorButtonCaller;
}

void AElevator::SetElevatorButtonCaller(UElevatorButton* ElevatorButton)
{
	ElevatorButtonCaller = ElevatorButton;
}

int32 AElevator::GetCurrentFloorNumber() const
{
	return CurrentFloorNumber;
}

int32 AElevator::GetTargetFloorNumber() const
{
	return NextTargetFloorNumber == -1 ? CurrentTargetFloorNumber : NextTargetFloorNumber;
}

int32 AElevator::GetMinFloorNumber() const
{
	return MinFloorNumber;
}

int32 AElevator::GetMaxFloorNumber() const
{
	return MaxFloorNumber;
}

UAudioComponent* AElevator::GetElevatorMovingAudioComponent() const
{
	return ElevatorMovingAudioComponent;
}

USoundWave* AElevator::GetElevatorMovingUpSoundWave() const
{
	return ElevatorMovingUpSoundWave;
}

USoundWave* AElevator::GetElevatorMovingDownSoundWave() const
{
	return ElevatorMovingDownSoundWave;
}
