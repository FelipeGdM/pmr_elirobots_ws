// #include <asio.hpp>
#include <cstdint>
#include <cstdlib>

#include <nlohmann/json.hpp>

#include <jsonrpccxx/client.hpp>
#include <jsonrpccxx/server.hpp>

#include "cpphttplibconnector.hpp"

int main() {
  const std::string host = "127.0.0.1";
  const uint16_t port = 8080;

  CppHttpLibClientConnector httpClient(host, port);

  jsonrpccxx::JsonRpcClient client(httpClient, jsonrpccxx::version::v2);

  try {
    client.CallMethod<uint16_t>(1, "my_method", {});
  } catch (jsonrpccxx::JsonRpcException &e) {
    std::cerr << "Error finding product: " << e.what() << "\n";
  }
  // jsonrpccxx::JsonRpcClient client(clientConnector, jsonrpccxx::version::v2);
  return 0;
}