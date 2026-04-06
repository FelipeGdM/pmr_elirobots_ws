#pragma once
#include "boost_aio_socket_connector.hpp"
#include "cpphttplibconnector.hpp"
#include <chrono>
#include <jsonrpccxx/client.hpp>

#include <cstdint>
#include <string>
#include <tuple>

namespace elite {

  struct MovementConfig {
    double speed = 100.0;
    int8_t cond_type = -1;
    int8_t cond_num = -1;
    int8_t cond_value = -1;
    std::string cond_judgment = "";
    int16_t acc = -1;
    int16_t dec = -1;
  };

  const uint8_t JOINT_COUNT = 6;
  class Robot {
  public:
    Robot(const std::string addr, uint16_t port)
        : addr(std::move(addr)), port(port), connector{this->addr, this->port},
          client(jsonrpccxx::JsonRpcClient(connector, jsonrpccxx::version::v2)) {}

    template <typename T>
    std::tuple<bool, T> call_method(const jsonrpccxx::id_type &id, const std::string &name,
                                    const jsonrpccxx::positional_parameter &params = {});

    template <typename T>
    std::tuple<bool, T> call_method_named(const jsonrpccxx::id_type &id, const std::string &name,
                                          const jsonrpccxx::named_parameter &params);

    /**
     *  Interface service
     */

    /**
     * 1. Servo service
     */

    /**
     * @brief 1.1. Get the servo status of the robotic arm
     *
     * @return std::tuple<bool, bool>
     */
    std::tuple<bool, bool> get_servo_status();

    /**
     * @brief 1.2. Set servo enable state
     *
     * @param status servo switch, range: int[0,1], 1 is on, 0 is off
     * @return std::tuple<bool, bool>
     */
    std::tuple<bool, bool> set_servo_status(uint8_t status);

    /**
     * @brief 1.3. Synchronize servo encoder data
     *
     * @return std::tuple<bool, bool>
     */
    std::tuple<bool, bool> sync_motor_status();

    /**
     * @brief 1.4. Clear alarm
     *
     * @return std::tuple<bool, bool>
     */
    std::tuple<bool, bool> clear_alarm();

    /**
     * @brief 1.5. Get synchronization status
     *
     * @return std::tuple<bool, bool>
     */
    std::tuple<bool, bool> get_motor_status();

    /**
     * 2. Parameter service
     */

    /**
     * @brief 2.1. Get robot status
     *
     * @return Stop state 0, pause state 1, emergency stop state 2,
     * running state 3, alarm state 4, collision state 5
     */
    std::tuple<bool, uint8_t> get_robot_state();

    /**
     * @brief 2.2. Get Robot Mode
     *
     * @return Teaching mode 0, operating mode 1, remote mode 2
     */
    std::tuple<bool, uint16_t> get_robot_mode();

    /**
     * @brief 2.3. Obtain the joint position information of the robot
     * output terminal
     *
     * @return std::tuple<bool, uint8_t>
     */
    std::tuple<bool, std::array<double, JOINT_COUNT>> get_joint_pos();

    /**
     * X.Y.Z. Helper functions
     */

    /**
     * @brief Performs basic operations to start the robot, such as sync motors and clear alarms
     *
     * This function should be called right after stabilishing connection
     *
     * @param max_retries How many retries for each operation
     */
    bool robot_servo_on(uint8_t max_retries = 3);

    /**
     * @brief Stops the program until the robot goes to stop state
     *
     * @param pool_period
     * @param timeout
     */
    bool wait_robot_stop(std::chrono::milliseconds pool_period = std::chrono::milliseconds(200),
                         std::chrono::seconds timeout = std::chrono::seconds(60));

    bool is_alive();

    /**
     * 3. Movement service
     */

    /**
     * @brief 3.1. Joint Movement
     *
     * This function controls robot motion to a target defined either by joint angles
     * (targetPos) or by Cartesian pose (target_pose). Additional parameters allow
     * specifying a reference position, motion speed, conditional I/O checks, and
     * acceleration/deceleration profiles.
     *
     * @param target A required parameter: choose one of the two forms.
     *   - `targetPos`: Joint angles as a `double[6]` array, unit: degrees, range: [-360, 360].
     *   - `target_pose`: Cartesian pose as a `double[6]` array:
     *        first three elements are position (x, y, z) in mm, range: [-1e9, 1e9];
     *        last three elements are orientation (Rx, Ry, Rz) in radians, range: [-π, π].
     * @param ref_pos Optional joint angles of the inverse reference point, `double[6]`, unit:
     * degrees, range: [-360, 360].
     *                If not specified, the current joint position is used as the reference point.
     * @param speed Operating speed, `double`, range: [0.01, 100].
     * @param cond_type Optional condition type, `int`, range: [0,2].
     *                  - 0: digital input X
     *                  - 1: digital output Y
     *                  - 2: user-defined input
     * @param cond_num Optional IO address, `int`, range: [0,63].
     * @param cond_value Optional IO status value, `int`, range: [0,1].
     *                   When the actual IO status matches this value, any unfinished movement is
     *                   immediately abandoned and the next instruction is executed.
     * @param cond_judgment Optional conditional judgment, `std::string` (or equivalent).
     *                      A user-defined IF statement. When the condition is met, any unfinished
     *                      movement is immediately abandoned and the next instruction is executed.
     * @param acc Optional acceleration percentage, `int`, range: [1,100]. Default is 80 if not
     * specified.
     *
     * @return std::tuple<bool, bool>
     */
    std::tuple<bool, bool> move_by_joint(const std::array<float, JOINT_COUNT> &target_pos,
                                         MovementConfig config);

    /**
     * @brief 3.2. Linear Motion
     *
     * @return std::tuple<bool, bool>
     */
    std::tuple<bool, bool> move_by_line();

    /**
     * @brief 3.9. Stop robot operation
     *
     * @return True for success, false for failure
     */
    std::tuple<bool, bool> stop();

    /**
     * @brief 3.10. Robot runs automatically
     *
     * @return True for success, false for failure
     */
    std::tuple<bool, bool> run();

    /**
     * @brief 3.11. Robot stop
     *
     * @return True for success, false for failure
     */
    std::tuple<bool, bool> pause();

  private:
    std::string addr;
    uint16_t port;
    BoostAioClientConnector connector;
    jsonrpccxx::JsonRpcClient client;
  };
} // namespace elite
