// #include <asio.hpp>
#include <chrono>
#include <cstdint>
#include <cstdlib>

#include <ostream>
#include <string>

#include <nlohmann/json.hpp>

#include <jsonrpccxx/client.hpp>

#include "eliterobots/robot.hpp"

const std::array<float, 6> home = {95.8199, -105.599, 131.866, -115.199, 90.0012, -61.1451};
const std::array<float, 6> pos = {180, 0, 0, -0, 0, 90};
const std::array<float, 6> pos2 = {180, -45, 0, -0, 0, 90};
const std::array<float, 6> pos3 = {180, -90, 0, -0, 0, 90};

int main() {

  std::cout << "Les goo\n";

  const std::string addr = "192.168.1.202";
  const uint16_t port = 8055;

  elite::Robot client(addr, port);

  auto retval1 = client.robot_servo_on();
  std::cout << "Servo on: " << (retval1 ? "True" : "False") << "\n";

  // auto ret = client.get_joint_pos();

  // auto val = std::get<1>(ret);

  // std::cout << "Return: " << "\n";

  // for (auto num : val) {
  //   std::cout << num << ", ";
  // }
  // std::cout << "\n";

  elite::MovementConfig config = {
      .speed = 10,
  };

  client.move_by_joint(pos, config);

  client.wait_robot_stop();

  client.move_by_joint(pos2, config);

  client.wait_robot_stop();

  client.move_by_joint(pos3, config);

  client.wait_robot_stop();

  return 0;
}