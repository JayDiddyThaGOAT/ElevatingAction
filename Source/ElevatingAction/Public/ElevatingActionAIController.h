

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

	UFUNCTION(BlueprintCallable, Category = Patrolling)
    void SetPatrolTime(float Time);
	
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccessOnly = "true"), Category = Attacking)
	float ShootPistolDelay;
	
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccessOnly = "true"), Category = Attacking)
	float MaxShootPistolDistance;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccessOnly = "true"), Category = Attacking)
	float PatrolDuration;;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccessOnly = "true"), Category = Attacking)
	float PercentRequiredAIShootPlayerWhileMoving;

private:
	class AElevatingActionSecretAgent* SecretAgentAI;
	class AElevatingActionSecretAgent* SecretAgentOtto;
	
	float ShootPistolTime, PatrolTime;

	bool bShouldGoUpStairs, bShouldGoDownStairs;
	bool bCanGoToSecretAgentOtto, bBlockedByWall;
	bool bCanGoLeft, bCanGoRight;

	float PercentChanceAIShootPlayerWhileMoving;

	FVector DirectionVector;
};
