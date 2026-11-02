CC ?= cc
CPPFLAGS ?=
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
LDFLAGS ?=

ROOT ?= ../..
CPPFLAGS += -Iinclude -I$(ROOT)/include
BUILD ?= build
TARGET := $(BUILD)/compliance.so
TEST := $(BUILD)/test_compliance
PREFIX ?= /opt/rictus
MODULEDIR := $(PREFIX)/modules/compliance
OBJS := $(BUILD)/compliance_model.o $(BUILD)/compliance_module.o

.PHONY: all test clean install uninstall
all: $(TARGET)

$(BUILD)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -MMD -MP -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) -shared $(LDFLAGS) -o $@ $^

$(TEST): tests/test_compliance.c src/compliance_model.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^

test: $(TEST)
	./$(TEST)

install: $(TARGET)
	install -d -o root -g root -m 0755 "$(MODULEDIR)"
	install -o root -g root -m 0644 module.conf "$(MODULEDIR)/module.conf"
	install -o root -g root -m 0755 "$(TARGET)" "$(MODULEDIR)/compliance.so"

uninstall:
	rm -f "$(MODULEDIR)/compliance.so" "$(MODULEDIR)/module.conf"

clean:
	rm -rf $(BUILD)

-include $(OBJS:.o=.d)
