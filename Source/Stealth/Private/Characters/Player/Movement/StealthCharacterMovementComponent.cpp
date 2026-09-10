#include "Characters/Player/Movement/StealthCharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "Characters/Player/StealthPlayerCharacter.h"
#include "Stealth/Stealth.h"


UStealthCharacterMovementComponent::UStealthCharacterMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	NavAgentProps.bCanCrouch = true;
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
		bCrouchMaintainsBaseLocation = true;
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

bool UStealthCharacterMovementComponent::IsMovingOnGround() const
{
	return Super::IsMovingOnGround() || IsInCustomMovementMode(CMOVE_Slide);
}

bool UStealthCharacterMovementComponent::CanAttemptJump() const
{
	// Allow jumping while crouching
	return IsJumpAllowed() && (IsMovingOnGround() || IsFalling());
}

float UStealthCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	if (MovementMode != MOVE_Custom)
	{
		return Super::GetMaxBrakingDeceleration();
	}

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return SlideMoveParams.BrakingDeceleration;
	default:
		UE_LOG(LogStealth, Fatal, TEXT("Invalid Custom Movement Mode"))
		return -1.0f;
	}
}

bool UStealthCharacterMovementComponent::IsInCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}

bool UStealthCharacterMovementComponent::ResizeCapsuleSize(float NewHalfHeight)
{
	if (!HasValidData() || !CharacterOwner || !CharacterOwner->GetCapsuleComponent())
	{
		return false;
	}

	UCapsuleComponent* CapsuleComp = CharacterOwner->GetCapsuleComponent();
	const float OldUnscaledHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
	const float OldUnscaledRadius = CapsuleComp->GetUnscaledCapsuleRadius();
	const float ClampedHalfHeight = FMath::Max(OldUnscaledRadius, NewHalfHeight);

	// If already at desired size, nothing to do
	if (FMath::IsNearlyEqual(OldUnscaledHalfHeight, ClampedHalfHeight) && FMath::IsNearlyEqual(OldUnscaledRadius, OldUnscaledRadius))
	{
		return true;
	}

	const float ComponentScale = CapsuleComp->GetShapeScale();
	const float HalfHeightAdjust = OldUnscaledHalfHeight - ClampedHalfHeight;
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	// If resizing to a larger height, verify we don't penetrate blocking geometry
	if (ClampedHalfHeight > OldUnscaledHalfHeight)
	{
		FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CapsuleResizeTrace), false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(CapsuleParams, ResponseParam);

		const bool bEncroached = GetWorld()->OverlapBlockingTestByChannel(
			UpdatedComponent->GetComponentLocation() + ScaledHalfHeightAdjust * GetGravityDirection(),
			GetWorldToGravityTransform(),
			UpdatedComponent->GetCollisionObjectType(),
			GetPawnCapsuleCollisionShape(SHRINK_None),
			CapsuleParams,
			ResponseParam
		);

		if (bEncroached)
		{
			return false;
		}
	}

	CapsuleComp->SetCapsuleSize(OldUnscaledRadius, ClampedHalfHeight);

	if (bCrouchMaintainsBaseLocation)
	{
		UpdatedComponent->MoveComponent(ScaledHalfHeightAdjust * GetGravityDirection(), UpdatedComponent->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags,
		                                ETeleportType::TeleportPhysics);
	}

	bForceNextFloorCheck = true;
	return true;
}

