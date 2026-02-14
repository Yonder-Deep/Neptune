import os
import time

import pybullet as p
import pybullet_data

physicsClient = p.connect(p.GUI)  # GUI mode to see the simulation
p.setAdditionalSearchPath(pybullet_data.getDataPath())
# gravity used for weight, buoyancy will be applied manually
p.setGravity(0, 0, -9.81)

# Load objects
planeId = p.loadURDF("plane.urdf")
cubeStartPos = [0, 0, 1]
cubeStartOrientation = p.getQuaternionFromEuler([0, 0, 0])
boxId = p.loadURDF("cube.urdf", cubeStartPos, cubeStartOrientation, useFixedBase=False)

# Water parameters
water_level = 1.0  # z of water surface (plane sits at z=0)
buoyancy_coefficient = 120.0  # tuning constant for buoyant force
drag_coefficient = 8.0  # linear drag multiplier

def apply_buoyancy_and_drag(body_id):
    # Apply buoyancy and linear drag to base and all links
    num_joints = p.getNumJoints(body_id)

    # Handle base (-1) first
    base_pos, base_orn = p.getBasePositionAndOrientation(body_id)
    base_vel_lin, base_vel_ang = p.getBaseVelocity(body_id)
    base_dyn = p.getDynamicsInfo(body_id, -1)
    base_mass = base_dyn[0] if base_dyn is not None else 0.0

    def handle_part(link_index, pos, vel_lin, mass):
        depth = water_level - pos[2]
        if depth > 0:
            # Buoyant force: proportional to mass and submerged depth
            buoyant_force = buoyancy_coefficient * depth * mass
            p.applyExternalForce(body_id, link_index, [0, 0, buoyant_force], pos, p.WORLD_FRAME)
        # Linear drag opposing motion
        drag = [-drag_coefficient * mass * v for v in vel_lin]
        p.applyExternalForce(body_id, link_index, drag, pos, p.WORLD_FRAME)

    handle_part(-1, base_pos, base_vel_lin, base_mass)

    # Now links
    for link_idx in range(num_joints):
        link_state = p.getLinkState(body_id, link_idx, computeLinkVelocity=1)
        # link_state indexes: 0=worldLinkFramePosition, 6=worldLinkLinearVelocity
        link_pos = link_state[0]
        link_vel = link_state[6] if len(link_state) > 6 and link_state[6] is not None else (0.0, 0.0, 0.0)
        dyn = p.getDynamicsInfo(body_id, link_idx)
        link_mass = dyn[0] if dyn is not None else 0.0
        handle_part(link_idx, link_pos, link_vel, link_mass)


start_time = time.time()
step = 0
try:
    while True:
        # Apply custom water forces then step
        apply_buoyancy_and_drag(boxId)
        p.stepSimulation()

        if step % 120 == 0:
            cubePos, cubeOrn = p.getBasePositionAndOrientation(boxId)
            print(f"step={step} base_z={cubePos[2]:.3f}")

        step += 1
        time.sleep(1.0 / 240.0)  # Run at 240 Hz (PyBullet default)
except KeyboardInterrupt:
    pass

p.disconnect()