#include "eliterobots/robot.hpp"
#include <cstddef>
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

  // template <typename T>
  // std::tuple<bool, T> Robot::call_method(const jsonrpccxx::id_type &id, const std::string &name)
  // {
  //   return this->call_method<T>(id, name, {});
  // }

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
    return this->call_method<std::array<double, JOINT_COUNT>>(1, "get_joint_pos");
  };

  bool Robot::robot_servo_on(uint8_t max_retries) {

    //         if self.mode != BaseEC.RobotMode.REMOTE:
    //             self.logger.error("Please set Robot Mode to remote")
    //             return False

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

    //         # Loop to clear alarm, excluding abnormal conditions
    //         clear_alarm_tries = 0
    //         while clear_alarm_tries < max_retries and self.state != BaseEC.RobotState.STOP:
    //             clear_alarm_tries += 1
    //             self.clear_alarm()
    //             time.sleep(0.2)

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

    //         if self.state != BaseEC.RobotState.STOP:
    //             self.logger.error("Alarm cannot be cleared, please check robot state")
    //             return False

    auto state_request = this->get_robot_state();

    if (!std::get<0>(state_request)) {
      return false;
    }

    if (std::get<1>(state_request) != 0) {
      return false;
    }

    //         self.logger.debug("Alarm cleared successfully")
    //         time.sleep(0.2)

    //         motor_status_tries = 0
    //         while motor_status_tries < max_retries and not self.sync_status:
    //             motor_status_tries += 1
    //             self.sync()
    //             time.sleep(2)

    bool motor_sync = false;
    auto motor_retries = 0;

    while (!motor_sync && (motor_retries < max_retries)) {
      auto motor_sync_request = this->sync_motor_status();

      if (!std::get<0>(motor_sync_request)) {
        return false;
      }

      motor_sync = std::get<1>(motor_sync_request);
    }

    auto motor_status_request = this->get_motor_status();
    //         if not self.sync_status:
    //             self.logger.error("MotorStatus sync failed")
    //             return False

    if (!std::get<0>(motor_status_request) || !std::get<1>(motor_status_request)) {
      return false;
    }

    //         self.logger.debug("MotorStatus synchronized successfully")
    //         time.sleep(0.2)

    //         # Loop to servo on
    //         servo_on_tries = 0
    //         while servo_on_tries < max_retries and not self.servo_status:
    //             servo_on_tries += 1
    //             self.set_servo_status()
    //             time.sleep(0.02)

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

    //         if not self.servo_status:
    //             self.logger.error("Servo status set failed")
    //             return False

    //         self.logger.debug("Servo status set successfully")
    //         return True

    return true;
  };

} // namespace elite