#pragma once
/**
 * @brief The abstract class that represents a motor on the robot
 *
 */
class Motor
{
public:
    /**
     * @brief sets the motor to run at speed until this or another method is called
     * @param speed A value between -1.0 and 1.0, with -1.0 being max reverse thrust, 0.0 being still, and 1.0 being
     * @return The return code. Negative indicates errors, otherwise the meaning depends on the impelmenting class
     */
    virtual int setSpeed(float speed);
    /**
     * @brief Sets the frequency of the motor pwm, ie how quick cycles are sent to the ESC
     * @param cycle The value, in hz, for the new cycle
     * @return The return code. Negative indicates errors, otherwise the meaning depends on the impelmenting class
     */
    virtual int setFrequency(float cycle);

    /**
     * @brief arms the motor. Likely a blocking method for a few seconds
     */
    virtual void armMotor();
    float speed;
    float cycle;
    protected:
    Motor(int speed,int  cycle) : speed(speed), cycle(cycle){};
};