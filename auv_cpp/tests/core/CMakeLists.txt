# =============================================================================
# tests/core/CMakeLists.txt - Core tests
# =============================================================================

add_executable(test_types test_types.cpp)
target_link_libraries(test_types PRIVATE auv_core Threads::Threads)
add_test(NAME test_types COMMAND test_types)

add_executable(test_ring_buffer rbtest.cpp)
target_link_libraries(test_ring_buffer PRIVATE auv_core Threads::Threads)
add_test(NAME test_ring_buffer COMMAND test_ring_buffer)

add_executable(test_time_utils test_time_utils.cpp)
target_link_libraries(test_time_utils PRIVATE auv_core Threads::Threads)
add_test(NAME test_time_utils COMMAND test_time_utils)

add_executable(test_thread_task test_thread_task.cpp)
target_link_libraries(test_thread_task PRIVATE auv_core Threads::Threads)
add_test(NAME test_thread_task COMMAND test_thread_task)
