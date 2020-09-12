

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ElevatingActionSecretAgent.h"
#include "ElevatingActionAIController.generated.h"

/**
* 
*/
UCLASS()
class ELEVATINGACTION_API AElevatingActionAIController : public AAIController
{
	GENERATED_BODY()

public:
	AElevatingActionAIController();

	virtual void TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

	float GetTargetLocationToUseStairs(class AActor* Stairs, bool CanGoUpStairs, bool CanGoDownStairs) const;

	FHitResult ObjectBetweenSecretAgents();

	UFUNCTION(BlueprintPure, Category = Patrolling)
    bool IsPatrollingLong();
	
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccessOnly = "true"), Category = Attacking)
	float ShootPistolDelay;
	
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccessOnly = "true"), Category = Attacking)
	float MaxShootPistolDistance;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccessOnly = "true"), Category = Attacking)
	float PercentRequiredAIShootPlayerWhileMoving;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccessOnly = "true"), Category = Attacking)
	float PercentRequiredAIDodgesPlayerProjectiles;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccessOnly = "true"), Category = Attacking)
	float PercentRequiredAIFollowsPlayerCrouching;

private:
	class AElevatingActionSecretAgent* SecretAgentAI;
	class AElevatingActionSecretAgent* SecretAgentOtto;

	class AActor* SecretAgentOttoLastShotProjectile;
	
	float ShootPistolTime;

	bool bShouldGoUpStairs, bShouldGoDownStairs;
	bool bCanGoToSecretAgentOtto, bBlockedByWall;
	bool bCanGoLeft, bCanGoRight;

	float PercentChanceAIShootPlayerWhileMoving;
	float PercentChanceAIDodgesPlayerProjectiles;
	float PercentChanceAIFollowsPlayerCrouching;

	FVector DirectionVector;
};
