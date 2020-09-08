

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
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = Score);
    int32 HighScore;
};
