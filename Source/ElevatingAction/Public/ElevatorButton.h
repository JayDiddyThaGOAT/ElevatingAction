
#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Elevator.h"
#include "ElevatorButton.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class ELEVATINGACTION_API UElevatorButton : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UElevatorButton();

	virtual void BeginPlay() override;

	AElevator* GetElevator() const;
	void CallElevatorTo(int32 FloorNumber);

	UFUNCTION(BlueprintSetter, Category = "Lighting")
    void SetButtonBrightness(float Brightness);

	UFUNCTION(BlueprintGetter, Category = "Lighting")
    float GetButtonBrightness() const;

private:
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "Elevator Summoning")
	class AElevator* Elevator;

	UPROPERTY(BlueprintSetter = SetButtonBrightness, BlueprintGetter = GetButtonBrightness, EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Lighting")
	float ButtonBrightness;
	
	class UMaterialInstance* ButtonMaterialInstance;
	class UMaterialInstanceDynamic* ButtonMaterial;
};