bool UStealthCharacterMovementComponent::RestoreDefaultCapsuleSize()
{
	if (!HasValidData() || !CharacterOwner || !CharacterOwner->GetCapsuleComponent())
	{
		return false;
	}

	const ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	const UCapsuleComponent* DefaultCapsule = DefaultCharacter->GetCapsuleComponent();
	const float DefaultUnscaledHalfHeight = DefaultCapsule->GetUnscaledCapsuleHalfHeight();
	const float DefaultUnscaledRadius = DefaultCapsule->GetUnscaledCapsuleRadius();

	UCapsuleComponent* CapsuleComp = CharacterOwner->GetCapsuleComponent();
	const float OldUnscaledHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
	const float OldUnscaledRadius = CapsuleComp->GetUnscaledCapsuleRadius();

	if (FMath::IsNearlyEqual(OldUnscaledHalfHeight, DefaultUnscaledHalfHeight) && FMath::IsNearlyEqual(OldUnscaledRadius, DefaultUnscaledRadius))
	{
		return true;
	}

	const float CurrentScaledHalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();
	const float ComponentScale = CapsuleComp->GetShapeScale();
	const float HalfHeightAdjust = DefaultUnscaledHalfHeight - OldUnscaledHalfHeight;
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	const FVector PawnLocation = UpdatedComponent->GetComponentLocation();

	// Encroachment check when growing back
	const UWorld* MyWorld = GetWorld();
	const float SweepInflation = UE_KINDA_SMALL_NUMBER * 10.f;
	FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CapsuleResizeTrace), false, CharacterOwner);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(CapsuleParams, ResponseParam);

	const FCollisionShape StandingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation - ScaledHalfHeightAdjust);
	const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
	bool bEncroached = true;

	if (!bCrouchMaintainsBaseLocation)
	{
		bEncroached = MyWorld->OverlapBlockingTestByChannel(PawnLocation, GetWorldToGravityTransform(), CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
	}
	else
	{
		FVector StandingLocation = PawnLocation + (StandingCapsuleShape.GetCapsuleHalfHeight() - CurrentScaledHalfHeight) * -GetGravityDirection();
		bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, GetWorldToGravityTransform(), CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);

		if (bEncroached && IsMovingOnGround())
		{
			const float MinFloorDist = UE_KINDA_SMALL_NUMBER * 10.f;
			if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
			{
				StandingLocation -= (CurrentFloor.FloorDist - MinFloorDist) * -GetGravityDirection();
				bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, GetWorldToGravityTransform(), CollisionChannel, StandingCapsuleShape, CapsuleParams,
				                                                    ResponseParam);
			}
		}

		if (!bEncroached)
		{
			UpdatedComponent->MoveComponent(StandingLocation - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags,
			                                ETeleportType::TeleportPhysics);
			bForceNextFloorCheck = true;
		}
	}

	if (bEncroached)
	{
		return false;
	}

	CapsuleComp->SetCapsuleSize(DefaultUnscaledRadius, DefaultUnscaledHalfHeight, true);

	return true;
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
	bool bValidSurface = CurrentFloor.bWalkableFloor;
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
		const float TimeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= TimeTick;

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
		Velocity += SlopeForce * SlideMoveParams.GravityForce * TimeTick;

		// If you want steering, adjust velocity direction rather than adding driving acceleration:
		if (!Acceleration.IsNearlyZero())
		{
			const float SteeringDot = FVector::DotProduct(Acceleration.GetSafeNormal(), UpdatedComponent->GetRightVector());
			if (FMath::Abs(SteeringDot) > 0.1f)
			{
				const float TurnAngle = SteeringDot * 45.0f * TimeTick; // Adjust turn sensitivity as desired
				Velocity = FRotator(0.f, TurnAngle, 0.f).RotateVector(Velocity);
			}
		}

		Acceleration = FVector::ZeroVector;

		// Apply friction and braking deceleration
		CalcVelocity(TimeTick, SlideMoveParams.Friction, true, GetMaxBrakingDeceleration());

		// Compute move parameters
		const FVector MoveVelocity = Velocity;
		const FVector Delta = TimeTick * MoveVelocity;
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
			MoveAlongFloor(MoveVelocity, TimeTick, &StepDownResult);

			if (IsFalling())
			{
				// pawn decided to jump up
				const float DesiredDist = Delta.Size();
				if (DesiredDist > KINDA_SMALL_NUMBER)
				{
					const float ActualDist = (UpdatedComponent->GetComponentLocation() - OldLocation).Size2D();
					remainingTime += TimeTick * (1.f - FMath::Min(1.f, ActualDist / DesiredDist));
				}
				StartNewPhysics(remainingTime, Iterations);
				return;
			}
			else if (IsSwimming()) //just entered water
			{
				StartSwimming(OldLocation, OldVelocity, TimeTick, remainingTime, Iterations);
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
				Velocity = NewDelta / TimeTick;
				remainingTime += TimeTick;
				continue;
			}
			else
			{
				// see if it is OK to jump
				// TODO: collision : only thing that can be problem is that oldbase has world collision on
				PRAGMA_DISABLE_DEPRECATION_WARNINGS
				bool bMustJump = bZeroDelta || (OldBase == nullptr || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				PRAGMA_ENABLE_DEPRECATION_WARNINGS

				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, TimeTick, Iterations, bMustJump))
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
					HandleWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, TimeTick);
					if (IsMovingOnGround())
					{
						// If still walking, then fall. If not, assume the user set a different mode they want to keep.
						StartFalling(Iterations, remainingTime, TimeTick, Delta, OldLocation);
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
				StartSwimming(OldLocation, Velocity, TimeTick, remainingTime, Iterations);
				return;
			}

			// See if we need to start falling.
			if (!CurrentFloor.IsWalkableFloor() && !CurrentFloor.HitResult.bStartPenetrating)
			{
				PRAGMA_DISABLE_DEPRECATION_WARNINGS
				const bool bMustJump = bJustTeleported || bZeroDelta || (OldBase == nullptr || (!OldBase->IsQueryCollisionEnabled() &&
					MovementBaseUtility::IsDynamicBase(OldBase)));
				PRAGMA_ENABLE_DEPRECATION_WARNINGS

				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, TimeTick, Iterations, bMustJump))
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
			if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && TimeTick >= MIN_TICK_TIME)
			{
				// TODO-RootMotionSource: Allow this to happen during partial override Velocity, but only set allowed axes?
				Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / TimeTick;
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
