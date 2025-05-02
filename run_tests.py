import subprocess
import os
import sys
import textwrap

# ANSI color codes
GREEN = "\033[92m"
RED = "\033[91m"
YELLOW = "\033[93m"
CYAN = "\033[96m"
RESET = "\033[0m"

# Indentation
INDENT = "    "

# Settings
EXECUTABLE = "./build/cryton"
TEST_DIR = "tests"
EXPECTED_OUTPUT_DIR = os.path.join(TEST_DIR, "expected_outputs")

def format_block(header, content):
    print(f"{INDENT}{CYAN}{header}:{RESET}")
    print(textwrap.indent(content, INDENT * 2))

def run_test(test_file, expected_file):
    try:
        result = subprocess.run([EXECUTABLE, test_file], capture_output=True, text=True, timeout=5)
    except subprocess.TimeoutExpired:
        print(f"{RED}[TIMEOUT]{RESET} {os.path.basename(test_file)}")
        return False

    with open(expected_file, 'r') as f:
        expected_output = f.read().strip().replace('\r\n', '\n')

    actual_output = result.stdout.strip().replace('\r\n', '\n')

    if actual_output == expected_output:
        print(f"{GREEN}[PASS]{RESET} {os.path.basename(test_file)}")
        return True
    else:
        print(f"{RED}[FAIL]{RESET} {os.path.basename(test_file)}")
        format_block("Expected", expected_output)
        format_block("Got", actual_output)
        if result.stderr.strip():
            format_block("Error output", result.stderr.strip())
        return False

def main():
    total = 0
    passed = 0
    failed = 0

    for filename in os.listdir(TEST_DIR):
        if filename.endswith(".py"):
            test_file = os.path.join(TEST_DIR, filename)
            expected_file = os.path.join(EXPECTED_OUTPUT_DIR, filename.replace(".py", ".out"))
            total += 1
            if os.path.exists(expected_file):
                if run_test(test_file, expected_file):
                    passed += 1
                else:
                    failed += 1
            else:
                print(f"{YELLOW}[MISSING]{RESET} {filename} — expected output file not found.")
                failed += 1  # Count missing output as a failed test

    print(f"\n{CYAN}{passed}/{total} tests passed, {failed} failed.{RESET}")

    sys.exit(1 if failed else 0)

if __name__ == "__main__":
    main()
