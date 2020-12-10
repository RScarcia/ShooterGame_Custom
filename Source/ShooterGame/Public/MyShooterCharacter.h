// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/ShooterCharacter.h"
#include "MyShooterCharacter.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERGAME_API AMyShooterCharacter : public AShooterCharacter
{
	GENERATED_BODY()

public:
	//Constructor
	AMyShooterCharacter(const FObjectInitializer& ObjectInitializer);

	//Bind new actions to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

#pragma region TELEPORT
	//Function callable inside the blueprint
	UFUNCTION(BlueprintCallable, Category = Teleport)
		void ShooterTeleport(FVector teleportDistance);
	
	//Actual functions that manages teleport
	virtual void PersonalTeleport(FVector newDistance);

	//Server management
	UFUNCTION(reliable, server, WithValidation)
		virtual void ServerShooterTeleport(FVector distance);
#pragma endregion

#pragma region JETPACK FUNCTIONS
	//Property sent to everyone to replicate the jetpack action
	UPROPERTY(Transient, Replicated)
		uint8 bIsUsingJetpack : 1;

	//Functions that manage the jetpack and can be called on blueprints
	UFUNCTION(BlueprintCallable, Category = Jetpack)
		virtual void StartJetpack();
	UFUNCTION(BlueprintCallable, Category = Jetpack)
		virtual void StopJetpack();

	//This two functions handle the "recovering time" of the jetpack
	virtual void EnableJetpack();
	virtual void DisableJetpack();
	
	//Control on cool down, if false jetpack can't be used
	bool bIsJetpackEnabled;

	//Time the jetpack has to cool down
	UPROPERTY(Category = "Jetpack", EditAnywhere, BlueprintReadWrite)
		float JetpackRecovery;

	//Function that allows the client to set the jetpack state in the server, so it can be replicated
	UFUNCTION(reliable, server, WithValidation)
		virtual void ServerSetJetpack(bool bNewJetpack);
#pragma endregion

#pragma region WALL JUMP
	//Params for line trace
	int WallJumpTraces;
	float WallJumpTraceDistance;

	//Properties editable inside a blueprint to change the strenght of jump
	UPROPERTY(Category = "WallJump", EditAnywhere, BlueprintReadWrite)
		float WalljumpHorizontalStrenght;
	UPROPERTY(Category = "WallJump", EditAnywhere, BlueprintReadWrite)
		float WalljumpUpwardsStrenght;
	
	//Function to handle wall jump
	UFUNCTION(BlueprintCallable, Category = WallJump)
		void WallJump();
	
	virtual bool CanJumpInternal_Implementation() const override;

	//Server management
	UFUNCTION(reliable, server, WithValidation)
		virtual void ServerAddForce(FVector force);
#pragma endregion

};
