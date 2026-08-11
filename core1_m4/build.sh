#!/bin/bash

BUILD_PATH="./build"

# Remove build folder if it exists in the current directory
if [ -d "$BUILD_PATH" ]; then
    rm -rf "$BUILD_PATH"
    echo "Build folder removed."
else
    echo "No build folder found"
fi

# Build for STM32 Nucleo-WL55JC
west build -p always -b nucleo_wl55jc