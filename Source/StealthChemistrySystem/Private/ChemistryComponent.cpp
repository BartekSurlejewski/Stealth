#include "ChemistryComponent.h"

UChemistryComponent::UChemistryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bAutoActivate = true;
}

FGameplayTagContainer UChemistryComponent::GetMaterialTags_Implementation() const
{
	return MaterialTags;
}

bool UChemistryComponent::OnReceiveChemistryEffect_Implementation(const FGameplayTag& EffectTag, const FElementApplication& Context)
{
	OnEffectReceived.Broadcast(EffectTag, Context);
	return false;
}

void UChemistryComponent::OnReceiveElementApplication_Implementation(const FElementApplication& Context)
{
	OnElementApplied.Broadcast(Context);
}

void UChemistryComponent::AddMaterialTag(const FGameplayTag& Tag)
{
	if (Tag.IsValid())
	{
		MaterialTags.AddTag(Tag);
	}
}

void UChemistryComponent::RemoveMaterialTag(const FGameplayTag& Tag)
{
	if (Tag.IsValid())
	{
		MaterialTags.RemoveTag(Tag);
	}
}

bool UChemistryComponent::HasMaterialTag(const FGameplayTag& Tag) const
{
	return MaterialTags.HasTag(Tag);
}
