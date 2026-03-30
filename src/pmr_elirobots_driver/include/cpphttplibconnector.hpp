
#include <cpp-httplib/httplib.h>
#include <jsonrpccxx/iclientconnector.hpp>
#include <jsonrpccxx/server.hpp>
#include <string>

class CppHttpLibClientConnector : public jsonrpccxx::IClientConnector {
public:
  explicit CppHttpLibClientConnector(const std::string &host, int port) : httpClient(host.c_str(), port) {}
  std::string Send(const std::string &request) override {
    auto res = httpClient.Post("/jsonrpc", request, "application/json");
    if (!res || res->status != 200) {
      throw jsonrpccxx::JsonRpcException(-32003, "client connector error, received status != 200");
    }
    return res->body;
  }

private:
  httplib::Client httpClient;
};
