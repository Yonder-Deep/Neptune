#pragma once
#include <atomic>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>

#include "../core/ring_buffer.hpp"
#include "../core/types.hpp"
#include "task.hpp"

// Thread wrapper that manages the lifecycle of a Task.
// Replaces both Python TTask and PTask — C++ version uses only threads
// with lock-free IPC (SPSC ring buffers) instead of multiprocessing.
//
// Lifecycle:
//   1. startup()    — spawns the thread (blocked, waiting for activate)
//   2. activate()   — unblocks the thread, loop() starts running
//   3. deactivate() — pauses the thread (stays alive, blocks until re-activated)
//   4. shutdown()   — signals the thread to exit
//   5. join()       — waits for the thread to finish
//
// Usage:
//   struct MyTask : ThreadTask {
//       MyTask() : ThreadTask("my-task") {}
//       void loop() override {
//           // read sensor, compute PID, etc.
//       }
//   };
//
//   MyTask task;
//   task.startup();
//   task.activate();
//   // ... later ...
//   task.shutdown();
//   task.join();
//
struct ThreadTask : Task {
    std::string name;

    // SPSC queues for IPC with main loop
    SPSC<OutboundMessage, 128> outbound_q;  // Task → main loop → base
    SPSC<InboundMessage, 64> inbound_q;     // Main loop → task (future use)

    // started == true means the thread should keep running.
    // Setting to false tells the run loop to exit after the current iteration.
    std::atomic<bool> started{false};

    // enabled == true means loop() should execute.
    // When false, the thread blocks on the condition variable until re-enabled.
    std::atomic<bool> enabled{false};

    // The actual OS thread. Default-constructed (not yet running).
    std::thread worker;

    // Condition variable for the enable/disable mechanism.
    // The thread sleeps here when disabled instead of busy-waiting.
    std::mutex mtx;
    std::condition_variable cv;

    explicit ThreadTask(std::string task_name) : name(std::move(task_name)) {}



    // Prevent copying — a thread can't be copied
    ThreadTask(const ThreadTask&) = delete;
    ThreadTask& operator=(const ThreadTask&) = delete;

    // Destructor ensures the thread is cleaned up.
    // If someone forgets to call shutdown()+join(), we do it here.
    ~ThreadTask() override {
        if (worker.joinable()) {
            shutdown();
            worker.join();
        }
    }

    // Spawn the thread. It will block immediately until activate() is called.
    void startup() {
        started.store(true);
        worker = std::thread(&ThreadTask::run, this);
    }

    // Unblock the thread — loop() starts executing.
    void activate() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            enabled.store(true);
        }
        cv.notify_one();
    }

    // Pause the thread — it blocks on the condition variable.
    // The thread stays alive, just sleeping. No CPU burned.
    void deactivate() {
        enabled.store(false);
    }

    // Signal the thread to exit. Must call join() after this.
    void shutdown() {
        started.store(false);
        // Set enabled to true so the thread wakes up from cv.wait()
        // and sees that started is false, then breaks out of the loop.
        {
            std::lock_guard<std::mutex> lock(mtx);
            enabled.store(true);
        }
        cv.notify_one();
    }

    // Wait for the thread to finish. Blocks until the thread exits.
    void join() {
        if (worker.joinable()) {
            worker.join();
        }
    }



    // Helper: enqueue a serialized protobuf Envelope to the outbound queue.
    // Typically called from logger.hpp when a log message is written.
    //
    // @param serialized_envelope: the raw bytes of a serialized Envelope protobuf
    // @return true if enqueued successfully, false if queue is full (log dropped)
    bool enqueue_message(const std::string& serialized_envelope) {
        OutboundMessage msg;
        msg.envelope_len = std::min(serialized_envelope.size(), size_t(512));
        std::memcpy(msg.envelope_bytes.data(), serialized_envelope.c_str(), msg.envelope_len);
        return outbound_q.push(msg);
    }



private:
    // The framework loop — not virtual. Subclasses override loop(), not this.
    // Mirrors the Python TTask.run() pattern:
    //   while True:
    //       if not enabled: wait()
    //       if not started: break
    //       loop()
    void run() {
        while (true) {
            {
                // Block here until enabled becomes true.
                // cv.wait releases the lock while sleeping, re-acquires on wake.
                // The predicate [this]{ return enabled.load(); } prevents
                // spurious wakeups — the thread only proceeds when truly enabled.
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this] { return enabled.load(); });
            }

            // If shutdown() was called, started is false — exit the loop.
            if (!started.load()) break;

            // Subclass does its work here.
            loop();
        }
    }
};
