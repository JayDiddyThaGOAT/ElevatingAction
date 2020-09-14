
#include "ElevatingActionSecretAgent.h"

#include "DrawDebugHelpers.h"
#include "ElevatingActionAIController.h"
#include "ElevatingActionGameInstance.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AElevatingActionSecretAgent::AElevatingActionSecretAgent()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bAllowTickBeforeBeginPlay = false;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);

	ConstructorHelpers::FObjectFinder<USkeletalMesh> SecretAgentOttoMesh(TEXT("SkeletalMesh'/Game/ElevatingActionSecretAgent/Character/Player/SK_SecretAgent_Otto.SK_SecretAgent_Otto'"));
	if (SecretAgentOttoMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(SecretAgentOttoMesh.Object);
		GetMesh()->SetGenerateOverlapEvents(true);
	}

	ConstructorHelpers::FClassFinder<UAnimInstance> ElevatingActionAnimBP(TEXT("AnimBlueprint'/Game/ElevatingActionSecretAgent/Character/Animations/ABP_ElevatingActionCharacter.ABP_ElevatingActionCharacter_C'"));
	if (ElevatingActionAnimBP.Succeeded())
		GetMesh()->SetAnimClass(ElevatingActionAnimBP.Class);

	ConstructorHelpers::FObjectFinder<USoundCue> DeathSound(TEXT("SoundCue'/Game/ElevatingActionAudio/GameMasterAudio/SecretAgentVoices/Death/S_Death_Cue.S_Death_Cue'"));
	if (DeathSound.Succeeded())
		DeathCue = DeathSound.Object;

	ConstructorHelpers::FObjectFinder<USoundCue> HurtSound(TEXT("SoundCue'/Game/ElevatingActionAudio/GameMasterAudio/SecretAgentVoices/Hurt/S_Hurt_Cue.S_Hurt_Cue'"));
	if (HurtSound.Succeeded())
		HurtCue = HurtSound.Object;

	ConstructorHelpers::FObjectFinder<USoundWave> ChimeSound(TEXT("SoundWave'/Game/ElevatingActionAudio/GameMasterAudio/SecretFileCollection/collect_item_chime_04.collect_item_chime_04'"));
	if (ChimeSound.Succeeded())
		SecretFileCollectingSound = ChimeSound.Object;

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -90.0f), FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetCollisionProfileName(TEXT("BlockAll"));

	DefaultWalkSpeed = 600.0f;
	GetCharacterMovement()->MaxWalkSpeed = 0.0f;

	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	GetCharacterMovement()->bOrientRotationToMovement = true;

	GetCharacterMovement()->MaxWalkSpeedCrouched = 0.0f;
	GetCharacterMovement()->CrouchedHalfHeight = 56.0f;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

	GetCharacterMovement()->JumpZVelocity = 415.0f;
	GetCharacterMovement()->AirControl = 0.0f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	CollisionQueryParams.AddIgnoredActor(this);
	bCanGoLeft = true;
	bCanGoRight = true;

	CurrentTransition = ETransitionState::None;

	RoomTargetLocation = 500.0f;
	PistolFireRate = 1.0f;
	ProjectilesShotCount = 0;
	ProjectilesShotMax = 3;
	bIsDamaged = false;
}

