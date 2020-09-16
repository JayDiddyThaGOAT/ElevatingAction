#pragma once


#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "ElevatorButton.h"
#include "Components/BoxComponent.h"
#include "Elevator.generated.h"

UENUM()
enum class EDirectionState : uint8
{
	Up UMETA(DisplayName = "Up"),
	Down UMETA(DisplayName = "Down"),
};

UENUM()
enum class EDoorsState : uint8
{
	Closed UMETA(DisplayName = "Closed"),
	Open UMETA(DisplayName = "Open"),
};

UCLASS()
class ELEVATINGACTION_API AElevator : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	AElevator();

	virtual void PostInitProperties() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Door Movement")
	void OpenDoors();

	UFUNCTION(BlueprintCallable, Category = "Door Movement")
	void CloseDoors();

	UFUNCTION(BlueprintPure, Category = "Door Movement")
	bool AreDoorsMoving() const;

	UFUNCTION(BlueprintPure, Category = "Door Movement")
    bool AreDoorsClosed() const;
	
	UFUNCTION(BlueprintPure, Category = "Elevator Movement")
	bool IsElevatorMoving() const;

	UFUNCTION(BlueprintPure, Category = "Elevator Movement")
    bool IsMovingUp() const;

	UFUNCTION(BlueprintPure, Category = "Elevator Movement")
	bool HasElevatorPassedStopTime() const;

	UFUNCTION(BlueprintCallable, Category = "Elevator Movement")
	void ResetStopTime();

	UFUNCTION(BlueprintPure, Category = "Elevator Movement")
	bool IsTargetFloorNumberSet() const;

	UFUNCTION(BlueprintPure, Category = "Elevator Movement")
	int32 GetCurrentFloorNumber() const;

	UFUNCTION(BlueprintPure, Category = "Elevator Movement")
	int32 GetTargetFloorNumber() const;

	UFUNCTION(BlueprintPure, Category = "Elevator Movement")
	int32 GetMinFloorNumber() const;

	UFUNCTION(BlueprintPure, Category = "Elevator Movement")
	int32 GetMaxFloorNumber() const;
	
	UFUNCTION(BlueprintCallable, Category = "Elevator Movement")
	void GoToNextFloor(EDirectionState Direction);

	UFUNCTION(BlueprintCallable, Category = "Elevator Movement")
	void GoToFloor(int32 FloorNumber);

	UFUNCTION(BlueprintCallable, Category = "Elevator Movement")
	void SetTargetFloorNumber(int32 FloorNumber);

	UFUNCTION(BlueprintSetter, Category = "Elevator Lighting")
	void SetElevatorBrightness(float Brightness);

	UFUNCTION(BlueprintGetter, Category = "Elevator Lighting")
	float GetElevatorBrightness() const;
	
	void SetElevatorButtonCaller(UElevatorButton* ElevatorButton);
	UElevatorButton* GetElevatorButtonCaller() const;

	UAudioComponent* GetElevatorMovingAudioComponent() const;
	USoundWave* GetElevatorMovingUpSoundWave() const;
	USoundWave* GetElevatorMovingDownSoundWave() const;
	
protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Doors")
	class UStaticMeshComponent* LeftDoor;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Doors")
	class UStaticMeshComponent* RightDoor;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Elevator Sounds")
	class UAudioComponent* ElevatorMovingAudioComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Inside Elevator")
	class UBoxComponent* InsideElevatorArea;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Door Movement")
	FVector LeftDoorTargetLocation;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Door Movement")
	FVector RightDoorTargetLocation;

	UPROPERTY(BlueprintSetter=SetElevatorBrightness, BlueprintGetter=GetElevatorBrightness, meta = (AllowPrivateAccess = "true"), Category="Elevator Lighting")
	float ElevatorBrightness;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Elevator Movement")
	FVector ElevatorTargetLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Elevator Movement")
	float ElevatorSpeed;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Elevator Movement")
	float ElevatorStoppedTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Elevator Movement")
	float ElevatorStoppedDuration;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Elevator Movement")
	EDirectionState ElevatorDirection;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Elevator Movement")
	class UElevatorButton* ElevatorButtonCaller;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Door Movement")
	float ElevatorDoorsSpeed;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Door Movement")
	EDoorsState ElevatorDoorsState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Door Movement")
	float OpenDoorOffset;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true", ClampMin = "0", ClampMax = "31", UIMin = "0", UIMax = "31"), Category = "Floor Number")
	int32 CurrentFloorNumber;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true", ClampMin = "0", ClampMax = "30", UIMin = "0", UIMax = "30"), Category = "Floor Number")
	int32 CurrentTargetFloorNumber;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true", ClampMin = "0", ClampMax = "30", UIMin = "0", UIMax = "30"), Category = "Floor Number")
	int32 NextTargetFloorNumber;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", ClampMin = "0", ClampMax = "30", UIMin = "1", UIMax = "30"), Category = "Floor Number")
	int32 MinFloorNumber;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true", ClampMin = "1", ClampMax = "31", UIMin = "1", UIMax = "31"), Category = "Floor Number")
	int32 MaxFloorNumber;

	class UMaterialInstance* ElevatorMaterialInstance;
	class UMaterialInstanceDynamic* ElevatorMaterial;

	class USoundWave* ElevatorMovingUpSoundWave;
	class USoundWave* ElevatorMovingDownSoundWave;
	class USoundWave* ElevatorDoorsOpeningSoundWave;
	class USoundWave* ElevatorDoorsClosingSoundWave;
};
