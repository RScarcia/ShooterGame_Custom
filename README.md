# ShooterGame_Custom

Custom version of UE4 Shooter Game example project, developed for a Multiplayer environment.

New Features:
* Teleport
* Jetpack
* WallJump

Added classes are:
* **MyShooterCharacter** (.h and .cpp): extension of ShooterCharacter class, manages teleport, walljump and some functions of the jetpack feature.
* **MyShooterCharacterMovement** (.h and cpp): extension of ShooterCharacterMovement class, handles everything concerning the jetpack movement.

Some minor changes inside **ShooterHUD** class, to display the percentage of jetpack fuel.
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
        Canvas->DrawText(BigFont, FString::FromInt(Energy), 10, 150);
      }
    }
......
......
}
```

Inside **PlayerPawn** blueprint:

<img src="https://github.com/RScarcia/ShooterGame_Custom/blob/master/Images/blueprint.PNG" width="900">
