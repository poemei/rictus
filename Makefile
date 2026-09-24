CC ?= cc

CPPFLAGS := -D_POSIX_C_SOURCE=200112L -Iinclude
CFLAGS := -std=c17 -Wall -Wextra -Wpedantic
BUILD_DIR := build/linux
TARGET := $(BUILD_DIR)/rictus
OBJECTS := $(BUILD_DIR)/main.o $(BUILD_DIR)/rictus.o $(BUILD_DIR)/artifact.o $(BUILD_DIR)/config.o $(BUILD_DIR)/irc_message.o $(BUILD_DIR)/event.o $(BUILD_DIR)/dispatch.o $(BUILD_DIR)/command.o $(BUILD_DIR)/module_inventory.o $(BUILD_DIR)/module_state.o $(BUILD_DIR)/module_lifecycle.o $(BUILD_DIR)/module_loader_linux.o $(BUILD_DIR)/module_discovery_linux.o
ABI_DIR := ../ABI
ABI_OBJECTS := $(BUILD_DIR)/abi_module.o $(BUILD_DIR)/abi_module_registry.o
LDLIBS := -lssl -lcrypto
CORE_LDLIBS := -ldl -lcrypto

IRC_MODULE := $(BUILD_DIR)/modules/irc.so

.PHONY: all clean

all: $(TARGET) $(IRC_MODULE)
	@mkdir -p state

$(IRC_MODULE): modules/irc/src/irc_module.c
	@mkdir -p $(BUILD_DIR)/modules
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -Imodules/irc/include -shared modules/irc/src/irc_module.c src/config.c src/irc.c src/irc_message.c src/event.c src/dispatch.c src/command.c src/tls_openssl.c platforms/linux/rictus_net_linux.c $(LDLIBS) -o $@

$(TARGET): $(OBJECTS) $(ABI_OBJECTS)
	$(CC) $(OBJECTS) $(ABI_OBJECTS) $(CORE_LDLIBS) -o $@

$(BUILD_DIR)/abi_module.o: $(ABI_DIR)/src/module.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -I$(ABI_DIR)/includes -c $< -o $@

$(BUILD_DIR)/abi_module_registry.o: $(ABI_DIR)/src/module_registry.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -I$(ABI_DIR)/includes -c $< -o $@

$(BUILD_DIR)/main.o: src/main.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/artifact.o: src/artifact.c
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

$(BUILD_DIR)/dispatch.o: src/dispatch.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/command.o: src/command.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/module_inventory.o: src/module_inventory.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/module_state.o: src/module_state.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/module_lifecycle.o: src/module_lifecycle.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/module_loader_linux.o: platforms/linux/rictus_module_loader_linux.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/module_discovery_linux.o: platforms/linux/rictus_module_discovery_linux.c
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
