#include "ChemistrySubsystem.h"

#include "ChemistryComponent.h"
#include "ChemistrySettings.h"
#include "ChemistryReactionDataAsset.h"
#include "ChemistryEffect.h"
#include "ChemistryReceiverInterface.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "StealthChemistrySystem/StealthChemistrySystem.h"

void UChemistrySubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	LoadConfiguration();
	RegisterMessageListener();
}

void UChemistrySubsystem::OnWorldEndPlay(UWorld& InWorld)
{
	UnregisterMessageListener();
	ActiveReactionRules.Empty();
	InstantiatedEffectHandlers.Empty();
	ActiveDataAsset = nullptr;

	Super::OnWorldEndPlay(InWorld);
}

void UChemistrySubsystem::LoadConfiguration()
{
	const UChemistrySettings* Settings = GetDefault<UChemistrySettings>();
	if (!Settings)
	{
		return;
	}

	ActiveMessageChannelTag = Settings->MessageChannelTag;

	// Load default reaction data asset if configured
	if (!Settings->DefaultReactionDataAsset.IsNull())
	{
		UChemistryReactionDataAsset* LoadedAsset = Settings->DefaultReactionDataAsset.LoadSynchronous();
		if (LoadedAsset)
		{
			SetReactionDataAsset(LoadedAsset);
		}
	}
}

void UChemistrySubsystem::RegisterMessageListener()
{
	if (!ActiveMessageChannelTag.IsValid())
	{
		return;
	}

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MessageListenerHandle = MessageSubsystem.RegisterListener<FElementApplication>(
		ActiveMessageChannelTag,
		this,
		&UChemistrySubsystem::HandleElementAppliedMessage);
}

void UChemistrySubsystem::UnregisterMessageListener()
{
	if (MessageListenerHandle.IsValid())
	{
		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
		MessageSubsystem.UnregisterListener(MessageListenerHandle);
	}
}

void UChemistrySubsystem::HandleElementAppliedMessage(FGameplayTag Channel, const FElementApplication& Payload)
{
	ProcessElementApplication(Payload);
}

void UChemistrySubsystem::ApplyElement(const FElementApplication& Application)
{
	if (ActiveMessageChannelTag.IsValid())
	{
		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
		MessageSubsystem.BroadcastMessage(ActiveMessageChannelTag, Application);
	}
	else
	{
		// Fallback to direct local processing if no message channel configured
		ProcessElementApplication(Application);
	}
}

void UChemistrySubsystem::ProcessElementApplication(const FElementApplication& Application)
{
	if (!Application.ElementTag.IsValid())
	{
		return;
	}

	AActor* TargetActor = Application.TargetActor.Get();
	AActor* EmitterActor = Application.Instigator.Get();

	if (IsValid(TargetActor))
	{
		EvaluateTargetReactions(TargetActor, Application); // TargetOnly + Both, matched on target materials
	}

	if (Application.Radius > 0.0f)
	{
		EvaluateRadialReactions(Application);
	}
	else if (!IsValid(TargetActor))
	{
		EvaluateLocationReactions(Application);
	}

	// Runs regardless of target/radius, so EmitterOnly/Both rules fire on direct hits too.
	if (IsValid(EmitterActor))
	{
		EvaluateEmitterReactions(EmitterActor, Application); // EmitterOnly + Both, matched on emitter materials
	}
}

void UChemistrySubsystem::EvaluateTargetReactions(AActor* TargetActor, const FElementApplication& Application)
{
	if (!IsValid(TargetActor))
	{
		return;
	}

	// Notify actor of element arrival
	if (TargetActor->GetClass()->ImplementsInterface(UChemistryReceiverInterface::StaticClass()))
	{
		IChemistryReceiverInterface::Execute_OnReceiveElementApplication(TargetActor, Application);
	}

	for (UActorComponent* Component : TargetActor->GetComponents())
	{
		if (IsValid(Component) && Component->GetClass()->ImplementsInterface(UChemistryReceiverInterface::StaticClass()))
		{
			IChemistryReceiverInterface::Execute_OnReceiveElementApplication(Component, Application);
		}
	}

	// HitComponent only applies to the actor that was actually struck directly —
	// not to actors incidentally caught by a radial/AoE sweep.
	const UPrimitiveComponent* RelevantHitComponent = (TargetActor == Application.TargetActor.Get()) ? Application.HitComponent.Get() : nullptr;
	const FGameplayTagContainer TargetMaterials = GetMaterialTagsForActor(TargetActor, RelevantHitComponent);

	for (const FChemistryReactionRule& Rule : ActiveReactionRules)
	{
		if (Rule.ReactionTarget != EChemistryReactionTarget::TargetOnly &&
			Rule.ReactionTarget != EChemistryReactionTarget::BothTargetAndEmitter)
		{
			continue;
		}
		if (MatchesRule(Rule, Application.ElementTag, TargetMaterials))
		{
			ApplyRuleEffects(Rule, Application, TargetActor); // applies only to TargetActor
		}
	}
}

