import pytest
import torch
from pmr_elirobots_agent.agent import (
    Agent,
    compute_tcp_pose,
    create_agent,
    create_ec63_robot,
)
from roboticstoolbox.robot.DHRobot import DHRobot

CHECKPOINT = "/home/ws/pth/ckpt_8192000_14.pt"


@pytest.fixture
def robot() -> DHRobot:
    return create_ec63_robot()


@pytest.fixture
def agent() -> Agent:
    return create_agent(CHECKPOINT)


# @pytest.mark.skip
def test_tcp(robot: DHRobot):

    qpos = torch.Tensor(
        [
            0.0000,
            -2.3562,
            1.9635,
            -1.1781,
            1.5708,
            0.0000,
        ]
    )

    expected_tcp_pose = torch.Tensor(
        [
            -2.5641e-01 + 0.4,  # 0.1436
            1.0300e-01,
            2.3089e-01,
            -2.6125e-06,
            -7.0711e-01,
            7.0711e-01,
            -7.7824e-08,
        ]
    )

    tcp_pose = compute_tcp_pose(robot, qpos).reshape((7,))

    diff = expected_tcp_pose[:3] - tcp_pose[:3]

    assert torch.linalg.vector_norm(diff) < 1e-3  # pyright: ignore[reportUnknownMemberType]


# def test_agent_action(agent: Agent):

#     qpos = torch.zeros((1, 6))
