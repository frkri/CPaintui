# [Choice] Debian / Ubuntu version (use Debian 11, Ubuntu 18.04/22.04 on local arm64/Apple Silicon): debian-11, debian-10, ubuntu-22.04, ubuntu-20.04, ubuntu-18.04
ARG VARIANT=debian-11
FROM mcr.microsoft.com/devcontainers/base:0-${VARIANT}
USER root

# Install needed packages.
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install build-essential ninja-build wget lsb-release software-properties-common pkg-config \
    && apt-get autoremove -y && apt-get clean -y && rm -rf /var/lib/apt/lists/*

# Get the latest version of LLVM and Clang (including the server Clangd for use with VS Code)
# See https://apt.llvm.org/ for more information.
RUN bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)"

ARG USERNAME=vscode