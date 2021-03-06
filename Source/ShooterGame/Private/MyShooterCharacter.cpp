// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "MyShooterCharacter.h"
#include "MyShooterCharacterMovement.h"
#include "Engine/Classes/Engine/World.h"
#include "Components/InputComponent.h"
#include "TimerManager.h"
#include "CollisionQueryParams.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "MyShooterWeapon.h"
#include "Engine/EngineBaseTypes.h"
#include "Kismet/KismetStringLibrary.h"
#include "ShooterHUD.h"
#include "GameFramework/HUD.h"

AMyShooterCharacter::AMyShooterCharacter(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer.SetDefaultSubobjectClass <UMyShooterCharacterMovement>(ACharacter::CharacterMovementComponentName)) {
	//Jetpack
	JetpackRecovery = 2;
	bIsUsingJetpack = false;
	bIsJetpackEnabled = true;
	
	//WallJump
	WallJumpTraces = 20;
	WalljumpHorizontalStrenght = 1200;
	WalljumpUpwardsStrenght = 1500;
	WallJumpTraceDistance = 200;

	//Freeze Gun
	bIsStun = false;
	stunRecovery = 20;
}

// Called to bind functionality to input
void AMyShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jetpack", EInputEvent::IE_Pressed, this, &AMyShooterCharacter::StartJetpack);
	PlayerInputComponent->BindAction("Jetpack", EInputEvent::IE_Released, this, &AMyShooterCharacter::StopJetpack);

	PlayerInputComponent->BindAction("WallJump", EInputEvent::IE_Pressed, this, &AMyShooterCharacter::WallJump);
}

//Takes care of replicating property
void AMyShooterCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// everyone except local owner: flag change is locally instigated
	DOREPLIFETIME_CONDITION(AMyShooterCharacter, bIsUsingJetpack, COND_SkipOwner);
	DOREPLIFETIME(AMyShooterCharacter, bIsStun);
}

#pragma region TELEPORT
//////////////////////////////////////////////////
//Function exposed in blueprint
void AMyShooterCharacter::ShooterTeleport(FVector teleportDistance) {
	//If the distance set in blueprint is not 0
	if (teleportDistance != teleportDistance.ZeroVector) {
		if (GetLocalRole() < ROLE_Authority) {
			//Notify server with param
			ServerShooterTeleport(teleportDistance);
		} else {
			//Teleport
			PersonalTeleport(teleportDistance);
		}
	}
}

//////////////////////////////////////////////////
//Use line trace to teleport in the view direction
//Add distance got from blueprint to the end of the trace
//Set actor location with new params
void AMyShooterCharacter::PersonalTeleport(FVector newDistance) {
	//////////////////////////////////////////////////
	//Line trace
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, GetInstigator());

	const FVector StartTrace = GetActorLocation();
	const FVector EndTrace = GetActorLocation() + (GetActorForwardVector() * 1500);
	FHitResult Impact;
	
	GetWorld()->LineTraceSingleByChannel(Impact, StartTrace, EndTrace, COLLISION_WEAPON, TraceParams);
	//////////////////////////////////////////////////
	AShooterPlayerController* MyPC = Cast<AShooterPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed() && GetCharacterMovement()->IsMovingOnGround()) {
		FVector NewLocation = Impact.TraceEnd + newDistance;
		MyPC->GetPawn()->SetActorLocation(NewLocation, true, nullptr, ETeleportType::TeleportPhysics);
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Teleport"));
	}
}

//////////////////////////////////////////////////
//Server Management.
void AMyShooterCharacter::ServerShooterTeleport_Implementation(FVector distance) {
	ShooterTeleport(distance);
}

bool AMyShooterCharacter::ServerShooterTeleport_Validate(FVector distance) {
	return true;
}
#pragma endregion

#pragma region JETPACK FUNCTIONS

//////////////////////////////////////////////////
//Start and Stop functions
void AMyShooterCharacter::StartJetpack() {
	if (bIsJetpackEnabled) {
		//If server
		if (GetLocalRole() < ROLE_Authority){
			//else send it to the server
			ServerSetJetpack(true);
		}
		//set the variable
		bIsUsingJetpack = true;
	}
}

void AMyShooterCharacter::StopJetpack() {
	if (GetLocalRole() < ROLE_Authority){
		ServerSetJetpack(false);
	}
	bIsUsingJetpack = false;
}

//////////////////////////////////////////////////
//Enable and Disable Jetpack. These functions manage the cool down of the jetpack
void AMyShooterCharacter::EnableJetpack() {
	// Debug Message
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("JetpackEnabled"));
	bIsJetpackEnabled = true;
}

//Called inside the movement component when the jetpack fuel is over. It sets a timer to re-enable the jetpack
void AMyShooterCharacter::DisableJetpack() {
	// Debug Message
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("JetpackDisabled"));
	bIsJetpackEnabled = false;

	FTimerHandle TimerHandle;
	FTimerDelegate JetpackDelegate = FTimerDelegate::CreateUObject(this, &AMyShooterCharacter::EnableJetpack);
	GetWorldTimerManager().SetTimer(TimerHandle, JetpackDelegate, JetpackRecovery, false);
}

//////////////////////////////////////////////////
//Server management. Replicates the variable to the network
bool AMyShooterCharacter::ServerSetJetpack_Validate(bool bNewJetpack) {
	return true;
}

void AMyShooterCharacter::ServerSetJetpack_Implementation(bool bNewJetpack) {
	bIsUsingJetpack = bNewJetpack;
}
#pragma endregion

