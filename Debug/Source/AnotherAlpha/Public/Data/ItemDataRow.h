#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemDataRow.generated.h"

/*
====================================================
    ITEM TYPE
====================================================
*/

UENUM(BlueprintType)
enum class EItemType : uint8
{
    Item        UMETA(DisplayName = "Item"),
    Armor       UMETA(DisplayName = "Armor"),
    Helmet      UMETA(DisplayName = "Helmet"),
    Backpack    UMETA(DisplayName = "Backpack")
};

/*
====================================================
    ITEM DATA ROW
====================================================
*/

USTRUCT(BlueprintType)
struct ANOTHERALPHA_API FItemDataRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UTexture2D* Icon = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MaxStackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bStackable = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UStaticMesh* WorldMesh = nullptr;

    /* NEW */

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EItemType ItemType = EItemType::Item;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 BackpackSlots = 0;
};