void UChemistrySubsystem::EvaluateEmitterReactions(AActor* EmitterActor, const FElementApplication& Application)
{
	if (!IsValid(EmitterActor))
	{
		return;
	}

	//TODO: add considering target materials too
	const FGameplayTagContainer EmitterMaterials = GetMaterialTagsForActor(EmitterActor);

	for (const FChemistryReactionRule& Rule : ActiveReactionRules)
	{
		if (Rule.ReactionTarget == EChemistryReactionTarget::EmitterOnly ||
			Rule.ReactionTarget == EChemistryReactionTarget::BothTargetAndEmitter)
		{
			if (MatchesRule(Rule, Application.ElementTag, EmitterMaterials))
			{
				ApplyRuleEffects(Rule, Application, EmitterActor);
			}
		}
	}
}

void UChemistrySubsystem::EvaluateRadialReactions(const FElementApplication& Application)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const UChemistrySettings* Settings = GetDefault<UChemistrySettings>();
	FCollisionObjectQueryParams ObjectQueryParams;
	if (Settings && Settings->RadialQueryCollisionChannels.Num() > 0)
	{
		for (ECollisionChannel Channel : Settings->RadialQueryCollisionChannels)
		{
			ObjectQueryParams.AddObjectTypesToQuery(Channel);
		}
	}
	else
	{
		ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
		ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
		ObjectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
		ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	}

	//TODO: Use DynamicOctree plugin? https://github.com/BenVlodgi/UE-DynamicOctree
	TArray<FOverlapResult> Overlaps;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(Application.Radius);
	World->OverlapMultiByObjectType(Overlaps, Application.Location, FQuat::Identity, ObjectQueryParams, SphereShape);

	TSet<AActor*> ProcessedActors;
	AActor* DirectTarget = Application.TargetActor.Get();
	if (IsValid(DirectTarget))
	{
		ProcessedActors.Add(DirectTarget);
	}

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (IsValid(HitActor) && !ProcessedActors.Contains(HitActor))
		{
			ProcessedActors.Add(HitActor);
			EvaluateTargetReactions(HitActor, Application);
		}
	}
}

void UChemistrySubsystem::EvaluateLocationReactions(const FElementApplication& Application)
{
	static const FGameplayTagContainer EmptyMaterials;

	for (const FChemistryReactionRule& Rule : ActiveReactionRules)
	{
		if (Rule.ReactionTarget != EChemistryReactionTarget::LocationOnly)
		{
			continue;
		}

		if (!Rule.RequiredMaterialTags.IsEmpty())
		{
			// Data-authoring mistake: a LocationOnly rule with required materials can never
			// match (there's no actor here to check tags against), so it would silently never fire.
			UE_LOG(LogStealthChemistry, Warning, TEXT("Chemistry: LocationOnly rule for %s has RequiredMaterialTags set and will never match."), *Rule.ElementTag.ToString());
			continue;
		}

		if (MatchesRule(Rule, Application.ElementTag, EmptyMaterials))
		{
			ApplyRuleEffects(Rule, Application, nullptr);
		}
	}
}

bool UChemistrySubsystem::MatchesRule(const FChemistryReactionRule& Rule, const FGameplayTag& ElementTag, const FGameplayTagContainer& MaterialTags) const
{
	if (Rule.ElementTag != ElementTag)
	{
		return false;
	}

	if (!Rule.BlockedMaterialTags.IsEmpty() && MaterialTags.HasAny(Rule.BlockedMaterialTags))
	{
		return false;
	}

	if (Rule.RequiredMaterialTags.IsEmpty())
	{
		return true;
	}

	if (Rule.bRequireAllMaterialTags)
	{
		return MaterialTags.HasAll(Rule.RequiredMaterialTags);
	}

	return MaterialTags.HasAny(Rule.RequiredMaterialTags);
}

