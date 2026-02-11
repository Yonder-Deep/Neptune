#pragma once

#include "types.hpp"
#include <iostream>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <cstring>
#include <string>
#include <map>
#include <memory>

const size_t BUFFER_SIZE = 1024;  // Fixed size; adjust as needed

using SPSCQueue = boost::lockfree::spsc_queue<State, boost::lockfree::capacity<BUFFER_SIZE>>;

// Global map to keep managed_shared_memory objects alive
static std::map<std::string, std::unique_ptr<boost::interprocess::managed_shared_memory>> g_segments;

inline SPSCQueue* create_shared_queue(const std::string& name) {
    try {
        // Remove existing if present
        boost::interprocess::shared_memory_object::remove(name.c_str());
        
        auto segment = std::make_unique<boost::interprocess::managed_shared_memory>(
            boost::interprocess::create_only, name.c_str(), 262144);  // 256KB segment
        
        SPSCQueue* queue = segment->construct<SPSCQueue>("queue")();
        g_segments[name] = std::move(segment);  // Keep segment alive
        return queue;
    } catch (const boost::interprocess::interprocess_exception& e) {
        std::cerr << "Failed to create shared queue: " << e.what() << std::endl;
        return nullptr;
    }
}

inline SPSCQueue* open_shared_queue(const std::string& name) {
    try {
        auto segment = std::make_unique<boost::interprocess::managed_shared_memory>(
            boost::interprocess::open_only, name.c_str());
        
        SPSCQueue* queue = segment->find<SPSCQueue>("queue").first;
        g_segments[name] = std::move(segment);  // Keep segment alive
        return queue;
    } catch (const boost::interprocess::interprocess_exception& e) {
        std::cerr << "Failed to open shared queue: " << e.what() << std::endl;
        return nullptr;
    }
}

inline void cleanup_shared_queue(const std::string& name) {
    // Remove from global map (destructor called automatically)
    g_segments.erase(name);
    // Remove shared memory object
    boost::interprocess::shared_memory_object::remove(name.c_str());
}

inline void print_state(const State& s) {
    std::cout << "Position: [" << s.position[0] << ", " << s.position[1] << ", " << s.position[2] << "]" << std::endl;
    std::cout << "Velocity: [" << s.velocity[0] << ", " << s.velocity[1] << ", " << s.velocity[2] << "]" << std::endl;
    std::cout << "Attitude: [" << s.attitude[0] << ", " << s.attitude[1] << ", " << s.attitude[2] << "]" << std::endl;
    std::cout << "Angular Velocity: [" << s.angular_velocity[0] << ", " << s.angular_velocity[1] << ", " << s.angular_velocity[2] << "]" << std::endl;
}

inline void worker_process(const std::string& name) {
    SPSCQueue* queue = open_shared_queue(name);
    if (!queue) return;

    State s;
    if (queue->pop(s)) {
        std::cout << "Worker: Read state:" << std::endl;
        print_state(s);
        s.position[0] = 10.0;  // Modify
        queue->push(s);
        std::cout << "Worker: Pushed modified state" << std::endl;
    }
}