#!/bin/bash

# This script automates the build and run process.
# It ensures that commands are executed from the correct directory
# and stops if any command fails.

# Exit immediately if a command exits with a non-zero status.
set -e

# 1. Create the build directory if it doesn't already exist.
# The '-p' flag ensures no error is thrown if the directory already exists.
mkdir -p build

# 2. Navigate into the build directory.
cd build

# 3. Run CMake to configure the project and generate build files.
cmake ..

# 4. Run Make to compile the project.
make

# 5. If compilation was successful, run the final executable.
echo "Build complete. Starting application..."
./solar-system
