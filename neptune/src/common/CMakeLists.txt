add_library(common INTERFACE)
target_include_directories(common INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

xrepo_package("yaml-cpp 0.9.0")
xrepo_package("reflect-cpp 0.25.0") # Newer versions not in xrepo registry

target_include_directories(common INTERFACE
    ${yaml-cpp_INCLUDE_DIRS}
    ${reflect-cpp_INCLUDE_DIRS}
)
target_link_directories(common INTERFACE
    ${yaml-cpp_LIBRARY_DIRS}
    ${reflect-cpp_LIBRARY_DIRS}
)
target_link_libraries(common INTERFACE
    ${yaml-cpp_LINK_LIBRARIES}
    ${reflect-cpp_LINK_LIBRARIES}
)
