#include <gtest/gtest.h>
#include "../control/thruster_allocator.hpp"
#include <cmath>
#include <stdexcept>

namespace
{
    ThrusterLayout oddLayout()
    /*
    * TODO: Update values
    */
    {
        ThrusterLayout layout{};
        layout[FrontRight] = {-0.6, 0.5, 0.25};
        layout[FrontLeft] = {0.9, 0.3, -0.35};
        layout[BackRight] = {0.7, -0.4, 0.3};
        layout[BackLeft] = {-0.5, -0.45, -0.2};
        return layout;
    }

    double peak(const std::array<double, 4> &s)
    {
        double p = 0;
        for (double v : s)
        {
            p = std::max(p, std::abs(v));
        }
        return p;
    }

    void expectPure(const ThrusterAllocator &a, BodyCommand cmd, int axis, double sign)
    {
        auto speeds = a.allocate(cmd);
        Eigen::Vector3d w = a.producedWrench(speeds);
        for (int i = 0; i < 3; ++i)
        {
            if (i == axis)
            {
                EXPECT_GT(sign * w[i], 1e-6);
            }
            else
            {
                EXPECT_NEAR(w[i], 0.0, 1e-9);
            }
        }
        EXPECT_NEAR(peak(speeds), 1.0, 1e-9);
    }
}

class AllocatorLayouts : public ::testing::TestWithParam<int>
{
protected:
    ThrusterAllocator make(double reverse = 1.0) const
    {
        return ThrusterAllocator(GetParam() == 0 ? ThrusterAllocator::defaultLayout() : oddLayout(), reverse);
    }
};

TEST_P(AllocatorLayouts, SurgeMovesForwardOnly)
{
    expectPure(make(), {1, 0, 0}, 0, 1);
    expectPure(make(), {-1, 0, 0}, 0, -1);
}

TEST_P(AllocatorLayouts, SwayMovesSidewaysOnly)
{
    expectPure(make(), {0, 1, 0}, 1, 1);
    expectPure(make(), {0, -1, 0}, 1, -1);
}

TEST_P(AllocatorLayouts, YawTurnsOnly)
{
    expectPure(make(), {0, 0, 1}, 2, 1);
    expectPure(make(), {0, 0, -1}, 2, -1);
}

TEST_P(AllocatorLayouts, ReverseEfficiencyKeepsMotionPure)
{
    expectPure(make(0.6), {0, 1, 0}, 1, 1);
    expectPure(make(0.6), {0, 0, 1}, 2, 1);
}

TEST_P(AllocatorLayouts, SaturationKeepsDirection)
{
    ThrusterAllocator a = make();
    auto full = a.allocate({1, 1, 1});
    auto small = a.allocate({0.05, 0.05, 0.05});
    EXPECT_LE(peak(full), 1.0 + 1e-12);

    double scale = peak(small);
    for (int i = 0; i < 4; ++i)
    {
        EXPECT_NEAR(full[i] / peak(full), small[i] / scale, 1e-9);
    }
}

TEST_P(AllocatorLayouts, OutOfRangeCommandsAreClamped)
{
    ThrusterAllocator a = make();
    EXPECT_EQ(a.allocate({5, 0, 0}), a.allocate({1, 0, 0}));
    EXPECT_EQ(a.allocate({0, -3, 0}), a.allocate({0, -1, 0}));
}

TEST_P(AllocatorLayouts, ZeroCommandStopsEverything)
{
    for (double s : make().allocate({0, 0, 0}))
    {
        EXPECT_DOUBLE_EQ(s, 0.0);
    }
}

INSTANTIATE_TEST_SUITE_P(Layouts, AllocatorLayouts, ::testing::Values(0, 1));

// Depends on the placeholder layout
TEST(AllocatorDefaultLayout, SignsOfEachMotor)
{
    ThrusterAllocator a;

    auto forward = a.allocate({1, 0, 0});
    for (double s : forward)
    {
        EXPECT_GT(s, 0.0);
    }

    auto clockwise = a.allocate({0, 0, 1});
    EXPECT_GT(clockwise[FrontLeft], 0.0);
    EXPECT_GT(clockwise[BackLeft], 0.0);
    EXPECT_LT(clockwise[FrontRight], 0.0);
    EXPECT_LT(clockwise[BackRight], 0.0);

    auto starboard = a.allocate({0, 1, 0});
    EXPECT_GT(starboard[FrontLeft], 0.0);
    EXPECT_GT(starboard[BackRight], 0.0);
    EXPECT_LT(starboard[FrontRight], 0.0);
    EXPECT_LT(starboard[BackLeft], 0.0);
}

TEST(AllocatorConstruction, RejectsLayoutThatCannotSway)
{
    ThrusterLayout layout = ThrusterAllocator::defaultLayout();
    for (auto &t : layout)
    {
        t.angle = 0.0;
    }
    EXPECT_THROW(ThrusterAllocator{layout}, std::invalid_argument);
}

TEST(AllocatorConstruction, RejectsBadReverseEfficiency)
{
    EXPECT_THROW(ThrusterAllocator(ThrusterAllocator::defaultLayout(), 0.0), std::invalid_argument);
    EXPECT_THROW(ThrusterAllocator(ThrusterAllocator::defaultLayout(), 1.5), std::invalid_argument);
}
