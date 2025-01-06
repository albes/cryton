BUILD_DIR := build

cryton: $(BUILD_DIR)/cryton

$(BUILD_DIR)/cryton: *.c
	mkdir -p $(BUILD_DIR)
	$(CC) $^ -o $@

clean:
	rm -rf $(BUILD_DIR)
