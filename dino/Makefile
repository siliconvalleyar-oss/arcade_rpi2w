TARGET = dino
CXX      = g++
CXXFLAGS = -O2 -std=c++11 -Wall -Iinclude
LDFLAGS  = -lpthread

SRCDIR   = src
INCDIR   = include
OBJDIR   = obj
BINDIR   = bin

SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))
TARGET_BIN = $(BINDIR)/$(TARGET)

GREEN = \033[0;32m
RED = \033[0;31m
YELLOW = \033[1;33m
NC = \033[0m

.PHONY: all clean run debug help

all: $(TARGET_BIN)
	@echo "Build OK - ejecutar con: sudo $(TARGET_BIN)"

$(TARGET_BIN): $(OBJS) | $(BINDIR) $(OBJDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo -e "$(RED)Limpiando...$(NC)"
	rm -rf $(OBJDIR)/*.o $(TARGET_BIN)
	@echo -e "$(GREEN)Limpieza completada$(NC)"

run: $(TARGET_BIN)
	@echo -e "$(GREEN)Ejecutando Dino...$(NC)"
	sudo $(TARGET_BIN)

debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET_BIN)
	@echo -e "$(GREEN)Modo debug...$(NC)"
	sudo $(TARGET_BIN)

check:
	@ls /dev/spidev0.0 2>/dev/null && echo "SPI OK" || echo "ERROR: habilitar dtparam=spi=on"
	@ls /dev/gpiochip0 2>/dev/null && echo "GPIO OK" || echo "ERROR: no gpiochip0"

help:
	@echo "make      - Compilar"
	@echo "make run  - Ejecutar (sudo)"
	@echo "make clean- Limpiar"
	@echo "make debug- Modo debug"
	@echo "make check- Verificar hardware"
