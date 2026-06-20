.PHONY: configure benchmark test

BUILD_DIR := build

configure:
	cmake -S . -B $(BUILD_DIR)

benchmark: configure
	cmake --build $(BUILD_DIR) --target benchmark

test: configure
	cmake --build $(BUILD_DIR)
	ctest --test-dir $(BUILD_DIR) --output-on-failure
