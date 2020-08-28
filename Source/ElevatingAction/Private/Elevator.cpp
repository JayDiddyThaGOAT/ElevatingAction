
#include "Elevator.h"

#include "Kismet/KismetMathLibrary.h"

AElevator::AElevator()
{
    PrimaryActorTick.bCanEverTick = true;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> ElevatorMesh(TEXT("/Game/PolygonOffice/Meshes/Buildings/SM_Bld_Elevator_01"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> ElevatorDoorMesh(TEXT("/Game/PolygonOffice/Meshes/Buildings/SM_Bld_Elevator_01_Door"));

	if (ElevatorMesh.Succeeded())
		GetStaticMeshComponent()->SetStaticMesh(ElevatorMesh.Object);
    GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);

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
			if (IsTargetFloorNumberSet() && CurrentFloorNumber == NextTargetFloorNumber)
				NextTargetFloorNumber = -1;
			
			CurrentFloorNumber = CurrentTargetFloorNumber;
			
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
		}
	}
}

void AElevator::CloseDoors()
{
	LeftDoorTargetLocation = FVector(0.0f, -5.0f, 0.0f);
	RightDoorTargetLocation = FVector(250.0f, -10.0f, 0.0f);
}

void AElevator::OpenDoors()
{
	LeftDoorTargetLocation = FVector(-OpenDoorOffset, -5.0f, 0.0f);
	RightDoorTargetLocation = FVector(250.0f + OpenDoorOffset, -10.0f, 0.0f);
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
	ElevatorDirection = Direction;

	if (ElevatorDirection == EDirectionState::Up)
		CurrentTargetFloorNumber = FMath::Clamp(CurrentFloorNumber + 1, MinFloorNumber, MaxFloorNumber);
	else if (ElevatorDirection == EDirectionState::Down)
		CurrentTargetFloorNumber = FMath::Clamp(CurrentFloorNumber - 1, MinFloorNumber, MaxFloorNumber);

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
