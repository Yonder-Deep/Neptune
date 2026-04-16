#include "motor.hpp"
/**
 * @brief Represents one of the blue robotics thrusters on the robot
 */
class Thruster : Motor
{
public:
    int pin;
    int handle;
    Thruster(int pin, int handle, int speed = 0, int cycle = 50) : pin(pin) , handle(handle), Motor(speed, cycle){}
private:
    const int STOP_PWM = 1500;
    const int MAX_PWM_DIST = 400;
    bool armed;
    /**
     * @brief gets the duty cycle to send to the motor in order to move the motor at the current speed
     */
    float getDutyCycle();
    /**
     * @brief updates the pwm values being sent. Must be called after any updates to class variables that will affect this value
     * @return the return value of the lgTxPwm call
     */
    int sendMotorPWM();

    int setSpeed(float speed) override;

    int setFrequency(float cycle) override;
};