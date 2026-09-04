#include "Characters/Player/Movement/StealthCharacterMovementComponent.h"

#include "Characters/Player/StealthPlayerCharacter.h"
#include "Stealth/Stealth.h"


UStealthCharacterMovementComponent::UStealthCharacterMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UStealthCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(true);
}

void UStealthCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	PlayerCharacterOwner = Cast<AStealthPlayerCharacter>(GetOwner());
}

void UStealthCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	if (MovementMode == MOVE_Walking && bWantsToSlide && CanSlide())
	{
		SetMovementMode(MOVE_Custom, CMOVE_Slide);
	}
	else if (IsInCustomMovementMode(CMOVE_Slide) && (!bWantsToSlide || !CanSlide()))
	{
		SetMovementMode(MOVE_Walking);
	}

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UStealthCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (MovementMode == MOVE_Custom)
	{
		OnCustomMovementModeEntered.Broadcast(static_cast<ECustomMovementMode>(CustomMovementMode));
	}

	if (PreviousMovementMode == MOVE_Custom)
	{
		OnCustomMovementModeExit.Broadcast(static_cast<ECustomMovementMode>(PreviousCustomMode));
	}

	if (IsInCustomMovementMode(CMOVE_Slide))
	{
		EnterSlide();
	}
	else if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Slide)
	{
		ExitSlide();
	}
}

void UStealthCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	Super::PhysCustom(DeltaTime, Iterations);

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		PhysSlide(DeltaTime, Iterations);
		break;
	default:
		UE_LOG(LogStealth, Fatal, TEXT("Invalid Custom Movement Mode"))
	}
}

bool UStealthCharacterMovementComponent::IsInCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}

void UStealthCharacterMovementComponent::EnterSlide()
{
	bWantsToCrouch = true;
	Velocity += Velocity.GetSafeNormal2D() * SlideMoveParams.EnterImpulse;

	FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, true, nullptr);
}

void UStealthCharacterMovementComponent::ExitSlide()
{
	bWantsToCrouch = false;
}

bool UStealthCharacterMovementComponent::CanSlide() const
{
	bool bValidSurface = CurrentFloor.IsWalkableFloor();
	bool bEnoughSpeed = Velocity.SizeSquared() > FMath::Square(SlideMoveParams.MinSpeed);

	return bValidSurface && bEnoughSpeed;
}

