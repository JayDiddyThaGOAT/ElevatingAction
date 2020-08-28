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

private:
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Health)
	int32 PlayerLives;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Score)
	int32 CurrentScore;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Score)
	int32 PreviousScore;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Score)
	int32 BonusScore;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Level)
	int32 LevelNumber;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Level)
	TArray<int32> SecretFileCounts;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Blackout)
	float BlackoutDuration;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Blackout)
	float BlackoutElapsedTime;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Blackout)
	bool bIsOfficeBlackedOut;
};

