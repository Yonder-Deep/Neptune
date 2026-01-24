add_library(auv_core INTERFACE)

target_include_directories(auv_core INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(auv_core INTERFACE
    Eigen3::Eigen
)
