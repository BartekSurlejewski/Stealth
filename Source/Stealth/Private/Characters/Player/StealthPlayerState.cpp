#include "Characters/Player/StealthPlayerState.h"

#include "QuestManagerSubsystem.h"
#include "Characters/Player/StealthCharacter.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/InventoryManagerSubsystem.h"
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

	if (InventoryComponent)
	{
		if (const auto InventoryManager = UInventoryManagerSubsystem::Get(this))
		{
			InventoryManager->RegisterPlayerInventory(InventoryComponent);
		}
	}

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	PlayerInventoryItemRemovedListenerHandle = MsgSubsystem.RegisterListener<FPlayerInventoryItemRemovedMessage>(StealthMessageChannels::TAG_Message_Inventory_ItemRemoved, this,
	                                                                                                             &AStealthPlayerState::OnPlayerInventoryItemRemoved);

	UQuestManagerSubsystem::Get(this)->PlayerInitialize();
}

void AStealthPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (InventoryComponent)
	{
		if (const auto InventoryManager = UInventoryManagerSubsystem::Get(this))
		{
			InventoryManager->UnregisterPlayerInventory(InventoryComponent);
		}
	}

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	MsgSubsystem.UnregisterListener(PlayerInventoryItemRemovedListenerHandle);

	Super::EndPlay(EndPlayReason);
}

void AStealthPlayerState::OnPlayerInventoryItemRemoved(FGameplayTag Channel, const FPlayerInventoryItemRemovedMessage& Message)
{
	if (!Message.bShouldDrop || !PlayerCharacter)
	{
		return;
	}

	TSoftClassPtr<AActor> SoftActorToDropClass = Message.RemovedItem->PickupActorClass;
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
		IPickable::Execute_Initialize(SpawnedActor, Message.RemovedItem, Message.RemovedQuantity, Message.RemovedItem->Name);
	}
}
