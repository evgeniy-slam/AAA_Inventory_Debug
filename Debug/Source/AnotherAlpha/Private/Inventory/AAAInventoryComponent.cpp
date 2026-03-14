#include "Inventory/AAAInventoryComponent.h"
#include "Inventory/InventoryTypes.h"
#include "Inventory/EquipmentTypes.h"
#include "Data/ItemDataRow.h"
#include "AAACharacter.h"
#include "Data/CharacterLoadout.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"

UAAAInventoryComponent::UAAAInventoryComponent()
{
    SetIsReplicatedByDefault(true);
}

void UAAAInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning,
        TEXT("InventoryComponent Owner=%s  HasAuthority=%d  LocalRole=%d"),
        *GetOwner()->GetName(),
        GetOwner()->HasAuthority(),
        (int32)GetOwner()->GetLocalRole());

    Inventory.Owner = this;

    if (!GetOwner()->HasAuthority())
    {
        Inventory.Owner = this;
    }

    if (GetOwner() && GetOwner()->HasAuthority())
    {
        BackpackCapacity = DefaultInventorySlots;

        UE_LOG(LogTemp, Warning,
            TEXT("Inventory component ready. BaseCapacity=%d"),
            BackpackCapacity);
    }
}

void UAAAInventoryComponent::InitializeInventory()
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
        return;

    if (Inventory.Items.Num() == BackpackCapacity)
        return;

    Inventory.Items.SetNum(BackpackCapacity);

    Inventory.MarkArrayDirty();

    for (int32 i = 0; i < BackpackCapacity; i++)
    {
        Inventory.Items[i].ItemRowName = NAME_None;
        Inventory.Items[i].Quantity = 0;
        Inventory.Items[i].SlotIndex = i;

        Inventory.MarkItemDirty(Inventory.Items[i]);
    }
}

void UAAAInventoryComponent::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UAAAInventoryComponent, Inventory);
    DOREPLIFETIME(UAAAInventoryComponent, BackpackCapacity);
    DOREPLIFETIME(UAAAInventoryComponent, Equipment);
}

void UAAAInventoryComponent::OnRep_BackpackCapacity()
{
    Inventory.Owner = this;
    NotifyInventoryUpdated();
}
void UAAAInventoryComponent::OnRep_Equipment()
{
    const FInventoryItem* Backpack =
        Equipment.GetItemConst(EEquipmentSlot::Backpack);

    if (Backpack && !Backpack->IsEmpty())
    {
        ApplyBackpackSlots(Backpack->ItemRowName);
    }
    else
    {
        BackpackCapacity = DefaultInventorySlots;
    }

    NotifyInventoryUpdated();
}

void UAAAInventoryComponent::ApplyLoadout(const FCharacterLoadout& Loadout)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
        return;

    /*
    ================================
        DETERMINE BACKPACK CAPACITY
    ================================
    */

    BackpackCapacity = DefaultInventorySlots;

    if (!Loadout.Backpack.IsNone() && ItemDataTable)
    {
        static const FString Context(TEXT("BackpackLookup"));

        if (FItemDataRow* Row =
            ItemDataTable->FindRow<FItemDataRow>(Loadout.Backpack, Context))
        {
            if (Row->ItemType == EItemType::Backpack && Row->BackpackSlots > 0)
            {
                BackpackCapacity = Row->BackpackSlots;
            }
        }
    }

    /*
    ================================
        BACKPACK → EQUIPMENT
    ================================
    */

    if (!Loadout.Backpack.IsNone())
    {
        FInventoryItem* BackpackSlot =
            Equipment.GetItem(EEquipmentSlot::Backpack);

        if (BackpackSlot)
        {
            FEquipmentContainer NewEquipment = Equipment;

            NewEquipment.Backpack.ItemRowName = Loadout.Backpack;
            NewEquipment.Backpack.Quantity = 1;

            Equipment = NewEquipment;
        }
    }

    /*
    ================================
        CREATE INVENTORY
    ================================
    */

    InitializeInventory();

    UE_LOG(LogTemp, Warning,
        TEXT("Inventory initialized AFTER loadout. Capacity=%d Items=%d"),
        BackpackCapacity,
        Inventory.Items.Num());

    NotifyInventoryUpdated();

    /*
    ================================
        HELMET
    ================================
    */

    if (!Loadout.Helmet.IsNone())
    {
        AddItem_Internal(Loadout.Helmet, 1, 0);

        const TArray<FInventoryItem>& Items = GetInventory();

        for (const FInventoryItem& Item : Items)
        {
            if (Item.ItemRowName == Loadout.Helmet)
            {
                Server_EquipItem(
                    Item.SlotIndex,
                    EEquipmentSlot::Helmet
                );
                break;
            }
        }
    }

    /*
    ================================
        ARMOR
    ================================
    */

    if (!Loadout.Armor.IsNone())
    {
        AddItem_Internal(Loadout.Armor, 1, 0);

        const TArray<FInventoryItem>& Items = GetInventory();

        for (const FInventoryItem& Item : Items)
        {
            if (Item.ItemRowName == Loadout.Armor)
            {
                Server_EquipItem(
                    Item.SlotIndex,
                    EEquipmentSlot::Armor
                );
                break;
            }
        }
    }
}

