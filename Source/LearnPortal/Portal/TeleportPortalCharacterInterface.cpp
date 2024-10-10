// Fill out your copyright notice in the Description page of Project Settings.


#include "TeleportPortalCharacterInterface.h"


// Add default functionality here for any IITeleportPortalCharacter functions that are not pure virtual.
UCameraComponent* ITeleportPortalCharacterInterface::GetCameraComponent_Implementation()
{
	return nullptr;
}

ACharacter* ITeleportPortalCharacterInterface::GetCharacter_Implementation()
{
	return nullptr;
}

APlayerController* ITeleportPortalCharacterInterface::GetPlayerController_Implementation()
{
	return nullptr;
}
