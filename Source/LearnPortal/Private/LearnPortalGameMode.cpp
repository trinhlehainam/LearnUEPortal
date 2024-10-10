// Copyright Epic Games, Inc. All Rights Reserved.

#include "LearnPortalGameMode.h"
#include "LearnPortalCharacter.h"
#include "UObject/ConstructorHelpers.h"

ALearnPortalGameMode::ALearnPortalGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
