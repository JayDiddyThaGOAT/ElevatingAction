
#include "ElevatingActionGameInstance.h"

UElevatingActionGameInstance::UElevatingActionGameInstance()
{
    CurrentScore = 0;
    PreviousScore = 0;
    BonusScore = 1000;

    LevelNumber = 1;
    PlayerLives = 3;

    bIsOfficeBlackedOut = false;
    BlackoutDuration = 4.0f;
    BlackoutElapsedTime = 0.0f;

    SecretFileCounts.Add(5);	//Level 1
    SecretFileCounts.Add(6);	//Level 2
    SecretFileCounts.Add(6);	//Level 3
    SecretFileCounts.Add(7);	//Level 4
    SecretFileCounts.Add(8);	//Level 5
    SecretFileCounts.Add(9);	//Level 6
    SecretFileCounts.Add(10);	//Level 7 & Up
}

void UElevatingActionGameInstance::AddPlayerScore(int32 Points)
{
    CurrentScore += Points;
}

void UElevatingActionGameInstance::ResetGame()
{
    CurrentScore = 0;
    PreviousScore = 0;
    BonusScore = 1000;

    LevelNumber = 1;
    PlayerLives = 3;
}

bool UElevatingActionGameInstance::IsOfficeBlackedOut() const
{
    return bIsOfficeBlackedOut;
}
