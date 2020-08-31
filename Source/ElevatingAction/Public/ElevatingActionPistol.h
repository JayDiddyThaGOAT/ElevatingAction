

#pragma once

#include "CoreMinimal.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/AudioComponent.h"
#include "ElevatingActionPistol.generated.h"

/**
* 
*/
UCLASS()
class ELEVATINGACTION_API AElevatingActionPistol : public ASkeletalMeshActor
{
	GENERATED_BODY()

public:
	AElevatingActionPistol();

	UFUNCTION(BlueprintCallable, Category = Firing)
    void PullTrigger();

	void SetFireRate(float FireRate);
	float GetFireRate() const;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Firing)
	class UAudioComponent* ProjectileShotAudioComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Firing)
	float FireRate;
	
	
private:
	UClass* Projectile;
	FTransform ProjectileSpawnTransform;
};
