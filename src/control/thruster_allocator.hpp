#pragma once
#include "../types/BodyCommand.hpp"
#include "../types/MotorLocation.hpp"

#include <Eigen/Dense>
#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>

/**
 * Position and push direction of one thruster. Body frame: +x forward, +y starboard.
 */
struct ThrusterConfig
{
    /** Radians from +x toward +y, the direction the boat is pushed at positive speed */
    double angle;
    /** Meters from the center of mass */
    double x;
    double y;
};

/** Indexed by MotorLocation */
using ThrusterLayout = std::array<ThrusterConfig, 4>;

/**
 * Converts a BodyCommand into a motor speed
 */
class ThrusterAllocator
{
public:
    /**
     * Placeholder geometry, replace later
     */
    static ThrusterLayout defaultLayout()
    {
        constexpr double quarter_pi = M_PI_4;
        constexpr double half_length = 0.40;
        constexpr double half_width = 0.30;

        ThrusterLayout layout{};
        layout[FrontRight] = {-quarter_pi, half_length, half_width};
        layout[FrontLeft] = {quarter_pi, half_length, -half_width};
        layout[BackRight] = {quarter_pi, -half_length, half_width};
        layout[BackLeft] = {-quarter_pi, -half_length, -half_width};
        return layout;
    }

    /**
     * @param layout Thruster geometry
     * @param reverse_efficiency Reverse thrust relative to forward, in (0, 1]
     * @throws std::invalid_argument If the layout can't control surge, sway and yaw independently
     */
    explicit ThrusterAllocator(ThrusterLayout layout = defaultLayout(), double reverse_efficiency = 1.0)
        : layout_(layout), reverse_efficiency_(reverse_efficiency)
    {
        if (!(reverse_efficiency > 0.0 && reverse_efficiency <= 1.0))
        {
            throw std::invalid_argument("reverse_efficiency must be in (0, 1]");
        }

        for (int i = 0; i < 4; ++i)
        {
            const ThrusterConfig &t = layout_[i];
            B_(0, i) = std::cos(t.angle);
            B_(1, i) = std::sin(t.angle);
            B_(2, i) = t.x * std::sin(t.angle) - t.y * std::cos(t.angle);
        }

        Eigen::CompleteOrthogonalDecomposition<Eigen::Matrix<double, 3, 4>> decomposition(B_);
        if (decomposition.rank() < 3)
        {
            throw std::invalid_argument("thruster layout cannot control surge, sway and yaw independently");
        }
        pseudo_inverse_ = decomposition.pseudoInverse();

        // Yaw is a torque, so normalize each axis to put the busiest thruster at full speed for a command of 1
        for (int axis = 0; axis < 3; ++axis)
        {
            pseudo_inverse_.col(axis) /= pseudo_inverse_.col(axis).cwiseAbs().maxCoeff();
        }
    }

    /**
     * @param command Desired motion, axes are clamped to [-1, 1]
     * @return Speed for each motor in [-1, 1], indexed by MotorLocation.
     * Requests beyond the motors' limits are scaled down evenly, which keeps the direction.
     */
    std::array<double, 4> allocate(const BodyCommand &command) const
    {
        Eigen::Vector3d wanted(std::clamp(command.surge, -1.0, 1.0),
                               std::clamp(command.sway, -1.0, 1.0),
                               std::clamp(command.yaw, -1.0, 1.0));

        Eigen::Vector4d speeds = pseudo_inverse_ * wanted;

        for (int i = 0; i < 4; ++i)
        {
            if (speeds[i] < 0.0)
            {
                speeds[i] /= reverse_efficiency_;
            }
        }

        double peak = speeds.cwiseAbs().maxCoeff();
        if (peak > 1.0)
        {
            speeds /= peak;
        }
        return {speeds[0], speeds[1], speeds[2], speeds[3]};
    }

    /**
     * The force (x, y) and torque (z) produced by these motor speeds, in unnormalized units.
     */
    Eigen::Vector3d producedWrench(const std::array<double, 4> &speeds) const
    {
        Eigen::Vector4d thrust;
        for (int i = 0; i < 4; ++i)
        {
            thrust[i] = speeds[i] < 0.0 ? speeds[i] * reverse_efficiency_ : speeds[i];
        }
        return B_ * thrust;
    }

private:
    ThrusterLayout layout_;
    double reverse_efficiency_;
    Eigen::Matrix<double, 3, 4> B_;
    Eigen::Matrix<double, 4, 3> pseudo_inverse_;
};
