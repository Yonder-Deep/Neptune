# Docker

Docker allows us to containerize our app to ensure the environment is the same across all everyones seperate computers. The Docker container determines what is installed into the container.

# Launching the project in the container

* Clone the repository and open it in vscode
* Make sure the [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension by microsoft is installed
* From you command palete (ctrl(cmd on mac) + shift + p) run the  `Dev containers rebuild and reopen in container` command.
* The first time, this may take awhile to build
You can tell the container is open if the bar at the top of vscode shows `Neptune [Dev Container: neptune]`

Later, you can relaunch directly in the contaier by choosing the [dev container] option when you select your workspace
