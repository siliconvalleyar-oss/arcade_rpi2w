#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Building Arcade Sound System for Raspberry Pi ===${NC}"

# Check if running on Raspberry Pi
if ! grep -q "Raspberry Pi" /proc/device-tree/model 2>/dev/null; then
    echo -e "${YELLOW}Warning: Not running on a Raspberry Pi?${NC}"
fi

# Check for bcm2835 library
if [ ! -f /usr/local/lib/libbcm2835.a ]; then
    echo -e "${RED}bcm2835 library not found. Installing...${NC}"
    cd /tmp
    wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.71.tar.gz
    tar zxvf bcm2835-1.71.tar.gz
    cd bcm2835-1.71
    ./configure
    make
    sudo make check
    sudo make install
    cd ..
    rm -rf bcm2835-1.71*
    echo -e "${GREEN}bcm2835 installed successfully${NC}"
fi

# Create directories
mkdir -p bin
mkdir -p obj

# Compile with modern C++ standards and optimizations
echo -e "${GREEN}Compiling sources...${NC}"

CXX=g++
CXXFLAGS="-std=c++17 -Wall -Wextra -O3 -march=armv8-a -mtune=cortex-a72"
INCLUDES="-I./include"
LIBS="-lbcm2835 -lpthread"

# Compile each source file
for src in src/*.cpp; do
    obj="obj/$(basename ${src%.cpp}.o)"
    echo "Compiling $src -> $obj"
    $CXX $CXXFLAGS $INCLUDES -c $src -o $obj
    if [ $? -ne 0 ]; then
        echo -e "${RED}Failed to compile $src${NC}"
        exit 1
    fi
done

# Link
echo -e "${GREEN}Linking...${NC}"
$CXX $CXXFLAGS obj/*.o -o bin/arcade_sound $LIBS

if [ $? -eq 0 ]; then
    echo -e "${GREEN}Build successful! Binary located at bin/arcade_sound${NC}"
    echo -e "${YELLOW}To run: sudo ./bin/arcade_sound (requires root for GPIO access)${NC}"
else
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi

# Make binary executable
chmod +x bin/arcade_sound

# Create a simple run script
cat > bin/run.sh << 'EOF'
#!/bin/bash
sudo ./arcade_sound
EOF
chmod +x bin/run.sh

echo -e "${GREEN}Done! Use ./bin/run.sh to start the program${NC}"

