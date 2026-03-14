#pragma once

#include "CoreMinimal.h"
#include "Inventory/InventoryTypes.h"
#include "EquipmentTypes.generated.h"

/*
====================================================
    EQUIPMENT SLOT TYPE
====================================================
*/

UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
    Helmet      UMETA(DisplayName = "Helmet"),
    Armor       UMETA(DisplayName = "Armor"),
    Backpack    UMETA(DisplayName = "Backpack")
};

/*
====================================================
    EQUIPMENT CONTAINER
====================================================
*/

USTRUCT(BlueprintType)
struct ANOTHERALPHA_API FEquipmentContainer
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadOnly)
    FInventoryItem Helmet;

    UPROPERTY(BlueprintReadOnly)
    FInventoryItem Armor;

    UPROPERTY(BlueprintReadOnly)
    FInventoryItem Backpack;

public:

    bool IsSlotEmpty(EEquipmentSlot Slot) const;

    FInventoryItem* GetItem(EEquipmentSlot Slot);

    const FInventoryItem* GetItemConst(EEquipmentSlot Slot) const;
};