// Called when the game starts or when spawned
void AElevatingActionSecretAgent::BeginPlay()
{
	Super::BeginPlay();
	
	FActorSpawnParameters WeaponSpawnParameters;
	FTransform WeaponTransform;
	
	Pistol = GetWorld()->SpawnActor<AElevatingActionPistol>(AElevatingActionPistol::StaticClass(), WeaponTransform, WeaponSpawnParameters);
	Pistol->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("Hand_R_PistolSocket"));
	Pistol->SetOwner(this);

	FHitResult GroundHitResult;
	FVector FeetLocation = GetMesh()->GetSocketTransform(TEXT("Root")).GetLocation();

	FCollisionShape SmallSphere = FCollisionShape::MakeSphere(10.0f);

	CurrentFloorNumber = 30 + FMath::FloorToInt(GetMesh()->GetSocketLocation(TEXT("eyes_end")).Z / 300);
	if (CurrentFloorNumber >= 31)
		CurrentLocation = ELocationState::Roof;
	else
	{
		if (GetWorld()->SweepSingleByChannel(GroundHitResult, FeetLocation, FeetLocation, FQuat::Identity, ECC_WorldStatic, SmallSphere, CollisionQueryParams))
		{
			if (GroundHitResult.Component->GetName().Equals("RoomFloors"))
				CurrentLocation = ELocationState::Room;
			else if (GroundHitResult.Actor->GetName().Contains("Street"))
			{
				CurrentLocation = ELocationState::Sidewalk;
				CurrentTransition = ETransitionState::Exit;
			}
			else
				CurrentLocation = ELocationState::Hallway;
		}
	}
}

void AElevatingActionSecretAgent::TraceOfficeWalls()
{
	FHitResult WallHitResult;

	FVector StartLocation = GetMesh()->GetSocketLocation(TEXT("eyes"));
	FVector EndLocation = StartLocation + FVector::LeftVector * 500.0f;

	if (GetWorld()->LineTraceSingleByObjectType(WallHitResult, StartLocation, EndLocation, FCollisionObjectQueryParams::AllObjects, CollisionQueryParams))
	{
		TracedStairs = nullptr;
		TracedStairsLocation = FVector::ZeroVector;
		
		if (Cast<UElevatorButton>(WallHitResult.Component))
		{
			TracedElevatorButton = Cast<UElevatorButton>(WallHitResult.Component);
			AElevator* Elevator = TracedElevatorButton->GetElevator();

			if (Elevator->GetOwner() || (!Elevator->IsElevatorMoving() && CurrentFloorNumber == Elevator->GetCurrentFloorNumber()))
				TracedElevatorButton->SetButtonBrightness(1.0f);
			else
			{
				if (Elevator->IsTargetFloorNumberSet())
				{
					if (CurrentFloorNumber == Elevator->GetTargetFloorNumber())
						TracedElevatorButton->SetButtonBrightness(3.0f);
					else
						TracedElevatorButton->SetButtonBrightness(1.0f);
				}
				else
					TracedElevatorButton->SetButtonBrightness(2.0f);
			}

			TracedElevator = nullptr;
			TracedDoor = nullptr;
		}
		else
		{
			if (TracedElevatorButton)
			{
				TracedElevatorButton->SetButtonBrightness(1.0f);
				TracedElevatorButton = nullptr;
			}

			if (Cast<AElevator>(WallHitResult.Actor))
			{
				if (WallHitResult.Component->GetName().Equals(TEXT("InsideElevatorArea")))
				{
					TracedElevator = Cast<AElevator>(WallHitResult.Actor);
				}
				
				TracedDoor = nullptr;
			}
			else if (Cast<UElevatingActionOfficeDoor>(WallHitResult.Component))
			{
				TracedDoor = Cast<UElevatingActionOfficeDoor>(WallHitResult.Component);
				TracedElevator = nullptr;
			}
			else
			{
				TracedElevator = nullptr;
				TracedDoor = nullptr;
			}
		}
	}
	else
	{
		if (TracedElevatorButton)
		{
			TracedElevatorButton->SetButtonBrightness(1.0f);
			TracedElevatorButton = nullptr;
		}

		TracedElevator = nullptr;
		TracedDoor = nullptr;
	}
}

