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

def format_block(header, content):
    print(f"{INDENT}{CYAN}{header}:{RESET}")
    print(textwrap.indent(content, INDENT * 2))


def extract_expected_output(test_file):
    expected_lines = []
    in_block = False
    with open(test_file, 'r') as f:
        for line in f:
            stripped = line.strip()
            if stripped == "# EXPECT START":
                in_block = True
                continue
            elif stripped == "# EXPECT END":
                in_block = False
                continue
            elif stripped.startswith("# EXPECT:") and not in_block:
                expected_lines.append(stripped[len("# EXPECT:"):].strip())
            elif in_block and stripped.startswith("#"):
                expected_lines.append(stripped[1:].lstrip())
    return "\n".join(expected_lines)


def run_test(test_file):
    expected_output = extract_expected_output(test_file)
    if not expected_output:
        print(f"{YELLOW}[SKIPPED]{RESET} {os.path.basename(test_file)} — no EXPECT comment found.")
        return None

    try:
        result = subprocess.run([EXECUTABLE, test_file], capture_output=True, text=True, timeout=5)
    except subprocess.TimeoutExpired:
        print(f"{RED}[TIMEOUT]{RESET} {os.path.basename(test_file)}")
        return False

    actual_output = result.stdout.strip().replace('\r\n', '\n')
    expected_output = expected_output.strip().replace('\r\n', '\n')

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
            result = run_test(test_file)
            if result is None:
                continue  # Skipped
            total += 1
            if result:
                passed += 1
            else:
                failed += 1

    print(f"\n{CYAN}{passed}/{total} tests passed, {failed} failed.{RESET}")
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
