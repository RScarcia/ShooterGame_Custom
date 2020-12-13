// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "MyShooterHUD.h"
#include "ShooterHUD.h"
#include "ShooterPlayerController.h"
#include "MyShooterCharacter.h"

#define LOCTEXT_NAMESPACE "ShooterGame.HUD.Menu"

AMyShooterHUD::AMyShooterHUD(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	HUDPurple = FColor(99, 31, 110, 255);
}

void AMyShooterHUD::DrawHUD() {
	Super::DrawHUD();
	AMyShooterCharacter* sc = Cast<AMyShooterCharacter>(PlayerOwner->GetPawn());
	if (sc) {
		if (sc->bIsStun) {
			Canvas->K2_DrawText(BigFont, "STUNNED", FVector2D(Canvas->ClipX / 2, Canvas->ClipY / 2), FVector2D(1,1), HUDPurple, 0 , HUDLight ,FVector2D(5,5), true, true, true, FLinearColor::White);
		}
	}
}
