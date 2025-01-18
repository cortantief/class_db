# Define compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -I src/headers -D_GNU_SOURCE -lm -g

# Directories
SRC_DIR = src
HEADERS_DIR = $(SRC_DIR)/headers
BUILD_DIR = build

# Target executable
TARGET = $(BUILD_DIR)/program

# Source files and object files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Default target
all: $(TARGET)

# Build target executable
$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Build object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS_DIR)/*.h
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -rf $(BUILD_DIR)

# Phony targets
.PHONY: all clean

