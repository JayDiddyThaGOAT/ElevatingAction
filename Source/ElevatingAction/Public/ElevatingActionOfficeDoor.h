
#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ElevatingActionOfficeDoor.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent, DisplayName = "OfficeDoor"))
class ELEVATINGACTION_API UElevatingActionOfficeDoor : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UElevatingActionOfficeDoor();

	UFUNCTION(BlueprintSetter, Category = "Frame")
    void SetFrameBrightness(float Brightness);

	UFUNCTION(BlueprintGetter, Category = "Frame")
    float GetFrameBrightness();

	UFUNCTION(BlueprintGetter, Category = "Door")
    bool IsClosed() const;

	UFUNCTION(BlueprintPure, Category = "Door")
    bool IsLocked() const;

	UFUNCTION(BlueprintCallable, Category = "Door")
    void Open();

	UFUNCTION(BlueprintCallable, Category = "Door")
    void Close();

protected:
	virtual void PostInitProperties() override;
	virtual void OnComponentCreated() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintSetter = SetFrameBrightness, BlueprintGetter = GetFrameBrightness, EditAnywhere, meta = (AllowPrivateAccess = "true"), Category = "Frame")
	float FrameBrightness;

	UPROPERTY(BlueprintGetter = IsClosed, meta = (AllowPrivateAccess = "true"), Category = "Door")
	bool bIsClosed;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Door")
	float DoorRotationSpeed;

private:
	class UMaterialInstance* FrameMaterialInstance;
	class UMaterialInstanceDynamic* FrameMaterial;

	FRotator DoorTargetRotation, DoorClosedRotation;
};
