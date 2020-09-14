
#include "ElevatingActionAIController.h"
#include "ElevatingActionGameInstance.h"
#include "Components/CapsuleComponent.h"
#include "Elevator.h"
#include "ElevatorButton.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AElevatingActionAIController::AElevatingActionAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    ShootPistolTime = 0.0f;
    ShootPistolDelay = 2.0f;
    MaxShootPistolDistance = 250.0f;
    
    PercentChanceAIShootPlayerWhileMoving = 0.0f;
    PercentRequiredAIShootPlayerWhileMoving = 0.0f;

    PercentChanceAIDodgesPlayerProjectiles = 0.0f;
    PercentRequiredAIDodgesPlayerProjectiles = 0.0f;
    
    PercentChanceAIFollowsPlayerCrouching = 0.0f;
    PercentRequiredAIFollowsPlayerCrouching = 0.0f;

    bCanGoToSecretAgentOtto = true;
    bBlockedByWall = false;
}

bool AElevatingActionAIController::IsPatrollingLong()
{
    ELocationState SecretAgentAILocation = SecretAgentAI->GetCurrentLocation();
    bool bIsAbovePlayer = SecretAgentAI->GetCurrentFloorNumber() > SecretAgentOtto->GetCurrentFloorNumber();
    return !SecretAgentAI->WasRecentlyRendered(5.0f) && bIsAbovePlayer && SecretAgentAILocation == ELocationState::Hallway;
}

float AElevatingActionAIController::GetTargetLocationToUseStairs(AActor* Stairs, bool CanGoUpStairs, bool CanGoDownStairs) const
{
    float TargetLocationX = 0.0f;

    if (Stairs)
    {
        if (Stairs->GetName().Contains("Left"))
        {
            if (CanGoUpStairs)
                TargetLocationX = (750.0f + 1000.0f) / 2.0f;
            else if (CanGoDownStairs)
                TargetLocationX = (1500.0f + 1750.0f) / 2.0f;
        }
        else if (Stairs->GetName().Contains("Right"))
        {
            if (CanGoUpStairs)
                TargetLocationX = (1000.0f + 1250.0f) / 2.0f;
            else if (CanGoDownStairs)
                TargetLocationX = (1750.0f + 2000.0f) / 2.0f;
        }
    }

    return TargetLocationX;
}

FHitResult AElevatingActionAIController::ObjectBetweenSecretAgents()
{
    FHitResult HitResult;
    FVector StartLocation = SecretAgentAI->GetMesh()->GetSocketLocation(TEXT("Hand_R"));
    FVector EndLocation = SecretAgentOtto->GetMesh()->GetSocketLocation(TEXT("Hand_R"));

    FCollisionQueryParams CollisionQueryParams;
    TArray<AActor*> SecretAgents;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AElevatingActionSecretAgent::StaticClass(), SecretAgents);
    for (AActor* SecretAgent : SecretAgents)
    {
        if (SecretAgent != SecretAgentOtto)
            CollisionQueryParams.AddIgnoredActor(SecretAgent);
    }

    FCollisionObjectQueryParams CollisionObjectQueryParams;
    CollisionObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);

    GetWorld()->LineTraceSingleByObjectType(HitResult, StartLocation, EndLocation, CollisionObjectQueryParams, CollisionQueryParams);

    return HitResult;
}

