// Fill out your copyright notice in the Description page of Project Settings.


#include "TeleportPortalCharacterInterface.h"


// Add default functionality here for any IITeleportPortalCharacter functions that are not pure virtual.
UCameraComponent* ITeleportPortalActorInterface::GetCameraComponent_Implementation()
{
	return nullptr;
}

ACharacter* ITeleportPortalActorInterface::GetCharacter_Implementation()
{
	return nullptr;
}

APlayerController* ITeleportPortalActorInterface::GetPlayerController_Implementation()
{
	return nullptr;
}
