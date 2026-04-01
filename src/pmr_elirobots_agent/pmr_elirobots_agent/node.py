# pyright: basic
from typing import final
import rclpy
from rclpy.executors import ExternalShutdownException
from rclpy.node import Node
from rclpy.action import ActionClient

from sensor_msgs.msg import JointState
from control_msgs.action import FollowJointTrajectory
from trajectory_msgs.msg import JointTrajectoryPoint
import numpy as np

from .agent import create_agent, ACTION_SPACE_SIZE

QPOS_SIZE = 8

CHECKPOINT = "/home/ws/pth/ckpt_2406400.pt"

@final
class JointActionClient(Node):

    def __init__(self):
        super().__init__('robot_action_client')
        self._action_client = ActionClient(self, FollowJointTrajectory, 'robot_cmd')
        self._send_goal_future = None
        self._agent = create_agent(CHECKPOINT)
        
        self.subscription = self.create_subscription(
            JointState,
            'joint_state',
            self.listener_callback,
            10)
        self.subscription  # prevent unused variable warning
        
        self.qpos = np.zeros((QPOS_SIZE,))
        self.busy = False

    def send_goal(self, joint_pos_list:list[list[float]]):
        self.busy = True
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
        self.busy = False
        
        # rclpy.shutdown()

    def listener_callback(self, msg: JointState):
        self.get_logger().info(f"Received {msg.position}")
        
        qpos = np.array([p for p in msg.position] + [0.0, 0.0]).astype(np.float64)
        action = self._agent.get_action_from_qpos(qpos*np.pi/180).numpy()  # pyright: ignore[reportArgumentType]
        
        qcmd = qpos[0:6] + action[0:6]*180/np.pi
        
        if not self.busy:
            self.send_goal([qcmd.tolist()])
        

def main(args=None):
    try:
        with rclpy.init(args=args):
            action_client = JointActionClient()

            # action_client.send_goal([
            #     [180.0, 0.0, 0.0, 0.0, 0.0, 90.0],
            #     [180.0, -90.0, 0.0, 0.0, 0.0, 90.0],
            #     [180.0, 0.0, 0.0, 0.0, 0.0, 90.0],
            #     [180.0, -90.0, 0.0, 0.0, 0.0, 90.0],
            # ])

            rclpy.spin(action_client)
    except (KeyboardInterrupt, ExternalShutdownException):
        pass


if __name__ == '__main__':
    main()
    