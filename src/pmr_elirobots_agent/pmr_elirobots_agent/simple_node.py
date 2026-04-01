# pyright: basic
from typing import final
import rclpy
from rclpy.executors import ExternalShutdownException
from rclpy.node import Node
from rclpy.action import ActionClient

from std_msgs.msg import String
from control_msgs.action import FollowJointTrajectory
from trajectory_msgs.msg import JointTrajectoryPoint

from .agent import create_agent, ACTION_SPACE_SIZE

CHECKPOINT = "/home/ws/pth/ckpt_2406400.pt"

@final
class JointActionClient(Node):

    def __init__(self):
        super().__init__('robot_action_client')
        self._action_client = ActionClient(self, FollowJointTrajectory, 'robot_cmd')
        self._send_goal_future = None

    def send_goal(self, joint_pos_list:list[list[float]]):
        goal_msg = FollowJointTrajectory.Goal()

        goal_msg.trajectory.points = [JointTrajectoryPoint(positions=p) for p in joint_pos_list]

        _ = self._action_client.wait_for_server()

        self._send_goal_future = self._action_client.send_goal_async(goal_msg)

        self._send_goal_future.add_done_callback(self.goal_response_callback)

    def goal_response_callback(self, future: rclpy.Future): 
        goal_handle = future.result()
        
        if goal_handle is None:
            return
        
        if not goal_handle.accepted:
            self.get_logger().info('Goal rejected :(')
            return

        self.get_logger().info('Goal accepted :)')

        self._get_result_future = goal_handle.get_result_async()
        self._get_result_future.add_done_callback(self.get_result_callback)

    def get_result_callback(self, future):
        result = future.result().result
        self.get_logger().info('Result: {0}'.format(result))
        rclpy.shutdown()

def main(args=None):
    try:
        with rclpy.init(args=args):
            action_client = JointActionClient()

            action_client.send_goal([
                [180.0, 0.0, 0.0, 0.0, 0.0, 90.0],
                [180.0, -90.0, 0.0, 0.0, 0.0, 90.0],
                [180.0, 0.0, 0.0, 0.0, 0.0, 90.0],
                [180.0, -90.0, 0.0, 0.0, 0.0, 90.0],
            ])

            rclpy.spin(action_client)
    except (KeyboardInterrupt, ExternalShutdownException):
        pass


if __name__ == '__main__':
    main()
    