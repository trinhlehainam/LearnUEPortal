// Fill out your copyright notice in the Description page of Project Settings.


#include "TeleportPortal.h"
#include "ITeleportableActor.h"
#include "UTool.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ATeleportPortal::ATeleportPortal()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent->Mobility = EComponentMobility::Static;

	PortalRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("PortalRootComponent"));
	PortalRootComponent->SetupAttachment(GetRootComponent());
	PortalRootComponent->Mobility = EComponentMobility::Static;

	// Create Mesh Plane and Bounding Box
	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
	PortalMesh->SetupAttachment(RootComponent);
	PortalMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	PortalMesh->Mobility = EComponentMobility::Static;

	ActorDetectionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ActorDetectionBox"));
	ActorDetectionBox->SetupAttachment(PortalMesh);
	ActorDetectionBox->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	ActorDetectionBox->SetCollisionResponseToAllChannels(ECR_Overlap);
	ActorDetectionBox->Mobility = EComponentMobility::Static;
	//
}

void ATeleportPortal::InitSceneCapture()
{
	SceneCapture = NewObject<USceneCaptureComponent2D>(
		this,
		USceneCaptureComponent2D::StaticClass(),
		*FString("PortalSceneCapture")
	);
	SceneCapture->AttachToComponent(
		GetRootComponent(),
		FAttachmentTransformRules::SnapToTargetIncludingScale
	);
	SceneCapture->RegisterComponent();
	
	SceneCapture->bCaptureEveryFrame = false;
	SceneCapture->bAlwaysPersistRenderingState = true;
	
	SceneCapture->bCaptureOnMovement = false;
	SceneCapture->TextureTarget = nullptr;
	SceneCapture->bEnableClipPlane = true;
	SceneCapture->bUseCustomProjectionMatrix = true;
	SceneCapture->CaptureSource = SCS_SceneColorHDRNoAlpha;
	SceneCapture->CompositeMode = SCCM_Overwrite;
	
	// Setup Post-Process of SceneCapture
	FPostProcessSettings CaptureSettings;
	CaptureSettings.bOverride_AmbientOcclusionQuality = true;
	CaptureSettings.bOverride_MotionBlurAmount = true;
	CaptureSettings.bOverride_SceneFringeIntensity = true;
	CaptureSettings.bOverride_ScreenSpaceReflectionQuality = true;

	CaptureSettings.AmbientOcclusionQuality = 0.f;
	CaptureSettings.MotionBlurAmount = 0.f;
	CaptureSettings.SceneFringeIntensity = 0.f;
	CaptureSettings.ScreenSpaceReflectionQuality = 0.f;

	SceneCapture->PostProcessSettings = CaptureSettings;
	
	GeneratePortalTexture();
	SceneCapture->TextureTarget = PortalTexture;
}

// Called when the game starts or when spawned
void ATeleportPortal::BeginPlay()
{
	Super::BeginPlay();

	ControllerOwner = Cast<APlayerController>(
		UGameplayStatics::GetPlayerController(this, 0));

	if (AActor* Character = ControllerOwner->GetCharacter(); Character != nullptr)
	{
		TeleportableActor = Character;
	}
	else if (AActor* Pawn = ControllerOwner->GetPawn(); Pawn != nullptr)
	{
		TeleportableActor = Pawn;
	}

	if (TeleportableActor != nullptr)
	{
		if (ITeleportableActor* Interface = Cast<ITeleportableActor>(TeleportableActor);
			Interface != nullptr)
		{
			TeleportActorInterface = Interface;
		}
		else
		{
			TeleportableActor = nullptr;
		}
	}
	
	if (bEnableCapture)
	{
		InitSceneCapture();
	}

	if (const ATeleportPortal* Target = GetLinkedPortal(); Target != nullptr)
	{
		const FVector TargetNormal = Target->PortalMesh->GetUpVector();
		if (bEnableCapture)
		{
			SceneCapture->ClipPlaneNormal = TargetNormal;
			SceneCapture->ClipPlaneBase = Target->GetActorLocation() + (SceneCapture->ClipPlaneNormal * -1.5f);
		}
	}
}

void ATeleportPortal::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateTeleport();
}