// Called every frame
void AElevatingActionSecretAgent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(!IsValid(GetCapsuleComponent()))
		return;

	TArray<AActor*> SecretAgents;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), StaticClass(), SecretAgents);
	CollisionQueryParams.AddIgnoredActors(SecretAgents);

	for (AActor* SecretAgentActor : SecretAgents)
	{
		MoveIgnoreActorAdd(SecretAgentActor);

		AElevatingActionSecretAgent* SecretAgent = Cast<AElevatingActionSecretAgent>(SecretAgentActor);
		if (IsValid(SecretAgent))
			CollisionQueryParams.AddIgnoredComponent(SecretAgent->GetMesh());
	}
	
	CurrentFloorNumber = 30 + FMath::FloorToInt(GetMesh()->GetSocketLocation(TEXT("eyes_end")).Z / 300);
	if (CurrentFloorNumber >= 16 && CurrentFloorNumber <= 20)
	{
		FHitResult StairHitResult;
		FVector FeetLocation = GetMesh()->GetSocketTransform(TEXT("Root")).GetLocation();
		FCollisionShape SmallSphere = FCollisionShape::MakeSphere(2.5f);

		if (GetWorld()->SweepSingleByChannel(StairHitResult, FeetLocation, FeetLocation, FQuat::Identity, ECC_Visibility, SmallSphere, CollisionQueryParams))
		{
			if (StairHitResult.Actor->GetName().Contains("Stairs"))
			{
				TracedStairs = StairHitResult.GetActor();
				TracedStairsLocation = StairHitResult.Location;
			}
			else
			{
				TracedStairs = nullptr;
				TracedStairsLocation = FVector::ZeroVector;
			}
		}
	}

	if (CurrentTransition == ETransitionState::None)
	{
		if (CurrentLocation == ELocationState::Roof)
		{
			TraceOfficeWalls();
			
			bCanTransition = false;
			
			AddMovementInput(FVector::ForwardVector);

			if (TracedElevator)
			{
				TracedElevator->SetOwner(this);
				CurrentTransition = ETransitionState::Enter;
			}
		}
		else if (CurrentLocation == ELocationState::Room)
			bCanTransition = true;
		else if (CurrentLocation == ELocationState::Elevator && TracedElevator)
		{
			AddMovementInput(FVector::RightVector);
			bCanTransition = CurrentFloorNumber <= 30 && !TracedElevator->IsElevatorMoving() && !TracedElevator->AreDoorsMoving() && TracedElevator->AreDoorsClosed();
		}
		else if (CurrentLocation == ELocationState::Hallway)
		{
			TraceOfficeWalls();
			
			SetActorLocation(FVector(GetActorLocation().X, 125.0f, GetActorLocation().Z));

			FHitResult LeftHitResult, RightHitResult;

			FVector StartLocation = GetMesh()->GetSocketLocation(TEXT("spine_01"));
			FVector LeftEndLocation = StartLocation + FVector::BackwardVector * 125.0f;
			FVector RightEndLocation = StartLocation + FVector::ForwardVector * 125.0f;
			
			bool bIsFacingLeft = FMath::RoundToInt(GetActorRotation().Yaw) != -180;
			bool bIsLeftSideBlocked = GetWorld()->LineTraceSingleByChannel(LeftHitResult, StartLocation, LeftEndLocation, ECC_Visibility, CollisionQueryParams);
			if (bIsLeftSideBlocked)
				bCanGoLeft = bIsFacingLeft;
			else
				bCanGoLeft = true;

			bool bIsFacingRight = FMath::RoundToInt(GetActorRotation().Yaw) != 0;
			bool bIsRightSideBlocked = GetWorld()->LineTraceSingleByChannel(RightHitResult, StartLocation, RightEndLocation, ECC_Visibility, CollisionQueryParams);
			if (bIsRightSideBlocked)
				bCanGoRight = bIsFacingRight;
			else
				bCanGoRight = true;
			
			if (GetCharacterMovement()->IsFalling() || bWasJumping)
			{
				if (!(bCanGoLeft && bCanGoRight))
					GetCharacterMovement()->Velocity = FVector(0.0f, 0.0f, GetCharacterMovement()->Velocity.Z);
			}

			if (TracedElevator)
				bCanTransition = TracedElevator->GetOwner() == nullptr &&
								!TracedElevator->IsElevatorMoving() &&
								!TracedElevator->AreDoorsClosed() &&
								!TracedElevator->HasElevatorPassedStopTime();
			else if (TracedElevatorButton)
				bCanTransition = UKismetMathLibrary::EqualEqual_FloatFloat(TracedElevatorButton->GetButtonBrightness(), 2.0f);
			else if (TracedStairs)
			{
				if (TracedStairs->GetName().Contains("Left"))
				{
					bCanGoDownStairs = UKismetMathLibrary::InRange_FloatFloat(FMath::Abs(TracedStairsLocation.X), 1500.0f, 1750.0f) && CurrentFloorNumber > 17;
					bCanGoUpStairs = UKismetMathLibrary::InRange_FloatFloat(FMath::Abs(TracedStairsLocation.X), 750.0f, 1000.0f) && CurrentFloorNumber < 20;
				}
				else if (TracedStairs->GetName().Contains("Right"))
				{
					bCanGoDownStairs = UKismetMathLibrary::InRange_FloatFloat(FMath::Abs(TracedStairsLocation.X), 1750.0f, 2000.0f) && CurrentFloorNumber > 16;
					bCanGoUpStairs = UKismetMathLibrary::InRange_FloatFloat(FMath::Abs(TracedStairsLocation.X), 1000.0f, 1250.0f) && CurrentFloorNumber < 20;
				}

				if (Cast<APlayerController>(GetController()))
					bCanTransition = bCanGoUpStairs || bCanGoDownStairs;
			}
		}
	}

	else if (CurrentTransition == ETransitionState::Enter)
	{
		if (TracedElevator)
		{
			FVector TargetLocation = FVector(TracedElevator->GetActorLocation().X + 125.0f, TracedElevator->GetActorLocation().Y - 125.0f, GetActorLocation().Z);
			FVector DirectionToTarget = (TargetLocation - GetActorLocation()).GetSafeNormal();
			AddMovementInput(DirectionToTarget);

			if (GetLastMovementInputVector().Y > 0.0f)
			{
				TracedElevator->CloseDoors();
				
				bCanGoLeft = false;
				bCanGoRight = false;

				GetCharacterMovement()->MaxWalkSpeed = 0.0f;
				CurrentLocation = ELocationState::Elevator;
				CurrentTransition = ETransitionState::None;
			}
		}
		else if (TracedStairs)
		{
			bCanGoLeft = true;
			bCanGoRight = true;

			if (GetActorLocation().Y <= -125.0f && bCanGoDownStairs || GetActorLocation().Y <= -250.0f && bCanGoUpStairs)
			{
				CurrentLocation = ELocationState::Stairs;
				
				if (bCanGoDownStairs)
				{
					if (TracedStairs->GetName().Contains("Left"))
					{
						AddMovementInput(FVector::ForwardVector);

						if (GetActorLocation().X >= -1000.0f + GetCapsuleComponent()->GetUnscaledCapsuleRadius())
							CurrentTransition = ETransitionState::Exit;
					}
					else if (TracedStairs->GetName().Contains("Right"))
					{
						AddMovementInput(FVector::BackwardVector);

						if (GetActorLocation().X <= 1250.0f - GetCapsuleComponent()->GetUnscaledCapsuleRadius())
							CurrentTransition = ETransitionState::Exit;
					}
				}
				else if (bCanGoUpStairs)
				{
					if (TracedStairs->GetName().Contains("Left"))
					{
						AddMovementInput(FVector::BackwardVector);

						if (GetActorLocation().X <= -1500.0f + GetCapsuleComponent()->GetUnscaledCapsuleRadius())
							CurrentTransition = ETransitionState::Exit;
					}
					else if (TracedStairs->GetName().Contains("Right"))
					{
						AddMovementInput(FVector::ForwardVector);

						if (GetActorLocation().X >= 1750.0f - GetCapsuleComponent()->GetUnscaledCapsuleRadius())
							CurrentTransition = ETransitionState::Exit;
					}
				}
			}
			else
			{
				AddMovementInput(FVector::LeftVector);
			}
		}
		else if (TracedDoor)
		{
			bCanGoLeft = false;
			bCanGoRight = false;

			FVector TargetLocation = FVector(
				TracedDoor->GetOwner()->GetActorLocation().X + 125.0f,
				TracedDoor->GetComponentLocation().Y - RoomTargetLocation,
				GetActorLocation().Z);
			FVector DirectionToTarget = (TargetLocation - GetActorLocation()).GetSafeNormal();
			AddMovementInput(DirectionToTarget);

			if (GetLastMovementInputVector().Y > 0.0f)
			{
				TracedDoor->Close();

				GetCharacterMovement()->MaxWalkSpeed = 0.0f;

				CurrentLocation = ELocationState::Room;
				CurrentTransition = ETransitionState::None;
			}
		}
	}
	else if (CurrentTransition == ETransitionState::Exit)
	{
		if (CurrentLocation != ELocationState::Sidewalk)
		{
			if (TracedElevator)
			{
				if (!TracedElevator->AreDoorsMoving() && !TracedElevator->AreDoorsClosed())
					GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
				else
					GetCharacterMovement()->MaxWalkSpeed = 0.0f;
			}

			float TargetLocationY = CurrentFloorNumber == 0 ? 375.0f : 125.0f;

			FVector TargetLocation = FVector(GetActorLocation().X, TargetLocationY, GetActorLocation().Z);
			FVector DirectionToTarget = (TargetLocation - GetActorLocation()).GetSafeNormal();
			AddMovementInput(DirectionToTarget);

			if (GetLastMovementInputVector().Y < 0.0f)
			{
				if (TracedDoor)
					TracedDoor->Close();
				else if (TracedElevator && TracedElevator->GetOwner())
					TracedElevator->SetOwner(nullptr);
					

				if (CurrentFloorNumber == 0)
					CurrentLocation = ELocationState::Sidewalk;
				else
				{
					CurrentLocation = ELocationState::Hallway;
					CurrentTransition = ETransitionState::None;
				}
			}
		}
	}

	GetCharacterMovement()->NavAgentProps.bCanJump = CurrentLocation == ELocationState::Hallway && CurrentTransition == ETransitionState::None;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = CurrentLocation == ELocationState::Hallway && CurrentTransition == ETransitionState::None;
}

