#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ElevatingActionGameInstance.generated.h"

UCLASS()
class ELEVATINGACTION_API UElevatingActionGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UElevatingActionGameInstance();

	UFUNCTION(BlueprintCallable, Category = Score)
    void AddPlayerScore(int32 Score);
	
	UFUNCTION(BlueprintCallable, Category = Score)
    void ResetGame();

	UFUNCTION(BlueprintPure, Category = Lighting)
    bool IsOfficeBlackedOut() const;

	UFUNCTION(BlueprintPure, Category = Level)
    bool IsMaxSecretAgentsMovingReached() const;
	
	void SetCurrentSecretAgentsMoving(int32 CurrentSecretAgentsMoving);
	int32 GetCurrentSecretAgentsMoving() const;

	void SetNumberOfPlayerLives(int32 Lives);
	int32 GetNumberOfPlayerLives();
	
private:
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Health)
	int32 PlayerLives;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Score)
	int32 CurrentScore;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Score)
	int32 PreviousScore;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Score)
	int32 BonusScore;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Score)
	TArray<int32> GoalScores;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Score)
	int32 CurrentGoalScoreIndex;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Score)
	bool bIsAllGoalsPassed;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Level)
	int32 LevelNumber;
	
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Level)
  	float PercentChanceNewSecretAgentsSpawn;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Level)
	TMap<int32, FInt32Range> SecretFileCounts;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Level)
	int32 MaxSecretAgentsMoving;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Level)
	int32 CurrentSecretAgentsMoving;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Blackout)
	float BlackoutDuration;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Blackout)
	float BlackoutElapsedTime;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Blackout)
	bool bIsOfficeBlackedOut;
	
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Volume)
    float MasterVolume;
    
    UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Volume)
    float MusicVolume;
    
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Volume)
	float FXVolume;
    
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Volume)
	float VoicesVolume;
};

