#include "AAACharacter.h"
#include "Inventory/AAAInventoryComponent.h"

AAACharacter::AAACharacter()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AAACharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    if (!HasAuthority())
        return;

    UAAAInventoryComponent* Inventory =
        FindComponentByClass<UAAAInventoryComponent>();

    if (!Inventory)
        return;

    Inventory->ApplyLoadout(DefaultLoadout);
}

void AAACharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAACharacter::SetupPlayerInputComponent(
    UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}