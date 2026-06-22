CC = gcc
BUILD_DIR ?= /tmp/sil4-build
SCRIPT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
SMOKE_SCRIPT := $(SCRIPT_DIR)run_smoke.sh
FAILOVER_SMOKE_SCRIPT := $(SCRIPT_DIR)run_failover_smoke.sh
CFLAGS = -Wall -Wextra -Werror -std=c11 -I../../include -I.
LDLIBS = \
	$(BUILD_DIR)/librsrx_api.a \
	$(BUILD_DIR)/librsrx_transport_supervisor.a \
	$(BUILD_DIR)/librsrx_orchestrator.a \
	$(BUILD_DIR)/librsrx_protocol_context.a \
	$(BUILD_DIR)/librsrx_channel_manager.a \
	$(BUILD_DIR)/librsrx_platform_adapters.a \
	$(BUILD_DIR)/librsrx_config_validator.a \
	$(BUILD_DIR)/librsrx_state_machine.a \
	$(BUILD_DIR)/librsrx_codec.a

SRCS = main.c posix_platform.c posix_transport.c
OBJS = $(SRCS:.c=.o)
TARGET = rsrx_echo_app

all: $(TARGET)

core-build:
	cmake --build $(BUILD_DIR) -j4

help:
	@echo "Targets:"
	@echo "  make -C sil4/examples/posix_echo BUILD_DIR=$(BUILD_DIR) all"
	@echo "  make -C sil4/examples/posix_echo BUILD_DIR=$(BUILD_DIR) smoke"
	@echo "  make -C sil4/examples/posix_echo BUILD_DIR=$(BUILD_DIR) failover-smoke"
	@echo "  make -C sil4/examples/posix_echo BUILD_DIR=$(BUILD_DIR) run-server"
	@echo "  make -C sil4/examples/posix_echo BUILD_DIR=$(BUILD_DIR) run-client"
	@echo "  make -C sil4/examples/posix_echo BUILD_DIR=$(BUILD_DIR) run-failover-server"
	@echo "  make -C sil4/examples/posix_echo BUILD_DIR=$(BUILD_DIR) run-failover-client"

smoke: $(TARGET)
	BUILD_DIR=$(BUILD_DIR) bash $(SMOKE_SCRIPT)

failover-smoke: $(TARGET)
	BUILD_DIR=$(BUILD_DIR) bash $(FAILOVER_SMOKE_SCRIPT)

run-server: $(TARGET)
	./$(TARGET) server --duration-sec 15

run-client: $(TARGET)
	./$(TARGET) client --duration-sec 10 --message "hello from client"

run-failover-server: $(TARGET)
	./$(TARGET) server --redundant 1 --duration-sec 12 --primary-down-after-rx 3

run-failover-client: $(TARGET)
	./$(TARGET) client --redundant 1 --duration-sec 10 --send-interval-ms 1000 --message "hello during failover" --primary-down-after-rx 3

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
