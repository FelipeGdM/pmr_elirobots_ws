#include "eliterobots/robot.hpp"
#include <cstddef>
#include <tuple>

namespace elite {

  Robot::Robot(const std::string &addr, uint16_t port)
      : addr(addr), port(port), httpClient(CppHttpLibClientConnector(addr, port)),
        client(jsonrpccxx::JsonRpcClient(httpClient, jsonrpccxx::version::v2)) {}

  template <typename T>
  std::tuple<bool, T> Robot::CallMethod(const jsonrpccxx::id_type &id, const std::string &name,
                                        const jsonrpccxx::positional_parameter &params) {
    try {
      auto retval = this->client.CallMethod<T>(id, name, params);
      return std::make_tuple(true, retval);
    } catch (jsonrpccxx::JsonRpcException &e) {
      return std::make_tuple(false, T{});
    }
  }

  template <typename T>
  std::tuple<bool, T> Robot::CallMethod(const jsonrpccxx::id_type &id, const std::string &name) {
    return this->CallMethod<T>(id, name, {});
  }

  template <typename T>
  std::tuple<bool, T> Robot::CallMethodNamed(const jsonrpccxx::id_type &id, const std::string &name,
                                             const jsonrpccxx::named_parameter &params) {
    try {
      auto retval = this->client.CallMethodNamed<T>(id, name, params);
      return std::make_tuple(true, retval);
    } catch (jsonrpccxx::JsonRpcException &e) {
      return std::make_tuple(false, T{});
    }
  };

  std::tuple<bool, bool> Robot::getServoStatus() {
    return this->CallMethod<bool>(1, "getServoStatus");
  }

  std::tuple<bool, bool> Robot::setServoStatus(uint8_t status) {
    nlohmann::json params;
    params["status"] = status;
    return this->CallMethodNamed<bool>(1, "set_servo_status", params);
  }

  std::tuple<bool, bool> Robot::syncMotorStatus() {
    return this->CallMethod<bool>(1, "syncMotorStatus");
  }

  std::tuple<bool, bool> Robot::clearAlarm() { return this->CallMethod<bool>(1, "clearAlarm"); }

  std::tuple<bool, bool> Robot::getMotorStatus() {
    return this->CallMethod<bool>(1, "getMotorStatus");
  }

  std::tuple<bool, uint8_t> Robot::getRobotState() {
    return this->CallMethod<uint8_t>(1, "getRobotState");
  };

  std::tuple<bool, uint8_t> Robot::getRobotMode() {
    return this->CallMethod<uint8_t>(1, "getRobotMode");
  };

  std::tuple<bool, std::array<double, 6>> Robot::getJointPos() {
    return this->CallMethod<std::array<double, 6>>(1, "get_joint_pos");
  };

} // namespace elite