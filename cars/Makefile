TARGET = cars
CC = g++
CFLAGS = -O2 -std=c++11 -Wall -Iinclude
LDFLAGS = -lpthread

SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
TARGET_BIN = $(BINDIR)/$(TARGET)

GREEN = \033[0;32m
RED = \033[0;31m
YELLOW = \033[1;33m
NC = \033[0m

.PHONY: all clean run debug help

all: $(TARGET_BIN)

$(TARGET_BIN): $(OBJECTS)
	@echo -e "$(GREEN)Linkeando...$(NC)"
	@mkdir -p $(BINDIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo -e "$(GREEN)Compilacion completada: $(TARGET_BIN)$(NC)"

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@echo -e "$(YELLOW)Compilando $<...$(NC)"
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo -e "$(RED)Limpiando...$(NC)"
	@rm -rf $(OBJDIR)/*.o $(BINDIR)/$(TARGET)
	@echo -e "$(GREEN)Limpieza completada$(NC)"

run: $(TARGET_BIN)
	@echo -e "$(GREEN)Ejecutando Cars...$(NC)"
	@echo -e "$(YELLOW)Ejecutar con sudo para GPIO/SPI$(NC)"
	sudo $(TARGET_BIN)

debug: CFLAGS += -g -DDEBUG
debug: $(TARGET_BIN)
	@echo -e "$(GREEN)Ejecutando en modo debug...$(NC)"
	sudo $(TARGET_BIN)

help:
	@echo -e "$(GREEN)Cars Makefile$(NC)"
	@echo "  make      - Compilar"
	@echo "  make run  - Compilar y ejecutar (sudo)"
	@echo "  make debug- Compilar con debug"
	@echo "  make clean- Limpiar"
	@echo "  make help - Ayuda"

check:
	@echo -e "$(YELLOW)Verificando sistema...$(NC)"
	@if [ -e /dev/spidev0.0 ]; then \
		echo -e "$(GREEN)SPI disponible$(NC)"; \
	else \
		echo -e "$(RED)SPI no disponible. Agregar dtparam=spi=on a /boot/config.txt$(NC)"; \
	fi
