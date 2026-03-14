#include "Inventory/InventoryTypes.h"
#include "Inventory/AAAInventoryComponent.h"

void FInventoryArray::PostReplicatedAdd(
    const TArrayView<int32> AddedIndices,
    int32 FinalSize)
{
    if (!Owner)
        return;

    Owner->NotifyInventoryUpdated();
}

void FInventoryArray::PostReplicatedChange(
    const TArrayView<int32> ChangedIndices,
    int32 FinalSize)
{
    if (!Owner)
        return;

    Owner->NotifyInventoryUpdated();
}

void FInventoryArray::PostReplicatedRemove(
    const TArrayView<int32> RemovedIndices,
    int32 FinalSize)
{
    if (!Owner)
        return;

    Owner->NotifyInventoryUpdated();

    UE_LOG(LogTemp, Verbose,
        TEXT("Inventory Remove replicated"));
}