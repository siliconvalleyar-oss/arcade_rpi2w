# ============================================================
#  Makefile - Donkey Kong para Raspberry Pi Zero 2W
#  Compilador: g++ con C++20
#  GPIO/SPI via ioctl del kernel (sin librerias externas)
# ============================================================

CXX      := g++
CXXFLAGS := -std=c++20 \
            -Wall -Wextra -Wpedantic \
            -O2 \
            -I./include

# Sin -lpigpio: usa ioctl /dev/gpiochip0 + /dev/spidev0.0
LDFLAGS  := -lpthread

TARGET   := bin/Game_App

# Todos los .cpp en src/
SRCS     := $(wildcard src/*.cpp)
OBJS     := $(patsubst src/%.cpp, obj/%.o, $(SRCS))

# ---- Reglas -----------------------------------------------
.PHONY: all clean run install-deps

all: bin obj $(TARGET)
	@echo ""
	@echo "  ✓ Build exitoso: $(TARGET)"
	@echo "  Ejecutar con: sudo ./$(TARGET)"
	@echo ""

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

obj/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Dependencias de headers (recompilar si cambia algún .h)
obj/Game.o:     src/Game.cpp      include/Game.h include/Types.h \
                include/Graphics.h include/Renderer.h include/Sound.h \
                include/Physics.h  include/Player.h   include/Enemy.h \
                include/Level.h    include/HardwareProfile.h include/Hw.h
obj/Graphics.o: src/Graphics.cpp  include/Graphics.h include/Types.h \
                include/fonts.h    include/picopng.h
obj/Renderer.o: src/Renderer.cpp  include/Renderer.h include/HardwareProfile.h \
                include/Graphics.h include/Hw.h
obj/Sound.o:    src/Sound.cpp     include/Sound.h include/HardwareProfile.h include/Hw.h
obj/Hw.o:       src/Hw.cpp        include/Hw.h include/HardwareProfile.h
obj/Physics.o:  src/Physics.cpp   include/Physics.h include/Types.h
obj/Player.o:   src/Player.cpp    include/Player.h include/Entity.h \
                include/Physics.h  include/Sound.h
obj/Enemy.o:    src/Enemy.cpp     include/Enemy.h include/Entity.h \
                include/Physics.h  include/Sound.h
obj/Level.o:    src/Level.cpp     include/Level.h include/picopng.h \
                include/Graphics.h include/Types.h
obj/Entity.o:   src/Entity.cpp    include/Entity.h include/Types.h \
                include/Graphics.h

bin:
	mkdir -p bin

obj:
	mkdir -p obj

clean:
	rm -rf obj bin
	@echo "  Limpiado."

run: all
	sudo ./$(TARGET)

install-deps:
	sudo apt-get update
	sudo apt-get install -y libpigpio-dev g++ build-essential
	@echo "  Dependencias instaladas."
