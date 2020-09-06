

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
	
private:
	class UAudioComponent* ProjectileShotAudioComponent;
	
	UClass* Projectile;
	FTransform ProjectileSpawnTransform;
};
