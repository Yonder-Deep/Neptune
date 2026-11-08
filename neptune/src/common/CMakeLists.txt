add_library(common INTERFACE)
target_include_directories(common INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

CPMAddPackage(
    NAME yaml-cpp
    GITHUB_REPOSITORY jbeder/yaml-cpp
    GIT_TAG yaml-cpp-0.9.0
)
CPMAddPackage(
    NAME reflectcpp
    GITHUB_REPOSITORY munich-data-engineering/reflect-cpp
    GIT_TAG v0.26.2
)

target_link_libraries(common INTERFACE
    yaml-cpp::yaml-cpp
    reflectcpp::reflectcpp
)
