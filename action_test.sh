#!/bin/bash

set -x

ros2 action send_goal --feedback /robot_cmd control_msgs/action/FollowJointTrajectory \
"
trajectory:
  header:
    stamp:
      sec: 0
      nanosec: 0
    frame_id: ''
  joint_names: []
  points:
    - positions: [180.0, 0.0, 0.0, 0.0, 0.0, 90.0]
    - positions: [180.0, -90.0, 0.0, 0.0, 0.0, 90.0]
    - positions: [180.0, 0.0, 0.0, 0.0, 0.0, 90.0]
    - positions: [180.0, -90.0, 0.0, 0.0, 0.0, 90.0]
multi_dof_trajectory:
  header:
    stamp:
      sec: 0
      nanosec: 0
    frame_id: ''
  joint_names: []
  points: []
path_tolerance: []
component_path_tolerance: []
goal_tolerance: []
component_goal_tolerance: []
goal_time_tolerance:
  sec: 0
  nanosec: 0"