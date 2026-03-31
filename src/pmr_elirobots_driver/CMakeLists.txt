cmake_minimum_required(VERSION 3.20)
project(pmr_elirobots_driver)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic)

# find dependencies
find_package(ament_cmake REQUIRED)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)

include_directories(vendor/)
include_directories(vendor/json-rpc-cxx/include)

include_directories(${CMAKE_BINARY_DIR})
include_directories(include)

add_subdirectory(vendor/json)

add_executable(driver 
  # src/driver.cpp
  src/main.cpp
  src/robot.cpp
)

target_link_libraries(driver
  nlohmann_json::nlohmann_json
)

target_include_directories(driver PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)
target_compile_features(driver PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17

install(TARGETS driver
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
