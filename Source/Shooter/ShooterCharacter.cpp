// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"


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
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
    
    /* Using UE_LOG */
    UE_LOG(LogTemp, Warning, TEXT("BeginPlay() called!"));
    
    /* Using variables in UE_LOG */
    int myInt{ 42 };
    UE_LOG(LogTemp, Warning, TEXT("int myInt: %d"), myInt);
    
    float myFloat{ 3.141592f };
    UE_LOG(LogTemp, Warning, TEXT("float myFloat: %f"), myFloat);
    
    double myDouble{ 0.000756 };
    UE_LOG(LogTemp, Warning, TEXT("float myDouble: %lf"), myDouble);
    
    char myChar{ 'J' };
    UE_LOG(LogTemp, Warning, TEXT("float myChar: %c"), myChar);
    
    wchar_t wideChar{ L'J' };
    UE_LOG(LogTemp, Warning, TEXT("wchar_t wideChar: %lc"), wideChar);
    
    bool myBool{ true };
    UE_LOG(LogTemp, Warning, TEXT("float myBool: %d"), myBool);
    
    UE_LOG(LogTemp, Warning, TEXT("int: %d, float: %f, bool: %d"), myInt, myFloat, myBool);
    
    /* Using FString in UE_LOG */
    FString myString{ TEXT("My String!!!") };
    UE_LOG(LogTemp, Warning, TEXT("FString myString: %s"), *myString);
    UE_LOG(LogTemp, Warning, TEXT("Name of instance: %s"), *GetName());
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
}