// Called to bind functionality to input
void AElevatingActionSecretAgent::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("Shoot"), EInputEvent::IE_Pressed, this, &AElevatingActionSecretAgent::ShootPistol);

	PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &AElevatingActionSecretAgent::Jump);
	PlayerInputComponent->BindAction(TEXT("Crouch"), EInputEvent::IE_Pressed, this, &AElevatingActionSecretAgent::ToggleCrouch);
	PlayerInputComponent->BindAction(TEXT("Transition"), EInputEvent::IE_Pressed, this, &AElevatingActionSecretAgent::Transition);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AElevatingActionSecretAgent::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveUp"), this, &AElevatingActionSecretAgent::MoveUp);
}

float AElevatingActionSecretAgent::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                              AController* EventInstigator, AActor* DamageCauser)
{
	if (GetMesh()->IsAnySimulatingPhysics())
		return 0;

	float DamageTaken = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	Pistol->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Pistol->GetSkeletalMeshComponent()->SetSimulatePhysics(true);
	Pistol->GetSkeletalMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));

	UElevatingActionGameInstance* GameInstance = Cast<UElevatingActionGameInstance>(GetWorld()->GetGameInstance());
	if (!Cast<APlayerController>(GetController()))
	{
		if (GameInstance)
		{
			int32 Score;
			if (GameInstance->IsOfficeBlackedOut())
				Score = FMath::RoundToInt(DamageTaken) + 50;
			else
				Score = FMath::RoundToInt(DamageTaken);

			GameInstance->AddPlayerScore(Score);
			GameInstance->SetCurrentSecretAgentsMoving(GameInstance->GetCurrentSecretAgentsMoving() - 1);
		}
	}
	else
		GameInstance->SetNumberOfPlayerLives(GameInstance->GetNumberOfPlayerLives() - 1);

	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->DestroyComponent();

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetAnimClass(nullptr);
	GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	if (TracedElevator)
	{
		if (TracedElevator->GetOwner() == this)
			TracedElevator->SetOwner(nullptr);

		if (!TracedElevator->HasElevatorPassedStopTime())
			TracedElevator->ResetStopTime();
		
		TracedElevator = nullptr;
	}
	else if (TracedDoor)
	{
		TracedDoor->Close();
		TracedDoor = nullptr;
	}
	else if (TracedStairs)
	{
		bCanGoDownStairs = false;
		bCanGoUpStairs = false;
		
		TracedStairsLocation = FVector::ZeroVector;
		TracedStairs = nullptr;
	}
	else if (IsValid(TracedElevatorButton))
	{
		TracedElevatorButton->SetButtonBrightness(1.0f);
		TracedElevatorButton = nullptr;
	}

	if (Cast<APlayerController>(GetController()))
	{
		if (GameInstance)
		{
			if (GameInstance->GetNumberOfPlayerLives() - 1 > 0)
			{
				if (IsValid(HurtCue))
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), HurtCue, GetMesh()->GetSocketLocation(TEXT("head")));
			}
			else
			{
				if (IsValid(DeathCue))
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), DeathCue, GetMesh()->GetSocketLocation(TEXT("head")));
			}
		}
	}
	else
	{
		if (IsValid(DeathCue))
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), DeathCue, GetMesh()->GetSocketLocation(TEXT("head")));

		if (IsValid(GetController()))
			GetController()->Destroy();
	}
	
	bIsDamaged = true;
	return DamageTaken;
}

