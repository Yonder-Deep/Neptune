

import pybullet as p
import pybullet_data

import numpy as np  
LIDAR_LINK_INDEX = 1
CAM_LINK_INDEX = 0

MAX_LIDAR_RANGE = 500 #hell if I know what units these are in

class Neptune:
    """
    The simulated neptune USV
    """
    simulation = None
    USV = -1
    Camera = None
    projection_matrix = p.computeProjectionMatrixFOV(
            fov=60,
            aspect=1.0,
            nearVal=0.01,
            farVal=10,
        ) # type: ignore
    def __init__(self, simulation, start_pos : list[float] = [0.0,0.0,0.0] ):   
        """
        generates a USV in the world
        :param self: 
        :param simulation: the physics sim
        :param start_pos: the start pos
        """
        self.simulation = simulation 
        self.USV = p.loadURDF("data/USV/USV.urdf", start_pos, p.getQuaternionFromEuler([0, 0, 0]), useFixedBase=False, )
        #p.setLinearFactor(self.USV, [1, 0, 1]) 
    
    def step(self, step : int):
       pass
    def lidar(self) -> float:
        link_state = p.getLinkState(self.USV, LIDAR_LINK_INDEX)
        
    def take_photo(self):
        link_state = p.getLinkState(self.USV, CAM_LINK_INDEX)
        pos = link_state[4]             # world position
        orn = link_state[5]             # world orientation
        rot_matrix = p.getMatrixFromQuaternion(orn)
        rot_matrix = np.array(rot_matrix).reshape(3, 3)
        camera_forward = rot_matrix[:, 0]
        camera_up = rot_matrix[:, 2]
        camera_target = pos + 0.1 * camera_forward
        view_matrix = p.computeViewMatrix(
            cameraEyePosition=pos,
            cameraTargetPosition=camera_target,
            cameraUpVector=camera_up # type: ignore
        )
        width, height, rgb, depth, seg = p.getCameraImage(
            width=640,
            height=480,
            viewMatrix=view_matrix,
            projectionMatrix=self.projection_matrix
        ) # type: ignore
        return rgb
def raycast(origin : list[float], t0: list[float]) -> float:
    ray = p.rayTest(origin,   np.array(p.getMatrixFromQuaternion(link_state[5])).reshape(3,3)[:, 0], 0 )#type: ignore
    print(ray)
    p.addUserDebugLine(link_state[4])