void UStealthCharacterMovementComponent::PhysSlide(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!CanSlide())
	{
		SetMovementMode(MOVE_Walking);
		StartNewPhysics(DeltaTime, Iterations);
		return;
	}

	bJustTeleported = false;
	bool bCheckedFall = false;
	bool bTriedLedgeMove = false;
	float remainingTime = DeltaTime;

	// Perform the move
	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || (
		CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)))
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		// Save current values
		UPrimitiveComponent* const OldBase = Cast<UPrimitiveComponent>(GetMovementBaseObject());
		const FVector PreviousBaseLocation = (OldBase != nullptr) ? OldBase->GetComponentLocation() : FVector::ZeroVector;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FFindFloorResult OldFloor = CurrentFloor;

		// Ensure velocity is horizontal.
		MaintainHorizontalGroundVelocity();
		const FVector OldVelocity = Velocity;

		FVector SlopeForce = CurrentFloor.HitResult.Normal;
		SlopeForce.Z = 0.f;
		Velocity += SlopeForce * SlideMoveParams.GravityForce * DeltaTime;

		if (FMath::Abs(FVector::DotProduct(Acceleration.GetSafeNormal(), UpdatedComponent->GetRightVector())) > 0.5f)
		{
			Acceleration = Acceleration.ProjectOnTo(UpdatedComponent->GetRightVector());
		}
		else
		{
			Acceleration = FVector::ZeroVector;
		}

		// Apply acceleration
		CalcVelocity(timeTick, SlideMoveParams.Friction, true, GetMaxBrakingDeceleration());

		// Compute move parameters
		const FVector MoveVelocity = Velocity;
		const FVector Delta = timeTick * MoveVelocity;
		const bool bZeroDelta = Delta.IsNearlyZero();
		FStepDownResult StepDownResult;
		bool bFloorWalkable = CurrentFloor.IsWalkableFloor();

		if (bZeroDelta)
		{
			remainingTime = 0.f;
		}
		else
		{
			// try to move forward
			MoveAlongFloor(MoveVelocity, timeTick, &StepDownResult);

			if (IsFalling())
			{
				// pawn decided to jump up
				const float DesiredDist = Delta.Size();
				if (DesiredDist > KINDA_SMALL_NUMBER)
				{
					const float ActualDist = (UpdatedComponent->GetComponentLocation() - OldLocation).Size2D();
					remainingTime += timeTick * (1.f - FMath::Min(1.f, ActualDist / DesiredDist));
				}
				StartNewPhysics(remainingTime, Iterations);
				return;
			}
			else if (IsSwimming()) //just entered water
			{
				StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
				return;
			}
		}

		// Update floor.
		// StepUp might have already done it for us.
		if (StepDownResult.bComputedFloor)
		{
			CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, NULL);
		}


		// check for ledges here
		const bool bCheckLedges = !CanWalkOffLedges();
		if (bCheckLedges && !CurrentFloor.IsWalkableFloor())
		{
			// calculate possible alternate movement
			const FVector NewDelta = bTriedLedgeMove ? FVector::ZeroVector : GetLedgeMove(OldLocation, Delta, CurrentFloor);
			if (!NewDelta.IsZero())
			{
				PRAGMA_DISABLE_DEPRECATION_WARNINGS
				// first revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, false);
				PRAGMA_ENABLE_DEPRECATION_WARNINGS

				// avoid repeated ledge moves if the first one fails
				bTriedLedgeMove = true;

				// Try new movement direction
				Velocity = NewDelta / timeTick;
				remainingTime += timeTick;
				continue;
			}
			else
			{
				// see if it is OK to jump
				// TODO: collision : only thing that can be problem is that oldbase has world collision on
				PRAGMA_DISABLE_DEPRECATION_WARNINGS
				bool bMustJump = bZeroDelta || (OldBase == nullptr || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				PRAGMA_ENABLE_DEPRECATION_WARNINGS

				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump))
				{
					return;
				}
				bCheckedFall = true;

				PRAGMA_DISABLE_DEPRECATION_WARNINGS
				// revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, true);
				PRAGMA_ENABLE_DEPRECATION_WARNINGS

				remainingTime = 0.f;
				break;
			}
		}
		else
		{
			// Validate the floor check
			if (CurrentFloor.IsWalkableFloor())
			{
				if (ShouldCatchAir(OldFloor, CurrentFloor))
				{
					HandleWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, timeTick);
					if (IsMovingOnGround())
					{
						// If still walking, then fall. If not, assume the user set a different mode they want to keep.
						StartFalling(Iterations, remainingTime, timeTick, Delta, OldLocation);
					}
					return;
				}

				AdjustFloorHeight();
				PRAGMA_DISABLE_DEPRECATION_WARNINGS
				SetBase(CurrentFloor.HitResult.Component.Get(), CurrentFloor.HitResult.BoneName);
				PRAGMA_ENABLE_DEPRECATION_WARNINGS
			}
			else if (CurrentFloor.HitResult.bStartPenetrating && remainingTime <= 0.f)
			{
				// The floor check failed because it started in penetration
				// We do not want to try to move downward because the downward sweep failed, rather we'd like to try to pop out of the floor.
				FHitResult Hit(CurrentFloor.HitResult);
				Hit.TraceEnd = Hit.TraceStart + FVector(0.f, 0.f, MAX_FLOOR_DIST);
				const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit);
				ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponent->GetComponentQuat());
				bForceNextFloorCheck = true;
			}

			// check if just entered water
			if (IsSwimming())
			{
				StartSwimming(OldLocation, Velocity, timeTick, remainingTime, Iterations);
				return;
			}

			// See if we need to start falling.
			if (!CurrentFloor.IsWalkableFloor() && !CurrentFloor.HitResult.bStartPenetrating)
			{
				PRAGMA_DISABLE_DEPRECATION_WARNINGS
				const bool bMustJump = bJustTeleported || bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				PRAGMA_ENABLE_DEPRECATION_WARNINGS

				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump))
				{
					return;
				}
				bCheckedFall = true;
			}
		}

		// Allow overlap events and such to change physics state and velocity
		if (IsMovingOnGround() && bFloorWalkable)
		{
			// Make velocity reflect actual move
			if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && timeTick >= MIN_TICK_TIME)
			{
				// TODO-RootMotionSource: Allow this to happen during partial override Velocity, but only set allowed axes?
				Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick;
				MaintainHorizontalGroundVelocity();
			}
		}

		// If we didn't move at all this iteration then abort (since future iterations will also be stuck).
		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}
	}


	FHitResult Hit;
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(Velocity.GetSafeNormal2D(), FVector::UpVector).ToQuat();
	SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, false, Hit);
}

void UStealthCharacterMovementComponent::SetSlide(bool bNewWantsToSlide)
{
	bWantsToSlide = bNewWantsToSlide;
}
