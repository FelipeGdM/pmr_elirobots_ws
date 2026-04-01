#include "control_msgs/action/follow_joint_trajectory.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

#include "rclcpp/logging.hpp"
#include "rclcpp/rclcpp.hpp"

#include <control_msgs/action/follow_joint_trajectory.hpp>
#include <cstdint>
#include <memory>
#include <optional>
#include <rclcpp/qos.hpp>
#include <rclcpp/utilities.hpp>
#include <string>

#include "eliterobots/robot.hpp"
#include "std_msgs/msg/bool.hpp"

using namespace std::chrono_literals;

using FollowJointTrajectory = control_msgs::action::FollowJointTrajectory;
using GoalHandle = rclcpp_action::ServerGoalHandle<FollowJointTrajectory>;

const std::string DEFAULT_ROBOT_IP = "192.168.1.202";
const uint16_t DEFAULT_ROBOT_PORT = 8055;

class EcDriver : public rclcpp::Node {
public:
  EcDriver(uint16_t loop_rate_hz) : Node("minimal_publisher"), loop_rate_hz(loop_rate_hz) {

    auto robot_addr = this->get_parameter_or<std::string>("robot_addr", DEFAULT_ROBOT_IP);
    auto port = this->get_parameter_or<uint16_t>("port", DEFAULT_ROBOT_PORT);

    robot.emplace(robot_addr, port);

    publisher_ = this->create_publisher<std_msgs::msg::Bool>("alive", rclcpp::SystemDefaultsQoS());
    auto timer_callback = [this]() -> void {
      auto message = std_msgs::msg::Bool();
      message.data = true; // this->robot->is_alive();
      this->publisher_->publish(message);
    };

    timer_ = this->create_wall_timer(1000ms, timer_callback);

    auto handle_goal = [this](const rclcpp_action::GoalUUID &uuid,
                              std::shared_ptr<const FollowJointTrajectory::Goal> goal) {
      RCLCPP_INFO_STREAM(this->get_logger(), "Received goal request!");
      (void)uuid;
      (void)goal;
      return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
    };

    auto handle_cancel = [this](const std::shared_ptr<GoalHandle> goal_handle) {
      RCLCPP_INFO(this->get_logger(), "Received request to cancel goal");
      (void)goal_handle;
      return rclcpp_action::CancelResponse::ACCEPT;
    };

    auto handle_accepted = [this](const std::shared_ptr<GoalHandle> goal_handle) {
      // this needs to return quickly to avoid blocking the executor,
      // so we declare a lambda function to be called inside a new thread
      auto execute_in_thread = [this, goal_handle]() { return this->execute(goal_handle); };
      std::thread{execute_in_thread}.detach();
    };

    this->action_server_ = rclcpp_action::create_server<FollowJointTrajectory>(
        this, "robot_cmd", handle_goal, handle_cancel, handle_accepted);
  }

private:
  uint16_t loop_rate_hz;
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr publisher_;
  rclcpp_action::Server<FollowJointTrajectory>::SharedPtr action_server_;

  std::optional<elite::Robot> robot;

  void execute(const std::shared_ptr<GoalHandle> goal_handle) {
    auto goal = goal_handle->get_goal();
    const auto &points = goal->trajectory.points;

    // If there are no trajectory points, just succeed
    if (points.empty()) {
      auto result = std::make_shared<FollowJointTrajectory::Result>();
      result->error_code = FollowJointTrajectory::Result::SUCCESSFUL;
      goal_handle->succeed(result);
      return;
    }

    // Simulate executing each trajectory point in order
    rclcpp::Rate rate(this->loop_rate_hz);
    // for (const auto &point : points) {
    size_t point_idx = 0;
    while (point_idx < points.size()) {

      const auto &point = points[point_idx];

      // Check for cancellation
      if (goal_handle->is_canceling()) {
        auto result = std::make_shared<FollowJointTrajectory::Result>();
        result->error_code = FollowJointTrajectory::Result::INVALID_GOAL;
        goal_handle->canceled(result);
        return;
      }

      // Check if robot is in stop state
      bool robot_stop = (std::get<uint8_t>(this->robot->get_robot_state()) == 0);

      if (!robot_stop) {

        auto feedback = std::make_shared<FollowJointTrajectory::Feedback>();

        // The actual positions (simulated) – here we just copy the desired positions

        const auto joint_pos = std::get<1>(this->robot->get_joint_pos());

        feedback->actual.positions.assign(std::begin(joint_pos), std::end(joint_pos));

        feedback->desired.positions = point.positions;

        // Also set the header stamp if needed (optional)
        feedback->header.stamp = this->now();

        goal_handle->publish_feedback(feedback);

        rate.sleep();

        continue;
      }

      std::array<float, 6> joint_cmd{};

      for (size_t i = 0; i < 6; i++) {
        joint_cmd.at(i) = (float)point.positions[i];
      }

      this->robot->move_by_joint(joint_cmd, {.speed = 10});

      point_idx++;

      // Robot is stopped and ready for new command
    }

    // All points executed – succeed
    auto result = std::make_shared<FollowJointTrajectory::Result>();
    result->error_code = FollowJointTrajectory::Result::SUCCESSFUL;
    goal_handle->succeed(result);
    RCLCPP_INFO(this->get_logger(), "Goal succeeded");
  }
};

int main(int argc, char *argv[]) {

  std::cout << "les goo\n";
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<EcDriver>(1));
  rclcpp::shutdown();
  return 0;
}

// RCLCPP_INFO_STREAM(this->get_logger(), "Published msg");
