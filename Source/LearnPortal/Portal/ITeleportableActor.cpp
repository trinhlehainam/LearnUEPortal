// Fill out your copyright notice in the Description page of Project Settings.


#include "ITeleportableActor.h"


// Add default functionality here for any IITeleportPortalCharacter functions that are not pure virtual.
UCameraComponent* ITeleportableActor::GetCameraComponent_Implementation()
{
	return nullptr;
}

ACharacter* ITeleportableActor::GetCharacter_Implementation()
{
	return nullptr;
}

APlayerController* ITeleportableActor::GetPlayerController_Implementation()
{
	return nullptr;
}
