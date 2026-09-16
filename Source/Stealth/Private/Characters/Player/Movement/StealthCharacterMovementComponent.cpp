#include "Characters/Player/Movement/StealthCharacterMovementComponent.h"

#include "Characters/Player/StealthPlayerCharacter.h"
#include "Stealth/Stealth.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"


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

void UStealthCharacterMovementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	TransitionRMS.Reset();

	Super::EndPlay(EndPlayReason);
}

void UStealthCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	PlayerCharacterOwner = Cast<AStealthPlayerCharacter>(GetOwner());
}

void UStealthCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (TransitionRMS_ID != 0)
	{
		if (!TransitionRMS.IsValid() || TransitionRMS->Status.HasFlag(ERootMotionSourceStatusFlags::Finished))
		{
			PlayerCharacterOwner->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

			// CharacterOwner->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			SetMovementMode(MOVE_Walking);
			TransitionRMS_ID = (uint16)0;
		}
	}
}

void UStealthCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	if (IsInMovementMode(MOVE_Walking) && bWantsToSlide && CanSlide())
	{
		SetMovementMode(MOVE_Custom, CMOVE_Slide);
	}
	else if (IsInCustomMovementMode(CMOVE_Slide) && (!bWantsToSlide || !CanSlide()))
	{
		SetMovementMode(MOVE_Walking);
	}

	if (PlayerCharacterOwner->bPressedJump_Stealth)
	{
		if (TryVault())
		{
			PlayerCharacterOwner->StopJumping();
		}
		else
		{
			SCREEN_LOG("Failed Vault, Reverting to jump")
			PlayerCharacterOwner->bPressedJump_Stealth = false;
			PlayerCharacterOwner->bPressedJump = true;
			UnCrouch();
			CharacterOwner->CheckJumpInput(DeltaSeconds);
			bOrientRotationToMovement = true;
		}
	}

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UStealthCharacterMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);
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

bool UStealthCharacterMovementComponent::IsInCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}

bool UStealthCharacterMovementComponent::IsInMovementMode(EMovementMode InMovementMode) const
{
	return MovementMode == InMovementMode;
}

bool UStealthCharacterMovementComponent::IsMovingOnGround() const
{
	return Super::IsMovingOnGround() || IsInCustomMovementMode(CMOVE_Slide);
}

