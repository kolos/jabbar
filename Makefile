# Compiler and flags
CC := gcc
#CFLAGS := -Wall -Wextra -std=c99 -Iinclude
CFLAGS := -Wextra -Iinclude
CFLAGS += $(shell pkg-config --cflags dbus-1)

# Directories
SRC_DIR := src
OBJ_DIR := build
LIB_DIR := libs

# Target executable
TARGET := bin/jabra

# Source and object files
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Libraries to link
LDFLAGS := -L$(LIB_DIR) -ljabra
LDFLAGS += $(shell pkg-config --libs dbus-1)

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	@echo "Linking $@..."
	$(CC) $(OBJS) -o $@ $(LDFLAGS) -Wl,-rpath,$(LIB_DIR)

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	@echo "Cleaning..."
	rm -rf $(OBJ_DIR) $(TARGET)

# Phony targets
.PHONY: all clean
