#include <gtest/gtest.h>
#include "thruster.hpp"
#include <stdexcept>
TEST(Thruster, DutyCycle){
    Thruster* thruster = new Thruster();
    thruster->armed = true;
    thruster->cycle = 50;
    thruster->speed = 0.0;
    EXPECT_EQ(7.5, thruster->getDutyCycle());

}

TEST(Thruster, UnarmedFailure){
    Thruster* thruster = new Thruster(0,0);
    thruster->armed = false;
    ASSERT_THROW(thruster->setSpeed(50, std::logic_error()));
}
