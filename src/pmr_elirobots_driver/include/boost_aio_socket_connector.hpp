
#include <boost/asio/io_context.hpp>
#include <cpp-httplib/httplib.h>
#include <jsonrpccxx/iclientconnector.hpp>
#include <jsonrpccxx/server.hpp>
#include <string>

#include <boost/asio.hpp>

const auto timeout = std::chrono::seconds(5);

class BoostAioClientConnector : public jsonrpccxx::IClientConnector {

public:
  explicit BoostAioClientConnector(const std::string &host, int port) : resolver(io_ctx), socket(io_ctx), host(host), port(std::to_string(port)) {
    auto endpoints = this->resolver.resolve(host, std::to_string(port));
    boost::asio::connect(socket, endpoints);
  }

  std::string Send(const std::string &request_raw) override {

    auto request = request_raw + "\n";

    std::cout << "Sending to: " << this->host << ":" << this->port <<"\n";
    
    std::cout << "Message: " << request;

    boost::asio::write(this->socket, boost::asio::buffer(request));

    std::cout << "Finish write!" << "\n";

    // Read response
    boost::asio::streambuf response;
    boost::system::error_code ec;

    boost::asio::read_until(socket, response, '\n', ec);
    std::cout << "Finish read!" << "\n";

    if (ec && ec != boost::asio::error::eof){
      std::cout << "Failed!\n";
      // throw boost::system::system_error(ec);
      return "";
    }

    std::string retval{(std::istreambuf_iterator<char>(&response)), std::istreambuf_iterator<char>()};

    std::cout << retval;

    return retval;
  }

private:
  boost::asio::io_context io_ctx;
  boost::asio::ip::tcp::resolver resolver;
  boost::asio::ip::tcp::socket socket;

  std::string host;
  std::string port;
};