void AElevatingActionSecretAgent::Destroyed()
{
	Super::Destroyed();

	UElevatingActionGameInstance* GameInstance = Cast<UElevatingActionGameInstance>(GetWorld()->GetGameInstance());
	if (GameInstance)
		GameInstance->SetCurrentSecretAgentsMoving(GameInstance->GetCurrentSecretAgentsMoving() - 1);

	if (IsValid(Pistol))
		Pistol->Destroy();

	if (IsValid(TracedElevator))
	{
		if (!TracedElevator->HasElevatorPassedStopTime())
			TracedElevator->ResetStopTime();

		TracedElevator->SetOwner(nullptr);
		TracedElevator = nullptr;
	}
	else if (IsValid(TracedDoor))
	{
		TracedDoor->Close();
		TracedDoor = nullptr;
	}
	else if (IsValid(TracedStairs))
	{
		bCanGoDownStairs = false;
		bCanGoUpStairs = false;
		
		TracedStairsLocation = FVector::ZeroVector;
		TracedStairs = nullptr;
	}
	else if (IsValid(TracedElevatorButton))
	{
		TracedElevatorButton->SetButtonBrightness(1.0f);
		TracedElevatorButton = nullptr;
	}

	if (Cast<AElevatingActionAIController>(GetController()))
	{
		if (IsValid(GetController()))
			GetController()->Destroy();
	}
}

