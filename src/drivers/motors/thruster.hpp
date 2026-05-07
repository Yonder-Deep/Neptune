#pragma once
#include "motor.hpp"
#include <lgpio.h>
/**
 * @brief Represents one of the blue robotics thrusters on the robot
 */
class Thruster : public Motor {
public:
  int pin;
  int handle;
  bool armed;

  Thruster(int pin, int handle, int speed = 0, int cycle = 50);
  ~Thruster();
  int setSpeed(float speed) override;

  int setFrequency(float cycle) override;

  void armMotor() override;
  /**
   * @brief gets the duty cycle to send to the motor in order to move the motor
   * at the current speed
   */
  float getDutyCycle();

protected:
  explicit Thruster() {};

private:
  const int STOP_PWM = 1500;
  const int MAX_PWM_DIST = 400;
  const int ARM_SECONDS = 3; // I dont think this actually has to be 7 seconds,
                             // like 3 works but better safe than sorry

  /**
   * @brief updates the pwm values being sent. Must be called after any updates
   * to class variables that will affect this value
   * @return the return value of the lgTxPwm call
   */
  int sendMotorPWM();
};
