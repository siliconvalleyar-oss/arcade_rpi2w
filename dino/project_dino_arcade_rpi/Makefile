# ============================================================
#  Makefile — Chrome Dino Arcade / RPi Zero 2W
#  Estructura reorganizada: src/, include/, bin/, obj/
# ============================================================

CXX      = g++
CXXFLAGS = -O2 -std=c++11 -Wall -Iinclude
LDFLAGS  = -lpthread

# Directorios
SRCDIR   = src
INCDIR   = include
OBJDIR   = obj
BINDIR   = bin

# Archivos fuente (todos los .cpp en src/)
SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))
TARGET = $(BINDIR)/dino

all: $(TARGET)
	@echo ""
	@echo "  Build OK — ejecutar con:  sudo ./$(TARGET)"
	@echo ""

$(TARGET): $(OBJS) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Crear directorios si no existen
$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

# Regla de compilación: compilar .cpp en obj/
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)/*.o $(TARGET)

distclean: clean
	rm -rf $(OBJDIR) $(BINDIR)

check:
	@ls /dev/spidev0.0 2>/dev/null && echo "SPI OK" || echo "ERROR: habilitar dtparam=spi=on"
	@ls /dev/gpiochip0 2>/dev/null && echo "GPIO OK" || echo "ERROR: no gpiochip0"

.PHONY: all clean distclean check
