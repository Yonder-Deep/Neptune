#include "ring_buffer.hpp"
#include "types.hpp"
#include <unistd.h>
#include <sys/wait.h>

void worker_push_states(const std::string& name) {
    SPSCQueue* queue = open_shared_queue(name);
    if (!queue) {
        std::cerr << "Worker: Failed to open queue" << std::endl;
        return;
    }

    std::cout << "Worker: Opened queue, pushing modified states" << std::endl;

    // Pop the initial state and modify it
    State s;
    if (queue->pop(s)) {
        std::cout << "Worker: Read initial state:" << std::endl;
        print_state(s);

        // Modify the state
        s.position[0] = 10.0;
        s.velocity[1] = 5.5;
        s.attitude[2] = 45.0;

        queue->push(s);
        std::cout << "Worker: Pushed modified state" << std::endl;

        // Push a couple more states to test buffering
        s.position[0] = 20.0;
        s.velocity[1] = 7.5;
        queue->push(s);

        s.position[0] = 30.0;
        queue->push(s);

        std::cout << "Worker: Pushed 3 states total" << std::endl;
    }

    munmap(queue, sizeof(SPSCQueue));
}

int main() {
    const std::string queue_name = "test_ring_buffer";

    std::cout << "=== Ring Buffer SPSC Queue Test ===" << std::endl;
    std::cout << "Creating shared queue: " << queue_name << std::endl;

    SPSCQueue* queue = create_shared_queue(queue_name);
    if (!queue) {
        std::cerr << "Failed to create shared queue" << std::endl;
        return 1;
    }

    // Create initial state using Vec3 from types.hpp
    Vec3 init_pos = {0.0, 0.0, 0.0};
    Vec3 init_vel = {0.0, 0.0, 0.0};
    Vec3 init_att = {0.0, 0.0, 0.0};
    Vec3 init_ang = {0.0, 0.0, 0.0};

    State initial_state(init_pos, init_vel, init_att, init_ang);

    std::cout << "\nMain: Pushing initial state:" << std::endl;
    print_state(initial_state);
    queue->push(initial_state);

    // Fork worker process
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        worker_push_states(queue_name);
        return 0;
    } else if (pid > 0) {
        // Parent process
        std::cout << "\nMain: Waiting for worker process..." << std::endl;
        waitpid(pid, nullptr, 0);

        std::cout << "\nMain: Worker finished, reading results from queue:" << std::endl;

        State result;
        int state_count = 0;
        while (queue->pop(result)) {
            state_count++;
            std::cout << "\n--- State #" << state_count << " ---" << std::endl;
            print_state(result);
        }

        if (state_count == 0) {
            std::cout << "No states received!" << std::endl;
        } else {
            std::cout << "\nMain: Successfully received " << state_count << " state(s)" << std::endl;
        }
    } else {
        std::cerr << "Fork failed" << std::endl;
        cleanup_shared_queue(queue_name);
        return 1;
    }

    std::cout << "\nCleaning up shared queue..." << std::endl;
    cleanup_shared_queue(queue_name);
    std::cout << "Test complete!" << std::endl;

    return 0;
}