int32 UAAAInventoryComponent::GetBackpackCapacity() const
{
    if (BackpackCapacity <= 0)
    {
        return DefaultInventorySlots;
    }

    return BackpackCapacity;
}

void UAAAInventoryComponent::SetBackpackCapacity(int32 NewCapacity)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
        return;

    if (NewCapacity <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("SetBackpackCapacity invalid capacity %d"), NewCapacity);
        return;
    }


    UE_LOG(LogTemp, Warning,
        TEXT("Inventory resize %d -> %d"),
        BackpackCapacity,
        NewCapacity);

    const int32 OldCapacity = Inventory.Items.Num();

    BackpackCapacity = NewCapacity;

    /*
    ================================
        HANDLE OVERFLOW
    ================================
    */

    if (Inventory.Items.Num() > NewCapacity)
    {
        TArray<FInventoryItem> OverflowItems;

        // собираем предметы за пределами нового размера
        for (int32 i = NewCapacity; i < Inventory.Items.Num(); ++i)
        {
            if (!Inventory.Items[i].IsEmpty())
            {
                OverflowItems.Add(Inventory.Items[i]);
            }
        }

        // реально уменьшаем массив
        Inventory.Items.SetNum(NewCapacity);
        Inventory.MarkArrayDirty();

        // пытаемся разместить предметы в пустые слоты
        for (const FInventoryItem& Overflow : OverflowItems)
        {
            bool bPlaced = false;

            for (int32 j = 0; j < NewCapacity; ++j)
            {
                if (Inventory.Items[j].IsEmpty())
                {
                    // очищаем слот (защита FastArray)
                    Inventory.Items[j].ItemRowName = NAME_None;
                    Inventory.Items[j].Quantity = 0;

                    // переносим данные предмета
                    Inventory.Items[j].ItemRowName = Overflow.ItemRowName;
                    Inventory.Items[j].Quantity = Overflow.Quantity;
                    Inventory.Items[j].SlotIndex = j;

                    Inventory.MarkItemDirty(Inventory.Items[j]);

                    bPlaced = true;
                    break;
                }
            }

            // если свободного места нет — дропаем предмет в мир
            if (!bPlaced)
            {
                if (!WorldItemClass)
                    continue;

                AActor* OwnerActor = GetOwner();
                if (!OwnerActor)
                    continue;

                FVector SpawnLocation =
                    OwnerActor->GetActorLocation() +
                    OwnerActor->GetActorForwardVector() * 150.f +
                    FVector(0.f, 0.f, 50.f);

                FActorSpawnParameters SpawnParams;
                SpawnParams.Owner = OwnerActor;
                SpawnParams.Instigator = Cast<APawn>(OwnerActor);
                SpawnParams.SpawnCollisionHandlingOverride =
                    ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

                AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
                    WorldItemClass,
                    SpawnLocation,
                    FRotator::ZeroRotator,
                    SpawnParams
                );

                if (!SpawnedActor)
                    continue;

                static FName ItemIDName(TEXT("ItemID"));
                if (FProperty* Property =
                    SpawnedActor->GetClass()->FindPropertyByName(ItemIDName))
                {
                    FName* ValuePtr =
                        Property->ContainerPtrToValuePtr<FName>(SpawnedActor);

                    if (ValuePtr)
                        *ValuePtr = Overflow.ItemRowName;
                }

                static FName QuantityName(TEXT("Quantity"));
                if (FProperty* QuantityProperty =
                    SpawnedActor->GetClass()->FindPropertyByName(QuantityName))
                {
                    int32* QuantityPtr =
                        QuantityProperty->ContainerPtrToValuePtr<int32>(SpawnedActor);

                    if (QuantityPtr)
                        *QuantityPtr = Overflow.Quantity;
                }
            }
        }
    }

    if (NewCapacity > OldCapacity)
    {
        for (int32 i = OldCapacity; i < NewCapacity; i++)
        {
            FInventoryItem NewItem;
            NewItem.ItemRowName = NAME_None;
            NewItem.Quantity = 0;
            NewItem.SlotIndex = i;

            Inventory.Items.Add(NewItem);
            Inventory.MarkArrayDirty();

            Inventory.MarkItemDirty(Inventory.Items.Last());
        }
    }

    NotifyInventoryUpdated();
}

