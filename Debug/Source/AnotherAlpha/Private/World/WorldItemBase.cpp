#include "World/WorldItemBase.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Inventory/AAAInventoryComponent.h"

AWorldItemBase::AWorldItemBase()
{
    bReplicates = true;
}

void AWorldItemBase::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AWorldItemBase, ItemID);
    DOREPLIFETIME(AWorldItemBase, Quantity);
}

/*
 * ===== INTERACT =====
 */

void AWorldItemBase::Interact_Implementation(ACharacter* InteractingCharacter)
{
    if (!HasAuthority())
        return;

    if (!InteractingCharacter)
        return;

    if (ItemID == NAME_None || Quantity <= 0)
        return;

    UAAAInventoryComponent* Inventory =
        InteractingCharacter->FindComponentByClass<UAAAInventoryComponent>();

    if (!Inventory)
        return;

    const bool bAdded =
        Inventory->AddItem_Internal(ItemID, Quantity, 0);

    if (!bAdded)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("WorldItemBase -> Add failed, item NOT destroyed"));
        return;
    }

    UE_LOG(LogTemp, Warning,
        TEXT("WorldItemBase -> Item added, destroying actor"));

    Destroy();
}

bool AWorldItemBase::CanInteract_Implementation(ACharacter* InteractingCharacter) const
{
    if (!InteractingCharacter)
        return false;

    if (ItemID == NAME_None)
        return false;

    if (Quantity <= 0)
        return false;

    return true;
}