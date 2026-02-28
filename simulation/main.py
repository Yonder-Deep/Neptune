import os
import time
import sys
import pybullet as p
import pybullet_data

from scripts.neptune import Neptune


last = ""
headless = False
step = 120.0
for arg in sys.argv:
    if arg == "-hl" or arg == "--headless":
        headless = True
    elif arg == "--help" or arg == "-h":
        print("Runs the pybullet simulation\nUsage: \n \t--help(-h): prints this message \n \t--headless(-hl) Runs the simulation without the GUI \n\t--step(-s) <int value>:sets the step time between physics steps in Hertz. Default: 120 HZ")
        quit()
    elif last == "-s" or last == "--step":
        step = int(arg)
    last = arg
    

#end of arg parsing

physicsClient = p.connect(p.DIRECT) if headless else p.connect(p.GUI)

p.setAdditionalSearchPath(pybullet_data.getDataPath())

p.loadURDF("plane.urdf", [0.0, 0.0, 0.0])
p.setGravity(0, 0, 0) 

neptune = Neptune(physicsClient)

start_time = time.time()

step = 0


try:
    while True:
        p.stepSimulation()
        step += 1
        neptune.step(step)
        time.sleep(1.0 / step) 
        if step % 500 == 0:
            (neptune.take_photo())
        
except KeyboardInterrupt:
    pass # pass to let disconnect get 

p.disconnect()