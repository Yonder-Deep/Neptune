# Running the project
Make sure you are in the dev container - see Docker.md for instructions on how to do that

enter src/build
```bash
cd src/build
```
Generate cmake files (this should only need to be ran once)
```bash
cmake ..
```
build the project, then run it
```bash
make
./neptune ...params
# or
./neptune_sim  # ensure the simulation is running
```
