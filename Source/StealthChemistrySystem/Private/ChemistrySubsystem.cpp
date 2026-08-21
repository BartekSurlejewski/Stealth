#include "ChemistrySubsystem.h"
#include "ChemistrySettings.h"
#include "ChemistryReactionDataAsset.h"
#include "ChemistryEffect.h"
#include "ChemistryReceiverInterface.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"

void UChemistrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UChemistrySubsystem::Deinitialize()
{
	UnregisterMessageListener();
	ActiveReactionRules.Empty();
	InstantiatedEffectHandlers.Empty();
	ActiveDataAsset = nullptr;

	Super::Deinitialize();
}

void UChemistrySubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	LoadConfiguration();
	RegisterMessageListener();
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
	else if (!Settings->DefaultReactionDataTable.IsNull())
	{
		UDataTable* LoadedTable = Settings->DefaultReactionDataTable.LoadSynchronous();
		if (LoadedTable)
		{
			LoadedTable->ForeachRow<FChemistryReactionRule>(
				TEXT("UChemistrySubsystem::LoadConfiguration"),
				[this](const FName& RowName, const FChemistryReactionRule& Row)
				{
					ActiveReactionRules.Add(Row);
				});
		}
	}

	// Register global effect handlers
	for (const auto& Kvp : Settings->GlobalEffectHandlers)
	{
		if (Kvp.Key.IsValid() && Kvp.Value)
		{
			RegisterEffectHandler(Kvp.Key, Kvp.Value);
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

	// 1. Direct Target Actor Reaction
	if (IsValid(TargetActor))
	{
		EvaluateTargetReactions(TargetActor, Application);
	}

	// 2. Radial / Area Reaction
	if (Application.Radius > 0.0f)
	{
		EvaluateRadialReactions(Application);
	}
	else if (!IsValid(TargetActor))
	{
		// 3. Location / Ambient Reaction without specific target
		EvaluateLocationReactions(Application);

		// 4. Standalone Emitter Reaction
		if (IsValid(EmitterActor))
		{
			EvaluateEmitterReactions(EmitterActor, Application);
		}
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

	const FGameplayTagContainer TargetMaterials = GetMaterialTagsForActor(TargetActor);
	AActor* EmitterActor = Application.Instigator.Get();

	for (const FChemistryReactionRule& Rule : ActiveReactionRules)
	{
		if (MatchesRule(Rule, Application.ElementTag, TargetMaterials))
		{
			DispatchEffects(Rule, Application, TargetActor, EmitterActor);
		}
	}
}

void UChemistrySubsystem::EvaluateEmitterReactions(AActor* EmitterActor, const FElementApplication& Application)
{
	if (!IsValid(EmitterActor))
	{
		return;
	}

	const FGameplayTagContainer EmitterMaterials = GetMaterialTagsForActor(EmitterActor);

	for (const FChemistryReactionRule& Rule : ActiveReactionRules)
	{
		if (Rule.ReactionTarget == EChemistryReactionTarget::EmitterOnly ||
			Rule.ReactionTarget == EChemistryReactionTarget::BothTargetAndEmitter)
		{
			if (MatchesRule(Rule, Application.ElementTag, EmitterMaterials))
			{
				DispatchEffects(Rule, Application, nullptr, EmitterActor);
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
	AActor* EmitterActor = Application.Instigator.Get();
	const FGameplayTagContainer EmptyMaterials;

	for (const FChemistryReactionRule& Rule : ActiveReactionRules)
	{
		// Location/ambient rules have no required materials
		if (Rule.RequiredMaterialTags.IsEmpty() && MatchesRule(Rule, Application.ElementTag, EmptyMaterials))
		{
			DispatchEffects(Rule, Application, nullptr, EmitterActor);
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

void UChemistrySubsystem::DispatchEffects(const FChemistryReactionRule& Rule, const FElementApplication& Application, AActor* TargetActor, AActor* EmitterActor)
{
	FElementApplication AdjustedContext = Application;
	AdjustedContext.Magnitude *= Rule.EffectMagnitudeMultiplier;

	TArray<FGameplayTag> EffectsArray;
	Rule.ResultingEffects.GetGameplayTagArray(EffectsArray);

	switch (Rule.ReactionTarget)
	{
	case EChemistryReactionTarget::TargetOnly:
		for (const FGameplayTag& EffectTag : EffectsArray)
		{
			ExecuteSingleEffect(EffectTag, AdjustedContext, TargetActor);
		}
		break;

	case EChemistryReactionTarget::EmitterOnly:
		for (const FGameplayTag& EffectTag : EffectsArray)
		{
			ExecuteSingleEffect(EffectTag, AdjustedContext, EmitterActor);
		}
		break;

	case EChemistryReactionTarget::BothTargetAndEmitter:
		for (const FGameplayTag& EffectTag : EffectsArray)
		{
			ExecuteSingleEffect(EffectTag, AdjustedContext, TargetActor);
			ExecuteSingleEffect(EffectTag, AdjustedContext, EmitterActor);
		}
		break;

	case EChemistryReactionTarget::LocationOnly:
		for (const FGameplayTag& EffectTag : EffectsArray)
		{
			ExecuteSingleEffect(EffectTag, AdjustedContext, nullptr);
		}
		break;

	default:
		break;
	}
}

void UChemistrySubsystem::ExecuteSingleEffect(const FGameplayTag& EffectTag, const FElementApplication& Context, AActor* AffectedActor)
{
	if (!EffectTag.IsValid())
	{
		return;
	}

	// 1. Give custom receiver logic on Actor and its components a chance to react
	if (IsValid(AffectedActor))
	{
		if (AffectedActor->GetClass()->ImplementsInterface(UChemistryReceiverInterface::StaticClass()))
		{
			IChemistryReceiverInterface::Execute_OnReceiveChemistryEffect(AffectedActor, EffectTag, Context);
		}

		for (UActorComponent* Component : AffectedActor->GetComponents())
		{
			if (IsValid(Component) && Component->GetClass()->ImplementsInterface(UChemistryReceiverInterface::StaticClass()))
			{
				IChemistryReceiverInterface::Execute_OnReceiveChemistryEffect(Component, EffectTag, Context);
			}
		}
	}

	// 2. Execute registered UChemistryEffect handler object
	if (TObjectPtr<UChemistryEffect>* FoundHandler = InstantiatedEffectHandlers.Find(EffectTag))
	{
		if (IsValid(*FoundHandler))
		{
			(*FoundHandler)->ExecuteEffect(EffectTag, Context, AffectedActor);
		}
	}
}

FGameplayTagContainer UChemistrySubsystem::GetMaterialTagsForActor(const AActor* Actor) const
{
	FGameplayTagContainer CombinedTags;
	if (!IsValid(Actor))
	{
		return CombinedTags;
	}

	// Check actor interface implementation
	if (Actor->GetClass()->ImplementsInterface(UChemistryReceiverInterface::StaticClass()))
	{
		CombinedTags.AppendTags(IChemistryReceiverInterface::Execute_GetMaterialTags(Actor));
	}

	// Check component interface implementations
	for (UActorComponent* Component : Actor->GetComponents())
	{
		if (IsValid(Component) && Component->GetClass()->ImplementsInterface(UChemistryReceiverInterface::StaticClass()))
		{
			CombinedTags.AppendTags(IChemistryReceiverInterface::Execute_GetMaterialTags(Component));
		}
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