#pragma region WALLJUMP FUNCTIONS
//////////////////////////////////////////////////
//When user jumps, get central point with GetActorLocation().
//Do traces in circle (20 by default).
//Get the closest hit and from that the normal direction of the wall
//Multiply normal by some strenght and add it to upwards strenghts so i can jump upward
//Launch character with standard function and calculated params
void AMyShooterCharacter::WallJump() {
	AShooterPlayerController* MyPC = Cast<AShooterPlayerController>(Controller);
	//If character is valid and its in the air
	if (MyPC && MyPC->IsGameInputAllowed() && !GetCharacterMovement()->IsMovingOnGround()) {
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Walljump"));
		
		//Param for line trace
		FVector TraceStart = GetActorLocation();
		FVector Front = GetActorRotation().Vector();
		FVector Side = FVector::CrossProduct(Front, FVector::UpVector);

		/*DrawDebugLine(GetWorld(), TraceStart, TraceStart + Front * WallJumpTraceDistance, FColor::Blue, true, 3);
		DrawDebugLine(GetWorld(), TraceStart, TraceStart + Side * WallJumpTraceDistance, FColor::Red, true, 3);*/

		float MinDistance = 9999999;
		FVector HitLocation = FVector::ZeroVector;
		FVector HitNormal;

		for (int i = 0; i < WallJumpTraces; i++) {
			//Angle of trace
			float traceangle = 360 / WallJumpTraces * i;

			//Directions from the 360 angle
			FVector TraceDir = Front * FMath::Sin(traceangle) + Side * FMath::Cos(traceangle);
			FVector TraceEnd = TraceStart + TraceDir * WallJumpTraceDistance;
			//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Black, true, 3);

			//////////////////////////////////////////////////
			//Copied from weapon linetrace
			static FName TraceTag = FName(TEXT("WeaponTrace"));
			// Perform trace to retrieve hit info
			FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, GetInstigator());
			TraceParams.bReturnPhysicalMaterial = true;

			FHitResult Hit(ForceInit);
			//Draw line with previous params
			GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, COLLISION_WEAPON, TraceParams);
			//////////////////////////////////////////////////

			//If hit something
			if (Hit.bBlockingHit) {
				//Find the closest hit 
				if ((Hit.Location - TraceStart).Size() < MinDistance) {
					HitLocation = Hit.Location;
					HitNormal = Hit.Normal;
					MinDistance = (Hit.Location - TraceStart).Size();
				}
			}
		}
		//if hit location is 0, than something was hit
		if (HitLocation != FVector::ZeroVector) {
			//DrawDebugSphere(GetWorld(), HitLocation, 20, 20, FColor::Yellow, true, 5);
			if (GetLocalRole() < ROLE_Authority) {
				//notify server
				ServerAddForce(HitNormal * WalljumpHorizontalStrenght + FVector::UpVector * WalljumpUpwardsStrenght);
			}
			//launch character
			LaunchCharacter(HitNormal * WalljumpHorizontalStrenght + FVector::UpVector * WalljumpUpwardsStrenght, false, true);
		}
	}
}

//Override of CanJump internal functions so the character can't wall jump if is crouched
bool AMyShooterCharacter::CanJumpInternal_Implementation() const {

	return !bIsCrouched && GetCharacterMovement() && GetCharacterMovement()->IsMovingOnGround() && GetCharacterMovement()->CanEverJump() && !GetCharacterMovement()->bWantsToCrouch;
}

//////////////////////////////////////////////////
//Server management. Replicates the variable to the network
void AMyShooterCharacter::ServerAddForce_Implementation(FVector force) {
	LaunchCharacter(force, false, true);
}

bool AMyShooterCharacter::ServerAddForce_Validate(FVector force) {
	return true;
}
#pragma endregion

#pragma region FREEZE GUN STATUS
//Function to enable the movement of the character if hit with the freeze gun
//Called by DisableMovement, set movement mode to WALKING so the character can move again
void AMyShooterCharacter::EnableMovement() {
	if (GetLocalRole() < ROLE_Authority) {
		ServerSetMovement(false);
	}
	bIsStun = false;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	/*AShooterPlayerController* MyPC = Cast<AShooterPlayerController>(Controller);
	if (MyPC) {
		EnableInput(MyPC);
	}*/

	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("MOVING"));
}

//Function to disable the movement of the character if hit with the freeze gun
//Set movement mode to NONE so the character does not move
//At the end of the timer, we call the function that enables the movement
void AMyShooterCharacter::DisableMovement() {
	if (GetLocalRole() < ROLE_Authority) {
		ServerSetMovement(true);
	}
	bIsStun = true;
	GetCharacterMovement()->SetMovementMode(MOVE_None);

	/*AShooterPlayerController* MyPC = Cast<AShooterPlayerController>(Controller);
	if (MyPC) {
		
	}*/

	//Timer for stun recovery
	FTimerHandle TimerHandle;
	FTimerDelegate inputDelegate = FTimerDelegate::CreateUObject(this, &AMyShooterCharacter::EnableMovement);
	GetWorldTimerManager().SetTimer(TimerHandle, inputDelegate, stunRecovery, false);
}

//Server replication of stun variable
bool AMyShooterCharacter::ServerSetMovement_Validate(bool bnewStun) {
	return true;
}

void AMyShooterCharacter::ServerSetMovement_Implementation(bool bnewStun) {
	bIsStun = bnewStun;	 
}

//void AMyShooterCharacter::OnRep_bIsStun() {
//	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "STUNNED");
//}
#pragma endregion