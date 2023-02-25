// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#include "RobotContainer.h"

#include <utility>

#include <frc/controller/PIDController.h>
#include <frc/geometry/Translation2d.h>
#include <frc/shuffleboard/Shuffleboard.h>
#include <frc/trajectory/Trajectory.h>
#include <frc/trajectory/TrajectoryGenerator.h>
#include <frc2/command/InstantCommand.h>
#include <frc2/command/SequentialCommandGroup.h>
#include <frc2/command/ParallelCommandGroup.h>
#include <frc2/command/SwerveControllerCommand.h>
#include <frc2/command/button/JoystickButton.h>
#include <units/angle.h>
#include <units/velocity.h>
#include <frc/filter/SlewRateLimiter.h>

#include "Constants.h"
#include "subsystems/DriveSubsystem.h"

using namespace DriveConstants;



RobotContainer::RobotContainer() {
  // Initialize all of your commands and subsystems here


  // Configure the button bindings
  ConfigureButtonBindings();

  // printf("x: %5.2f y: %5.2f rot: %5.2f\n", xSpeed, ySpeed, rot);

  m_drive.SetDefaultCommand(DefaultDriveCMD(&m_drive, 
          [this] {return (-m_speedLimitx.Calculate(frc::ApplyDeadband(m_driverController.GetLeftY(), 0.1)) * (double)AutoConstants::kMaxSpeed);}, 
          [this] {return (m_speedLimity.Calculate(frc::ApplyDeadband(m_driverController.GetLeftX(), 0.1)) * (double)AutoConstants::kMaxSpeed);},
          [this] {return (m_speedLimitz.Calculate(frc::ApplyDeadband(m_driverController.GetRightX(), 0.1)) * (double)AutoConstants::kMaxAngularSpeed);}, 
          [this] {return true;}));

  m_Extension.SetDefaultCommand(DefaultExtendCMD(&m_Extension, 
          [this] {return m_driverController.GetRightBumper();},
          [this] {return m_driverController.GetLeftBumper();}));

}

void RobotContainer::ConfigureButtonBindings() {
  frc2::JoystickButton(&m_driverController, frc::XboxController::Button::kLeftBumper).WhenPressed(
    frc2::RunCommand([this] {m_drive.ZeroHeading();}, {&m_drive}));
  
  frc2::JoystickButton(&m_driverController, frc::XboxController::Button::kY).WhenHeld(
    frc2::RunCommand([this] {m_Winch.RunWinch(1.0);}, {&m_Winch}));

  frc2::JoystickButton(&m_driverController, frc::XboxController::Button::kA).WhenHeld(
    frc2::RunCommand([this] {m_Winch.RunWinch(-0.5);}, {&m_Winch}));

}

frc2::Command* RobotContainer::GetAutonomousCommand() {
  // Set up config for trajectory
  frc::TrajectoryConfig config(AutoConstants::kMaxSpeed,
                               AutoConstants::kMaxAcceleration);
  // Add kinematics to ensure max speed is actually obeyed
  config.SetKinematics(m_drive.kDriveKinematics);

  // An example trajectory to follow.  All units in meters.
  auto exampleTrajectory = frc::TrajectoryGenerator::GenerateTrajectory(
      // Start at the origin facing the +X direction
      frc::Pose2d(0_m, 0_m, frc::Rotation2d(0_deg)),
      // Pass through these two interior waypoints, making an 's' curve path
      {frc::Translation2d(1_m, 1_m), frc::Translation2d(2_m, -1_m)},
      // End 3 meters straight ahead of where we started, facing forward
      frc::Pose2d(3_m, 0_m, frc::Rotation2d(0_deg)),
      // Pass the config
      config);

  frc::ProfiledPIDController<units::radians> thetaController{
      AutoConstants::kPThetaController, 0, 0,
      AutoConstants::kThetaControllerConstraints};

  thetaController.EnableContinuousInput(units::radian_t(-std::numbers::pi),
                                        units::radian_t(std::numbers::pi));

  m_Routine = frc::SmartDashboard::GetNumber("Auto", 0);

  //switch to make it easy to add additions
  switch (m_Routine) {
    case 1:
    m_SelectedTrajectory = exampleTrajectory;
    break;
    case 2:
    m_SelectedTrajectory = m_auto.GetTrajectory1();
    break;
  }


  frc2::SwerveControllerCommand<4> swerveControllerCommand(
      m_SelectedTrajectory, [this]() { return m_drive.GetPose(); },

      m_drive.kDriveKinematics,

      frc2::PIDController(AutoConstants::kPXController, 0, 0),
      frc2::PIDController(AutoConstants::kPYController, 0, 0), thetaController,

      [this](auto moduleStates) { m_drive.SetModuleStates(moduleStates); },

      {&m_drive});

  // Reset odometry to the starting pose of the trajectory.
  m_drive.ResetOdometry(m_SelectedTrajectory.InitialPose());

  // no auto // I do not know why this says no auto although I have heard issues about this auto here
  // I think that for actualy robot code that will include important things like shooting in 2023
  // will have a "wrapper" command group to run multiple actions at once using parellel command group
  return new frc2::ParallelCommandGroup(
  frc2::SequentialCommandGroup(
      std::move(swerveControllerCommand),
      frc2::InstantCommand(
          [this]() {
            m_drive.Drive(units::meters_per_second_t(0),
                          units::meters_per_second_t(0),
                          units::radians_per_second_t(0), false);
          },
          {})));
}
