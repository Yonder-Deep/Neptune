# Control system executable
add_executable(auv_control main.cpp)

# Link required libraries
target_link_libraries(auv_control PRIVATE
    auv_core
    Threads::Threads
    spdlog::spdlog
    yaml-cpp
    neptune_messages
    protobuf::libprotobuf
)

# Include websocketpp headers (header-only library)
target_include_directories(auv_control PRIVATE
    /usr/include
)

# Set output directory for the executable
set_target_properties(auv_control PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
