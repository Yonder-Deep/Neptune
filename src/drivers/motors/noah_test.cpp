#include "motor.hpp"
#include "sim_motor.hpp"
#include <cstdlib>
#include <ctime>
#include <random>
int main(){
    srand(time(0));
    Motor* m = new SimulatedMotor(SimulatedMotor::BackRight);
    Motor* m1 = new SimulatedMotor(SimulatedMotor::FrontRight);
    Motor* m2 = new SimulatedMotor(SimulatedMotor::FrontLeft);
    Motor* m3 = new SimulatedMotor(SimulatedMotor::BackLeft);
    m->armMotor();
    m1->armMotor();
    m2->armMotor();
    m3->armMotor();
     std::random_device rd;  
    std::mt19937 gen(rd()); 
     std::uniform_real_distribution<double> dis(-1.0, 1.0);
    while (true)
    {
        if(rand() % 2) m->setSpeed(dis(gen));
        if(rand() % 2)  m1->setSpeed(dis(gen));
        sleep(3);
    }
}