bool AElevatingActionSecretAgent::IsDamaged() const
{
	return bIsDamaged;
}

void AElevatingActionSecretAgent::Transition()
{
	if (!bCanTransition)
		return;

	if (GetCharacterMovement()->IsFalling() || GetCharacterMovement()->IsCrouching())
		return;

	if (CurrentLocation == ELocationState::Hallway)
	{
		if (TracedElevator)
		{
			TracedElevator->SetOwner(this);

			CurrentTransition = ETransitionState::Enter;
			bCanTransition = false;
		}
		else if (TracedElevatorButton)
		{
			TracedElevatorButton->CallElevatorTo(CurrentFloorNumber);
		}
		else if (TracedDoor)
		{
			TracedDoor->Open();

			if (Cast<APlayerController>(GetController()))
				UGameplayStatics::PlaySound2D(GetWorld(), SecretFileCollectingSound);

			CurrentTransition = ETransitionState::Enter;
			bCanTransition = false;
		}
		else if (TracedStairs)
		{
			CurrentTransition = ETransitionState::Enter;
			bCanTransition = false;
		}

	}
	else if (CurrentLocation == ELocationState::Room)
	{
		if (TracedDoor)
		{
			TracedDoor->Open();
			GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;

			CurrentTransition = ETransitionState::Exit;
			bCanTransition = false;
		}
	}
	else if (CurrentLocation == ELocationState::Elevator)
	{
		if (TracedElevator)
		{
			TracedElevator->OpenDoors();
			TracedElevator->ResetStopTime();

			CurrentTransition = ETransitionState::Exit;
			bCanTransition = false;
		}
	}
}

