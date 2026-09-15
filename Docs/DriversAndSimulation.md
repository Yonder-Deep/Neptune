# Drivers and Simulation
Drivers are classes in the codebase that interface with some hardware component, for example motors or gps.
Its not possible for use to constantly test our code on the actual hardware, nor can we realistically develop code without testing that the logic is correct. Simulation allows us to test higher level logic without being dependent on actual hardware.
## Creating a Driver
In order to create a driver, you'll need to create 3 classes:
- A parent class, that says what this piece of hardware *does* but does *not* define how to do it. This should be a mostly blank abstract class.
- A hardware class, which inherits the parent class and overrides the methods to use hardware to do whatever operation the method defines.
- A simulated class, which communicates with the flask server in the [sim repo](https://github.com/Yonder-Deep/Neptune-Sim).

## Using a driver
The general pattern to use a driver is as follows:
- Define a field with a type of *(your parent class) on the neptune object
- In the `sim_main.cpp` and `main.cpp` files, initialize the field to the simulated and main object respectively.
