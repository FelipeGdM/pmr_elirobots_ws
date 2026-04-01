#include "rclcpp/rclcpp.hpp"

#include <memory>
#include <optional>
#include <rclcpp/qos.hpp>
#include <string>

#include "eliterobots/robot.hpp"
#include "std_msgs/msg/bool.hpp"

using namespace std::chrono_literals;

const std::string DEFAULT_ROBOT_IP = "192.168.1.202";
const uint16_t DEFAULT_ROBOT_PORT = 8055;

class EcDriver : public rclcpp::Node {
public:
  EcDriver() : Node("minimal_publisher") {

    auto robot_addr = this->get_parameter_or<std::string>("robot_addr", DEFAULT_ROBOT_IP);
    auto port = this->get_parameter_or<uint16_t>("port", DEFAULT_ROBOT_PORT);

    robot.emplace(robot_addr, port);

    publisher_ = this->create_publisher<std_msgs::msg::Bool>("alive", rclcpp::SystemDefaultsQoS());
    auto timer_callback = [this]() -> void {
      auto message = std_msgs::msg::Bool();
      message.data = this->robot->is_alive();
      this->publisher_->publish(message);
    };
    timer_ = this->create_wall_timer(1000ms, timer_callback);
  }

private:
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr publisher_;

  std::optional<elite::Robot> robot;
};

int main(int argc, char *argv[]) {

  std::cout << "les goo\n";
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<EcDriver>());
  rclcpp::shutdown();
  return 0;
}

// RCLCPP_INFO_STREAM(this->get_logger(), "Published msg");
