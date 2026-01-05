#pragma once
#include "cpphttplibconnector.hpp"
#include <jsonrpccxx/client.hpp>

#include <cstdint>
#include <string>
#include <tuple>

namespace elite {
  class Robot {
  public:
    Robot(const std::string &addr, uint16_t port);

    template <typename T>
    std::tuple<bool, T> CallMethod(const jsonrpccxx::id_type &id, const std::string &name,
                                   const jsonrpccxx::positional_parameter &params);

    template <typename T>
    std::tuple<bool, T> CallMethod(const jsonrpccxx::id_type &id, const std::string &name);

    template <typename T>
    std::tuple<bool, T> CallMethodNamed(const jsonrpccxx::id_type &id, const std::string &name,
                                        const jsonrpccxx::named_parameter &params);

    /**
     * 2.2. Interface service
     */

    /**
     * 2.2.1. Servo service
     */

    /**
     * @brief 2.2.1.1. Get the servo status of the robotic arm
     *
     * @return std::tuple<bool, bool>
     */
    std::tuple<bool, bool> getServoStatus();

    /**
     * @brief 2.2.1.2. Set servo enable state
     *
     * @param status servo switch, range: int[0,1], 1 is on, 0 is off
     * @return std::tuple<bool, bool>
     */
    std::tuple<bool, bool> setServoStatus(uint8_t status);

    /**
     * @brief 2.2.1.3. Synchronize servo encoder data
     *
     * @return std::tuple<bool, bool>
     */
    std::tuple<bool, bool> syncMotorStatus();

    /**
     * @brief 2.2.1.4. Clear alarm
     *
     * @return std::tuple<bool, bool>
     */
    std::tuple<bool, bool> clearAlarm();

    /**
     * @brief 2.2.1.5. Get synchronization status
     *
     * @return std::tuple<bool, bool>
     */
    std::tuple<bool, bool> getMotorStatus();

    /**
     * 2.2.2. Parameter service
     */

    /**
     * @brief 2.2.2.1. Get robot status
     *
     * @return Stop state 0, pause state 1, emergency stop state 2,
     * running state 3, alarm state 4, collision state 5
     */
    std::tuple<bool, uint8_t> getRobotState();

    /**
     * @brief 2.2.2.2. Get Robot Mode
     *
     * @return Teaching mode 0, operating mode 1, remote mode 2
     */
    std::tuple<bool, uint8_t> getRobotMode();

    /**
     * @brief 2.2.2.3. Obtain the joint position information of the robot
     * output terminal
     *
     * @return std::tuple<bool, uint8_t>
     */
    std::tuple<bool, std::array<double, 6>> getJointPos();

  private:
    CppHttpLibClientConnector httpClient;
    jsonrpccxx::JsonRpcClient client;
    std::string addr;
    uint16_t port;
  };
} // namespace elite
