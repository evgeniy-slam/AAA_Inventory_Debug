#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

UINTERFACE(Blueprintable)
class ANOTHERALPHA_API UInteractable : public UInterface
{
    GENERATED_BODY()
};

class ANOTHERALPHA_API IInteractable
{
    GENERATED_BODY()

public:

    /*
     * ===== INTERACTION =====
     */

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void Interact(ACharacter* InteractingCharacter);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    bool CanInteract(ACharacter* InteractingCharacter) const;


    /*
     * ===== FOCUS (AAA) =====
     */

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void OnFocusGained();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void OnFocusLost();
};