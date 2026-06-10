TARGET = space_invaders
CXX = g++
CXXFLAGS = -O2 -std=c++11 -Wall -Iinclude
LDFLAGS = -lpthread

SRCDIR = src
OBJDIR = obj
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
TARGET_BIN = $(BINDIR)/$(TARGET)

.PHONY: all clean run

all: $(TARGET_BIN)

$(TARGET_BIN): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "✓ Compiled: $@"

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)/*.o $(BINDIR)/$(TARGET)

run: $(TARGET_BIN)
	sudo $(TARGET_BIN)
