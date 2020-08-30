

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Elevator.h"
#include "ElevatorButton.h"
#include "ElevatingActionPistol.h"
#include "ElevatingActionOfficeDoor.h"
#include "ElevatingActionSecretAgent.generated.h"

UENUM(BlueprintType)
enum class ETransitionState : uint8
{
	None UMETA(DisplayName = "None"),
    Enter UMETA(DisplayName = "Entering"),
    Exit UMETA(DisplayName = "Exiting"),
};

UENUM(BlueprintType)
enum class ELocationState : uint8
{
	Roof UMETA(DisplayName = "Roof"),
    Room UMETA(DisplayName = "Room"),
    Elevator UMETA(DisplayName = "Elevator"),
    Hallway UMETA(DisplayName = "Hallway"),
    Stairs UMETA(DisplayName = "Stairs"),
    Sidewalk UMETA(DisplayName = "Sidewalk"),
    Car UMETA(DisplayName = "Car"),
};

UCLASS()
class ELEVATINGACTION_API AElevatingActionSecretAgent : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AElevatingActionSecretAgent();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	virtual void Destroyed() override;

	bool IsDamaged() const;
	bool IsShootButtonPressed() const;
	bool CanGoLeft() const;
	bool CanGoRight() const;
	bool CanTransition() const;
	bool CanGoUpStairs() const;
	bool CanGoDownStairs() const;
	float GetDefaultWalkSpeed() const;
	
	ETransitionState GetCurrentTransition() const;
	ELocationState GetCurrentLocation() const;

	FCollisionQueryParams GetCollisionQueryParams() const;
	AElevator* GetTracedElevator() const;
	UElevatorButton* GetTracedElevatorButton() const;
	AActor* GetTracedStairs() const;
	UElevatingActionOfficeDoor* GetTracedDoor() const;
	FVector GetTracedStairsLocation() const;
	
	int32 GetCurrentFloorNumber() const;

	void ShootPistol();

	UFUNCTION(BlueprintCallable, Category = State)
    void StartTransition();

	UFUNCTION(BlueprintCallable, Category = State)
	void Transition();
	
	void MoveForward(float AxisValue);
	void MoveUp(float AxisValue);

private:
	void TraceOfficeWalls();
	void ToggleCrouch();

protected:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Floor)
	int32 CurrentFloorNumber;
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Movement)
	bool bCanGoLeft;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Movement)
	bool bCanGoRight;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Movement)
	float DefaultWalkSpeed;

	//How far will the secret agent walk to inside the room?
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Movement)
	float RoomTargetLocation;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = State)
	ETransitionState CurrentTransition;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = State)
	ELocationState CurrentLocation;
	
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = State)
	bool bCanTransition;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = State)
	bool bCanGoUpStairs;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = State)
	bool bCanGoDownStairs;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Tracing)
	class AElevator* TracedElevator;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Tracing)
	class UElevatorButton* TracedElevatorButton;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Tracing)
	class AActor* TracedStairs;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Tracing)
	class UElevatingActionOfficeDoor* TracedDoor;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Tracing)
	FVector TracedStairsLocation;
	
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Attacking)
	bool bShootButtonPressed;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Attacking)
	bool bIsDamaged;
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Attacking)
	class AElevatingActionPistol* Pistol;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Attacking)
	float PistolFireRate;
	
	FCollisionQueryParams CollisionQueryParams;
};