void AElevatingActionSecretAgent::ShootPistol()
{
	if (!(bCanGoLeft && bCanGoRight))
		return;
	
	if (!(CurrentLocation == ELocationState::Hallway && CurrentTransition == ETransitionState::None))
		return;

	if (ProjectilesShotCount >= ProjectilesShotMax)
		return;

	if (!bShootButtonPressed)
		bShootButtonPressed = true;
}

void AElevatingActionSecretAgent::StartTransition()
{
	bCanTransition = true;
}

void AElevatingActionSecretAgent::ToggleCrouch()
{
	if (GetCharacterMovement()->IsFalling())
		return;

	if (!GetCharacterMovement()->IsCrouching())
		Crouch();
	else
		UnCrouch();
}

void AElevatingActionSecretAgent::MoveForward(float AxisValue)
{
	if (AxisValue == 0.0f)
		return;

	if (CurrentFloorNumber < 1)
		return;

	if (CurrentLocation == ELocationState::Hallway && CurrentTransition == ETransitionState::None)
		AddMovementInput(FVector::ForwardVector, AxisValue - (bCanGoLeft - bCanGoRight));
}

void AElevatingActionSecretAgent::MoveUp(float AxisValue)
{
	if (!IsValid(TracedElevator))
		return;

	if (AxisValue == 0)
		return;

	if (CurrentLocation == ELocationState::Elevator)
	{
		if (bCanTransition)
		{
			if (AxisValue > 0.0f)
				TracedElevator->GoToNextFloor(EDirectionState::Up);
			else if (AxisValue < 0.0f)
				TracedElevator->GoToNextFloor(EDirectionState::Down);
		}
	}
}

bool AElevatingActionSecretAgent::IsShootButtonPressed() const
{
	return bShootButtonPressed;
}

bool AElevatingActionSecretAgent::CanGoLeft() const
{
	return bCanGoLeft;
}

bool AElevatingActionSecretAgent::CanGoRight() const
{
	return bCanGoRight;
}

float AElevatingActionSecretAgent::GetDefaultWalkSpeed() const
{
	return DefaultWalkSpeed;
}

ETransitionState AElevatingActionSecretAgent::GetCurrentTransition() const
{
	return CurrentTransition;
}

ELocationState AElevatingActionSecretAgent::GetCurrentLocation() const
{
	return CurrentLocation;
}

FCollisionQueryParams AElevatingActionSecretAgent::GetCollisionQueryParams() const
{
	return CollisionQueryParams;
}

bool AElevatingActionSecretAgent::CanTransition() const
{
	return bCanTransition;
}

bool AElevatingActionSecretAgent::CanGoUpStairs() const
{
	return bCanGoUpStairs;
}

bool AElevatingActionSecretAgent::CanGoDownStairs() const
{
	return bCanGoDownStairs;
}

AElevator* AElevatingActionSecretAgent::GetTracedElevator() const
{
	return TracedElevator;
}

UElevatorButton* AElevatingActionSecretAgent::GetTracedElevatorButton() const
{
	return TracedElevatorButton;
}

AActor* AElevatingActionSecretAgent::GetTracedStairs() const
{
	return TracedStairs;
}

FVector AElevatingActionSecretAgent::GetTracedStairsLocation() const
{
	return TracedStairsLocation;
}

UElevatingActionOfficeDoor* AElevatingActionSecretAgent::GetTracedDoor() const
{
	return TracedDoor;
}

int32 AElevatingActionSecretAgent::GetCurrentFloorNumber() const
{
	return CurrentFloorNumber;
}

void AElevatingActionSecretAgent::SetShootButtonPressed(bool bButtonPressed)
{
	bShootButtonPressed = bButtonPressed;
}
