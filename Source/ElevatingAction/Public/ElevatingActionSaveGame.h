

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ElevatingActionSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class ELEVATINGACTION_API UElevatingActionSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
    int32 GetHighScore() const;
    void SetHighScore(int32 HighScore);
    
private:
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Score);
    int32 HighScore;
};
