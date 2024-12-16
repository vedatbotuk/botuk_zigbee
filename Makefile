# Path to the ESP-IDF directory
IDF_PATH := $(HOME)/esp/esp-idf

# Project name
PROJECT_NAME := zigbee_with_esp32h2

# Output directory for the build
BUILD_DIR := build

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

# Build and flash in one step
flash_monitor: flash monitor

# Remove the build directory completely
fullclean:
	@echo "Removing build directory and related files..."
	@rm -rf $(BUILD_DIR) ota managed_components || true
	@rm -f sdkconfig sdkconfig.old sdkconfig.defaults dependencies.lock || true
	@echo "Clean up completed."

.PHONY: all build flash monitor clean distclean flash_monitor include_idf_env
