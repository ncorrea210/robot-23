// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#include <frc2/command/SwerveControllerCommand.h>

#include "commands/autos/TestSeqCMD.h"
#include "commands/FollowPPPathCMD.h"

// NOTE:  Consider using this command inline, rather than writing a subclass.
// For more information, see:
// https://docs.wpilib.org/en/stable/docs/software/commandbased/convenience-features.html
TestSeqCMD::TestSeqCMD(DriveSubsystem* drive) {
  // Add your commands here, e.g.
  // AddCommands(FooCommand{}, BarCommand{});
  AddCommands(FollowPPPathCMD(drive, "TPath"), StopCMD(drive));
}
