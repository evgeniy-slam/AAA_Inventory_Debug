#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Data/CharacterLoadout.h"
#include "AAACharacter.generated.h"

UCLASS()
class ANOTHERALPHA_API AAACharacter : public ACharacter
{
    GENERATED_BODY()

public:

    AAACharacter();

protected:

    virtual void PossessedBy(AController* NewController) override;

public:

    virtual void Tick(float DeltaTime) override;

    virtual void SetupPlayerInputComponent(
        class UInputComponent* PlayerInputComponent
    ) override;

public:

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loadout")
    FCharacterLoadout DefaultLoadout;
};