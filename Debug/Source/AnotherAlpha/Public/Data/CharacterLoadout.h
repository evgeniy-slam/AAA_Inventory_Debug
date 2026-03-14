#pragma once

#include "CoreMinimal.h"
#include "CharacterLoadout.generated.h"

USTRUCT(BlueprintType)
struct FCharacterLoadout
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName Backpack = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName Helmet = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName Armor = NAME_None;
};