void UChemistrySubsystem::ApplyRuleEffects(const FChemistryReactionRule& Rule, const FElementApplication& Application, AActor* AffectedActor)
{
	FElementApplication AdjustedContext = Application;
	AdjustedContext.Magnitude *= Rule.EffectMagnitudeMultiplier;

	TArray<FGameplayTag> EffectsArray;
	Rule.ResultingEffects.GetGameplayTagArray(EffectsArray);
	for (const FGameplayTag& EffectTag : EffectsArray)
	{
		ExecuteSingleEffect(EffectTag, AdjustedContext, AffectedActor);
	}
}

void UChemistrySubsystem::ExecuteSingleEffect(const FGameplayTag& EffectTag, const FElementApplication& Context, AActor* AffectedActor)
{
	if (!EffectTag.IsValid())
	{
		return;
	}

	// 1. Give custom receiver logic on Actor and its components a chance to react
	bool bHandled = false;
	if (IsValid(AffectedActor))
	{
		if (AffectedActor->GetClass()->ImplementsInterface(UChemistryReceiverInterface::StaticClass()))
		{
			bHandled |= IChemistryReceiverInterface::Execute_OnReceiveChemistryEffect(AffectedActor, EffectTag, Context);
		}
		for (UActorComponent* Component : AffectedActor->GetComponents())
		{
			if (IsValid(Component) && Component->GetClass()->ImplementsInterface(UChemistryReceiverInterface::StaticClass()))
			{
				bHandled |= IChemistryReceiverInterface::Execute_OnReceiveChemistryEffect(Component, EffectTag, Context);
			}
		}
	}

	if (!bHandled)
	{
		// 2. Execute registered UChemistryEffect handler object
		if (TObjectPtr<UChemistryEffect>* FoundHandler = InstantiatedEffectHandlers.Find(EffectTag))
		{
			if (IsValid(*FoundHandler))
			{
				(*FoundHandler)->ExecuteEffect(EffectTag, Context, AffectedActor);
			}
		}
	}
}

FGameplayTagContainer UChemistrySubsystem::GetMaterialTagsForActor(const AActor* Actor, const UPrimitiveComponent* HitComponent) const
{
	FGameplayTagContainer CombinedTags;
	if (!IsValid(Actor))
	{
		return CombinedTags;
	}

	if (Actor->GetClass()->ImplementsInterface(UChemistryReceiverInterface::StaticClass()))
	{
		CombinedTags.AppendTags(IChemistryReceiverInterface::Execute_GetMaterialTags(Actor));
	}

	//TODO: avoid iterating through components
	// Check component interface implementations
	for (UActorComponent* Component : Actor->GetComponents())
	{
		if (!IsValid(Component) || !Component->GetClass()->ImplementsInterface(UChemistryReceiverInterface::StaticClass()))
		{
			continue;
		}

		// Component-scoped receivers only contribute their tags when their specific
		// primitive is the one that was actually hit.
		if (const UChemistryComponent* ChemComponent = Cast<UChemistryComponent>(Component))
		{
			UPrimitiveComponent* Scope = ChemComponent->AssociatedPrimitiveComponent.Get();
			if (Scope && Scope != HitComponent)
			{
				continue;
			}
		}

		CombinedTags.AppendTags(IChemistryReceiverInterface::Execute_GetMaterialTags(Component));
	}

	return CombinedTags;
}

void UChemistrySubsystem::SetReactionDataAsset(UChemistryReactionDataAsset* InDataAsset)
{
	ActiveDataAsset = InDataAsset;
	ActiveReactionRules.Empty();

	if (IsValid(ActiveDataAsset))
	{
		ActiveDataAsset->GetAllRules(ActiveReactionRules);

		for (const auto& Kvp : ActiveDataAsset->EffectHandlers)
		{
			if (Kvp.Key.IsValid() && Kvp.Value)
			{
				RegisterEffectHandler(Kvp.Key, Kvp.Value);
			}
		}
	}
}

void UChemistrySubsystem::RegisterReactionRule(const FChemistryReactionRule& Rule)
{
	ActiveReactionRules.Add(Rule);
}

void UChemistrySubsystem::RegisterEffectHandler(const FGameplayTag& EffectTag, TSubclassOf<UChemistryEffect> EffectClass)
{
	if (!EffectTag.IsValid() || !EffectClass)
	{
		return;
	}

	UChemistryEffect* Handler = NewObject<UChemistryEffect>(this, EffectClass);
	InstantiatedEffectHandlers.Add(EffectTag, Handler);
}
