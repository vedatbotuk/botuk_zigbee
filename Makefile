# Path to the ESP-IDF directory
IDF_PATH := $(HOME)/esp/esp-idf

# Project name
PROJECT_NAME := zigbee_with_esp32h2

# Output directory for the build
BUILD_DIR := build
TEST_BUILD_DIR := tests/build

# Default serial port (can be overridden)
ESPPORT ?= /dev/ttyUSB0

# Default target: build the project
all: build flash monitor

# Include ESP-IDF environment variables
include_idf_env:
	@. $(IDF_PATH)/export.sh

# Build the project
build: include_idf_env
	idf.py set-target esp32h2
	idf.py build

# Flash the project to the device
flash: include_idf_env
	idf.py -p $(ESPPORT) flash

# Monitor the serial output
monitor: include_idf_env
	idf.py -p $(ESPPORT) monitor

# Clean the build directory
clean: include_idf_env
	idf.py fullclean

# Build and run host-side tests without touching the ESP-IDF build
test:
	cmake -S tests -B $(TEST_BUILD_DIR) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	cmake --build $(TEST_BUILD_DIR)
	ctest --test-dir $(TEST_BUILD_DIR) --output-on-failure

# Remove only the host test build; clean and fullclean preserve it
test-clean:
	rm -rf $(TEST_BUILD_DIR)

# Build and flash in one step
flash_monitor: flash monitor

# Remove the build directory completely
fullclean:
	@echo "Removing build directory and related files..."
	@rm -rf $(BUILD_DIR) ota managed_components || true
	@rm -f sdkconfig sdkconfig.old sdkconfig.defaults dependencies.lock || true
	@echo "ESP-IDF cleanup completed. Preserved $(TEST_BUILD_DIR)."

.PHONY: all build flash monitor clean test test-clean distclean flash_monitor include_idf_env
