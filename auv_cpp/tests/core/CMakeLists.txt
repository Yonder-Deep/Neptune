# =============================================================================
# tests/core/CMakeLists.txt - Core types tests
# =============================================================================

# Create test executable
add_executable(test_types test_types.cpp)

# Link against our core library
target_link_libraries(test_types PRIVATE
    auv_core
    Threads::Threads
)

# Register with CTest
add_test(NAME test_types COMMAND test_types)
