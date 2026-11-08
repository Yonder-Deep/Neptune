# ========================================================================================
# This top-level Makefile is a simple script runner for repeatable actions in the project.
# The `default_enter_container` target forwards some commands to within the container
# (It enters the container if it was not invoked from inside the container)
# ========================================================================================

# ======================================= #
# Commands for inside & outside container #
# ======================================= #

.PHONY: help 

help:
	# Script runner for Yonder-Deep/Neptune
	# Commands that run anywhere:
	# 	help      - Displays this message
	#
	# Commands that can only be ran outside of the container
	#
	# 	container - Builds/rebuilds the devcontainer
	# 	up        - Starts up the devcontainer (make sure `dockerd` is running)
	# 	enter     - Executes a bash shell inside the devcontainer
	#
	# Commands that will run inside the container
	# (The container will automatically be entered if required)
	#
	# 	build        - Builds the C++ in `src/`
	# 	prepare      - Generates the build artifacts in `src/build` (aids language servers)
	# 	clang-format - Runs clang-format on the C++ in `src/`

# This checks if make is NOT being invoked inside the container.
ifneq "$(CONTAINER_ACTIVE)" "true"

# ========================== #
# Commands outside container #
# ========================== #

.PHONY: container up enter build

%: 
	@echo "Invoked outside of container."
	@echo "Re-running inside devcontainer: 'make $(MAKECMDGOALS)'"
	devcontainer exec make $(if $(MAKECMDGOALS),$(MAKECMDGOALS),help)

container:
	@echo "Rebuilding devcontainer"
	devcontainer build --workspace-folder .

up:
	@echo "Bringing up devcontainer"
	devcontainer up --workspace-folder .

enter:
	@echo "Entering devcontainer"
	@# The `|| true` is so make will not see the error if you fail to gracefully
	@# exit the devcontainer shell
	devcontainer exec bash || true

# Separate forwarding since make can see the `build/` directory
build:
	devcontainer exec make build

# If make is being invoked from inside the container, either from the re-invocation of make by
# `default_enter_container`, or by a manual invocation from a shell inside the container
else

# ========================= #
# Commands inside container #
# ========================= #

.PHONY: build prepare clang-format

build:
	@echo "Building code inside container..."
	cmake -B build -S . && cd build && make

prepare:
	@echo "Generating build artifacts"
	cmake -B build -S .

clang-format:
	@echo "Running clang-format..."
	@# Using find because clang-format does not include file globbing
	find src \( -name "*.cpp" -o -name "*.hpp" \) -exec clang-format -i --verbose {} +

endif
