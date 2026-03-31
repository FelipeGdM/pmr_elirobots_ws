// #include <asio.hpp>
#include <cstdint>
#include <cstdlib>

#include <ostream>
#include <string>

#include <nlohmann/json.hpp>

#include <jsonrpccxx/client.hpp>

#include "eliterobots/robot.hpp"

int main() {

  std::cout << "Les goo\n";

  const std::string addr = "192.168.1.202";
  const uint16_t port = 8055;

  elite::Robot client(addr, port);

  auto retval1 = client.robot_servo_on();
  std::cout << "Servo on: " << (retval1 ? "True" : "False") << "\n";

  // auto retval = client.get_servo_status();

  // auto suc = std::get<0>(retval);
  // auto value = std::get<1>(retval);

  // std::cout << "Success: " << suc << " Value: " << value << "\n";

  // client.set_servo_status(1);

  // auto ret = client.get_joint_pos();

  // auto val = std::get<1>(ret);

  // std::cout << "Return: " << "\n";

  // for (auto num : val) {
  //   std::cout << num << ", ";
  // }
  // std::cout << "\n";

  // jsonrpccxx::JsonRpcClient client(clientConnector, jsonrpccxx::version::v2);
  return 0;
}