// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);

    /** UpdateAimInterpolation */
    void CameraInterpZoom(float DeltaTime);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    
    /* Called for forwards / backwards input */
    void MoveForward(float value);
    
    /* Called for side-to-side input */
    void MoveRight(float value);
    
    /** Called via input to turn at a given rate.
     @param Rate This is a normalized rate, i.e. 1.0 means 100% of desired rate.
     */
    void TurnAtRate(float Rate);
    
    /** Called via input to look up / down at a given rate.
     @param Rate This is a normalized rate, i.e. 1.0 means 100% of desired rate.
     */
    void LookUpAtRate(float Rate);
    
    /** Called when the fire button is pressed.
     */
    void FireWeapon();

    /** */
    void AimingButtonPressed();
    void AimingButtonReleased();

private:
    /** Camera boom positioning the camera behind the character */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class USpringArmComponent* CameraBoom;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* FollowCamera;
    
    /** Base turn rate in degrees per second. Other scaling may affect final turn rate. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    float BaseTurnRate;
    
    /** Base look up / down rate in degrees per second. Other scaling may affect final turn rate. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    float BaseLookUpRate;
    
    /** Randomized gunshot sound cue */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    class USoundCue* FireSound;
    
    /** Flash spawned at barrel socket */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    class UParticleSystem* MuzzleFlash;
    
    /** Flash spawned at barrel socket */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    class UParticleSystem* ImpactParticles;

    /** Montage for firing the weapon */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    class UAnimMontage* HipFireMontage;

    /** Smoke trail for bullets */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
    class UParticleSystem* BeamParticles;

    /** True when Aiming. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
        bool bAiming;

    /** Default camera field of view value. */
    float CameraDefaultFOV;

    /** Field of view value for when zoomed in. */
    float CameraZoomedFOV;

    /** Current field of view this frame. */
    float CameraCurrentFOV;

    /**  */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = true))
    float ZoomInterpSpeed;

public:
    /* Returns CameraBoom subobject */
    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    
    /* Returns FollowCamera subobject */
    FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

    /**  */
    FORCEINLINE bool GetAiming() const { return bAiming; }
};
