/**
@Header ThrusterController
@brief 4-motor X-config thrust allocation for a planar surface AUV.

Takes (forward, strafe, yaw) command axes in [-1, 1] and writes
PWM values to the four corner thrusters. Does not own the Motors.

Geometry: 26.565 deg pinwheel on a perfect-square chassis. Each
thruster is splayed outward from the longitudinal axis by 26.565 deg.
This yields cos(26.565) = 0.8944 forward authority and
sin(26.565) = 0.4472 strafe/yaw authority per motor.
*/

#pragma once

#include <algorithm>
#include <cmath>
#include "motor.hpp"

class ThrusterController {
    private:
        Motor& lt;  // front-left
        Motor& rt;  // front-right
        Motor& lb;  // back-left
        Motor& rb;  // back-right

        // PWM swing from neutral, both directions (us).
        // ESC dead zone is ~1475-1525, so MIN_SWING avoids it.
        static constexpr int PWM_SWING = 400;  // max offset from 1500 us

        static int toPwm(float u) {
            int offset = static_cast<int>(u * PWM_SWING);
            return Motor::NEUTRAL_PWM + offset;
        }

    public:
        ThrusterController(Motor& lt_, Motor& rt_, Motor& lb_, Motor& rb_)
            : lt(lt_), rt(rt_), lb(lb_), rb(rb_) {}

        // 26.565 deg pinwheel allocation coefficients
        static constexpr float EFF_FWD = 0.894427f;  // cos(26.565 deg)
        static constexpr float EFF_LAT = 0.447214f;  // sin(26.565 deg)

        // forward / strafe / yaw each in [-1, 1]. Saturation is handled by
        // scaling all four motor outputs down uniformly, preserving the
        // commanded direction even when the sum of axes exceeds unit magnitude.
        void setAxes(float forward, float strafe, float yaw) {
            float u_lt = ( EFF_FWD * forward) + (-EFF_LAT * strafe) + ( EFF_LAT * yaw);
            float u_rt = ( EFF_FWD * forward) + ( EFF_LAT * strafe) + (-EFF_LAT * yaw);
            float u_lb = (-EFF_FWD * forward) + (-EFF_LAT * strafe) + (-EFF_LAT * yaw);
            float u_rb = (-EFF_FWD * forward) + ( EFF_LAT * strafe) + ( EFF_LAT * yaw);

            float maxMag = std::max({std::abs(u_lt), std::abs(u_rt),
                                     std::abs(u_lb), std::abs(u_rb)});
            if (maxMag > 1.0f) {
                u_lt /= maxMag; u_rt /= maxMag;
                u_lb /= maxMag; u_rb /= maxMag;
            }

            lt.setPwm(toPwm(u_lt));
            rt.setPwm(toPwm(u_rt));
            lb.setPwm(toPwm(u_lb));
            rb.setPwm(toPwm(u_rb));
        }
};
