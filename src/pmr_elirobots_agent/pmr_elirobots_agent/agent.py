
from typing import cast, final
from collections import OrderedDict
from spatialmath import UnitQuaternion
import torch
from torch.distributions.normal import Normal
from roboticstoolbox import DHRobot, RevoluteDH
from torch import nn, Tensor
import numpy as np

SQRT_2 = cast("float", np.sqrt(2))

OBSERVATION_SPACE_SIZE = 39
ACTION_SPACE_SIZE = 7

def degrees_to_rad(deg: float):
    return deg * np.pi / 180


def rad_to_degree(rad: float):
    return rad * 180 / np.pi


def layer_init(layer: nn.Module, std: float = SQRT_2, bias_const: float = 0.0):
    _ = torch.nn.init.orthogonal_(layer.weight, std)
    _ = torch.nn.init.constant_(layer.bias, bias_const)
    return layer


def build_state(qpos: Tensor, qvel: Tensor, tcp_pose: Tensor, goal_pos: Tensor, obj_pose: Tensor):
    
    print(f"{qpos.shape=}")
    print(f"{qvel.shape=}")
    print(f"{tcp_pose.shape=}")
    return torch.cat((qpos, qvel, tcp_pose, goal_pos, obj_pose), -1)

def create_ec63_robot()->DHRobot:
    full_spin = degrees_to_rad(360) * np.array([-1, 1])
    partial_spin = degrees_to_rad(158) * np.array([-1, 1])
    links = [
        RevoluteDH(d=0.1400, a=0.000, alpha=-np.pi / 2, qlim=full_spin),
        RevoluteDH(d=0.0000, a=0.270, alpha=0.0, qlim=full_spin),
        RevoluteDH(d=0.0000, a=0.256, alpha=0.0, qlim=partial_spin),
        RevoluteDH(d=0.1035, a=0.000, alpha=-np.pi / 2, qlim=full_spin),
        RevoluteDH(d=0.0980, a=0.000, alpha=-np.pi / 2, qlim=full_spin),
        RevoluteDH(d=0.0890, a=0.000, alpha=0.0, qlim=full_spin),
    ]

    return DHRobot(links)

@final
class Agent(nn.Module):
    def __init__(self, observation_space_size: int, action_space_size: int, device: torch.device):
        super().__init__()
        self.observation_space_size = observation_space_size
        self.action_space_size = action_space_size
        self.device = device
        self.ec_robot = create_ec63_robot()
        self.critic = nn.Sequential(
            layer_init(nn.Linear(observation_space_size, 256)),
            nn.Tanh(),
            layer_init(nn.Linear(256, 256)),
            nn.Tanh(),
            layer_init(nn.Linear(256, 256)),
            nn.Tanh(),
            layer_init(nn.Linear(256, 1)),
        )
        self.actor_mean = nn.Sequential(
            layer_init(nn.Linear(observation_space_size, 256)),
            nn.Tanh(),
            layer_init(nn.Linear(256, 256)),
            nn.Tanh(),
            layer_init(nn.Linear(256, 256)),
            nn.Tanh(),
            layer_init(nn.Linear(256, action_space_size), std=0.01 * SQRT_2),
        )
        self.actor_logstd = nn.Parameter(torch.ones(1, action_space_size) * -0.5)

    def get_value(self, x: Tensor) -> Tensor:
        return cast("Tensor", self.critic(x))

    def get_action(self, x: Tensor, deterministic: bool = False):  # noqa: FBT001, FBT002
        action_mean = cast("Tensor", self.actor_mean(x))
        if deterministic:
            return action_mean
        action_logstd = self.actor_logstd.expand_as(action_mean)
        action_std = torch.exp(action_logstd)
        probs = Normal(action_mean, action_std)
        return probs.sample()

    def get_action_and_value(self, x: Tensor, action: Tensor | None = None):
        action_mean = cast("Tensor", self.actor_mean(x))
        action_logstd = self.actor_logstd.expand_as(action_mean)
        action_std = torch.exp(action_logstd)
        probs = Normal(action_mean, action_std)
        if action is None:
            action = probs.sample()
        return action, probs.log_prob(action).sum(1), probs.entropy().sum(1), self.critic(x)
    
    def get_action_from_qpos(self, qpos_: np.ndarray)->Tensor:
        
        qpos = torch.Tensor(qpos_).reshape((1,8))

        qvel = torch.zeros((1, 8))

        tcp_pose = self.compute_tcp_pose(qpos_[:6])

        goal_pos = Tensor([[0.1792, 0.0844, 0.0010]])

        obj_pose = Tensor([[-0.0208, 0.0844, 0.0200, 1.0000, 0.0000, 0.0000, 0.0000]])
        
        state = build_state(qpos, qvel, tcp_pose, goal_pos, obj_pose).to(self.device)
        
        return self.get_action(state, deterministic=True)

    def compute_tcp_pose(self, qpos: Tensor)->Tensor:
        SE_matrix = self.ec_robot.fkine(qpos)  # pyright: ignore[reportUnknownMemberType, reportCallIssue, reportAttributeAccessIssue]
        
        quat = UnitQuaternion(SE_matrix)
        translation = SE_matrix.A[:3,3]

        return Tensor([*translation.tolist(), *quat.data])

def create_agent(checkpoint: str) -> Agent:
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    model = cast("OrderedDict[str, Tensor]", torch.load(checkpoint, map_location=device))
    agent = Agent(OBSERVATION_SPACE_SIZE, ACTION_SPACE_SIZE, device).to(device)
    
    _ = agent.load_state_dict(model)
    
    return agent