bool UStealthCharacterMovementComponent::CanAttemptJump() const
{
	// Allow jumping while sliding
	if (IsInCustomMovementMode(CMOVE_Slide))
	{
		return CanEverJump();
	}

	return Super::CanAttemptJump();
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


#pragma region Slide
void UStealthCharacterMovementComponent::SetSlide(bool bNewWantsToSlide)
{
	bWantsToSlide = bNewWantsToSlide;
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

#pragma endregion

#pragma region Vault
void UStealthCharacterMovementComponent::SetJumpInputActive(bool bNewIsActive)
{
	bIsJumpInputActive = bNewIsActive;
}

bool UStealthCharacterMovementComponent::TryVault()
{
	const bool bCanVault = (IsInMovementMode(MOVE_Walking) && !IsCrouching()) || IsInMovementMode(MOVE_Falling);
	if (!bCanVault)
	{
		return false;
	}

	const float CapsuleHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float CapsuleRadius = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();

	// Helper Variables
	FVector BaseLoc = UpdatedComponent->GetComponentLocation() + FVector::DownVector * CapsuleHalfHeight;
	FVector Fwd = UpdatedComponent->GetForwardVector().GetSafeNormal2D();
	auto Params = PlayerCharacterOwner->GetIgnoreCharacterParams();
	float MaxHeight = CapsuleHalfHeight * 2 + VaultMoveParams.ReachHeight;
	float CosMWSA = FMath::Cos(FMath::DegreesToRadians(VaultMoveParams.MinWallSteepnessAngle));
	float CosMSA = FMath::Cos(FMath::DegreesToRadians(VaultMoveParams.MaxSurfaceAngle));
	float CosMAA = FMath::Cos(FMath::DegreesToRadians(VaultMoveParams.MaxAlignmentAngle));

	SCREEN_LOG(TEXT("Starting vault attempt"));

	// Check Front Face
	FHitResult FrontHit;
	// float CheckDistance = FMath::Clamp(Velocity | Fwd, CapsuleRadius + 30, VaultMoveParams.MaxDistance);
	float CheckDistance = FMath::Clamp(Velocity | Fwd, CapsuleRadius, VaultMoveParams.MaxDistance);
	FVector FrontStart = BaseLoc + FVector::UpVector * (MaxStepHeight - 1);
	for (int i = 0; i < VaultMoveParams.VaultTracesCount; i++)
	{
		LINE(FrontStart, FrontStart + Fwd * CheckDistance, FColor::Red)
		if (GetWorld()->LineTraceSingleByProfile(FrontHit, FrontStart, FrontStart + Fwd * CheckDistance, "BlockAll", Params)) break;
		FrontStart += FVector::UpVector * ((2.f * CapsuleHalfHeight - (MaxStepHeight - 1)) / (VaultMoveParams.VaultTracesCount - 1));
	}
	if (!FrontHit.IsValidBlockingHit()) return false;
	float CosWallSteepnessAngle = FrontHit.Normal | FVector::UpVector;
	if (FMath::Abs(CosWallSteepnessAngle) > CosMWSA || (Fwd | -FrontHit.Normal) < CosMAA) return false;

	POINT(FrontHit.Location, FColor::Red);

	// Check Height
	TArray<FHitResult> HeightHits;
	FHitResult SurfaceHit;
	FVector WallUp = FVector::VectorPlaneProject(FVector::UpVector, FrontHit.Normal).GetSafeNormal();
	float WallCos = FVector::UpVector | FrontHit.Normal;
	float WallSin = FMath::Sqrt(1 - WallCos * WallCos);
	FVector TraceStart = FrontHit.Location + Fwd + WallUp * (MaxHeight - (MaxStepHeight - 1)) / WallSin;
	LINE(TraceStart, FrontHit.Location + Fwd, FColor::Orange)
	if (!GetWorld()->LineTraceMultiByProfile(HeightHits, TraceStart, FrontHit.Location + Fwd, "BlockAll", Params))
	{
		return false;
	}
	for (const FHitResult& Hit : HeightHits)
	{
		if (Hit.IsValidBlockingHit())
		{
			SurfaceHit = Hit;
			break;
		}
	}
	if (!SurfaceHit.IsValidBlockingHit() || (SurfaceHit.Normal | FVector::UpVector) < CosMSA)
	{
		return false;
	}
	float Height = (SurfaceHit.Location - BaseLoc) | FVector::UpVector;

	SCREEN_LOG(FString::Printf(TEXT("Height: %f"), Height))
	POINT(SurfaceHit.Location, FColor::Blue);

	if (Height < 0 || Height > MaxHeight)
	{
		return false;
	}

	// Check Clearance
	float SurfaceCos = FVector::UpVector | SurfaceHit.Normal;
	float SurfaceSin = FMath::Sqrt(1 - SurfaceCos * SurfaceCos);
	FVector ClearCapLoc = SurfaceHit.Location + Fwd * CapsuleRadius + FVector::UpVector * (CapsuleHalfHeight + 1 + CapsuleRadius * 2 * SurfaceSin);
	FCollisionShape CapShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);
	if (GetWorld()->OverlapAnyTestByProfile(ClearCapLoc, FQuat::Identity, "BlockAll", CapShape, Params))
	{
		CAPSULE(ClearCapLoc, FColor::Red, CapsuleHalfHeight, CapsuleRadius)
		return false;
	}
	else
	{
		CAPSULE(ClearCapLoc, FColor::Green, CapsuleHalfHeight, CapsuleRadius)
	}
	SCREEN_LOG("Can Vault")

	// Vault Selection
	FVector TallVaultTarget = GetVaultStartLocation(FrontHit, SurfaceHit, true);

	bool bTallVault = false;
	if (IsInMovementMode(MOVE_Walking) && Height > CapsuleHalfHeight * VaultMoveParams.TallVaultHeightThresholdMultiplier)
	{
		bTallVault = true;
	}
	else if (IsInMovementMode(MOVE_Falling) && (Velocity | FVector::UpVector) < 0)
	{
		if (!GetWorld()->OverlapAnyTestByProfile(TallVaultTarget, FQuat::Identity, "BlockAll", CapShape, Params))
		{
			bTallVault = true;
		}
	}

	if (!bTallVault)
	{
		// If obstacle is low, allow jumping over it
		FFindFloorResult FloorResult;
		FindFloor(FrontHit.Location + Fwd * (VaultMoveParams.JumpThroughDistance + CapsuleRadius), FloorResult, false);
		if (FloorResult.IsWalkableFloor())
		{
			// FVector FurtherClearCapLoc = FloorResult.HitResult.Location;
			FVector FurtherClearCapLoc = FloorResult.HitResult.Location + FVector::UpVector * (CapsuleHalfHeight * 0.8);
			if (GetWorld()->OverlapAnyTestByProfile(FurtherClearCapLoc, FQuat::Identity, "BlockAll", CapShape, Params))
			{
				CAPSULE(FurtherClearCapLoc, FColor::Red, CapsuleHalfHeight, CapsuleRadius)
			}
			else
			{
				CAPSULE(FurtherClearCapLoc, FColor::Green, CapsuleHalfHeight, CapsuleRadius)
				ClearCapLoc = FurtherClearCapLoc;
			}
		}
		else
		{
			FVector FurtherClearCapLoc = FloorResult.HitResult.Location + FVector::UpVector * (CapsuleHalfHeight);
			CAPSULE(FurtherClearCapLoc, FColor::Red, CapsuleHalfHeight, CapsuleRadius)
		}
	}

	// Perform Transition to Vault
	CAPSULE(UpdatedComponent->GetComponentLocation(), FColor::Red, CapsuleHalfHeight, CapsuleRadius)

	float UpSpeed = Velocity | FVector::UpVector;
	float TransDistance = FVector::Dist(ClearCapLoc, UpdatedComponent->GetComponentLocation());

	TransitionQueuedMontageSpeed = FMath::GetMappedRangeValueClamped(FVector2D(-500, 750), FVector2D(.9f, 1.2f), UpSpeed);
	TransitionRMS.Reset();
	TransitionRMS = MakeShared<FRootMotionSource_MoveToForce>();
	TransitionRMS->AccumulateMode = ERootMotionAccumulateMode::Override;

	TransitionRMS->Duration = FMath::Clamp(TransDistance / VaultMoveParams.MaxDistance, VaultMoveParams.MinTransitionTime, VaultMoveParams.MaxTransitionTime);
	SCREEN_LOG(FString::Printf(TEXT("Duration: %f"), TransitionRMS->Duration))
	TransitionRMS->StartLocation = UpdatedComponent->GetComponentLocation();
	TransitionRMS->TargetLocation = ClearCapLoc;
	TransitionRMS->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::SetVelocity;
	TransitionRMS->FinishVelocityParams.SetVelocity = FVector::ZeroVector;

	// Apply Transition Root Motion Source
	Velocity = FVector::ZeroVector;
	// Disable collision for the duration of the move
	PlayerCharacterOwner->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetMovementMode(MOVE_Flying);
	TransitionRMS_ID = ApplyRootMotionSource(TransitionRMS);
	TransitionName = "Vault";

	//TODO: Add vault animations
	// Animations
	// if (bTallVault)
	// {
	// 	TransitionQueuedMontage = VaultMoveParams.TallVaultMontage;
	// 	PlayerCharacterOwner->PlayAnimMontage(VaultMoveParams.TransitionTallVaultMontage, 1 / TransitionRMS->Duration);
	// }
	// else
	// {
	// 	TransitionQueuedMontage = VaultMoveParams.ShortVaultMontage;
	// 	PlayerCharacterOwner->PlayAnimMontage(VaultMoveParams.TransitionShortVaultMontage, 1 / TransitionRMS->Duration);
	// }

	return true;
}

FVector UStealthCharacterMovementComponent::GetVaultStartLocation(FHitResult FrontHit, FHitResult SurfaceHit, bool bTallVault) const
{
	const float CapsuleHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float CapsuleRadius = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();

	float CosWallSteepnessAngle = FrontHit.Normal | FVector::UpVector;
	float DownDistance = bTallVault ? CapsuleHalfHeight * 2.f : MaxStepHeight - 1;
	FVector EdgeTangent = FVector::CrossProduct(SurfaceHit.Normal, FrontHit.Normal).GetSafeNormal();

	FVector VaultStart = SurfaceHit.Location;
	VaultStart += FrontHit.Normal.GetSafeNormal2D() * (2.f + CapsuleRadius);
	VaultStart += UpdatedComponent->GetForwardVector().GetSafeNormal2D().ProjectOnTo(EdgeTangent) * CapsuleRadius * .3f;
	VaultStart += FVector::UpVector * CapsuleHalfHeight;
	VaultStart += FVector::DownVector * DownDistance;
	VaultStart += FrontHit.Normal.GetSafeNormal2D() * CosWallSteepnessAngle * DownDistance;

	return VaultStart;
}

#pragma endregion
