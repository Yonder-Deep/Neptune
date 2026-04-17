cmake_minimum_required(VERSION 3.16)
project(neptune_base_station VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)

# Build protobuf library (same messages as embedded)
add_library(neptune_messages_client STATIC
    messages.pb.cc
    messages.pb.h
)

target_link_libraries(neptune_messages_client PUBLIC
    protobuf::libprotobuf
)

target_include_directories(neptune_messages_client PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROTOBUF_INCLUDE_DIR}
)

# Build log client executable
add_executable(log_client log_client.cpp)

target_link_libraries(log_client PRIVATE
    Threads::Threads
    neptune_messages_client
    protobuf::libprotobuf
)

# Include websocketpp headers (header-only library)
target_include_directories(log_client PRIVATE
    /usr/include
)

# Set output directory for the executable
set_target_properties(log_client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Installation
install(TARGETS log_client
    RUNTIME DESTINATION bin
)
