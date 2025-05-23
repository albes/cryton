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

INDENT = "   "
EXECUTABLE = "./build/cryton"
TEST_DIR = "tests"
VALGRIND_MODE = "--valgrind" in sys.argv


def format_block(header, content):
    print(f"{INDENT}{CYAN}{header}:{RESET}")
    print(textwrap.indent(content.strip(), INDENT * 2))


def extract_expected_output(test_file):
    expected_lines = []
    expected_error = None
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
            elif stripped.startswith("# EXPECT ERROR:"):
                expected_error = stripped[len("# EXPECT ERROR:"):].strip()
            elif in_block and stripped.startswith("#"):
                expected_lines.append(stripped[1:].lstrip())

    return "\n".join(expected_lines), expected_error


def parse_valgrind_leaks(stderr_output):
    leaks = {
        "definitely lost": 0,
        "indirectly lost": 0,
        "possibly lost": 0,
        "still reachable": 0,
    }

    for line in stderr_output.splitlines():
        for key in leaks:
            if key in line:
                parts = line.split(":")
                if len(parts) > 1:
                    value_str = parts[1].strip().split(" ")[0].replace(",", "")
                    try:
                        leaks[key] = int(value_str)
                    except ValueError:
                        pass
    return leaks


def run_valgrind(test_file):
    result = subprocess.run([
        "valgrind", "--leak-check=full", "--error-exitcode=99", EXECUTABLE, test_file
    ], capture_output=True, text=True)

    stderr = result.stderr.strip()
    leaks = parse_valgrind_leaks(stderr)

    has_errors = (
        leaks["definitely lost"] > 0 or
        "Invalid read" in stderr or
        "uninitialised" in stderr
    )

    if has_errors:
        print(f"{RED}[MEMORY ERROR]{RESET} {os.path.relpath(test_file, TEST_DIR)}")
        format_block("Valgrind Leak Summary", f"Definitely lost: {leaks['definitely lost']} bytes")
        format_block("Valgrind Output", stderr)
        return False
    else:
        print(f"{GREEN}[MEMORY OK]{RESET} {os.path.relpath(test_file, TEST_DIR)}")
        return True


def run_test(test_file):
    if VALGRIND_MODE:
        return run_valgrind(test_file)

    expected_output, expected_error = extract_expected_output(test_file)

    result = subprocess.run([EXECUTABLE, test_file], capture_output=True, text=True, timeout=5)
    actual_output = result.stdout.strip().replace('\r\n', '\n')
    stderr_output = result.stderr.strip()

    if expected_error:
        if expected_error in stderr_output:
            print(f"{GREEN}[EXPECTED ERROR]{RESET} {os.path.relpath(test_file, TEST_DIR)}")
            return True
        else:
            print(f"{RED}[UNEXPECTED PASS/ERROR]{RESET} {os.path.relpath(test_file, TEST_DIR)}")
            format_block("Expected error", expected_error)
            format_block("Got", stderr_output or "(no error)")
            return False

    expected_output = expected_output.strip().replace('\r\n', '\n')
    if actual_output == expected_output and stderr_output == "":
        print(f"{GREEN}[PASS]{RESET} {os.path.relpath(test_file, TEST_DIR)}")
        return True
    else:
        print(f"{RED}[FAIL]{RESET} {os.path.relpath(test_file, TEST_DIR)}")
        format_block("Expected", expected_output)
        format_block("Got", actual_output)
        if stderr_output:
            format_block("Error output", stderr_output)
        return False


def main():
    total = 0
    passed = 0
    failed = 0

    for root, _, files in os.walk(TEST_DIR):
        for filename in sorted(files):
            if filename.endswith(".py"):
                test_file = os.path.join(root, filename)
                result = run_test(test_file)
                if result is None:
                    continue
                total += 1
                if result:
                    passed += 1
                else:
                    failed += 1

    mode_msg = "Memory check (Valgrind)" if VALGRIND_MODE else "Functional test"
    print(f"\n{CYAN}{mode_msg} result: {passed}/{total} passed, {failed} failed.{RESET}")
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
