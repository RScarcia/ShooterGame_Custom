// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/ShooterCharacterMovement.h"
#include "MyShooterCharacterMovement.generated.h"

/**
 * 
 */

UCLASS()
class SHOOTERGAME_API UMyShooterCharacterMovement : public UShooterCharacterMovement {
	
	GENERATED_BODY()
	
	//Constructor
	UMyShooterCharacterMovement();

public:
#pragma region JETPACK	
	//Override the Perform Movement function to add the jetpack logic
	virtual void PerformMovement(float DeltaTime) override;

	//Upwards Strenght of the jetpack, the more it is, the bigger is the acceleration for the jetpack, if its too low, the gravity has more power and you don't fly
	UPROPERTY(Category = "Jetpack", EditAnywhere, BlueprintReadWrite)
		float JetpackStrenght;

	//Max fuel for the jetpack in seconds (example: if set to 2, you have 2 seconds of jetpack)
	UPROPERTY(Category = "Jetpack", EditAnywhere, BlueprintReadWrite)
		float JetpackMaxFuel;

	//Multiplier for jetpack fuel regeneration, uses the time, if its 0.5, and the JetpackMaxFuel is 2 seconds, that means that it will take 4 seconds to be back at 100%
	UPROPERTY(Category = "Jetpack", EditAnywhere, BlueprintReadWrite)
		float JetpackRefuelRate;

	//Holds the current fuel amount
	float Jetpackfuel;
#pragma endregion
};