void ATeleportPortal::GeneratePortalTexture()
{
	const FTransform BoundTransform;
	int32 CurrentSizeX = FMath::FloorToInt32(PortalMesh->CalcBounds(BoundTransform).BoxExtent.X);
	int32 CurrentSizeY = FMath::FloorToInt32(PortalMesh->CalcBounds(BoundTransform).BoxExtent.Y);
	float TextureScale = 1.f;
	if (CurrentSizeY < 2000.f)
	{
		TextureScale = 2000.f / CurrentSizeY;
	}
	if (TextureScale > 1.f)
	{
		CurrentSizeX = FMath::FloorToInt32(CurrentSizeX * TextureScale);
		CurrentSizeY = 2000;
	}
	UE_LOG(LogTemp, Warning, TEXT("PlaneMesh X is %d, Y is %d"), CurrentSizeX, CurrentSizeY);

	if (PortalTexture == nullptr)
	{
		PortalTexture = NewObject<UTextureRenderTarget2D>(
			this,
			UTextureRenderTarget2D::StaticClass(),
			*FString("PortalRenderTarget")
			);
		check(PortalTexture);

		PortalTexture->RenderTargetFormat = RTF_RGBA16f;
		PortalTexture->Filter = TF_Bilinear;
		PortalTexture->SizeX = CurrentSizeX;
		PortalTexture->SizeY = CurrentSizeY;
		PortalTexture->ClearColor = FLinearColor::Black;
		PortalTexture->TargetGamma = 2.2f;
		PortalTexture->bNeedsTwoCopies = false;
		PortalTexture->AddressX = TA_Wrap;
		PortalTexture->AddressY = TA_Wrap;
		PortalTexture->bAutoGenerateMips = false;
		
		PortalTexture->UpdateResource();
	}
}

void ATeleportPortal::UpdateRenderTarget()
{
	if (ControllerOwner == nullptr || TeleportActorInterface == nullptr || TeleportableActor == nullptr)
	{
		return;
	}
	
	const FVector PortalLocation = GetActorLocation();
	const FVector PlayerLocation = TeleportableActor->GetActorLocation();

	// UE_LOG(LogTemp, Warning, TEXT("Distance is %.2f"), Distance);
	if (const float Distance = FMath::Abs(FVector::Dist(PlayerLocation, PortalLocation));
		Distance <= 1000.0f)
	{
		if (!bIsActive)
		{
			SetActive(true);
			SetRenderTarget(PortalTexture);
		}
	}
	else
	{
		if (bIsActive)
		{
			SetActive(false);
			ClearRenderTarget();
		}
	}
}

void ATeleportPortal::UpdateCapture()
{
	if (!bEnableCapture)
	{
		return;
	}
	
	if (SceneCapture == nullptr || PortalTexture == nullptr 
		|| TeleportActorInterface == nullptr || TeleportableActor == nullptr
		|| bIsActive == false)
	{
		return;
	}

	// Place the SceneCapture to the linkedportal
	if (ATeleportPortal* Target = GetLinkedPortal(); Target != nullptr)
	{
		const UCameraComponent* PlayerCamera = TeleportActorInterface->Execute_GetCameraComponent(TeleportableActor);
		if (PlayerCamera == nullptr)
		{
			return;
		}

		const FVector NewLocation = FUTool::ConvertCameraLocation(
			PlayerCamera->GetComponentLocation(),
			this,
			Target
		);
		SceneCapture->SetWorldLocation(NewLocation);

		const FRotator NewRotation = FUTool::ConvertRotationToActorSpace(
			PlayerCamera->GetComponentTransform().GetRotation().Rotator(),
			this,
			Target
		);
		
		SceneCapture->SetWorldRotation(NewRotation);
		SceneCapture->CustomProjectionMatrix = GetCameraProjectionMatrix();
		SceneCapture->CaptureScene();
	}
}

void ATeleportPortal::UpdateTeleport()
{
	if (ControllerOwner == nullptr || TeleportActorInterface == nullptr || TeleportableActor == nullptr)
	{
		return;
	}

	const UCameraComponent* PlayerCamera = TeleportActorInterface->Execute_GetCameraComponent(TeleportableActor);
	if (PlayerCamera == nullptr)
	{
		return;
	}
	const FVector CameraLocation = PlayerCamera->GetComponentLocation();
	const FVector PortalLocation = PortalRootComponent->GetComponentLocation();
	const FVector PortalNormal = PortalRootComponent->GetForwardVector() * -1.f;

	if (!IsPointInsideBox(CameraLocation, ActorDetectionBox))
	{
		return;
	}

	if (!IsPointCrossingPortal(CameraLocation, PortalLocation, PortalNormal))
	{
		return;
	}

	RequestTeleportByPortal(TeleportableActor);
}

void ATeleportPortal::RequestTeleportByPortal(AActor* TargetToTeleport)
{
	if (TargetToTeleport != nullptr)
	{
		TeleportActor(TargetToTeleport);
		ForceTick();
		UpdateRenderTarget();
		UpdateCapture();
	}
}