void UAAAInventoryComponent::Server_EquipItem_Implementation(
    int32 InventorySlot,
    EEquipmentSlot EquipmentSlot
)
{
    UE_LOG(LogTemp, Warning, TEXT("SERVER EQUIP EXECUTED"));

    if (InventorySlot < 0)
        return;

    if (!Inventory.Items.IsValidIndex(InventorySlot))
        return;

    FInventoryItem& Item = Inventory.Items[InventorySlot];

    if (Item.IsEmpty())
        return;

    /*
    ================================
        VALIDATE BACKPACK TYPE
    ================================
    */

    if (EquipmentSlot == EEquipmentSlot::Backpack)
    {
        if (!ItemDataTable)
            return;

        static const FString Context(TEXT("EquipCheck"));

        FItemDataRow* Row =
            ItemDataTable->FindRow<FItemDataRow>(Item.ItemRowName, Context);

        if (!Row)
            return;

        if (Row->ItemType != EItemType::Backpack)
            return;
    }

    FInventoryItem* EquipSlot = Equipment.GetItem(EquipmentSlot);

    if (!EquipSlot)
        return;

    /*
    ================================
        DETERMINE FUTURE BACKPACK
    ================================
    */

    int32 NewCapacity = BackpackCapacity;

    if (EquipmentSlot == EEquipmentSlot::Backpack)
    {
        NewCapacity = DefaultInventorySlots;

        if (ItemDataTable)
        {
            static const FString Context(TEXT("BackpackLookup"));

            if (FItemDataRow* Row =
                ItemDataTable->FindRow<FItemDataRow>(Item.ItemRowName, Context))
            {
                if (Row->ItemType == EItemType::Backpack)
                {
                    NewCapacity = Row->BackpackSlots;
                }
            }
        }
    }

    /*
    ================================
        SWAP INVENTORY ↔ EQUIPMENT
    ================================
    */

    if (EquipSlot->IsEmpty())
    {
        *EquipSlot = Item;

        Item.ItemRowName = NAME_None;
        Item.Quantity = 0;
    }
    else
    {
        FInventoryItem Temp = *EquipSlot;
        *EquipSlot = Item;
        Item = Temp;
    }

    Item.SlotIndex = InventorySlot;

    Inventory.MarkItemDirty(Item);
    //Inventory.MarkArrayDirty();

    /*
    ================================
        APPLY BACKPACK CAPACITY
    ================================
    */

    if (EquipmentSlot == EEquipmentSlot::Backpack)
    {
        SetBackpackCapacity(NewCapacity);
    }

    /*
    ================================
        UI NOTIFY
    ================================
    */

    OnInventoryItemAdded.Broadcast(InventorySlot);
    OnInventorySlotChanged.Broadcast(InventorySlot);

    NotifyInventoryUpdated();
}

