// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Engine/EngineTypes.h"
#include "DrawDebugHelpers.h"
#include "particles/ParticleSystemComponent.h"


// Sets default values
AShooterCharacter::AShooterCharacter() :
    BaseTurnRate(45.f),
    BaseLookUpRate(45.f),
    bAiming(false),
    CameraDefaultFOV(0.f), //set in BeginPlay()
    CameraZoomedFOV(35.f),
    CameraCurrentFOV(0.f),
    ZoomInterpSpeed(20.f)
{

 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    
    /* Create a CameraBoom (pulls in toward a character if there is a collision) */
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 180.0f; //The camera follows at this distance behind the character
    CameraBoom->bUsePawnControlRotation = true; //Rotate the arm based on the controller
    CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

    /* Create a FollowCamera */
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); //Attach camera to end of Boom
    FollowCamera->bUsePawnControlRotation = false; //Camera does not rotate relative to arm

    /* Don't rotate when the controller rotates. Let the controller only affect the camera. */
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = true;
    bUseControllerRotationRoll = false;
    
    /* Configure character movement */
    GetCharacterMovement()->bOrientRotationToMovement = false; //Character moves in the direction of movement...
    GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); //... at this rotation rate
    GetCharacterMovement()->JumpZVelocity = 600.f;
    GetCharacterMovement()->AirControl = 0.2f;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

    if (FollowCamera) {
        CameraDefaultFOV = GetFollowCamera()->FieldOfView;
        CameraCurrentFOV = CameraDefaultFOV;
    }
}

/* Called for forwards / backwards input */
void AShooterCharacter::MoveForward(float Value) {
    if ((Controller != nullptr) && (Value != 0)) {
        const FRotator Rotation{ Controller->GetControlRotation() };
        const FRotator YawRotation{ 0, Rotation.Yaw, 0 };
        
        const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };
        AddMovementInput(Direction, Value);
    }
}

/* Called for side-to-side input */
void AShooterCharacter::MoveRight(float Value) {
    
    if ((Controller != nullptr) && (Value != 0)) {
        const FRotator Rotation{ Controller->GetControlRotation() };
        const FRotator YawRotation{ 0, Rotation.Yaw, 0 };
        
        const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };
        AddMovementInput(Direction, Value);
    }
}

void AShooterCharacter::TurnAtRate(float Rate) {
    AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds()); //deg/sec * sec/frame = deg/frame ...seconds cancel out and you get the degrees per frame to get a consistent input rate no matter how slow the computer.
}


void AShooterCharacter::LookUpAtRate(float Rate) {
    AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds()); //deg/frame
}

/* */
void AShooterCharacter::FireWeapon() {
    //Play weapon fire sound.
    if (FireSound) {
        UGameplayStatics::PlaySound2D(this, FireSound);
    }
    
    const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
    
    //Spawn particle system using Transform of BarrelSocket on Character Mesh
    if (BarrelSocket) {
        const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());
        
        if (MuzzleFlash) {
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
        }
        
        FVector BeamEndPoint;
        bool bBeamEnd = GetBeamEndLocation(
            SocketTransform.GetLocation(),
            BeamEndPoint);

        if (bBeamEnd) {
            //Spawn impact particles after updating BeamEndPoint
            if (ImpactParticles) {
                UGameplayStatics::SpawnEmitterAtLocation(
                    GetWorld(),
                    ImpactParticles,
                    BeamEndPoint);
            }

            if (BeamParticles) {
                UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
                    GetWorld(),
                    BeamParticles,
                    SocketTransform);
                if (Beam) {
                    Beam->SetVectorParameter(FName("Target"), BeamEndPoint);
                }
            }
        }

    }
    
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

    //Play AnimationMontage.
    if (AnimInstance && HipFireMontage) {
        AnimInstance->Montage_Play(HipFireMontage);
        AnimInstance->Montage_JumpToSection(FName("StartFire"));
    }
}

void AShooterCharacter::AimingButtonPressed()
{
    bAiming = true;
}

void AShooterCharacter::AimingButtonReleased()
{
    bAiming = false;
}

void AShooterCharacter::CameraInterpZoom( float DeltaTime )
{
    //Set current camera field of view
    if (bAiming) {
        //Interpolate to zoomed FOV
        CameraCurrentFOV = FMath::FInterpTo(
            CameraCurrentFOV,
            CameraZoomedFOV,
            DeltaTime,
            ZoomInterpSpeed
        );
    }
    else {
        //Interpolate to default FOV
        CameraCurrentFOV = FMath::FInterpTo(
            CameraCurrentFOV,
            CameraDefaultFOV,
            DeltaTime,
            ZoomInterpSpeed
        );
    }
    GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
    //Get current size of viewport.
    FVector2D ViewportSize;
    if (GEngine && GEngine->GameViewport) {
        GEngine->GameViewport->GetViewportSize(ViewportSize);
    }

    //Get Screenspace location of crosshairs
    FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
    CrosshairLocation.Y -= 50.f;
    FVector CrosshairWorldPosition;
    FVector CrosshairWorldDirection;

    //Get world position and direction of crosshairs
    bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
        UGameplayStatics::GetPlayerController(this, 0),
        CrosshairLocation,
        CrosshairWorldPosition,
        CrosshairWorldDirection);

    if (bScreenToWorld) { //Was deprojection successful?
        FHitResult ScreenTraceHit;
        const FVector Start{ CrosshairWorldPosition };
        const FVector End{ CrosshairWorldPosition + CrosshairWorldDirection * 50'000.f };

        //Set beam end point to line trace end point
        OutBeamLocation = End;

        //Trace outward from crosshairs world location
        GetWorld()->LineTraceSingleByChannel(
            ScreenTraceHit,
            Start,
            End,
            ECollisionChannel::ECC_Visibility);

        if (ScreenTraceHit.bBlockingHit) { //was there a trace hit?
            //Beam end point is now trace hit location
            OutBeamLocation = ScreenTraceHit.Location;
        }

        //Perform second trace, this time from the gun barrel
        FHitResult WeaponTraceHit;
        const FVector WeaponTraceStart{ MuzzleSocketLocation };
        const FVector WeaponTraceEnd{ OutBeamLocation };
        GetWorld()->LineTraceSingleByChannel(
            WeaponTraceHit,
            WeaponTraceStart,
            WeaponTraceEnd,
            ECollisionChannel::ECC_Visibility);

        if (WeaponTraceHit.bBlockingHit) {
            OutBeamLocation = WeaponTraceHit.Location;
        }

        return true;
    }
    return false;
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    
    CameraInterpZoom(DeltaTime);
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
    check(PlayerInputComponent);
    
    PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
    PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
    PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);
    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
    
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
    
    PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireWeapon);

    PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
    PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);
    
}

