# Protobuf message definitions and code generation

find_package(Protobuf REQUIRED)

# Generate C++ code from .proto files
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS
    messages.proto
)

# Create a library with the generated protobuf code
add_library(neptune_messages STATIC
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

# Link against protobuf library
target_link_libraries(neptune_messages PUBLIC
    protobuf::libprotobuf
)

# Include directories for generated headers
target_include_directories(neptune_messages PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROTOBUF_INCLUDE_DIR}
)

# Ensure this directory is generated first
set_source_files_properties(
    ${PROTO_SRCS}
    PROPERTIES GENERATED TRUE
)
