// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TeleportPortalCharacterInterface.generated.h"

class UCameraComponent;
// This class does not need to be modified.
UINTERFACE()
class UTeleportPortalCharacterInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class LEARNPORTAL_API ITeleportPortalCharacterInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UCameraComponent* GetCameraComponent();
	virtual UCameraComponent* GetCameraComponent_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	ACharacter* GetCharacter();
	virtual ACharacter* GetCharacter_Implementation();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	APlayerController* GetPlayerController();
	virtual APlayerController* GetPlayerController_Implementation();
};