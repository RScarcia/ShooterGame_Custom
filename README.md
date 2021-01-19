# ShooterGame_Custom

Custom version of UE4 Shooter Game example project, developed for a Multiplayer environment.

New Features:
* Teleport
* Jetpack
* WallJump
* Freeze Gun

Added classes are:
* **MyShooterCharacter** (.h and .cpp): extension of ShooterCharacter class, manages teleport, walljump, some functions of the jetpack feature and the "freeze" status.
* **MyShooterCharacterMovement** (.h and .cpp): extension of ShooterCharacterMovement class, handles everything concerning the jetpack movement.
* **MyShooterWeapon** (.h and .cpp): extension of ShooterWeapon_Instant class, handles the logic behind the freeze gun.

Some minor changes inside **ShooterHUD** class, to display the percentage of jetpack fuel and the STUN status.
```c++
void AShooterHUD::DrawHUD() {
  AMyShooterCharacter* sc = Cast<AMyShooterCharacter>(GetOwningPawn());
    if (sc && sc->IsAlive()) {
      // Get the movement component, and check if its the correct class
      UMyShooterCharacterMovement* movecmp = Cast<UMyShooterCharacterMovement>(sc->GetMovementComponent());
      if (movecmp) {
        // calculate the percentage of fuel the jetpack has
        int Energy = movecmp->Jetpackfuel / movecmp->JetpackMaxFuel * 100;
        // and draw the number to the screen, left side.
        Canvas->K2_DrawText(BigFont, FString::FromInt(Energy),
                            FVector2D(10, Canvas->ClipY / 5), FVector2D(1, 1),
                            FLinearColor::Black, 0, HUDLight, FVector2D(5, 5),
                            false, false, true, HUDLight);
      }
      
      if (sc->bIsStun) {
	Canvas->K2_DrawText(BigFont, "STUNNED",
			    FVector2D(Canvas->ClipX / 2, Canvas->ClipY / 2), FVector2D(1, 1),
			    HUDPurple, 0, HUDLight, FVector2D(5, 5), true, true, true, FLinearColor::White);
	}
    }
......
......
}
```
Example of STUNNED status on HUD:

<img src="https://github.com/RScarcia/ShooterGame_Custom/blob/master/Images/stunned.png" width="900">

Inside **PlayerPawn** blueprint:

<img src="https://github.com/RScarcia/ShooterGame_Custom/blob/master/Images/blueprint.PNG" width="900">
