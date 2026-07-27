#include "Characters/Player/StealthPlayerState.h"

#include "QuestManagerSubsystem.h"
#include "Characters/Player/StealthCharacter.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/Pickable.h"
#include "Inventory/Items/InventoryItem.h"
#include "Inventory/Items/ItemDefinition.h"
#include "Messages/StealthMessages.h"
#include "Stealth/Stealth.h"

AStealthPlayerState::AStealthPlayerState()
{
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("Inventory Component");
}

void AStealthPlayerState::SetIsInRestrictedArea(bool newIsInRestrictedArea)
{
	bIsInRestrictedArea = newIsInRestrictedArea;

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	FBooleanMessage Message(bIsInRestrictedArea);
	MsgSubsystem.BroadcastMessage(StealthMessageChannels::TAG_Message_Player_IsInRestrictedAreaChanged, Message);
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

	UQuestManagerSubsystem::Get(this)->PlayerInitialize();
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

	TSoftClassPtr<AActor> SoftActorToDropClass = Item.ItemDefinition->PickupActorClass;
	if (SoftActorToDropClass.IsNull())
	{
		return;
	}

	TSubclassOf<AActor> ActorToDropClass = SoftActorToDropClass.LoadSynchronous();
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
