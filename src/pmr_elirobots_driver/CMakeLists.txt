cmake_minimum_required(VERSION 3.20)
project(pmr_elirobots_driver CXX)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic)

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(std_msgs REQUIRED)
find_package(control_msgs REQUIRED)
find_package(trajectory_msgs REQUIRED)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)

include_directories(vendor/)
include_directories(vendor/json-rpc-cxx/include)

include_directories(${CMAKE_BINARY_DIR})
include_directories(include)

add_subdirectory(vendor/json)

add_library(robot
  src/robot.cpp
)

add_executable(main
  src/main.cpp
)

add_executable(driver 
  src/driver.cpp
)

target_link_libraries(robot
  nlohmann_json::nlohmann_json
)

target_link_libraries(main
  robot
)

target_link_libraries(driver
  robot
  rclcpp::rclcpp
  rclcpp_action::rclcpp_action
  ${std_msgs_TARGETS}
  ${control_msgs_TARGETS}
  ${trajectory_msgs_TARGETS}
)

target_include_directories(robot PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
)

# target_compile_features(driver PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17

install(TARGETS driver main robot
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
