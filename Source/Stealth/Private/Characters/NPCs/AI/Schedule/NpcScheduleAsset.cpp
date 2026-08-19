#include "Characters/NPCs/AI/Schedule/NpcScheduleAsset.h"

bool UNpcScheduleAsset::GetSlotForTime(int32 TimeInMinutes, FNpcScheduleSlot& OutSlot) const
{
	for (const FNpcScheduleSlot& Slot : Slots)
	{
		if (Slot.IsActiveAtTime(TimeInMinutes))
		{
			OutSlot = Slot;
			return true;
		}
	}
	return false;
}

bool UNpcScheduleAsset::GetNextSlot(int32 TimeInMinutes, FNpcScheduleSlot& OutSlot) const
{
	if (Slots.Num() == 0)
	{
		return false;
	}

	const FNpcScheduleSlot* BestSlot = nullptr;
	int32 MinDiff = MAX_int32;

	for (const FNpcScheduleSlot& Slot : Slots)
	{
		int32 SlotStart = Slot.GetStartTimeAsMinutes();
		int32 Diff = SlotStart - TimeInMinutes;
		if (Diff < 0)
		{
			Diff += 24 * 60; // Wrap around to next day
		}

		if (Diff > 0 && Diff < MinDiff)
		{
			MinDiff = Diff;
			BestSlot = &Slot;
		}
	}

	if (BestSlot)
	{
		OutSlot = *BestSlot;
		return true;
	}

	OutSlot = Slots[0];
	return true;
}
