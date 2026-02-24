// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/StealthPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

void AStealthPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UE_LOG(LogTemp, Warning, TEXT("PC SetupInputComponent called"));
	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		UE_LOG(LogTemp, Warning, TEXT("Is Local Player Controller"));
		// Add Input Mapping Context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			UE_LOG(LogTemp, Warning, TEXT("Got Enhanced Input Subsystem"));
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			
			UE_LOG(LogTemp, Warning, TEXT("Added mapping context: %s"), *DefaultMappingContext->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get Enhanced Input Subsystem!"));
		}
	}
}