void UAAAInventoryComponent::Server_UnequipItem_Implementation(
    EEquipmentSlot EquipmentSlot,
    int32 InventorySlot
)
{
    if (!Inventory.Items.IsValidIndex(InventorySlot))
        return;

    FInventoryItem* EquipItem = Equipment.GetItem(EquipmentSlot);
    if (!EquipItem || EquipItem->IsEmpty())
        return;

    FInventoryItem& Target = Inventory.Items[InventorySlot];

    // слот инвентаря должен быть пустой
    if (!Target.IsEmpty())
        return;

    /*
    ===============================
        MOVE EQUIPMENT → INVENTORY
    ===============================
    */

    Target.ItemRowName = EquipItem->ItemRowName;
    Target.Quantity = EquipItem->Quantity;
    Target.SlotIndex = InventorySlot;

    EquipItem->Reset();

    Inventory.MarkItemDirty(Target);

    /*
    ================================
        BACKPACK CAPACITY UPDATE
    ================================
    */

    if (EquipmentSlot == EEquipmentSlot::Backpack)
    {
        FInventoryItem* Backpack =
            Equipment.GetItem(EEquipmentSlot::Backpack);

        if (Backpack && !Backpack->IsEmpty())
        {
            ApplyBackpackSlots(Backpack->ItemRowName);
        }
        else
        {
            BackpackCapacity = DefaultInventorySlots;
        }

        SetBackpackCapacity(BackpackCapacity);
    }

    OnInventoryItemAdded.Broadcast(InventorySlot);
    OnInventorySlotChanged.Broadcast(InventorySlot);

    NotifyInventoryUpdated();
}

bool UAAAInventoryComponent::AddItem_Internal(
    FName ItemRowName,
    int32 Quantity,
    int32 ContainerID
)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        Server_AddItem(ItemRowName, Quantity, ContainerID);
        return false;
    }

    if (ItemRowName == NAME_None || Quantity <= 0)
        return false;

    for (FInventoryItem& Item : Inventory.Items)
    {
        if (Item.ItemRowName == ItemRowName &&
            Item.ItemRowName != NAME_None)
        {
            if (!ItemDataTable)
                break;

            static const FString Context(TEXT("StackCheck"));

            FItemDataRow* Row = ItemDataTable->FindRow<FItemDataRow>(
                ItemRowName,
                Context
            );

            if (!Row || !Row->bStackable)
                break;

            if (Item.Quantity >= Row->MaxStackSize)
                break;

            Item.Quantity += Quantity;

            Inventory.MarkItemDirty(Item);

            OnInventorySlotChanged.Broadcast(Item.SlotIndex);

            NotifyInventoryUpdated();
            return true;
        }
    }

    for (int32 i = 0; i < Inventory.Items.Num(); i++)
    {
        FInventoryItem& Item = Inventory.Items[i];

        if (Item.ItemRowName == NAME_None)
        {
            Item.ItemRowName = ItemRowName;
            Item.Quantity = Quantity;
            Item.SlotIndex = i;

            Inventory.MarkItemDirty(Item);

            OnInventoryItemAdded.Broadcast(i);
            OnInventorySlotChanged.Broadcast(i);

            NotifyInventoryUpdated();
            return true;
        }
    }

    return false;
}

void UAAAInventoryComponent::Server_AddItem_Implementation(
    FName ItemRowName,
    int32 Quantity,
    int32 ContainerID
)
{
    AddItem_Internal(ItemRowName, Quantity, ContainerID);
}

void UAAAInventoryComponent::RemoveItem_Internal(
    int32 SlotIndex,
    int32 Quantity
)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        Server_RemoveItem(SlotIndex, Quantity);
        return;
    }

    if (!Inventory.Items.IsValidIndex(SlotIndex))
        return;

    FInventoryItem& Item = Inventory.Items[SlotIndex];

    if (Item.ItemRowName == NAME_None)
        return;

    Item.Quantity -= Quantity;

    if (Item.Quantity <= 0)
    {
        Item.ItemRowName = NAME_None;
        Item.Quantity = 0;
    }

    Inventory.MarkItemDirty(Item);

    OnInventorySlotChanged.Broadcast(SlotIndex);

    NotifyInventoryUpdated();
}

void UAAAInventoryComponent::Server_RemoveItem_Implementation(
    int32 SlotIndex,
    int32 Quantity
)
{
    RemoveItem_Internal(SlotIndex, Quantity);
}

