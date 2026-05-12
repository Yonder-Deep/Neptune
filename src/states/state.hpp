#pragma once
#include "../neptune.hpp"
#include <chrono>
//Represents a state the usv can be in
class State{
    public:
    State(){

    }

    /**
     * @brief The driving logic for this tick
     * @returns The time, in ms, to wait before ticking again
     */
    virtual std::chrono::milliseconds tick(){
        return std::chrono::milliseconds(5000);
    }
};
