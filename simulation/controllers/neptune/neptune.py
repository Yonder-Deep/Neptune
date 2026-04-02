"""neptune controller."""
import io
import threading
# You may need to import some classes of the controller module. Ex:
#  from controller import Robot, Motor, DistanceSensor
from controller import Robot, Motor, Supervisor, Keyboard
import http.server
import socketserver
from flask import Flask, send_file
from PIL import Image
import struct
import math
robot = Supervisor() # Note this is supervise as in a node with control over the simulation, not the super class
node = robot.getSelf()
keyboard = Keyboard()
timestep = int(robot.getBasicTimeStep())

front_cam_node = robot.getCamera("NeptuneFront")
front_cam_node.enable(5)

imu = robot.getInertialUnit("inertial unit")
imu.enable(5)

gyro = robot.getGyro("gyro")
gyro.enable(5)

gps = robot.getGPS("gps")
gps.enable(5)
#ignore - the robot velocity is controlled manually instead of using motors
#TODO in the future, we should move over to using motors.
#lmotor = robot.getMotor("left_motor")
#rmotor = robot.getMotor("right_motor")
#lmotor.setPosition(float('inf'))

#rmotor.setPosition(float('inf'))


SIM_PORT = 8000

app = Flask(__name__ )
#flat multiplier since the cordinates are in huge amounts
ROBOT_SPEED_MULTI = 50.0 
MAX_ROBOT_SPEED = 20.0
ROBOT_ACCEL = 1.0

ROBOT_MAX_ROTATION_VEL = 0.2
ROBOT_ROT_ACCEL = 0.01
class RobotState():
    left_motor = False
    right_motor = False
    speed = 0.0
    time = 0
    rotation = 0.0
    def __init__(self):
        pass
    def tick(self):
        self.time += 1
        if not self.right_motor and not self.left_motor:
            self.speed = max(0, min(self.speed - ROBOT_ACCEL, MAX_ROBOT_SPEED))
            self.rotation *= 0.95
        elif self.left_motor and not self.right_motor:
             self.speed = max(0, min(self.speed - ROBOT_ACCEL, MAX_ROBOT_SPEED))
             self.rotation = min(self.rotation + ROBOT_ROT_ACCEL, ROBOT_MAX_ROTATION_VEL)
        elif self.right_motor and not self.left_motor:
             self.speed = max(0, min(self.speed - ROBOT_ACCEL, MAX_ROBOT_SPEED))
             self.rotation = max(self.rotation - ROBOT_ROT_ACCEL, -ROBOT_MAX_ROTATION_VEL)
        else:
            self.speed = min(self.speed + ROBOT_ACCEL, MAX_ROBOT_SPEED)
            self.rotation *= 0.95
        
    def getVel(self):
        forward = node.getOrientation()[0:3]
        x = forward[0] * self.speed* ROBOT_SPEED_MULTI
        y = forward[1] * self.speed* ROBOT_SPEED_MULTI
        z = forward[2] * self.speed * ROBOT_SPEED_MULTI
        return [x, z, y, 0, 0, self.rotation]
state = RobotState()  
@app.route("/toggle_left")
def left():
    state.left_motor = not state.left_motor
    return "ok  "
@app.route("/toggle_right")
def right():
    state.right_motor = not state.right_motor
    return "ok"
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
    return send_file(
        img_bytes,
        mimetype='image/png',
        download_name='image.png'
    )
@app.route("/imu")
def get_imu():
    return imu.getRollPitchYaw()


@app.route("/gyro")
def get_gyro():
    return gyro.getValues()

@app.route("/gps")
def get_gps():
    return gps.getValues()
# @app.route("/front_lidar")
# def front_lidar
def start():
    app.run(port = SIM_PORT)
t = threading.Thread(target = start)
t.daemon = True
t.start()

print("abc")
while robot.step(timestep) != -1:
    state.tick()
    node.setVelocity(state.getVel())
    key = keyboard.getKey()
    #rmotor.setVelocity(10)
    #lmotor.setVelocity(10)

