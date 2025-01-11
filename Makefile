BUILD_DIR := build

cryton: $(BUILD_DIR)/cryton

$(BUILD_DIR)/cryton: *.c *.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(wildcard *.c) -o $@

clean:
	rm -rf $(BUILD_DIR)
