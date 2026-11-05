#include <gtest/gtest.h>
#include "../drivers/motors/thruster.hpp"
#include <stdexcept>
class ThrusterTest :  public Thruster {
public:
    // Use the protected constructor in the fixture's initializer list
    ThrusterTest() : Thruster() {}

};

TEST(Thruster, DutyCycle){
    ThrusterTest* thruster = new ThrusterTest();
    thruster->armed = true;
    thruster->cycle = 50;
    thruster->speed = 0.0;
    EXPECT_FLOAT_EQ(7.5f, thruster->getDutyCycle());

}

TEST(Thruster, UnarmedFailure){
    Thruster* thruster = new ThrusterTest();
    thruster->armed = false;
    ASSERT_ANY_THROW(thruster->setSpeed(0.0));
}
