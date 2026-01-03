
#include <cstdlib>
#include <iostream>
#include <jsonrpccpp/client.h>
#include <jsonrpccpp/client/connectors/tcpsocketclient.h>

int main() {
  std::string host = "127.0.0.1";
  const uint16_t port = 6543;

  jsonrpc::TcpSocketClient client(host, port);
  jsonrpc::Client c(client);
}