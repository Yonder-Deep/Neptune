#pragma once

/**
 * A requested motion in the boat's own frame, each axis bounded [-1, 1].
 * +surge is forward, +sway is toward starboard, +yaw is clockwise seen from above.
 */
struct BodyCommand
{
    double surge = 0.0;
    double sway = 0.0;
    double yaw = 0.0;
};
