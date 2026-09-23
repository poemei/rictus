CC ?= cc

CPPFLAGS := -Iinclude
CFLAGS := -std=c17 -Wall -Wextra -Wpedantic
BUILD_DIR := build/linux
TARGET := $(BUILD_DIR)/rictus
SOURCES := src/main.c src/rictus.c
OBJECTS := $(SOURCES:src/%.c=$(BUILD_DIR)/%.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
