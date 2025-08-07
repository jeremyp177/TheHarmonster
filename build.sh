#!/bin/bash

# Brasscaster VST Plugin Build Script
echo "Building Brasscaster VST Plugin..."

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
fi

# Change to build directory
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake ..

# Build the project
echo "Building..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo "Build complete!"
echo ""
echo "Plugin files have been generated:"
echo "- VST3: Check your system VST3 directory"
echo "- AU (macOS): Check your system AU directory"  
echo "- Standalone: Look in the build directory"
echo ""
echo "To test the plugin, load it in your favorite DAW and play some guitar!" 