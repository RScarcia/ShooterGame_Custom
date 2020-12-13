// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "MyShooterWeapon.h"
#include "MyShooterCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetStringLibrary.h"
#include "ShooterHUD.h"
#include "MyShooterHUD.h"

void AMyShooterWeapon::FireWeapon() {
	
	const int32 RandomSeed = FMath::Rand();
	FRandomStream WeaponRandomStream(RandomSeed);
	const float CurrentSpread = GetCurrentSpread();
	const float ConeHalfAngle = FMath::DegreesToRadians(CurrentSpread * 0.5f);

	const FVector AimDir = GetAdjustedAim();
	const FVector StartTrace = GetCameraDamageStartLocation(AimDir);
	const FVector ShootDir = WeaponRandomStream.VRandCone(AimDir, ConeHalfAngle, ConeHalfAngle);
	const FVector EndTrace = StartTrace + ShootDir * InstantConfig.WeaponRange;

	const FHitResult Impact = WeaponTrace(StartTrace, EndTrace);
	ProcessStunHit(Impact, StartTrace, ShootDir, RandomSeed, CurrentSpread);
}

bool AMyShooterWeapon::ServerNotifyStun_Validate(const FHitResult& Impact, FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread) {
	return true;
}

void AMyShooterWeapon::ServerNotifyStun_Implementation(const FHitResult& Impact, FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread) {

	const float WeaponAngleDot = FMath::Abs(FMath::Sin(ReticleSpread * PI / 180.f));
	AMyShooterCharacter* sc = Cast<AMyShooterCharacter>(GetPawnOwner());

	// if we have an instigator, calculate dot between the view and the shot
	if (sc->GetInstigator() && (Impact.GetActor() || Impact.bBlockingHit)) {
		const FVector Origin = GetMuzzleLocation();
		const FVector ViewDir = (Impact.Location - Origin).GetSafeNormal();

		// is the angle between the hit and the view within allowed limits (limit + weapon max angle)
		const float ViewDotHitDir = FVector::DotProduct(sc->GetInstigator()->GetViewRotation().Vector(), ViewDir);
		if (ViewDotHitDir > InstantConfig.AllowedViewDotHitDir - WeaponAngleDot) {
			if (CurrentState != EWeaponState::Idle) {
				if (Impact.GetActor() == NULL) {
					if (Impact.bBlockingHit) {
						ProcessStunHit_Confirmed(Impact, Origin, ShootDir, RandomSeed, ReticleSpread);
					}
				}
				// assume it told the truth about static things because the don't move and the hit 
				// usually doesn't have significant gameplay implications
				else if (Impact.GetActor()->IsRootComponentStatic() || Impact.GetActor()->IsRootComponentStationary()) {
					ProcessStunHit_Confirmed(Impact, Origin, ShootDir, RandomSeed, ReticleSpread);
				} else {
					// Get the component bounding box
					const FBox HitBox = Impact.GetActor()->GetComponentsBoundingBox();

					// calculate the box extent, and increase by a leeway
					FVector BoxExtent = 0.5 * (HitBox.Max - HitBox.Min);
					BoxExtent *= InstantConfig.ClientSideHitLeeway;

					// avoid precision errors with really thin objects
					BoxExtent.X = FMath::Max(20.0f, BoxExtent.X);
					BoxExtent.Y = FMath::Max(20.0f, BoxExtent.Y);
					BoxExtent.Z = FMath::Max(20.0f, BoxExtent.Z);

					// Get the box center
					const FVector BoxCenter = (HitBox.Min + HitBox.Max) * 0.5;

					// if we are within client tolerance
					if (FMath::Abs(Impact.Location.Z - BoxCenter.Z) < BoxExtent.Z &&
						FMath::Abs(Impact.Location.X - BoxCenter.X) < BoxExtent.X &&
						FMath::Abs(Impact.Location.Y - BoxCenter.Y) < BoxExtent.Y) {
						ProcessStunHit_Confirmed(Impact, Origin, ShootDir, RandomSeed, ReticleSpread);
					} else {
						UE_LOG(LogShooterWeapon, Log, TEXT("%s Rejected client side hit of %s (outside bounding box tolerance)"), *GetNameSafe(this), *GetNameSafe(Impact.GetActor()));
					}
				}
			}
		} else if (ViewDotHitDir <= InstantConfig.AllowedViewDotHitDir) {
			UE_LOG(LogShooterWeapon, Log, TEXT("%s Rejected client side hit of %s (facing too far from the hit direction)"), *GetNameSafe(this), *GetNameSafe(Impact.GetActor()));
		} else {
			UE_LOG(LogShooterWeapon, Log, TEXT("%s Rejected client side hit of %s"), *GetNameSafe(this), *GetNameSafe(Impact.GetActor()));
		}
	}
}

