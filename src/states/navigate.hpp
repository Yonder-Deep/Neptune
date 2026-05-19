#pragma once
#include "state.hpp"
#include "../../neptune.hpp"
#include <optional>
#include <array>
#include <cmath>
#include <iostream>
/**Tries to solve with no negative inputs, so motors don't fire backwards
 * This is vibe coded slop. In the future, we should add a linear algebra lib instead of using this
 */
std::optional<std::array<double, 4>> solveNonNegative(const std::array<std::array<double, 5>, 2>& mat)
{
    constexpr int    VARS = 4;
    constexpr double EPS  = 1e-9;

    // Try every pair of "basic" variables; the remaining two are fixed at 0.
    for (int p = 0; p < VARS; ++p) {
        for (int q = p + 1; q < VARS; ++q) {

            // Extract the 2x2 sub-system for columns p and q.
            const double a00 = mat[0][p],  a01 = mat[0][q],  b0 = mat[0][4];
            const double a10 = mat[1][p],  a11 = mat[1][q],  b1 = mat[1][4];

            const double det = a00 * a11 - a01 * a10;
            if (std::abs(det) < EPS)
                continue;   // columns are linearly dependent — skip

            // Cramer's Rule
            const double xp = (b0 * a11 - b1 * a01) / det;
            const double xq = (a00 * b1 - a10 * b0) / det;

            if (xp < -EPS || xq < -EPS)
                continue;   // a basic variable went negative — skip

            // Valid non-negative basic feasible solution found.
            std::array<double, 4> solution{};   // nosn-basics default to 0
            solution[p] = std::max(0.0, xp);    // clamp -0 artefacts
            solution[q] = std::max(0.0, xq);
            return solution;
        }
    }

    return std::nullopt;   // all 6 bases checked, none feasible
}



class Navigate : public State {
    public:
    bool finished;
    GPSCoordinate goal;
    Navigate(GPSCoordinate goal) : goal(goal){

    }


    std::chrono::milliseconds tick() override {
        double current_heading = Neptune::instance->get_heading();
        GPSCoordinate* loc = Neptune::instance->gps->location();
        GPSCoordinate goal_dif =  goal - *loc;
        double mag = pow( goal_dif.latitude * goal_dif.latitude + goal_dif.longitude * goal_dif.longitude, 0.5);
        std::cout << "Dist:" << mag << std::endl;
        if(mag < 0.1){
            //super close, get a random new cord
            goal.latitude = ((rand() % 30) - 15);
            goal.longitude = ((rand() % 30) - 15);
        }
        goal_dif.latitude /= mag;
        goal_dif.longitude /= mag;
        /**
         * We know we want to translate along goal_dif
         * We can form goal_dif by combining the vectors of each motor's thrust vectors
         * Thus, our problem is to form a linear combination of the goal vector through the 4 thrusters, and take one solution
         * ex an array
         * m1x m2x m3x m4x gx
         * and for y
         */
        std::array<std::array<double, 5>, 2> mat = Neptune::instance->GetMotorVectorMatrice();
        mat[0][4] = goal_dif.latitude;
        mat[1][4] = goal_dif.longitude;
        std::optional<std::array<double, 4>> out = solveNonNegative(mat);
        std::cout << goal <<   *loc << goal_dif  << std::endl;
        if(out.has_value()){
            for(int i = 0; i < out.value().size(); i ++) {
                std::cout << i << ":" << out.value()[i] << std::endl;
                //TODO normalize to some constant range based on desired activation
                Neptune::instance->all_motors[i]->setSpeed(out.value()[i]);
            }
        }
        else {
            std::cout << "Failed to solve" << std::endl;
        }
        delete loc;
        return std::chrono::milliseconds(1000);
    }
};
