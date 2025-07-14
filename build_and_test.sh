#!/bin/bash

# Script to build and install the plugin with favorites support

echo "=== Building Custom Desktop Menu with favorites support ==="

# Clean previous build
rm -rf _build

# Create build directory
mkdir -p _build

# Configure with CMake
cd src
cmake --fresh -B ../_build .
cd ../_build

# Compile
echo "Compiling..."
make clean
make

if [ $? -eq 0 ]; then
    echo "✓ Compilation successful"
    
    # Make executable
    chmod 755 bin/plasma/containmentactions/matmoul-customdesktopmenu.so
    
    # Install
    echo "Installing plugin..."
    sudo cp bin/plasma/containmentactions/matmoul-customdesktopmenu.so /usr/lib/qt6/plugins/plasma/containmentactions/
    
    if [ $? -eq 0 ]; then
        echo "✓ Plugin installed successfully"
        echo ""
        echo "=== Features implemented ==="
        echo "• Complete favorites support using KActivitiesStats"
        echo "• Configurable maximum number of favorites (1-20)"
        echo "• Fallback to common applications if no statistics available"
        echo "• Hidden applications filtering"
        echo "• Duplicate prevention"
        echo "• Tooltips with application descriptions"
        echo ""
        echo "=== Usage ==="
        echo "1. Restart KWin: kwin_x11 --replace & (or kwin_wayland --replace &)"
        echo "2. Go to System Settings > Desktop Behavior > Desktop Actions"
        echo "3. Configure 'Custom Desktop Menu' for right-click"
        echo "4. In configuration, adjust maximum favorites if needed"
        echo "5. The {favorites} token in configuration will show most used applications"
        echo ""
        echo "=== Example configuration ==="
        echo "{favorites}"
        echo "-"
        echo "[menu]	Applications	kde"
        echo "{programs}"
        echo "[end]"
        echo ""
    else
        echo "✗ Error installing plugin"
        exit 1
    fi
else
    echo "✗ Compilation error"
    exit 1
fi
