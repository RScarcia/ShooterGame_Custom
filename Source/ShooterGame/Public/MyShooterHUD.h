// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/ShooterHUD.h"
#include "MyShooterHUD.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERGAME_API AMyShooterHUD : public AShooterHUD
{
	GENERATED_BODY()

public:
	//Constructor
	AMyShooterHUD(const FObjectInitializer& ObjectInitializer);

	/** Purple HUD color. */
	FColor HUDPurple;

	virtual void DrawHUD() override;
};