bool AMyShooterWeapon::ServerNotifyStunMiss_Validate(FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread) {
	return true;
}

void AMyShooterWeapon::ServerNotifyStunMiss_Implementation(FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread) {
	const FVector Origin = GetMuzzleLocation();

	// play FX on remote clients
	HitNotify.Origin = Origin;
	HitNotify.RandomSeed = RandomSeed;
	HitNotify.ReticleSpread = ReticleSpread;

	// play FX locally
	if (GetNetMode() != NM_DedicatedServer) {
		const FVector EndTrace = Origin + ShootDir * InstantConfig.WeaponRange;
		SpawnTrailEffect(EndTrace);
	}
}

void AMyShooterWeapon::ProcessStunHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread) {
	
	AMyShooterCharacter* sc = Cast<AMyShooterCharacter>(GetPawnOwner());
	if (sc && sc->IsLocallyControlled() && GetNetMode() == NM_Client) {
		// if we're a client and we've hit something that is being controlled by the server
		if (Impact.GetActor() && Impact.GetActor()->GetRemoteRole() == ROLE_Authority) {
			// notify the server of the hit
			ServerNotifyStun(Impact, ShootDir, RandomSeed, ReticleSpread);
		} else if (Impact.GetActor() == NULL) {
			if (Impact.bBlockingHit) {
				// notify the server of the hit
				ServerNotifyStun(Impact, ShootDir, RandomSeed, ReticleSpread);
			} else {
				// notify server of the miss
				ServerNotifyStunMiss(ShootDir, RandomSeed, ReticleSpread);
			}
		}
	}
	// process a confirmed stun hit
	ProcessStunHit_Confirmed(Impact, Origin, ShootDir, RandomSeed, ReticleSpread);
}

void AMyShooterWeapon::ProcessStunHit_Confirmed(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread) {

	// handle stun damage
	if (ShouldDealStun(Impact.GetActor())) {
		DealStun(Impact, ShootDir);
	}

	// play FX on remote clients
	if (GetLocalRole() == ROLE_Authority) {
		HitNotify.Origin = Origin;
		HitNotify.RandomSeed = RandomSeed;
		HitNotify.ReticleSpread = ReticleSpread;
	}

	// play FX locally
	if (GetNetMode() != NM_DedicatedServer) {
		const FVector EndTrace = Origin + ShootDir * InstantConfig.WeaponRange;
		const FVector EndPoint = Impact.GetActor() ? Impact.ImpactPoint : EndTrace;

		SpawnTrailEffect(EndPoint);
		SpawnImpactEffects(Impact);
	}
}

bool AMyShooterWeapon::ShouldDealStun(AActor* TestActor) const {
	// if we're an actor on the server, or the actor's role is authoritative, we should register damage
	if (TestActor) {
		if (GetNetMode() != NM_Client ||
			TestActor->GetLocalRole() == ROLE_Authority ||
			TestActor->GetTearOff()) {
			return true;
		}
	}

	return false;
}

void AMyShooterWeapon::DealStun(const FHitResult& Impact, const FVector& ShootDir) {
	
	FPointDamageEvent PointDmg;
	PointDmg.DamageTypeClass = InstantConfig.DamageType;
	PointDmg.HitInfo = Impact;
	PointDmg.ShotDirection = ShootDir;
	PointDmg.Damage = InstantConfig.HitDamage;

	Impact.GetActor()->TakeDamage(PointDmg.Damage, PointDmg, MyPawn->Controller, this);

	AMyShooterCharacter* sc = Cast<AMyShooterCharacter>(Impact.GetActor());
	if (sc) {
		sc->DisableMovement();
	}
}