bool ATeleportPortal::IsPointInsideBox(const FVector& Point, const UBoxComponent* Box)
{
	if (Box != nullptr)
	{
		const FVector Center = Box->GetComponentLocation();
		const FVector Half = Box->GetScaledBoxExtent();
		const FVector DirectionX = Box->GetForwardVector();
		const FVector DirectionY = Box->GetRightVector();
		const FVector DirectionZ = Box->GetUpVector();

		const FVector Direction = Point - Center;
		const bool IsInside = FMath::Abs(FVector::DotProduct(Direction, DirectionX)) <= Half.X
			&& FMath::Abs(FVector::DotProduct(Direction, DirectionY)) <= Half.Y
			&& FMath::Abs(FVector::DotProduct(Direction, DirectionZ)) <= Half.Z;
		return IsInside;
	}
	
	return false;
}

FMatrix ATeleportPortal::GetCameraProjectionMatrix() const
{
   FMatrix ProjectionMatrix;

   if (const ULocalPlayer* LocalPlayer = ControllerOwner->GetLocalPlayer())
   {
      FSceneViewProjectionData PlayerProjectionData;
      LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, PlayerProjectionData, INDEX_NONE);

      ProjectionMatrix = PlayerProjectionData.ProjectionMatrix;
   }

   return ProjectionMatrix;
}

bool ATeleportPortal::IsActive() const
{
	return bIsActive;
}

void ATeleportPortal::SetActive(const bool NewActive)
{
	bIsActive = NewActive;
}

void ATeleportPortal::SetRenderTarget_Implementation(UTexture* RenderTexture)
{
}

void ATeleportPortal::ClearRenderTarget_Implementation()
{
}

void ATeleportPortal::ForceTick_Implementation()
{
}

ATeleportPortal* ATeleportPortal::GetLinkedPortal() const
{
	return LinkedPortal;
}

void ATeleportPortal::SetLinkedPortal(ATeleportPortal* NewLinkedPortal)
{
	LinkedPortal = NewLinkedPortal;
}

bool ATeleportPortal::IsPointInFrontOfPortal(const FVector Point, const FVector PortalLocation, const FVector PortalNormal)
{
	const FPlane PortalPlane = FPlane(PortalLocation, PortalNormal);
	const float PortalPointDistance = PortalPlane.PlaneDot(Point);

	// If < 0, Point is behind plane
	return PortalPointDistance >= 0;
}

bool ATeleportPortal::IsPointCrossingPortal(const FVector Point, const FVector PortalLocation, const FVector PortalNormal)
{
	FVector IntersectionPoint;
	const FPlane PortalPlane = FPlane(PortalLocation, PortalNormal);
	const bool IsIntersect = FMath::SegmentPlaneIntersection(LastPosition, Point, PortalPlane, IntersectionPoint);

	// TODO: Check which direction character is currently facing to portal
	
	LastPosition = Point;

	return IsIntersect;
}

void ATeleportPortal::TeleportActor(AActor* TargetToTeleport)
{
	if (LinkedPortal == nullptr || TargetToTeleport == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("LinkedPortal or ActorToTeleport is null"));
		return;
	}

	// TODO: Process later when know how to deal with character moving direction when crossing portal
	// if (bLastInFront == false)
	// {
	// 	return;
	// }

	FVector SavedVelocity = FVector::ZeroVector;
	SavedVelocity = TargetToTeleport->GetVelocity();

	// Apply new location/rotation for actor after
	FHitResult HitResult;
	FVector NewLocation = FUTool::ConvertLocationToActorSpace(
		TargetToTeleport->GetActorLocation(),
		this,
		LinkedPortal);
	TargetToTeleport->SetActorLocation(NewLocation,
		false,
		&HitResult,
		ETeleportType::TeleportPhysics);

	FRotator NewRotation = FUTool::ConvertRotationToActorSpace(
		TargetToTeleport->GetActorRotation(),
		this,
		LinkedPortal
	);
	TargetToTeleport->SetActorRotation(NewRotation);

	if (ITeleportableActor* Interface = Cast<ITeleportableActor>(TargetToTeleport);
		Interface != nullptr)
	{
		if (ACharacter* Character = Interface->Execute_GetCharacter(TargetToTeleport);
			Character != nullptr)
		{
			if (APlayerController* PortalPlayerController = Interface->Execute_GetPlayerController(TargetToTeleport);
				PortalPlayerController != nullptr)
			{
				NewRotation = FUTool::ConvertRotationToActorSpace(
					PortalPlayerController->GetControlRotation(),
					this,
					LinkedPortal
				);
				PortalPlayerController->SetControlRotation(NewRotation);
			}
			FVector NewVelocity = FUTool::ConvertVelocityToActorSpace(SavedVelocity, this, LinkedPortal);
			Character->GetCharacterMovement()->Velocity = NewVelocity;
		}
	}

	LastPosition = NewLocation;
	LinkedPortal->SetLastInFront(false);
	bLastInFront = false;
}

void ATeleportPortal::SetLastInFront(const bool IsInFront)
{
	bLastInFront = IsInFront;
}