void AElevatingActionAIController::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
    Super::TickActor(DeltaTime, TickType, ThisTickFunction);

    if (!IsValid(SecretAgentAI) && !IsValid(SecretAgentOtto))
    {
        SecretAgentAI = Cast<AElevatingActionSecretAgent>(GetCharacter());
        SecretAgentOtto = Cast<AElevatingActionSecretAgent>(GetWorld()->GetFirstPlayerController()->GetCharacter());
    }
    else if (!SecretAgentAI->IsDamaged())
    {
        if (!SecretAgentOtto->IsDamaged())
        {
            ETransitionState SecretAgentAITransition = SecretAgentAI->GetCurrentTransition();
            ELocationState SecretAgentAILocation = SecretAgentAI->GetCurrentLocation();

            ETransitionState SecretAgentOttoTransition = SecretAgentOtto->GetCurrentTransition();
            ELocationState SecretAgentOttoLocation = SecretAgentOtto->GetCurrentLocation();

            int32 SecretAgentAIFloorNumber = SecretAgentAI->GetCurrentFloorNumber();
            int32 SecretAgentOttoFloorNumber = SecretAgentOtto->GetCurrentFloorNumber();

            bCanGoLeft = SecretAgentAI->CanGoLeft();
            bCanGoRight = SecretAgentAI->CanGoRight();

            bool bIsOfficeBlackedOut = Cast<UElevatingActionGameInstance>(GetGameInstance())->IsOfficeBlackedOut();
        
            if (SecretAgentAITransition == ETransitionState::None)
            {
                if (SecretAgentAILocation == ELocationState::Hallway)
                {
                    if (SecretAgentAIFloorNumber == SecretAgentOttoFloorNumber &&
                        SecretAgentAITransition == SecretAgentOttoTransition &&
                        SecretAgentAILocation == SecretAgentOttoLocation)
                    {
                        if (ObjectBetweenSecretAgents().Actor == SecretAgentOtto)
                        {
                            if (SecretAgentAI->GetCharacterMovement()->IsCrouching() && IsValid(SecretAgentOttoLastShotProjectile))
                            {
                                FVector ProjectileLocation = SecretAgentOttoLastShotProjectile->GetActorForwardVector();
                                FVector DirectionToProjectile = (SecretAgentAI->GetActorLocation() - SecretAgentOttoLastShotProjectile->GetActorLocation()).GetSafeNormal();
                                if (FVector::DotProduct(ProjectileLocation, DirectionToProjectile) < 0.0f)
                                    SecretAgentAI->ToggleCrouch();
                            }
                            
                            if (SecretAgentAI->GetActorLocation().X > SecretAgentOtto->GetActorLocation().X)
                                DirectionVector = FVector::BackwardVector;
                            else if (SecretAgentAI->GetActorLocation().X < SecretAgentOtto->GetActorLocation().X)
                                DirectionVector = FVector::ForwardVector;
                            
                            bCanGoToSecretAgentOtto = true;
                            bBlockedByWall = false;
                           
                            float DistanceRequiredToShootPlayer = !bIsOfficeBlackedOut ? MaxShootPistolDistance : MaxShootPistolDistance - 125.0f;
                        
                            float DistanceBetweenAgents = FVector::Distance(SecretAgentOtto->GetActorLocation(), SecretAgentAI->GetActorLocation());
                            if (DistanceBetweenAgents <= DistanceRequiredToShootPlayer)
                            {
                                if (PercentRequiredAIShootPlayerWhileMoving >= PercentChanceAIShootPlayerWhileMoving)
                                    SecretAgentAI->GetCharacterMovement()->MaxWalkSpeed = 0.0f;
                                else
                                    SecretAgentAI->GetCharacterMovement()->MaxWalkSpeed = SecretAgentAI->GetDefaultWalkSpeed();
                                
                                float CurrentShootPistolDelay = !bIsOfficeBlackedOut ? ShootPistolDelay : ShootPistolDelay + 2.0f;
                            
                                ShootPistolTime += DeltaTime;
                                if (ShootPistolTime >= CurrentShootPistolDelay)
                                {
                                    SecretAgentAI->ShootPistol();
                                    ShootPistolTime = 0.0f;
                                    
                                    PercentChanceAIFollowsPlayerCrouching = UKismetMathLibrary::RandomFloat();
                                    PercentChanceAIDodgesPlayerProjectiles = UKismetMathLibrary::RandomFloat();
                                }
                                else
                                {
                                    if (PercentChanceAIFollowsPlayerCrouching > PercentRequiredAIFollowsPlayerCrouching)
                                    {
                                        if (SecretAgentAI->GetCharacterMovement()->IsCrouching() != SecretAgentOtto->GetCharacterMovement()->IsCrouching())
                                        {
                                            SecretAgentAI->ToggleCrouch();
                                            PercentChanceAIFollowsPlayerCrouching = UKismetMathLibrary::RandomFloat();
                                        }
                                    }
                                }
                            }
                            else
                            {
                                SecretAgentAI->GetCharacterMovement()->MaxWalkSpeed = SecretAgentAI->GetDefaultWalkSpeed();
                                
                                if (PercentRequiredAIShootPlayerWhileMoving < PercentChanceAIShootPlayerWhileMoving)
                                {
                                    float CurrentShootPistolDelay = !bIsOfficeBlackedOut ? ShootPistolDelay : ShootPistolDelay + 2.0f;
                            
                                    ShootPistolTime += DeltaTime;
                                    if (ShootPistolTime >= CurrentShootPistolDelay)
                                    {
                                        SecretAgentAI->ShootPistol();
                                        ShootPistolTime = 0.0f;
                                        
                                        PercentChanceAIFollowsPlayerCrouching = UKismetMathLibrary::RandomFloat();
                                        PercentChanceAIDodgesPlayerProjectiles = UKismetMathLibrary::RandomFloat();
                                    }
                                    else
                                    {
                                        if (PercentChanceAIFollowsPlayerCrouching > PercentRequiredAIFollowsPlayerCrouching)
                                        {
                                            if (SecretAgentAI->GetCharacterMovement()->IsCrouching() != SecretAgentOtto->GetCharacterMovement()->IsCrouching())
                                            {
                                                SecretAgentAI->ToggleCrouch();
                                                PercentChanceAIFollowsPlayerCrouching = UKismetMathLibrary::RandomFloat();
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else if (ObjectBetweenSecretAgents().Actor->GetInstigator() == SecretAgentOtto)
                        {
                            if (PercentRequiredAIDodgesPlayerProjectiles < PercentChanceAIDodgesPlayerProjectiles)
                            {
                                SecretAgentOttoLastShotProjectile = ObjectBetweenSecretAgents().GetActor();
                               
                                if (!SecretAgentOtto->GetCharacterMovement()->IsCrouching())
                                {
                                    if (!SecretAgentAI->GetCharacterMovement()->IsCrouching() && ObjectBetweenSecretAgents().Distance <= 250.0f)
                                    {
                                        SecretAgentAI->ToggleCrouch();
                                        PercentChanceAIDodgesPlayerProjectiles = UKismetMathLibrary::RandomFloat();
                                    }
                                }
                                else
                                {
                                    if (!SecretAgentAI->GetCharacterMovement()->IsFalling() && ObjectBetweenSecretAgents().Distance <= 250.0f)
                                    {
                                        SecretAgentAI->Jump();
                                        PercentChanceAIDodgesPlayerProjectiles = UKismetMathLibrary::RandomFloat();
                                    }
                                }
                            }
                        }
                        else if (ObjectBetweenSecretAgents().Actor->GetName().Contains("Office"))
                        {
                            if (SecretAgentAI->GetCharacterMovement()->IsCrouching())
                            {
                                if (IsValid(SecretAgentOttoLastShotProjectile))
                                {
                                    FVector ProjectileLocation = SecretAgentOttoLastShotProjectile->GetActorForwardVector();
                                    FVector DirectionToProjectile = (SecretAgentAI->GetActorLocation() - SecretAgentOttoLastShotProjectile->GetActorLocation()).GetSafeNormal();
                                    if (FVector::DotProduct(ProjectileLocation, DirectionToProjectile) < 0.0f)
                                        SecretAgentAI->ToggleCrouch();
                                }
                            }
                            
                            SecretAgentAI->GetCharacterMovement()->MaxWalkSpeed = SecretAgentAI->GetDefaultWalkSpeed();
                            
                            bCanGoToSecretAgentOtto = false;
                            bBlockedByWall = true;

                            if (SecretAgentAI->GetActorLocation().X < SecretAgentOtto->GetActorLocation().X)
                                DirectionVector = FVector::BackwardVector;
                            else if (SecretAgentAI->GetActorLocation().X > SecretAgentOtto->GetActorLocation().X)
                                DirectionVector = FVector::ForwardVector;
                        
                            AActor* TracedStairs = SecretAgentAI->GetTracedStairs();
                            FVector TracedStairsLocation = SecretAgentAI->GetTracedStairsLocation();

                            if (!bIsOfficeBlackedOut)
                            {
                                if (TracedStairs)
                                {
                                    if (SecretAgentAI->CanGoDownStairs())
                                    {
                                        bShouldGoDownStairs = true;
                                        bShouldGoUpStairs = false;
                                    }
                                    else if (SecretAgentAI->CanGoUpStairs())
                                    {
                                        bShouldGoDownStairs = false;
                                        bShouldGoUpStairs = true;
                                    }

                                    float TargetLocationX = GetTargetLocationToUseStairs(TracedStairs, bShouldGoUpStairs, bShouldGoDownStairs);

                                    bool bInMiddleOfStairPlatform = UKismetMathLibrary::NearlyEqual_FloatFloat
                                    (FMath::Abs(TracedStairsLocation.X), TargetLocationX, 15.0f);

                                    if ((bShouldGoUpStairs || bShouldGoDownStairs) && bInMiddleOfStairPlatform)
                                    {
                                        SecretAgentAI->StartTransition();
                                        SecretAgentAI->Transition();
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if (!(bCanGoLeft && bCanGoRight))
                        {
                            bBlockedByWall = true;
                            
                            if (DirectionVector == FVector::BackwardVector)
                                DirectionVector = FVector::ForwardVector;
                            else if (DirectionVector == FVector::ForwardVector)
                                DirectionVector = FVector::BackwardVector;
                        }

                        if (SecretAgentAI->GetCharacterMovement()->IsCrouching())
                            SecretAgentAI->ToggleCrouch();
                        
                        UElevatorButton* TracedElevatorButton = SecretAgentAI->GetTracedElevatorButton();
                        AElevator* TracedElevator = SecretAgentAI->GetTracedElevator();

                        AActor* TracedStairs = SecretAgentAI->GetTracedStairs();
                        FVector TracedStairsLocation = SecretAgentAI->GetTracedStairsLocation();

                        if (SecretAgentAIFloorNumber != SecretAgentOttoFloorNumber && !bIsOfficeBlackedOut)
                        {
                            if (TracedElevatorButton)
                            {
                                bBlockedByWall = false;
                        
                                AElevator* Elevator = TracedElevatorButton->GetElevator();
                        
                                float DistanceX = (TracedElevatorButton->GetComponentLocation() - SecretAgentAI->GetActorLocation()).X;
                                float TracedElevatorButtonBrightness = TracedElevatorButton->GetButtonBrightness();

                                if (TracedElevatorButtonBrightness == 2.0f)
                                {
                                    int32 ElevatorMinFloorNumber = Elevator->GetMinFloorNumber();
                                    int32 ElevatorMaxFloorNumber = Elevator->GetMaxFloorNumber();
                       
                                    bool bElevatorCantGoDownToOtto = SecretAgentAIFloorNumber == ElevatorMinFloorNumber && SecretAgentAIFloorNumber > SecretAgentOttoFloorNumber;
                                    bool bElevatorCantGoUpToOtto = SecretAgentAIFloorNumber == ElevatorMaxFloorNumber && SecretAgentAIFloorNumber < SecretAgentOttoFloorNumber;
                       
                                    if (UKismetMathLibrary::NearlyEqual_FloatFloat(DistanceX, 0.0f, 1.0f) && !bElevatorCantGoDownToOtto && !bElevatorCantGoUpToOtto)
                                    {
                                        SecretAgentAI->Transition();
                                        SecretAgentAI->GetCharacterMovement()->MaxWalkSpeed = 0.0f;
                                    }
                                }
                                else if (TracedElevatorButtonBrightness == 1.0f && SecretAgentAI->GetCharacterMovement()->MaxWalkSpeed == 0.0f)
                                {
                                    SecretAgentAI->GetCharacterMovement()->MaxWalkSpeed = SecretAgentAI->GetDefaultWalkSpeed();

                                    if (Elevator->GetActorLocation().X + 125.0f > SecretAgentAI->GetActorLocation().X)
                                        DirectionVector = FVector::ForwardVector;
                                    else if (Elevator->GetActorLocation().X + 125.0f < SecretAgentAI->GetActorLocation().X)
                                        DirectionVector = FVector::BackwardVector;
                                }
                            }
                            else if (TracedElevator && SecretAgentAI->CanTransition())
                            {
                                SecretAgentAI->GetCharacterMovement()->MaxWalkSpeed = SecretAgentAI->GetDefaultWalkSpeed();
                            
                                AElevator* Elevator = SecretAgentAI->GetTracedElevator();
                                int32 ElevatorMinFloorNumber = Elevator->GetMinFloorNumber();
                                int32 ElevatorMaxFloorNumber = Elevator->GetMaxFloorNumber();
                        
                                float DistanceX = ((TracedElevator->GetActorLocation() + FVector::ForwardVector * 125.0f) - SecretAgentAI->GetActorLocation()).X;

                                bool bAtMiddleOfElevator = UKismetMathLibrary::NearlyEqual_FloatFloat(DistanceX, 0.0f, 15.0f);
                                bool bCantGoLeftOrRight = !(bCanGoLeft && bCanGoRight);
                                bool bElevatorCantGoDownToOtto = SecretAgentAIFloorNumber == ElevatorMinFloorNumber && SecretAgentAIFloorNumber > SecretAgentOttoFloorNumber;
                                bool bElevatorCantGoUpToOtto = SecretAgentAIFloorNumber == ElevatorMaxFloorNumber && SecretAgentAIFloorNumber < SecretAgentOttoFloorNumber;
                                if ((bCantGoLeftOrRight || bAtMiddleOfElevator) && !bElevatorCantGoDownToOtto && !bElevatorCantGoUpToOtto)
                                    SecretAgentAI->Transition();
                            }
                            else if (TracedStairs)
                            {
                                SecretAgentAI->GetCharacterMovement()->MaxWalkSpeed = SecretAgentAI->GetDefaultWalkSpeed();
                            
                                bShouldGoUpStairs = SecretAgentAI->CanGoUpStairs() && SecretAgentAIFloorNumber < SecretAgentOttoFloorNumber;
                                bShouldGoDownStairs = SecretAgentAI->CanGoDownStairs() && SecretAgentAIFloorNumber > SecretAgentOttoFloorNumber;

                                float TargetLocationX = GetTargetLocationToUseStairs(TracedStairs, bShouldGoUpStairs, bShouldGoDownStairs);

                                bool bInMiddleOfStairPlatform = UKismetMathLibrary::NearlyEqual_FloatFloat(FMath::Abs(TracedStairsLocation.X), TargetLocationX, 15.0f);
                                bool bBothAgentsOnLeftSide = SecretAgentAI->GetActorLocation().X < 0 && SecretAgentOtto->GetActorLocation().X < 0;
                                bool bBothAgentsOnRightSide = SecretAgentAI->GetActorLocation().X > 0 && SecretAgentOtto->GetActorLocation().X > 0;
                                bool bOnBottomLeftStairs = TracedStairs->GetName().Contains("Left") && SecretAgentAI->GetCurrentFloorNumber() == 17;
                                bool bOnBottomRightStairs = TracedStairs->GetName().Contains("Right") && SecretAgentAI->GetCurrentFloorNumber() == 16;

                                if (bCanGoToSecretAgentOtto)
                                {
                                    if (bInMiddleOfStairPlatform)
                                    {
                                        if ((bShouldGoUpStairs || bShouldGoDownStairs) &&
                                            (bBothAgentsOnLeftSide || bBothAgentsOnRightSide || bOnBottomLeftStairs || bOnBottomRightStairs || bBlockedByWall))
                                        {
                                            SecretAgentAI->StartTransition();
                                            SecretAgentAI->Transition();
                                        }
                                    }
                                }
                                else
                                {
                                    if (bBothAgentsOnLeftSide || bBothAgentsOnRightSide)
                                        bCanGoToSecretAgentOtto = true;
                                }
                            }
                            else
                            {
                                SecretAgentAI->GetCharacterMovement()->MaxWalkSpeed = SecretAgentAI->GetDefaultWalkSpeed();
                            }
                        }
                    }

                    SecretAgentAI->MoveForward(DirectionVector.X);
                }
                else if (SecretAgentAILocation == ELocationState::Elevator && SecretAgentAI->GetTracedElevator())
                {
                    AElevator* Elevator = SecretAgentAI->GetTracedElevator();
                    int32 ElevatorCurrentFloorNumber = Elevator->GetCurrentFloorNumber();
                    int32 ElevatorMinFloorNumber = Elevator->GetMinFloorNumber();
                    int32 ElevatorMaxFloorNumber = Elevator->GetMaxFloorNumber();
                    int32 ElevatorTargetFloor = FMath::Clamp(SecretAgentOttoFloorNumber, ElevatorMinFloorNumber, ElevatorMaxFloorNumber);
                    
                    if (SecretAgentAI->CanTransition())
                    {
                        if (ElevatorCurrentFloorNumber == ElevatorTargetFloor)
                        {
                            bool bIsInHallway = SecretAgentOttoLocation == ELocationState::Hallway && SecretAgentOttoTransition != ETransitionState::Enter;
                            bool bIsGettingOutRoom = SecretAgentOttoLocation == ELocationState::Room && SecretAgentOttoTransition == ETransitionState::Exit;
                            bool bIsAboveMaxFloor = ElevatorTargetFloor == ElevatorMaxFloorNumber && ElevatorTargetFloor < SecretAgentOttoFloorNumber;
                            bool bIsBelowMinFloor = ElevatorTargetFloor == ElevatorMinFloorNumber && ElevatorTargetFloor > SecretAgentOttoFloorNumber;

                            if ((bIsInHallway || bIsGettingOutRoom) || bIsAboveMaxFloor || bIsBelowMinFloor) 
                                SecretAgentAI->Transition();
                        }
                        else if (ElevatorCurrentFloorNumber > ElevatorTargetFloor)
                            SecretAgentAI->MoveUp(-1.0f);
                        else if (ElevatorCurrentFloorNumber < ElevatorTargetFloor)
                            SecretAgentAI->MoveUp(1.0f);
                    }
                }
            }
            else if (SecretAgentAITransition == ETransitionState::Exit)
            {
                if (SecretAgentOttoFloorNumber == SecretAgentAIFloorNumber && SecretAgentOttoLocation == ELocationState::Hallway)
                {
                    PercentChanceAIShootPlayerWhileMoving = UKismetMathLibrary::RandomFloat();
                    PercentChanceAIDodgesPlayerProjectiles = UKismetMathLibrary::RandomFloat();
                    
                    if (ObjectBetweenSecretAgents().Actor == SecretAgentOtto)
                    {
                        bBlockedByWall = false;
                        
                        if (SecretAgentAI->GetActorLocation().X < SecretAgentOtto->GetActorLocation().X)
                            DirectionVector = FVector::ForwardVector;
                        else if (SecretAgentAI->GetActorLocation().X > SecretAgentOtto->GetActorLocation().X)
                            DirectionVector = FVector::BackwardVector;
                    }
                    else if (ObjectBetweenSecretAgents().Component->GetName().Equals(TEXT("HallwayWalls")))
                    {
                        bBlockedByWall = true;
                        
                        if (SecretAgentAI->GetActorLocation().X < SecretAgentOtto->GetActorLocation().X)
                            DirectionVector = FVector::BackwardVector;
                        else if (SecretAgentAI->GetActorLocation().X > SecretAgentOtto->GetActorLocation().X)
                            DirectionVector = FVector::ForwardVector;
                    }
                }
                
                FHitResult WallHitResult;
                
                FVector SecretAgentAILeftSide = FVector::ZeroVector;
                FVector SecretAgentAIRightSide = FVector::ZeroVector;
                
                if (SecretAgentAILocation == ELocationState::Room || SecretAgentAILocation == ELocationState::Elevator)
                {
                    SecretAgentAILeftSide = SecretAgentAI->GetMesh()->GetSocketLocation(TEXT("eyes")) + FVector::BackwardVector * 120.0f;
                    SecretAgentAIRightSide = SecretAgentAI->GetMesh()->GetSocketLocation(TEXT("eyes")) + FVector::ForwardVector * 120.0f;
                }
                else if (SecretAgentAILocation == ELocationState::Stairs)
                {
                    AActor* SecretAgentAITracedStairs = SecretAgentAI->GetTracedStairs();

                    if (IsValid(SecretAgentAITracedStairs))
                    {
                        if (SecretAgentAI->CanGoUpStairs())
                        {
                            if (SecretAgentAITracedStairs->GetActorLocation().X < 0.0f)
                                DirectionVector = FVector::ForwardVector;
                            else if (SecretAgentAITracedStairs->GetActorLocation().X > 0.0f)
                                DirectionVector = FVector::BackwardVector;
                        }
                        else if (SecretAgentAI->CanGoDownStairs())
                        {
                            if (SecretAgentAIFloorNumber == 17 && SecretAgentAITracedStairs->GetActorLocation().X < 0.0f)
                                DirectionVector = FVector::ForwardVector;
                            else if (SecretAgentAIFloorNumber == 16 && SecretAgentAITracedStairs->GetActorLocation().X > 0.0f)
                                DirectionVector = FVector::BackwardVector;
                            else
                            {
                                if (SecretAgentAI->GetActorLocation().X > SecretAgentOtto->GetActorLocation().X)
                                    DirectionVector = FVector::BackwardVector;
                                else if (SecretAgentAI->GetActorLocation().X < SecretAgentOtto->GetActorLocation().X)
                                    DirectionVector = FVector::ForwardVector;
                            }
                        }
                    }
                }

                if (SecretAgentAILeftSide.Equals(SecretAgentAIRightSide))
                    return;

                if (DirectionVector.Equals(FVector::ZeroVector))
                {
                    if (GetWorld()->LineTraceSingleByChannel(WallHitResult, SecretAgentAILeftSide, SecretAgentAIRightSide,
                            ECC_Visibility, SecretAgentAI->GetCollisionQueryParams()))
                    {
                        if (WallHitResult.Component->GetName() == TEXT("HallwayWalls"))
                        {
                            bBlockedByWall = true;
                            
                            if (WallHitResult.Location.X < SecretAgentAI->GetActorLocation().X)
                                DirectionVector = FVector::BackwardVector;
                            else if (WallHitResult.Location.X > SecretAgentAI->GetActorLocation().X)
                                DirectionVector = FVector::ForwardVector;
                        }
                    }
                    else
                    {
                        bBlockedByWall = false;
                        
                        if ((SecretAgentOttoLocation == ELocationState::Hallway || SecretAgentOttoLocation == ELocationState::Stairs) &&
                            SecretAgentOttoFloorNumber > SecretAgentAIFloorNumber)
                        {
                            if (SecretAgentAI->GetActorLocation().X < SecretAgentOtto->GetActorLocation().X)
                                DirectionVector = FVector::BackwardVector;
                            else if (SecretAgentAI->GetActorLocation().X > SecretAgentOtto->GetActorLocation().X)
                                DirectionVector = FVector::ForwardVector;
                        }
                        else
                            DirectionVector = UKismetMathLibrary::RandomBool() ? FVector::ForwardVector : FVector::BackwardVector;
                    }
                }
                
            }
            else if (SecretAgentAITransition == ETransitionState::Enter)
            {
                bBlockedByWall = false;
                DirectionVector = FVector::ZeroVector;
            }
        }
        else
            SecretAgentAI->GetCharacterMovement()->MaxWalkSpeed = 0.0f;
    }
}