void UAAAInventoryComponent::Server_DropItem_Implementation(
    int32 SlotIndex,
    int32 Quantity
)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
        return;

    if (!Inventory.Items.IsValidIndex(SlotIndex))
        return;

    FInventoryItem& Item = Inventory.Items[SlotIndex];

    if (Item.IsEmpty())
        return;

    FName RowName = Item.ItemRowName;
    int32 DropQuantity = Item.Quantity;

    RemoveItem_Internal(SlotIndex, Quantity);

    if (!WorldItemClass)
        return;

    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
        return;

    FVector SpawnLocation =
        OwnerActor->GetActorLocation() +
        OwnerActor->GetActorForwardVector() * 150.f +
        FVector(0.f, 0.f, 50.f);

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = OwnerActor;
    SpawnParams.Instigator = Cast<APawn>(OwnerActor);
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    UWorld* World = GetWorld();
    if (!World)
        return;

    AActor* SpawnedActor = World->SpawnActor<AActor>(
        WorldItemClass,
        SpawnLocation,
        FRotator::ZeroRotator,
        SpawnParams
    );

    if (!SpawnedActor)
        return;

    static FName ItemIDName(TEXT("ItemID"));
    if (FProperty* Property =
        SpawnedActor->GetClass()->FindPropertyByName(ItemIDName))
    {
        FName* ValuePtr =
            Property->ContainerPtrToValuePtr<FName>(SpawnedActor);

        if (ValuePtr)
            *ValuePtr = RowName;
    }

    static FName QuantityName(TEXT("Quantity"));
    if (FProperty* QuantityProperty =
        SpawnedActor->GetClass()->FindPropertyByName(QuantityName))
    {
        int32* QuantityPtr =
            QuantityProperty->ContainerPtrToValuePtr<int32>(SpawnedActor);

        if (QuantityPtr)
            *QuantityPtr = DropQuantity;
    }
}

void UAAAInventoryComponent::ApplyBackpackSlots(FName BackpackRow)
{
    if (!ItemDataTable)
    {
        BackpackCapacity = DefaultInventorySlots;
        return;
    }

    const FItemDataRow* Row =
        ItemDataTable->FindRow<FItemDataRow>(
            BackpackRow,
            TEXT("BackpackLookup")
        );

    if (!Row)
    {
        BackpackCapacity = DefaultInventorySlots;
        return;
    }

    if (Row->ItemType != EItemType::Backpack)
    {
        BackpackCapacity = DefaultInventorySlots;
        return;
    }

    BackpackCapacity = Row->BackpackSlots;
}


void UAAAInventoryComponent::Server_SwapSlots_Implementation(
    int32 FromIndex,
    int32 ToIndex
)
{
    UE_LOG(LogTemp, Warning,
        TEXT("SERVER SWAP From=%d To=%d InventorySize=%d"),
        FromIndex,
        ToIndex,
        Inventory.Items.Num());

    if (!Inventory.Items.IsValidIndex(FromIndex) ||
        !Inventory.Items.IsValidIndex(ToIndex) ||
        FromIndex == ToIndex)
    {
        return;
    }

    UE_LOG(LogTemp, Warning,
        TEXT("BEFORE SWAP: Slot %d = %s | Slot %d = %s"),
        FromIndex,
        *Inventory.Items[FromIndex].ItemRowName.ToString(),
        ToIndex,
        *Inventory.Items[ToIndex].ItemRowName.ToString());

    Inventory.Items.Swap(FromIndex, ToIndex);

    UE_LOG(LogTemp, Warning,
        TEXT("AFTER SWAP: Slot %d = %s | Slot %d = %s"),
        FromIndex,
        *Inventory.Items[FromIndex].ItemRowName.ToString(),
        ToIndex,
        *Inventory.Items[ToIndex].ItemRowName.ToString());

    Inventory.Items[FromIndex].SlotIndex = FromIndex;
    Inventory.Items[ToIndex].SlotIndex = ToIndex;

    Inventory.MarkItemDirty(Inventory.Items[FromIndex]);
    Inventory.MarkItemDirty(Inventory.Items[ToIndex]);

    Inventory.MarkArrayDirty();

    NotifyInventoryUpdated();
}

const TArray<FInventoryItem>& UAAAInventoryComponent::GetInventory() const
{
    return Inventory.Items;
}

const FEquipmentContainer& UAAAInventoryComponent::GetEquipment() const
{
    return Equipment;
}

FInventoryItem UAAAInventoryComponent::GetEquipmentItem(EEquipmentSlot Slot) const
{
    const FInventoryItem* Item = Equipment.GetItemConst(Slot);

    if (Item)
    {
        return *Item;
    }

    return FInventoryItem();
}

void UAAAInventoryComponent::NotifyInventoryUpdated()
{
    OnInventoryUpdated.Broadcast(Inventory.Items);
}