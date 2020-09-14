


#include "ElevatingActionSaveGame.h"

#include "Kismet/GameplayStatics.h"

int32 UElevatingActionSaveGame::GetHighScore() const
{
    return HighScore;
}

void UElevatingActionSaveGame::SetHighScore(int32 Score)
{
    HighScore = Score;
    UGameplayStatics::SaveGameToSlot(this, TEXT("HighScore"), 0);
}
