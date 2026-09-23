CC ?= cc

CPPFLAGS := -D_POSIX_C_SOURCE=200112L -Iinclude
CFLAGS := -std=c17 -Wall -Wextra -Wpedantic
BUILD_DIR := build/linux
TARGET := $(BUILD_DIR)/rictus
OBJECTS := $(BUILD_DIR)/main.o $(BUILD_DIR)/rictus.o $(BUILD_DIR)/config.o $(BUILD_DIR)/irc_message.o $(BUILD_DIR)/event.o $(BUILD_DIR)/irc.o $(BUILD_DIR)/tls_openssl.o $(BUILD_DIR)/rictus_net_linux.o
LDLIBS := -lssl -lcrypto

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDLIBS) -o $@

$(BUILD_DIR)/main.o: src/main.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/rictus.o: src/rictus.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/config.o: src/config.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/irc_message.o: src/irc_message.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/event.o: src/event.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/irc.o: src/irc.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/tls_openssl.o: src/tls_openssl.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/rictus_net_linux.o: platforms/linux/rictus_net_linux.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
