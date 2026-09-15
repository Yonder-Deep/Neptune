# Starting documentation
This will walk you through running a simulated run of the code from a fresh install.
This guide assumes you are using visual studio code. If you don't use vscode, you will need to start or access the docker container through your choice of IDE's container management tools. Adding sections to Docker.md for other ides would be greatly appreciated.
## Installs

**Install git:**
- Preferably use your package manager
- If you don't have one, use https://git-scm.com/install/

**Clone the GitHub repositories** into wherever you store your code:
- https://github.com/Yonder-Deep/Neptune
- https://github.com/Yonder-Deep/Neptune-Sim

**Install docker:**
- Linux/WSL: https://docs.docker.com/engine/install/
- Mac: https://docs.docker.com/desktop/setup/install/mac-install/
- Non-WSL Windows: https://docs.docker.com/desktop/setup/install/windows-install/
- Mac/Windows must launch Docker Desktop to start the engine in the background

**Install webots:**
- All platforms: https://cyberbotics.com/doc/guide/installation-procedure
- For WSL: Note that webots must be installed and ran in the same system as the onboard code. If not, you'll have to change the IP address for the simulation in the neptune repository to point towards the other system. Preferably, just install into the same system

**Install python:**
- The simulation needs a Python interpreter on the path. If you already have Python installed, you can use that installation
- If you do not have python installed, install it from your package manager or from https://www.python.org/downloads/ Any recent version should work.
- See also webots documentation on installing python: https://cyberbotics.com/doc/guide/using-python#installation

**Install Flask**
- `pip install flask`
- If you don't want packages in your global python installation, create a venv and make the WEBOTS_PYTHON variable point to the venv binary.

## Start Simulation
- Open the Neptune-Sim directory cloned in the first step. From within the root directory, run `webots worlds/neptune_meters.wbt`.
- The simulation window should open, and the console will stall with no output. However, the robot hasn't got any movement commands yet, so it will stay still

## Start Onboard
- Open the Neptune repository in your IDE.
- Launch the docker container: Consult Docker.md for IDE specific instructions
- Once you are in the docker container, run:
```bash
cd src
mkdir build
cd build
cmake ..
make
```
This will generate the build artifacts. If you run `ls`, you should see some executables generated:
```bash
> ls
neptune neptune_sim ...(more files)
```
The two executables that matter are neptune and neptune_sim:
- neptune is for running with real hardware hooked up, which often only happens on the pi
- neptune_sim is for using the simulated "hardware" in the webots simulation
To run the code, run the neptune sim exe
```bash
./neptune_sim
```
This will start the code. If a motor is turned on, it should change in the webots simulation
