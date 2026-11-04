CC ?= cc
CPPFLAGS ?=
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
LDFLAGS ?=
LDLIBS ?= -pthread -lssl -lcrypto

ROOT ?= ../..
BUILD_DIR ?= build
PREFIX ?= /opt/rictus
INSTALL_USER ?= rictus
INSTALL_GROUP ?= rictus
MODULEDIR ?= $(PREFIX)/modules/web

CPPFLAGS += -Isrc -I$(ROOT)/include
TARGET := $(BUILD_DIR)/web.so
TEST_TARGET := $(BUILD_DIR)/test_web

.PHONY: all test install uninstall clean
all: $(TARGET)

$(TARGET): src/web_module.c src/web_protocol.c src/web_protocol.h
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -shared $(LDFLAGS) -o $@ src/web_module.c src/web_protocol.c $(LDLIBS)

$(TEST_TARGET): tests/test_web.c src/web_protocol.c src/web_protocol.h
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ tests/test_web.c src/web_protocol.c

test: $(TEST_TARGET)
	./$(TEST_TARGET)

install: $(TARGET)
	install -d -o $(INSTALL_USER) -g $(INSTALL_GROUP) -m 0755 $(MODULEDIR)
	install -o $(INSTALL_USER) -g $(INSTALL_GROUP) -m 0644 module.conf $(MODULEDIR)/module.conf
	install -o $(INSTALL_USER) -g $(INSTALL_GROUP) -m 0755 $(TARGET) $(MODULEDIR)/web.so

uninstall:
	rm -f $(MODULEDIR)/web.so $(MODULEDIR)/module.conf

clean:
	rm -rf $(BUILD_DIR)
