# CPaintui

TUI Paint program written in C

Uses Ncurses Lib for the UI

# Getting Started

This project uses the [Meson](https://mesonbuild.com/Quick-guide.html) build system.

## Dependencies

To install dependencies, run the following commands:

```bash

# Python, pip, and ninja
apt-get update
apt-get install python3 python3-pip ninja-build

# Meson (latest version with pip)
pip3 install meson

```

## Building

To build, run the following commands:

_Install will be based on the Compiler you use_

```bash

# With default (should be GCC on most systems)
meson setup build-default
cd build-default

# With Clang
# Requires LLVM to be installed, see Dockerfile in .devcontainers for more info
CC=clang-15 CXX=clang-cpp-15 meson setup build-clang
cd build-clang

# Inside the build directory
meson compile

# Optional
meson install

```
