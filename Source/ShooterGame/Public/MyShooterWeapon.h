// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/ShooterWeapon_Instant.h"
#include "MyShooterWeapon.generated.h"

/**
 * 
 */

UCLASS()
class SHOOTERGAME_API AMyShooterWeapon : public AShooterWeapon_Instant
{
	GENERATED_BODY()

public:
	virtual void FireWeapon() override;

	////////////////////////////////////////////////////////////////////////
	// Weapon usage
	/** server notified of hit from client to verify */
	UFUNCTION(reliable, server, WithValidation)
		void ServerNotifyStun(const FHitResult& Impact, FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread);

	/** server notified of miss to show trail FX */
	UFUNCTION(unreliable, server, WithValidation)
		void ServerNotifyStunMiss(FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread);

	/** process the instant hit and notify the server if necessary */
	void ProcessStunHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread);

	/** continue processing the instant hit, as if it has been confirmed by the server */
	void ProcessStunHit_Confirmed(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread);

	/** handle damage */
	void DealStun(const FHitResult& Impact, const FVector& ShootDir);

	/** check if weapon should deal damage to actor */
	bool ShouldDealStun(AActor* TestActor) const;
	
};
