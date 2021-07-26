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


// Sets default values
AShooterCharacter::AShooterCharacter() :
    BaseTurnRate(45.f),
    BaseLookUpRate(45.f)
{
    
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    
    /* Create a CameraBoom (pulls in toward a character if there is a collision) */
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 300.f; //The camera follows at this distance behind the character
    CameraBoom->bUsePawnControlRotation = true; //Rotate the arm based on the controller
    
    /* Create a FollowCamera */
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); //Attach camera to end of Boom
    FollowCamera->bUsePawnControlRotation = false; //Camera does not rotate relative to arm

    /* Don't rotate when the controller rotates. Let the controller only affect the camera. */
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
    
    /* Configure character movement */
    GetCharacterMovement()->bOrientRotationToMovement = true; //Character moves in the direction of movement...
    GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); //... at this rotation rate
    GetCharacterMovement()->JumpZVelocity = 600.f;
    GetCharacterMovement()->AirControl = 0.2f;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
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
    
    //Spawn ParticleSystem using Transform of BarrelSocket on Character Mesh
    if (BarrelSocket) {
        const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());
        
        if (MuzzleFlash) {
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
        }
        
        FHitResult FireHit;
        
        const FVector Start{ SocketTransform.GetLocation() };
        const FQuat Rotation{ SocketTransform.GetRotation() };
        const FVector RotationAxis{ Rotation.GetAxisX() };
        const FVector End{ Start + RotationAxis * 50'000.f }; //You can use as many apostrophies in numbers to make them more readable.
        
        GetWorld()->LineTraceSingleByChannel(FireHit, Start, End, ECollisionChannel::ECC_Visibility);
        
        if (FireHit.bBlockingHit) {
            DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.f);
            DrawDebugPoint(GetWorld(), FireHit.Location, 5, FColor::Red, false, 2.f);
        }
    }
    
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

    //Play AnimationMontage.
    if (AnimInstance && HipFireMontage) {
        AnimInstance->Montage_Play(HipFireMontage);
        AnimInstance->Montage_JumpToSection(FName("StartFire"));
    }
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
    
}

