BUILD_DIR := build
CCOMP_DIR := ccomp
DEBUG_DIR := debug

cryton: $(BUILD_DIR)/cryton

$(BUILD_DIR)/cryton: *.c *.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(wildcard *.c) -o $@ -lreadline

ccomp: $(CCOMP_DIR)/cryton

$(CCOMP_DIR)/cryton: *.c *.h
	mkdir -p $(CCOMP_DIR)
	ccomp -fstruct-passing $(wildcard *.c) -o $@ -lreadline

debug: $(DEBUG_DIR)/cryton

$(DEBUG_DIR)/cryton: *.c *.h
	mkdir -p $(DEBUG_DIR)
	$(CC) -g $(wildcard *.c) -o $@ -lreadline

test: cryton
	python3 run_tests.py

test-valgrind: cryton
	python3 run_tests.py --valgrind

test-all: cryton
	@ (python3 run_tests.py | tail -1) && (python3 run_tests.py --valgrind | tail -1)

clean:
	rm -rf $(BUILD_DIR) $(CCOMP_DIR)
