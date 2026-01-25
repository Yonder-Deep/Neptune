FROM debian:bookworm-slim

# Install build tools and C++ dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    git \
    clang-format \
    clang-tidy \
    # C++ dependencies (from CLAUDE.md section 5.5)
    libeigen3-dev \
    libboost-all-dev \
    libyaml-cpp-dev \
    # Testing & coverage
    lcov \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy source code
COPY . .

# Default command: build and test
CMD ["bash", "-c", "cmake -B build -G Ninja -DBUILD_TESTS=ON && cmake --build build && ctest --test-dir build --output-on-failure"]
