cmake_minimum_required(VERSION 3.22)

# set(CMAKE_C_COMPILER "/usr/bin/clang")
# set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

project(eliterobots_cpp CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(vendor/)
include_directories(vendor/json-rpc-cxx/include)

include_directories(${CMAKE_BINARY_DIR})
include_directories(include)

add_subdirectory(vendor/json)

add_executable(app
    ./src/main.cpp
)

# add_dependencies(app
    # httplib::httplib
# )

target_link_libraries(app
    nlohmann_json::nlohmann_json
)

