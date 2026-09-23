CC ?= cc

CPPFLAGS := -D_POSIX_C_SOURCE=200112L -Iinclude
CFLAGS := -std=c17 -Wall -Wextra -Wpedantic
BUILD_DIR := build/linux
TARGET := $(BUILD_DIR)/rictus
SOURCES := src/main.c src/rictus.c src/config.c platforms/linux/rictus_net_linux.c
OBJECTS := $(BUILD_DIR)/main.o $(BUILD_DIR)/rictus.o $(BUILD_DIR)/config.o $(BUILD_DIR)/rictus_net_linux.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

$(BUILD_DIR)/main.o: src/main.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/rictus.o: src/rictus.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/config.o: src/config.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/rictus_net_linux.o: platforms/linux/rictus_net_linux.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
