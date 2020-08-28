

#pragma once

#include "CoreMinimal.h"
#include "Animation/SkeletalMeshActor.h"
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

	float GetFireRate() const;

	protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Firing)
	float FireRate;

	private:
	UClass* Projectile;
	FTransform ProjectileSpawnTransform;
};
