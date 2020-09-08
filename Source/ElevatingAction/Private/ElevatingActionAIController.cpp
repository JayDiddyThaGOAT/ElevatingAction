
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

    PatrolTime = 0.0f;
    PatrolDuration = 2.0f;
    
    PercentChanceAIShootPlayerWhileMoving = 0.0f;
    PercentRequiredAIShootPlayerWhileMoving = 0.0f;

    PercentChanceAIDodgesPlayerProjectiles = 0.0f;
    PercentRequiredAIDodgesPlayerProjectiles = 0.0f;

    bCanGoToSecretAgentOtto = true;
    bBlockedByWall = false;
}

bool AElevatingActionAIController::IsPatrollingLong()
{
    return PatrolTime >= PatrolDuration;
}

void AElevatingActionAIController::SetPatrolTime(float Time)
{
    PatrolTime = Time;
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

    if (!SecretAgentAI && !SecretAgentOtto)
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
                            
                            PatrolTime = 0.0f;
                            
                            bCanGoToSecretAgentOtto = true;
                            bBlockedByWall = false;
                           
                            float DistanceRequiredToShootPlayer = !bIsOfficeBlackedOut ? MaxShootPistolDistance : MaxShootPistolDistance - 125.0f;
                        
                            float DistanceBetweenAgents = FVector::Distance(SecretAgentOtto->GetActorLocation(), SecretAgentAI->GetActorLocation());
                            if (DistanceBetweenAgents <= DistanceRequiredToShootPlayer)
                            {
                                SecretAgentAI->GetCharacterMovement()->MaxWalkSpeed = 0.0f;
                                
                                float CurrentShootPistolDelay = !bIsOfficeBlackedOut ? ShootPistolDelay : ShootPistolDelay + 2.0f;
                            
                                ShootPistolTime += DeltaTime;
                                if (ShootPistolTime >= CurrentShootPistolDelay)
                                {
                                    SecretAgentAI->ShootPistol();
                                    ShootPistolTime = 0.0f;
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
                            
                            PatrolTime += DeltaTime;
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
                                        PatrolTime = 0.0f;
                                    
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
                            if (DirectionVector == FVector::BackwardVector)
                                DirectionVector = FVector::ForwardVector;
                            else if (DirectionVector == FVector::ForwardVector)
                                DirectionVector = FVector::BackwardVector;
                        }
                        
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
                                    else
                                        PatrolTime += DeltaTime;
                                }
                                else if (TracedElevatorButtonBrightness == 1.0f && SecretAgentAI->GetCharacterMovement()->MaxWalkSpeed == 0.0f)
                                {
                                    SecretAgentAI->GetCharacterMovement()->MaxWalkSpeed = SecretAgentAI->GetDefaultWalkSpeed();
                                    PatrolTime += DeltaTime;

                                    if (Elevator->GetActorLocation().X + 125.0f > SecretAgentAI->GetActorLocation().X)
                                        DirectionVector = FVector::ForwardVector;
                                    else if (Elevator->GetActorLocation().X + 125.0f < SecretAgentAI->GetActorLocation().X)
                                        DirectionVector = FVector::BackwardVector;
                                }
                            }
                            else if (TracedElevator && !TracedElevator->GetOwner())
                            {
                                SecretAgentAI->GetCharacterMovement()->MaxWalkSpeed = SecretAgentAI->GetDefaultWalkSpeed();
                                
                                PatrolTime += DeltaTime;
                            
                                AElevator* Elevator = SecretAgentAI->GetTracedElevator();
                                int32 ElevatorMinFloorNumber = Elevator->GetMinFloorNumber();
                                int32 ElevatorMaxFloorNumber = Elevator->GetMaxFloorNumber();
                        
                                float DistanceX = ((TracedElevator->GetActorLocation() + FVector::ForwardVector * 125.0f) - SecretAgentAI->GetActorLocation()).X;

                                bool bAtMiddleOfElevator = UKismetMathLibrary::NearlyEqual_FloatFloat(DistanceX, 0.0f, 5.0f);
                                bool bCantGoLeftOrRight = !(bCanGoLeft && bCanGoRight);
                                bool bElevatorCantGoDownToOtto = SecretAgentAIFloorNumber == ElevatorMinFloorNumber && SecretAgentAIFloorNumber > SecretAgentOttoFloorNumber;
                                bool bElevatorCantGoUpToOtto = SecretAgentAIFloorNumber == ElevatorMaxFloorNumber && SecretAgentAIFloorNumber < SecretAgentOttoFloorNumber;
                                if ((bCantGoLeftOrRight || bAtMiddleOfElevator) && !bElevatorCantGoDownToOtto && !bElevatorCantGoUpToOtto)
                                {
                                    PatrolTime = 0.0f;
                                    SecretAgentAI->Transition();
                                }
                            }
                            else if (TracedStairs)
                            {
                                SecretAgentAI->GetCharacterMovement()->MaxWalkSpeed = SecretAgentAI->GetDefaultWalkSpeed();
                                PatrolTime += DeltaTime;
                            
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
                                            PatrolTime = 0.0f;
                                        
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
                                PatrolTime += DeltaTime;
                            }
                        }
                    }

                    SecretAgentAI->MoveForward(DirectionVector.X);
                }
                else if (SecretAgentAILocation == ELocationState::Elevator)
                {
                    AElevator* Elevator = SecretAgentAI->GetTracedElevator();
                    int32 ElevatorMinFloorNumber = Elevator->GetMinFloorNumber();
                    int32 ElevatorMaxFloorNumber = Elevator->GetMaxFloorNumber();
                    int32 ElevatorTargetFloor = FMath::Clamp(SecretAgentOttoFloorNumber, ElevatorMinFloorNumber, ElevatorMaxFloorNumber);

                    if (Elevator->AreDoorsMoving() || !Elevator->AreDoorsClosed())
                    {
                        if (Elevator->GetCurrentFloorNumber() == ElevatorTargetFloor)
                        {
                            SecretAgentAI->StartTransition();
                            SecretAgentAI->Transition();
                        }
                    }
                    else
                    {
                        if (Elevator->GetCurrentFloorNumber() > ElevatorTargetFloor)
                            SecretAgentAI->MoveUp(-1.0f);
                        else if (Elevator->GetCurrentFloorNumber() < ElevatorTargetFloor)
                            SecretAgentAI->MoveUp(1.0f);
                        else if (Elevator->GetCurrentFloorNumber() == ElevatorTargetFloor)
                        {
                            if (!Elevator->IsElevatorMoving())
                            {
                                if (Elevator->GetCurrentFloorNumber() == SecretAgentOttoFloorNumber)
                                {
                                    if (SecretAgentOttoLocation == ELocationState::Hallway && SecretAgentOttoTransition == ETransitionState::None)
                                    {
                                        SecretAgentAI->StartTransition();
                                        SecretAgentAI->Transition();
                                    }
                                }
                                else
                                {
                                    SecretAgentAI->StartTransition();
                                    SecretAgentAI->Transition();
                                }
                            }
                        }
                    }
                }
            }
            else if (SecretAgentAITransition == ETransitionState::Exit)
            {
                FHitResult WallHitResult;
                FVector SecretAgentAILeftSide = SecretAgentAI->GetMesh()->GetSocketLocation(TEXT("spine_03")) + FVector::BackwardVector * 500.0f;
                FVector SecretAgentAIRightSide = SecretAgentAI->GetMesh()->GetSocketLocation(TEXT("spine_03")) + FVector::ForwardVector * 500.0f;
                FCollisionQueryParams CollisionQueryParams = SecretAgentAI->GetCollisionQueryParams();

                FCollisionObjectQueryParams WallObjectQueryParams;
                WallObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);

                if (SecretAgentAIFloorNumber == SecretAgentOttoFloorNumber)
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
                    else if (ObjectBetweenSecretAgents().Actor->GetName().Contains("Office"))
                    {
                        bBlockedByWall = true;
                        
                        if (SecretAgentAI->GetActorLocation().X < SecretAgentOtto->GetActorLocation().X)
                            DirectionVector = FVector::BackwardVector;
                        else if (SecretAgentAI->GetActorLocation().X > SecretAgentOtto->GetActorLocation().X)
                            DirectionVector = FVector::ForwardVector;
                    }
                    
                    PatrolTime = 0.0f;
                }
                else
                {
                    if (GetWorld()->LineTraceSingleByObjectType(WallHitResult, SecretAgentAILeftSide, SecretAgentAIRightSide, WallObjectQueryParams, CollisionQueryParams))
                    {
                        bBlockedByWall = true;
                    
                        if (WallHitResult.Location.X < SecretAgentAI->GetActorLocation().X)
                            DirectionVector = FVector::BackwardVector;
                        else if (WallHitResult.Location.X > SecretAgentAI->GetActorLocation().X)
                            DirectionVector = FVector::ForwardVector;
                    }
                    else
                    {
                        bBlockedByWall = false;
                    
                        AActor* TracedStairs = SecretAgentAI->GetTracedStairs();
                        if (TracedStairs)
                        {
                            if (TracedStairs->GetName().Contains("Left") && SecretAgentAIFloorNumber >= 17)
                                DirectionVector = FVector::ForwardVector;
                            else if (TracedStairs->GetName().Contains("Right") && SecretAgentAIFloorNumber >= 16)
                                DirectionVector = FVector::BackwardVector;
                        }
                        else
                            DirectionVector = UKismetMathLibrary::RandomBool() ? FVector::ForwardVector : FVector::BackwardVector;
                    }
                }
            }
            else if (SecretAgentAITransition == ETransitionState::Enter)
            {
                if (bBlockedByWall)
                    bBlockedByWall = false;
            }
        }
        else
            SecretAgentAI->GetCharacterMovement()->MaxWalkSpeed = 0.0f;
    }
}
