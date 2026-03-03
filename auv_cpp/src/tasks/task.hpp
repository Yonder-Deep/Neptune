#pragma once

// Base interface for all AUV subsystem tasks.
// Subclasses override loop() with their work (sensor reading, PID, etc.).
// ThreadTask handles the thread lifecycle around this.
struct Task {
    virtual ~Task() = default;
    virtual void loop() = 0;
};
