TARGET = mario
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
	@echo -e "$(GREEN)Compilado: $(TARGET_BIN)$(NC)"

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@echo -e "$(YELLOW)Compilando $<...$(NC)"
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo -e "$(RED)Limpiando...$(NC)"
	rm -rf $(OBJDIR)/*.o $(BINDIR)/$(TARGET)
	@echo -e "$(GREEN)Limpieza completada$(NC)"

run: $(TARGET_BIN)
	@echo -e "$(GREEN)Ejecutando Mario...$(NC)"
	sudo $(TARGET_BIN)

debug: CFLAGS += -g -DDEBUG
debug: $(TARGET_BIN)
	@echo -e "$(GREEN)Modo debug...$(NC)"
	sudo $(TARGET_BIN)

help:
	@echo "make      - Compilar"
	@echo "make run  - Ejecutar (sudo)"
	@echo "make clean- Limpiar"
	@echo "make debug- Modo debug"
