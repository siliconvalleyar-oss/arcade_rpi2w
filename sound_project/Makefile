TARGET = arcade_sound
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
INCLUDES = -I./include
LDFLAGS = -lpthread

SRCDIR = src
OBJDIR = obj
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
TARGET_BIN = $(BINDIR)/$(TARGET)

GREEN = \033[0;32m
RED = \033[0;31m
YELLOW = \033[1;33m
NC = \033[0m

.PHONY: all clean run help

all: $(TARGET_BIN)
	@echo -e "$(GREEN)Compilado: $(TARGET_BIN)$(NC)"

$(TARGET_BIN): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo -e "$(RED)Limpiando...$(NC)"
	rm -rf $(OBJDIR)/*.o $(TARGET_BIN)
	@echo -e "$(GREEN)Limpieza completada$(NC)"

run: $(TARGET_BIN)
	@echo -e "$(YELLOW)Nota: Sonido por bit-bang en GPIO via /dev/gpiochip0 (pasivo)$(NC)"
	sudo $(TARGET_BIN)

help:
	@echo "make      - Compilar"
	@echo "make run  - Ejecutar (sudo)"
	@echo "make clean- Limpiar"
