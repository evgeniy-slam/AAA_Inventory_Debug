#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "InventoryTypes.generated.h"

class UAAAInventoryComponent;

/*
====================================================
    INVENTORY ITEM
====================================================
*/

USTRUCT(BlueprintType)
struct ANOTHERALPHA_API FInventoryItem : public FFastArraySerializerItem
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemRowName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SlotIndex = INDEX_NONE;

    bool IsEmpty() const
    {
        return ItemRowName.IsNone() || Quantity <= 0;
    }

    void Reset()
    {
        ItemRowName = NAME_None;
        Quantity = 0;
        SlotIndex = INDEX_NONE;
    }
};

/*
====================================================
    FAST ARRAY
====================================================
*/

USTRUCT(BlueprintType)
struct ANOTHERALPHA_API FInventoryArray : public FFastArraySerializer
{
    GENERATED_BODY()

public:

    UPROPERTY()
    TArray<FInventoryItem> Items;

    UPROPERTY(NotReplicated)
    UAAAInventoryComponent* Owner = nullptr;

public:

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
    {
        return FFastArraySerializer::FastArrayDeltaSerialize<
            FInventoryItem,
            FInventoryArray
        >(Items, DeltaParams, *this);
    }

    void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
    void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
    void PostReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
};

template<>
struct TStructOpsTypeTraits<FInventoryArray> : public TStructOpsTypeTraitsBase2<FInventoryArray>
{
    enum
    {
        WithNetDeltaSerializer = true,
    };
};