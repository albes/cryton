BUILD_DIR := build
CCOMP_DIR := ccomp

cryton: $(BUILD_DIR)/cryton

$(BUILD_DIR)/cryton: *.c *.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(wildcard *.c) -o $@

ccomp: $(CCOMP_DIR)/cryton

$(CCOMP_DIR)/cryton: *.c *.h
	mkdir -p $(CCOMP_DIR)
	ccomp -fstruct-passing $(wildcard *.c) -o $@

clean:
	rm -rf $(BUILD_DIR) $(CCOMP_DIR)
