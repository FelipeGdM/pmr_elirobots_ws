// #include <asio.hpp>
#include <cstdint>
#include <cstdlib>

#include <nlohmann/json.hpp>

#include <jsonrpccxx/client.hpp>

#include "eliterobots/robot.hpp"

int main() {
  const std::string addr = "127.0.0.1";
  const uint16_t port = 8080;

  elite::Robot client(addr, port);

  auto retval = client.getServoStatus();

  auto suc = std::get<0>(retval);
  auto value = std::get<1>(retval);

  client.setServoStatus(1);

  auto ret = client.getJointPos();

  auto val = std::get<1>(ret);

  std::cout << "Return: " << std::endl;

  for (auto num : val) {
    std::cout << num << ", ";
  }
  std::cout << std::endl;

  // jsonrpccxx::JsonRpcClient client(clientConnector, jsonrpccxx::version::v2);
  return 0;
}