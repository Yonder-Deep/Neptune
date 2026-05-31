
#pragma once
#include "state.hpp"
class Idle : public State {
    public:

    std::chrono::milliseconds tick() override {
        std::cout << "Idling" << std::endl;
        return std::chrono::milliseconds(5000);
    }
};
