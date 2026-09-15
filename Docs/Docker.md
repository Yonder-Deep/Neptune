# Docker

Docker allows us to containerize our app to ensure the environment is the same across all everyone's separate computers. The Docker container determines what is installed into the container that the code runs in.

## Developing in docker
In order to develop the project, you must start the docker container and develop from within it. See StartHere.md for installations.
How to develop within a container is dependent on your ide. Visual Studio Code (or a fork capable of using extensions built for vscode) is the recommended approach. If your ide is not on your list, you'll have to figure out how to develop inside a docker container yourself. Creating a new section for your IDE would be appreciated.


### VSCode
* Open the repository in vscode
* Make sure the [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension by Microsoft is installed
* From you command palette (ctrl/cmd on mac + shift + p) run the  `Dev containers rebuild and reopen in container` command.
* When running for the first time, this will take a long time.
You can tell the container is open if the bar at the top of vscode shows `Neptune [Dev Container: neptune]`

Later, you can relaunch directly in the container by choosing the Neptune [Dev Container] option when you select your workspace.
