#include "Inventory/EquipmentTypes.h"
#include "Inventory/InventoryTypes.h"

bool FEquipmentContainer::IsSlotEmpty(EEquipmentSlot Slot) const
{
    switch (Slot)
    {
    case EEquipmentSlot::Helmet:
        return Helmet.IsEmpty();

    case EEquipmentSlot::Armor:
        return Armor.IsEmpty();

    case EEquipmentSlot::Backpack:
        return Backpack.IsEmpty();
    }

    return true;
}

FInventoryItem* FEquipmentContainer::GetItem(EEquipmentSlot Slot)
{
    switch (Slot)
    {
    case EEquipmentSlot::Helmet:
        return &Helmet;

    case EEquipmentSlot::Armor:
        return &Armor;

    case EEquipmentSlot::Backpack:
        return &Backpack;
    }

    return nullptr;
}

const FInventoryItem* FEquipmentContainer::GetItemConst(EEquipmentSlot Slot) const
{
    switch (Slot)
    {
    case EEquipmentSlot::Helmet:
        return &Helmet;

    case EEquipmentSlot::Armor:
        return &Armor;

    case EEquipmentSlot::Backpack:
        return &Backpack;
    }

    return nullptr;
}