CC ?= cc
CPPFLAGS ?=
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
LDFLAGS ?=
LDLIBS ?= -lssl -lcrypto -pthread

ROOT ?= ../..
ABI_INCLUDE_DIR ?= $(ROOT)/include
INCLUDE_DIR := include
BUILD_DIR ?= build
TARGET := $(BUILD_DIR)/irc.so
TEST_TARGET := $(BUILD_DIR)/test_irc

PREFIX ?= /opt/rictus
MODULE_ROOT ?= $(PREFIX)/modules/irc
MODULE_BIN_DIR ?= $(MODULE_ROOT)

CPPFLAGS += -I$(INCLUDE_DIR) -I$(ABI_INCLUDE_DIR)
MODULE_CFLAGS := $(CFLAGS) -fPIC
MODULE_SRCS := src/irc_module.c src/irc_config.c src/irc_protocol.c
MODULE_OBJS := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(MODULE_SRCS))
TEST_SRCS := tests/test_irc.c src/irc_protocol.c

.PHONY: all test clean install uninstall
all: $(TARGET)

$(TARGET): $(MODULE_OBJS)
	@mkdir -p $(dir $@)
	$(CC) -shared $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(MODULE_CFLAGS) -MMD -MP -c $< -o $@

$(TEST_TARGET): $(TEST_SRCS)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lcrypto

test: $(TEST_TARGET)
	./$(TEST_TARGET)

install: $(TARGET)
	install -d $(MODULE_BIN_DIR)
	install -m 0755 $(TARGET) $(MODULE_BIN_DIR)/irc.so

uninstall:
	rm -f $(MODULE_BIN_DIR)/irc.so

clean:
	rm -rf $(BUILD_DIR)

-include $(MODULE_OBJS:.o=.d)
