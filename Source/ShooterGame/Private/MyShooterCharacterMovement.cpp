// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "ConstructorHelpers.h"
#include "Engine/Classes/GameFramework/PlayerController.h"
#include "Engine/Classes/GameFramework/Controller.h"
#include "GameFramework/Character.h"
#include "Engine/Classes/Engine/World.h"
#include "MyShooterCharacter.h"
#include "MyShooterCharacterMovement.h"


UMyShooterCharacterMovement::UMyShooterCharacterMovement() {

	//Start values for properties, can be changed from the blueprint
	JetpackStrenght = 3000;
	JetpackMaxFuel = 2;
	JetpackRefuelRate = 0.5;
	Jetpackfuel = 0;
}

///////////////////////////////////////////////////
//This function is called every frame to perform the movement of the character. Handles the jetpack movement
void UMyShooterCharacterMovement::PerformMovement(float DeltaTime) {

	//Get the owner 
	AMyShooterCharacter* sc = Cast<AMyShooterCharacter>(PawnOwner);
	if (sc) {
		//Using jetpack, so fly up
		if (sc->bIsUsingJetpack) {
			//Make fuel decrease
			Jetpackfuel -= DeltaTime;
			//Jetpack fuel ended, disable it and stop flying
			if (Jetpackfuel < 0) {
				sc->StopJetpack();
				sc->DisableJetpack();
			}
			//Add some acceleration to the Upward velocity, so the character is propulsed upwards
			Velocity.Z += JetpackStrenght * DeltaTime;
		} else if (sc->bIsJetpackEnabled == true) {
			//Refuel jetpack only when you are not flying and the jetpack is enabled
			Jetpackfuel += DeltaTime * JetpackRefuelRate;
			if (Jetpackfuel >= JetpackMaxFuel) {
				Jetpackfuel = JetpackMaxFuel;
			}
		}
	}

	//Call the CharacterMovement version of PerformMovement, this function is the one that does the normal movement calculations.
	Super::PerformMovement(DeltaTime);
}