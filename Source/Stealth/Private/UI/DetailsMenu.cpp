#include "UI/DetailsMenu.h"

#include "Components/WidgetSwitcher.h"
#include "Messages/StealthMessages.h"

void UDetailsMenu::NativeConstruct()
{
	Super::NativeConstruct();

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	PrevSubmenuInputListenerHandle = MsgSubsystem.RegisterListener<FInputMessage>(StealthMessageChannels::TAG_Message_Input_DetailsMenu_Prev, this,
	                                                                              &UDetailsMenu::OnPrevSubmenuInput);
	NextSubmenuInputListenerHandle = MsgSubsystem.RegisterListener<FInputMessage>(StealthMessageChannels::TAG_Message_Input_DetailsMenu_Next, this,
	                                                                              &UDetailsMenu::OnNextSubmenuInput);

	DailyRegimenSubmenuInfo.Button = DailyRegimenButton;
	InventorySubmenuInfo.Button = InventoryButton;
	JournalSubmenuInfo.Button = JournalButton;

	SubmenusInfo = {DailyRegimenSubmenuInfo, InventorySubmenuInfo, JournalSubmenuInfo};
}

void UDetailsMenu::NativeDestruct()
{
	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	MsgSubsystem.UnregisterListener(PrevSubmenuInputListenerHandle);
	MsgSubsystem.UnregisterListener(NextSubmenuInputListenerHandle);
	Super::NativeDestruct();
}

void UDetailsMenu::Show(FGameplayTag SubmenuTag)
{
	// Since we're expecting small number of submenus, iterating over them wouldn't take noticeably longer than using a map
	for (const FSubmenuInfo& SubmenuInfo : SubmenusInfo)
	{
		if (SubmenuInfo.Tag.MatchesTagExact(SubmenuTag))
		{
			WidgetSwitcher->SetActiveWidgetIndex(SubmenuInfo.Index);
			return;
		}
	}
}

void UDetailsMenu::OnPrevSubmenuInput(FGameplayTag Channel, const FInputMessage& Message)
{
	WidgetSwitcher->SetActiveWidgetIndex((WidgetSwitcher->GetActiveWidgetIndex() - 1 + WidgetSwitcher->GetChildrenCount()) % WidgetSwitcher->GetChildrenCount());
}

void UDetailsMenu::OnNextSubmenuInput(FGameplayTag Channel, const FInputMessage& Message)
{
	WidgetSwitcher->SetActiveWidgetIndex((WidgetSwitcher->GetActiveWidgetIndex() + 1) % WidgetSwitcher->GetChildrenCount());
}
