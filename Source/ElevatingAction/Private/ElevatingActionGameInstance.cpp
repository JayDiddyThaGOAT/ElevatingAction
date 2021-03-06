
#include "ElevatingActionGameInstance.h"
#include "Kismet/GameplayStatics.h"

UElevatingActionGameInstance::UElevatingActionGameInstance()
{
    GoalScores.Add(10000);
    GoalScores.Add(15000);
    GoalScores.Add(20000);
    GoalScores.Add(25000);
    
    ResetGame();

    MasterVolume = 0.5f;
    MusicVolume = 0.5f;
    FXVolume = 0.5f;
    VoicesVolume = 0.5f;

    //InKey = Number Of Secret Files Spawned Per Level
    //InValue = How low or high each secret file can spawn in level
    SecretFileCounts.Add(5, FInt32Range(9, 20));	//Level 1
    SecretFileCounts.Add(6, FInt32Range(3, 20));	//Level 2
    SecretFileCounts.Add(7, FInt32Range(2, 20));	//Level 3
    SecretFileCounts.Add(8, FInt32Range(2, 21));	//Level 4
    SecretFileCounts.Add(9, FInt32Range(2, 30));	//Level 5
    SecretFileCounts.Add(10, FInt32Range(1, 30)); //Level 6 & Up
   
}

void UElevatingActionGameInstance::AddPlayerScore(int32 Points)
{
    CurrentScore += Points;
}

void UElevatingActionGameInstance::UpdateHighScore()
{
    if (!UGameplayStatics::DoesSaveGameExist(TEXT("HighScore"), 0))
        SavedGame = Cast<UElevatingActionSaveGame>(UGameplayStatics::CreateSaveGameObject(UElevatingActionSaveGame::StaticClass()));
    else
        SavedGame = Cast<UElevatingActionSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("HighScore"), 0));
    
    if (CurrentScore <= SavedGame->GetHighScore())
        return;

    if (CurrentGoalScoreIndex < GoalScores.Num())
    {
        while (CurrentScore > GoalScores[CurrentGoalScoreIndex])
        {
            CurrentGoalScoreIndex++;
            PlayerLives++;

            if (CurrentGoalScoreIndex == GoalScores.Num())
                break;
        }
    }
    
    if (CurrentGoalScoreIndex >= GoalScores.Num())
        SavedGame->SetHighScore(CurrentScore);
    else
        SavedGame->SetHighScore(GoalScores[CurrentGoalScoreIndex]);
}

void UElevatingActionGameInstance::ResetGame()
{
    CurrentScore = 0;
    PreviousScore = 0;
    BonusScore = 1000;

    bIsOfficeBlackedOut = false;
    BlackoutDuration = 5.0f;
    BlackoutElapsedTime = 0.0f;

    LevelNumber = 0;

    PercentChanceNewSecretAgentsSpawn = 0.0f;
    MaxSecretAgentsMoving = 21;
    CurrentSecretAgentsMoving = 0;

    CurrentGoalScoreIndex = 0;
    PlayerLives = 3;
    if (!UGameplayStatics::DoesSaveGameExist(TEXT("HighScore"), 0))
    {
        SavedGame = Cast<UElevatingActionSaveGame>(UGameplayStatics::CreateSaveGameObject(UElevatingActionSaveGame::StaticClass()));
        SavedGame->SetHighScore(GoalScores[CurrentGoalScoreIndex]);
    }
    else
    {
        SavedGame = Cast<UElevatingActionSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("HighScore"), 0));
        
		if (CurrentGoalScoreIndex < GoalScores.Num())
		{
			while (GoalScores[CurrentGoalScoreIndex] < SavedGame->GetHighScore())
			{
				CurrentGoalScoreIndex++;
				PlayerLives++;

				if (CurrentGoalScoreIndex == GoalScores.Num())
					break;
			}
		}
    }
}

void UElevatingActionGameInstance::SetNumberOfPlayerLives(int32 Lives)
{
    PlayerLives = Lives;
}

int32 UElevatingActionGameInstance::GetNumberOfPlayerLives()
{
    return PlayerLives;
}

bool UElevatingActionGameInstance::IsOfficeBlackedOut() const
{
    return bIsOfficeBlackedOut;
}

bool UElevatingActionGameInstance::IsMaxSecretAgentsMovingReached() const
{
    return CurrentSecretAgentsMoving >= MaxSecretAgentsMoving;
}

int32 UElevatingActionGameInstance::GetCurrentSecretAgentsMoving() const
{
    return CurrentSecretAgentsMoving;
}

void UElevatingActionGameInstance::SetCurrentSecretAgentsMoving(int32 CurrentSecretAgents)
{
    CurrentSecretAgentsMoving = CurrentSecretAgents;
}
