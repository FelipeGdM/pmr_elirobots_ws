# AGENTS.md

ROS 2 (colcon) workspace for controlling an **EC Elite 6-DOF robot arm** with an
RL agent and OAK-D perception. C++23 driver + Python agent/perception.

## Build

```bash
git submodule update --init --recursive   # first checkout
make                                       # = colcon build --symlink-install --merge-install
# or: colcon build --symlink-install
make clean                                 # rm -rf build/ install/ log/
```

Source the overlay before running nodes: `source install/setup.bash`.

## Packages (`src/`)

- **pmr_elirobots_driver** — C++/ament_cmake. JSON-RPC 2.0 client for the robot
  controller over HTTP (default `192.168.1.202:8055`).
  - `src/robot.cpp` + `include/eliterobots/robot.hpp` → `elite::Robot` library
    (servo/motor control, `move_by_joint`, `get_joint_pos`, state helpers).
  - `src/driver.cpp` → `EcDriver` ROS node: `FollowJointTrajectory` action server
    on `/robot_cmd`; publishes `joint_state` (sensor_msgs) and `alive` (std_msgs)
    at 1 Hz. Params: `robot_addr`, `port`. Servo auto-enabled on startup.
  - `src/main.cpp` → standalone demo executable (no ROS).
  - `rsc/spec.json` → partial JSON-RPC method spec. `test/server.py` → mock server.
  - Vendored (git submodules): `vendor/json-rpc-cxx`, `vendor/json` (nlohmann),
    `vendor/cpp-httplib`. Build with `-Wall -Wextra -Wpedantic`, `CMAKE_BUILD_TYPE=Debug`.
- **pmr_elirobots_agent** — Python. PPO-style actor/critic (`agent.py`: torch,
  roboticstoolbox, spatialmath) + `JointActionClient` node that drives the driver's
  action server. Entry point `node = pmr_elirobots_agent.node:main`.
- **pmr_elirobots_perception** — Python. OAK-D pipeline via `depthai_ros_driver`
  (`config/camera.yaml` = RGBD 1280x800, depth aligned to CAM_A, spatial filter;
  `launch/camera.launch.yaml`). `node.py` is a WIP stub; no entry point yet.
- **src/vendor/depthai-ros** — git submodule.

## Conventions / gotchas

- Joint values are **degrees** on the wire (`moveByJoint`, `FollowJointTrajectory`
  points, `action_test.sh`); the agent internally uses radians. Angle conversion is
  still inconsistent — see `TODO.md` ("Convert angles to rad").
- Checkpoints are hardcoded to `/home/ws/pth/ckpt_*.pt` in `agent.py`/`node.py`,
  but devcontainers mount the workspace at `/home/rosbot/ws` — paths drift, fix per env.
- `torch`, `roboticstoolbox`, `spatialmath` are **not** declared in `package.xml`
  (pip-only). Perception uses ROS `rolling`.
- Tests: `ament_copyright/flake8/mypy/pep257/xmllint` + `pytest`. Driver linting
  is disabled in `CMakeLists.txt` (`BUILD_TESTING`); its C++ tests are not wired up.
- `.clangd` reads `build/compile_commands.json`; uses `-std=c++23`.
- Devcontainers: `.devcontainer/rolling` (primary, workspaces at `/home/rosbot/ws`,
  `ROS_DOMAIN_ID=42`), `lyrical`, `WSL`. The old `Linux` variant was removed.

## TODO (from repo)

Tests, refactor logs, convert angles to radians.
