#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "WorldItemBase.generated.h"

UCLASS()
class ANOTHERALPHA_API AWorldItemBase
    : public AActor
    , public IInteractable
{
    GENERATED_BODY()

public:

    AWorldItemBase();

    /*
     * ===== REPLICATED DATA =====
     */

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Item")
    FName ItemID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Item")
    int32 Quantity = 1;


    /*
     * ===== INTERFACE IMPLEMENTATION =====
     */

    virtual void Interact_Implementation(ACharacter* InteractingCharacter) override;

    virtual bool CanInteract_Implementation(ACharacter* InteractingCharacter) const override;


protected:

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps
    ) const override;
};