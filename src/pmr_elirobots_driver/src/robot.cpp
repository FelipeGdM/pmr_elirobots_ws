#include "eliterobots/robot.hpp"
#include <boost/asio.hpp>
#include <cstdint>
#include <string>
#include <tuple>

namespace elite {

  Robot::Robot(const std::string &addr, uint16_t port)
      : addr(addr), port(port), httpClient{addr, port},
        client(jsonrpccxx::JsonRpcClient(httpClient, jsonrpccxx::version::v2)) {}

  template <typename T>
  std::tuple<bool, T> Robot::call_method(const jsonrpccxx::id_type &id, const std::string &name,
                                         const jsonrpccxx::positional_parameter &params) {
    try {
      auto retval = this->client.CallMethod<T>(id, name, params);
      return std::make_tuple(true, retval);
    } catch (jsonrpccxx::JsonRpcException &e) {
      std::cout << "Bad vibes\n";
      std::cout << e.Message() << "\n";
      return std::make_tuple(false, T{});
    }
  }

  template <typename T>
  std::tuple<bool, T> Robot::call_method_named(const jsonrpccxx::id_type &id,
                                               const std::string &name,
                                               const jsonrpccxx::named_parameter &params) {
    try {
      auto retval = this->client.CallMethodNamed<T>(id, name, params);
      return std::make_tuple(true, retval);
    } catch (jsonrpccxx::JsonRpcException &e) {
      return std::make_tuple(false, T{});
    }
  };

  std::tuple<bool, bool> Robot::get_servo_status() {
    return this->call_method<bool>(1, "getServoStatus");
  }

  std::tuple<bool, bool> Robot::set_servo_status(uint8_t status) {
    nlohmann::json params;
    params["status"] = status;

    auto request = this->call_method_named<std::string>(1, "set_servo_status", params);
    auto valid = std::get<bool>(request);

    return {valid, std::get<std::string>(request) == "true"};
  }

  std::tuple<bool, bool> Robot::sync_motor_status() {
    auto request = this->call_method<std::string>(1, "syncMotorStatus");
    auto valid = std::get<bool>(request);

    return {valid, std::get<std::string>(request) == "true"};
  }

  std::tuple<bool, bool> Robot::clear_alarm() {
    auto request = this->call_method<std::string>(1, "clearAlarm");
    auto valid = std::get<bool>(request);

    return {valid, std::get<std::string>(request) == "true"};
  }

  std::tuple<bool, bool> Robot::get_motor_status() {
    auto request = this->call_method<std::string>(1, "getMotorStatus");
    auto valid = std::get<bool>(request);

    return {valid, std::get<std::string>(request) == "true"};
  }

  std::tuple<bool, uint8_t> Robot::get_robot_state() {
    auto request = this->call_method<std::string>(1, "getRobotState");
    auto valid = std::get<bool>(request);

    return {valid, std::stoi(std::get<std::string>(request))};
  };

  std::tuple<bool, uint16_t> Robot::get_robot_mode() {
    auto request = this->call_method<std::string>(1, "getRobotMode");
    auto valid = std::get<bool>(request);

    return {valid, std::stoi(std::get<std::string>(request))};
  };

  std::tuple<bool, std::array<double, JOINT_COUNT>> Robot::get_joint_pos() {

    auto request = this->call_method<std::string>(1, "get_joint_pos");
    auto valid = std::get<bool>(request);

    auto result = nlohmann::json::parse(std::get<std::string>(request));

    return {valid, result.get<std::array<double, JOINT_COUNT>>()};
  };

  bool Robot::robot_servo_on(uint8_t max_retries) {

    auto mode_request = this->get_robot_mode();

    std::cout << "Mode request suc: " << (std::get<0>(mode_request) ? "True" : "False") << '\n';

    if (!std::get<0>(mode_request)) {
      return false;
    }

    auto mode = std::get<1>(mode_request);
    std::cout << "Mode request: " << std::format("{}\n", mode);

    if (std::get<1>(mode_request) != 2) {
      std::cout << "Should be in remote mode!\n";
      return false;
    }

    bool alarm_ok = false;
    uint8_t alarm_retries = 0;

    while (!alarm_ok && (alarm_retries < max_retries)) {
      alarm_retries++;
      auto clear_alarm = this->clear_alarm();

      if (!std::get<0>(clear_alarm)) {
        return false;
      }

      alarm_ok = std::get<1>(clear_alarm);
    }

    auto state_request = this->get_robot_state();

    if (!std::get<0>(state_request)) {
      return false;
    }

    if (std::get<1>(state_request) != 0) {
      return false;
    }

    bool motor_sync = false;
    auto motor_retries = 0;

    while (!motor_sync && (motor_retries < max_retries)) {
      motor_retries += 1;
      auto motor_sync_request = this->sync_motor_status();

      motor_sync = std::get<1>(motor_sync_request);

      if (!motor_sync) {
        boost::asio::io_context io_ctx;
        boost::asio::steady_timer timer(io_ctx, std::chrono::seconds(5));
        timer.wait(); // blocks for 5 seconds
      }
    }

    if (!motor_sync)
      return false;

    auto motor_status_request = this->get_motor_status();

    if (!std::get<0>(motor_status_request) || !std::get<1>(motor_status_request)) {
      return false;
    }

    uint8_t servo_on_retries = 0;
    bool servo_status = false;

    while (!servo_status && servo_on_retries < max_retries) {
      servo_on_retries++;
      auto servo_status_request = this->set_servo_status(1);

      if (!std::get<0>(servo_status_request)) {
        return false;
      }

      servo_status = std::get<1>(servo_status_request);
    }

    return true;
  };

} // namespace elite