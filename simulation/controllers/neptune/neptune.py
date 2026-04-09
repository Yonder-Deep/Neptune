"""neptune controller."""
import io
import threading
import math
import struct

from controller import Supervisor, Keyboard
from flask import Flask, send_file
from PIL import Image

robot = Supervisor()
node = robot.getSelf()

keyboard = Keyboard()
keyboard.enable(int(robot.getBasicTimeStep()))  

timestep = int(robot.getBasicTimeStep())

# Sensors
front_cam_node = robot.getCamera("NeptuneFront")
front_cam_node.enable(5)

imu = robot.getInertialUnit("inertial unit")
imu.enable(5)

gyro = robot.getGyro("gyro")
gyro.enable(5)

gps = robot.getGPS("gps")
gps.enable(5)

front_lidar = robot.getLidar("front_lidar")
front_lidar.enable(5)
front_lidar.enablePointCloud()

# Motors
l_t_motor = robot.getMotor("l_t_rot_motor")
r_t_motor = robot.getMotor("r_t_rot_motor")
l_b_motor = robot.getMotor("l_b_rot_motor")
r_b_motor = robot.getMotor("r_b_rot_motor")

all_motors = [l_t_motor, r_t_motor, l_b_motor, r_b_motor]

for m in all_motors:
    m.setPosition(float('inf'))  # velocity control mode
    m.setVelocity(0)

MOTOR_VELOCITY = 3.0  # rad/s — tuned for meter-scale boat with thrustConstants 50 0

SIM_PORT = 8000
app = Flask(__name__)


class RobotState():
    top_l_motor = False
    bottom_l_motor = False
    top_r_motor = False
    bottom_r_motor = False
    time = 0

    def __init__(self):
        pass

    def tick(self):
        self.time += 1

    def apply_motors(self):
        l_t_motor.setVelocity(MOTOR_VELOCITY if self.top_l_motor    else 0)
        r_t_motor.setVelocity(MOTOR_VELOCITY if self.top_r_motor    else 0)
        l_b_motor.setVelocity(MOTOR_VELOCITY if self.bottom_l_motor else 0)
        r_b_motor.setVelocity(MOTOR_VELOCITY if self.bottom_r_motor else 0)


state = RobotState()


# --- Flask routes ---

@app.route("/motor/top_left/toggle", methods=["POST", "GET"])
def toggle_top_left():
    state.top_l_motor = not state.top_l_motor
    return {"status": state.top_l_motor}

@app.route("/motor/top_right/toggle", methods=["POST", "GET"])
def toggle_top_right():
    state.top_r_motor = not state.top_r_motor
    return {"status": state.top_r_motor}

@app.route("/motor/bottom_left/toggle", methods=["POST", "GET"])
def toggle_bottom_left():
    state.bottom_l_motor = not state.bottom_l_motor
    return {"status": state.bottom_l_motor}

@app.route("/motor/bottom_right/toggle", methods=["POST", "GET"])
def toggle_bottom_right():
    state.bottom_r_motor = not state.bottom_r_motor
    return {"status": state.bottom_r_motor}

@app.route("/front_cam")
def front_cam():
    img = front_cam_node.getImage()
    width = front_cam_node.getWidth()
    height = front_cam_node.getHeight()
    image_data = struct.unpack('B' * (width * height * 4), img)
    pil_image = Image.frombytes('RGBA', (width, height), bytes(image_data))
    img_bytes = io.BytesIO()
    pil_image.save(img_bytes, format='PNG')
    img_bytes.seek(0)
    return send_file(img_bytes, mimetype='image/png', download_name='image.png')

@app.route("/imu")
def get_imu():
    return list(imu.getRollPitchYaw())

@app.route("/gyro")
def get_gyro():
    return list(gyro.getValues())

@app.route("/gps")
def get_gps():
    return list(gps.getValues())

@app.route("/front_lidar")
def get_front_lidar():
    return [
        [a.x, a.y, a.z]
        for a in front_lidar.getLayerPointCloud(0)
        if math.isfinite(a.x) and math.isfinite(a.y) and math.isfinite(a.z)
    ]

@app.route("/state")
def get_state():
    return {
        "top_l_motor":    state.top_l_motor,
        "top_r_motor":    state.top_r_motor,
        "bottom_l_motor": state.bottom_l_motor,
        "bottom_r_motor": state.bottom_r_motor,
        "time":           state.time,
        "gps":            list(gps.getValues()),
    }


def start_flask():
    app.run(port=SIM_PORT)

t = threading.Thread(target=start_flask)
t.daemon = True
t.start()

# --- Main simulation loop ---
while robot.step(timestep) != -1:
    state.tick()
    state.apply_motors()
    key = keyboard.getKey()