CC      ?= cc
CFLAGS  ?= -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -O2
LDFLAGS ?=

SRC_DIR    := src
BUILD_DIR  := build
BIN        := $(BUILD_DIR)/minish

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

.PHONY: all clean run debug

all: $(BIN)

$(BIN): $(OBJS) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $@

run: $(BIN)
	./$(BIN)

debug: CFLAGS += -g3 -O0
debug: clean all

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
