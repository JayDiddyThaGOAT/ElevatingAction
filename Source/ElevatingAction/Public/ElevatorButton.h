
#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "ElevatorButton.generated.h"

class AElevator;

UCLASS(meta = (BlueprintSpawnableComponent))
class ELEVATINGACTION_API UElevatorButton : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UElevatorButton();
	
	virtual void BeginPlay() override;
	
	AElevator* GetElevator() const;
	void CallElevatorTo(int32 FloorNumber);
	USoundWave* GetElevatorArrivedAlarm() const;

	int32 GetCurrentFloorNumber() const;
	UFUNCTION(BlueprintSetter, Category = "Lighting")
    void SetButtonBrightness(float Brightness);

	UFUNCTION(BlueprintGetter, Category = "Lighting")
    float GetButtonBrightness() const;
private:
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Elevator Summoning")
	class AElevator* Elevator;

	UPROPERTY(BlueprintSetter = SetButtonBrightness, BlueprintGetter = GetButtonBrightness, EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Lighting")
	float ButtonBrightness;

	int32 CurrentFloorNumber;
	
	class UMaterialInstance* ButtonMaterialInstance;
	class UMaterialInstanceDynamic* ButtonMaterial;

	class USoundCue* ElevatorButtonClickCue;
	class USoundWave* ElevatorButtonHighlightSound;
	class USoundWave* ElevatorArrivedAlarm;
};
