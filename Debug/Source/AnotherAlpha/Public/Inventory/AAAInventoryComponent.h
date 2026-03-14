#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DataTable.h"

#include "Inventory/InventoryTypes.h"
#include "Inventory/EquipmentTypes.h"
#include "Data/CharacterLoadout.h"

#include "AAAInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnInventoryUpdated,
    const TArray<FInventoryItem>&,
    Inventory
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnInventoryItemAdded,
    int32,
    SlotIndex
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnInventorySlotChanged,
    int32,
    SlotIndex
);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ANOTHERALPHA_API UAAAInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UAAAInventoryComponent();
    virtual void BeginPlay() override;

protected:

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps
    ) const override;

    UPROPERTY(Replicated)
    FInventoryArray Inventory;

    UPROPERTY(ReplicatedUsing = OnRep_BackpackCapacity)
    int32 BackpackCapacity;

    UFUNCTION()
    void OnRep_BackpackCapacity();

    void InitializeInventory();

public:


    /*
    ====================================================
        ARC RAIDERS SLOT MODEL
    ====================================================
    */

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Arc")
    int32 DefaultInventorySlots = 10;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Arc")
    int32 CurrentInventorySlots = 10;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Arc")
    int32 QuickUseSlots = 4;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Arc")
    int32 SafePocketSlots = 1;

    /*
    ====================================================
        LEGACY CAPACITY
    ====================================================
    */

    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    TSubclassOf<AActor> WorldItemClass;

    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    UDataTable* ItemDataTable = nullptr;

    /*
    ====================================================
        DEFAULT BACKPACK
    ====================================================
    */

    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    FName DefaultBackpackItem;

    /*
    ====================================================
        EVENTS
    ====================================================
    */

    UPROPERTY(BlueprintAssignable)
    FOnInventoryUpdated OnInventoryUpdated;

    UPROPERTY(BlueprintAssignable)
    FOnInventoryItemAdded OnInventoryItemAdded;

    UPROPERTY(BlueprintAssignable)
    FOnInventorySlotChanged OnInventorySlotChanged;

    /*
    ====================================================
        EQUIPMENT
    ====================================================
    */

    UPROPERTY(ReplicatedUsing = OnRep_Equipment)
    FEquipmentContainer Equipment;

    UFUNCTION()
    void OnRep_Equipment();

    /*
    ====================================================
        LOADOUT
    ====================================================
    */

    UFUNCTION(BlueprintCallable)
    void ApplyLoadout(const FCharacterLoadout& Loadout);

    /*
    ====================================================
        BACKPACK
    ====================================================
    */

    UFUNCTION()
    void ApplyBackpackSlots(FName BackpackRow);

    UFUNCTION(BlueprintCallable)
    int32 GetBackpackCapacity() const;

    UFUNCTION(BlueprintCallable)
    void SetBackpackCapacity(int32 NewCapacity);

    /*
    ====================================================
        EQUIP
    ====================================================
    */

    UFUNCTION(Server, Reliable, BlueprintCallable)
    void Server_EquipItem(int32 InventorySlot, EEquipmentSlot EquipmentSlot);

    UFUNCTION(Server, Reliable, BlueprintCallable)
    void Server_UnequipItem(EEquipmentSlot EquipmentSlot, int32 InventorySlot);

    /*
    ====================================================
        ADD
    ====================================================
    */

    UFUNCTION(BlueprintCallable)
    bool AddItem_Internal(FName ItemRowName, int32 Quantity, int32 ContainerID);

    UFUNCTION(Server, Reliable)
    void Server_AddItem(FName ItemRowName, int32 Quantity, int32 ContainerID);

    /*
    ====================================================
        REMOVE
    ====================================================
    */

    UFUNCTION(BlueprintCallable)
    void RemoveItem_Internal(int32 SlotIndex, int32 Quantity);

    UFUNCTION(Server, Reliable)
    void Server_RemoveItem(int32 SlotIndex, int32 Quantity);

    /*
    ====================================================
        DROP
    ====================================================
    */

    UFUNCTION(Server, Reliable, BlueprintCallable)
    void Server_DropItem(int32 SlotIndex, int32 Quantity);

    /*
    ====================================================
        SWAP
    ====================================================
    */

    UFUNCTION(Server, Reliable, BlueprintCallable)
    void Server_SwapSlots(int32 FromIndex, int32 ToIndex);

    void Server_SwapSlots_Implementation(int32 FromIndex, int32 ToIndex);

    /*
    ====================================================
        GET
    ====================================================
    */

    UFUNCTION(BlueprintCallable)
    const TArray<FInventoryItem>& GetInventory() const;

    UFUNCTION(BlueprintPure)
    const FEquipmentContainer& GetEquipment() const;

    UFUNCTION(BlueprintPure)
    FInventoryItem GetEquipmentItem(EEquipmentSlot Slot) const;

    void NotifyInventoryUpdated();
};