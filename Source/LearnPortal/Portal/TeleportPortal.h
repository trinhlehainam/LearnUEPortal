// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TeleportPortal.generated.h"

class ITeleportableActor;
class UBoxComponent;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

UCLASS()
class LEARNPORTAL_API ATeleportPortal : public AActor
{
	GENERATED_BODY()

public:
	ATeleportPortal();

	void InitSceneCapture();

	UFUNCTION(BlueprintPure)
	bool IsActive() const;

	UFUNCTION(BlueprintCallable)
	void SetActive(bool NewActive);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetRenderTarget(UTexture* RenderTexture);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ClearRenderTarget();

	UFUNCTION(BlueprintNativeEvent)
	void ForceTick();

	UFUNCTION(BlueprintPure)
	ATeleportPortal* GetLinkedPortal() const;

	UFUNCTION(BlueprintCallable)
	void SetLinkedPortal(ATeleportPortal* NewLinkedPortal);

	UFUNCTION(BlueprintCallable)
	static bool IsPointInFrontOfPortal(FVector Point, FVector PortalLocation, FVector PortalNormal);

	UFUNCTION(BlueprintCallable)
	bool IsPointCrossingPortal(FVector Point, FVector PortalLocation, FVector PortalNormal);

	UFUNCTION(BlueprintCallable)
	void TeleportActor(AActor* TargetToTeleport);

	UFUNCTION(BlueprintCallable)
	void SetLastInFront(bool IsInFront);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* PortalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBoxComponent* ActorDetectionBox;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// TODO: Use overlap event to handle teleport actor
	// virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly)
	USceneComponent* PortalRootComponent;
private:
	void GeneratePortalTexture();
	void UpdateRenderTarget();
	void UpdateCapture();
	void UpdateTeleport();

	UFUNCTION(BlueprintCallable)
	void RequestTeleportByPortal(AActor* TargetToTeleport);

	UFUNCTION(BlueprintPure)
	static bool IsPointInsideBox(const FVector& Point, const UBoxComponent* Box);

	FMatrix GetCameraProjectionMatrix() const;

	UPROPERTY()
	USceneCaptureComponent2D* SceneCapture;

	UPROPERTY(Transient)
	UTextureRenderTarget2D* PortalTexture;

	UPROPERTY()
	APlayerController* ControllerOwner;

	UPROPERTY()
	AActor* TeleportableActor;

	bool bIsActive{false};
	bool bLastInFront{false};
	FVector LastPosition;
	UPROPERTY(EditAnywhere)
	bool bEnableCapture{false};

	ITeleportableActor* TeleportActorInterface;

	UPROPERTY(EditAnywhere)
	ATeleportPortal* LinkedPortal;
};