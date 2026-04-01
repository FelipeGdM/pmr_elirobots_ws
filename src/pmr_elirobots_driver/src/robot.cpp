#include "eliterobots/robot.hpp"
#include <boost/asio.hpp>
#include <cstdint>
#include <string>
#include <tuple>

namespace elite {

  template <typename T>
  std::tuple<bool, T> Robot::call_method(const jsonrpccxx::id_type &id, const std::string &name,
                                         const jsonrpccxx::positional_parameter &params) {
    try {
      std::cout << "call method\n";
      auto request = this->client.CallMethod<std::string>(id, name, params);
      auto result = nlohmann::json::parse(request);

      return {true, result.template get<T>()};
    } catch (jsonrpccxx::JsonRpcException &e) {
      std::cout << "Bad vibes\n";
      std::cout << e.Message() << "\n";
      return {false, T{}};
    }
  }

  template <typename T>
  std::tuple<bool, T> Robot::call_method_named(const jsonrpccxx::id_type &id,
                                               const std::string &name,
                                               const jsonrpccxx::named_parameter &params) {
    try {
      auto request = this->client.CallMethodNamed<std::string>(id, name, params);
      auto result = nlohmann::json::parse(request);

      return {true, result.template get<T>()};
    } catch (jsonrpccxx::JsonRpcException &e) {
      return {false, T{}};
    }
  };

  std::tuple<bool, bool> Robot::get_servo_status() {
    return this->call_method<bool>(1, "getServoStatus");
  }

  std::tuple<bool, bool> Robot::set_servo_status(uint8_t status) {
    nlohmann::json params;
    params["status"] = status;

    return this->call_method_named<bool>(1, "set_servo_status", params);
  }

  std::tuple<bool, bool> Robot::sync_motor_status() {
    return this->call_method<bool>(1, "syncMotorStatus");
  }

  std::tuple<bool, bool> Robot::clear_alarm() { return this->call_method<bool>(1, "clearAlarm"); }

  std::tuple<bool, bool> Robot::get_motor_status() {
    return this->call_method<bool>(1, "getMotorStatus");
  }

  std::tuple<bool, uint8_t> Robot::get_robot_state() {
    return this->call_method<uint8_t>(1, "getRobotState");
  };

  std::tuple<bool, uint16_t> Robot::get_robot_mode() {
    return this->call_method<uint16_t>(1, "getRobotMode");
  };

  std::tuple<bool, std::array<double, JOINT_COUNT>> Robot::get_joint_pos() {
    return this->call_method<std::array<double, JOINT_COUNT>>(1, "get_joint_pos");
  };

  std::tuple<bool, bool> Robot::move_by_joint(const std::array<float, JOINT_COUNT> &target_pos,
                                              MovementConfig config) {

    nlohmann::json params;
    params["targetPos"] = target_pos;

    if (config.speed != 0.0)
      params["speed"] = config.speed;

    if (config.cond_type != -1)
      params["cond_type"] = config.cond_type;

    if (config.cond_num != -1)
      params["cond_num"] = config.cond_num;

    if (config.cond_value != -1)
      params["cond_value"] = config.cond_value;

    if (config.acc != -1)
      params["acc"] = config.acc;

    if (config.dec != -1)
      params["dec"] = config.dec;

    return this->call_method_named<bool>(1, "moveByJoint", params);
  }

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

  bool Robot::is_alive() {
    if (!this->connector.isAlive())
      return false;

    auto request = this->get_robot_state();

    return std::get<0>(request);
  };

} // namespace elite