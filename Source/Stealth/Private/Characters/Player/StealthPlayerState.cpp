#include "Characters/Player/StealthPlayerState.h"

#include "Characters/Player/StealthCharacter.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/Pickable.h"
#include "Inventory/Items/InventoryItem.h"
#include "Inventory/Items/ItemDefinition.h"
#include "Stealth/Stealth.h"

AStealthPlayerState::AStealthPlayerState()
{
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("Inventory Component");
}

void AStealthPlayerState::BeginPlay()
{
	Super::BeginPlay();

	PlayerCharacter = Cast<AStealthCharacter>(GetPawn());
	if (!PlayerCharacter)
	{
		UE_LOG(LogStealth, Error, TEXT("PlayerState: Couldn't cast player character to AStealthCharacter."));
	}

	InventoryComponent->OnItemRemoved.AddDynamic(this, &AStealthPlayerState::InventoryComponent_OnItemRemoved);
}

void AStealthPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	InventoryComponent->OnItemRemoved.RemoveAll(this);

	Super::EndPlay(EndPlayReason);
}

void AStealthPlayerState::InventoryComponent_OnItemRemoved(FInventoryItem& Item, int Quantity, int QuantityInInventory, bool bShouldDrop)
{
	if (!bShouldDrop || !PlayerCharacter)
	{
		return;
	}

	TSubclassOf<AActor> ActorToDropClass = Item.ItemDefinition->PickupActorClass;
	if (!ActorToDropClass)
	{
		return;
	}

	AActor* SpawnedActor = PlayerCharacter->TryDropItem(ActorToDropClass);
	if (SpawnedActor && SpawnedActor->Implements<UPickable>())
	{
		IPickable::Execute_Initialize(SpawnedActor, Item.ItemDefinition, Item.Quantity, Item.ItemDefinition->Name);
	}
}
