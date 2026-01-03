cmake_minimum_required(VERSION 3.22)

project(eliterobots_cpp CXX)

find_package(jsoncpp REQUIRED)
find_package(libjson-rpc-cpp REQUIRED COMPONENTS client)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/gen)

add_custom_target(client_stub ALL
    jsonrpcstub ${CMAKE_CURRENT_SOURCE_DIR}/rsc/spec.json --cpp-client=StubClient --cpp-client-file=${CMAKE_BINARY_DIR}/gen/eliterobots.h
    DEPENDS rsc/spec.json
    COMMENT "Generating client stubfile"
)

include_directories(${CMAKE_BINARY_DIR})
include_directories(include)

add_executable(app
    ./src/main.cpp
)

add_dependencies(app
    client_stub
)

target_link_libraries(app
    jsoncpp
    curl
    jsonrpccpp::common
    jsonrpccpp::client
)

