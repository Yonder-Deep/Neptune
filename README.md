# Neptune
Yonder Deep 2026

## To Get Started:
READ DOCS/docker.md

## Branch Management

### Main Branch
- `main` is the primary working branch and should always contain stable, tested code
- All code in `main` must pass automated tests and peer review
- Do not force push to `main`, create your own branch before working on your issue.

### Development Branches
- Create feature branches for individual or subteam work: `feature/sensor-integration`, `feature/navigation-system`
- Use descriptive branch names: `andytgarcia/pressure-sensor-calibration`, `yonderdeep/motor-controller-pid`
- Branch naming convention: `yourgithubusername/brief-descriptive-name`
- Delete branches after merging to keep the repository clean

### Merge Process
1. Create a pull request from your branch to `main`
2. Ensure all tests pass (or write your own)
3. Request review from at least one team member
4. Address review comments
5. Merge only after approval

## Code Style Guidelines

### Indentation
- Nested blocks should be indented one level deeper

### Line Lengths
- Make lines entirely readable on screen
- Break long lines logically at operators, commas, or before opening parentheses
- Comments should also respect the line length limit

### Brace Style

```cpp
if (condition) {
    // code here
}
else {
    // alternative code
}

void functionName() {
    // function body
}
```

### Naming Conventions

#### Variables
- Use `camelCase` for local variables: `sensorData`, `motorSpeed`
- Use `UPPER_SNAKE_CASE` for constants: `MAX_DEPTH`, `SENSOR_TIMEOUT_MS`
- Use descriptive names that convey purpose: `currentDepth` instead of `d`

#### Functions
- Use `camelCase` for function names: `calculateBuoyancy()`, `updateMotorController()`
- Use verb-noun pairs for clarity: `readPressureSensor()`, `setThrottle()`

#### Classes
- Use `PascalCase` for class names: `NavigationSystem`, `PressureSensor`
- Use nouns or noun phrases: `MotorController`, `DataLogger`

#### Files
- Use `snake_case` for filenames: `motor_controller.cpp`, `pressure_sensor.h`
- Header files use `.h` or `.hpp` extension
- Implementation files use `.cpp` or `.c` extension

## Documentation Standards

### Header Files
Use '#pragma once' for all .h and .hpp files

```cpp

#pragma once

class ClassName

```

### Class Headers
Document every class with its purpose and usage:

```cpp
/**
 * @class PressureSensor
 * @brief Interface for MS5837 pressure/temperature sensor
 *
 * Provides depth and temperature readings using I2C communication.
 * Handles calibration and conversion to engineering units.
 */
class PressureSensor {
    // class definition
};
```

### Function Headers
Document all public functions and complex private functions:


### Inline Comments
- Use inline comments for complex logic or non-obvious decisions
- Explain the "why," not the "what"
- Keep comments up-to-date with code changes


### First Time Contributors
1. Read this entire README
2. Review the existing codebase to understand project structure
3. Set up your development environment
4. Start with a small issue labeled "good first issue"
5. Ask questions in Discord


For questions or suggestions about these guidelines, please open an issue or contact the project maintainer.
