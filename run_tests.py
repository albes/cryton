import subprocess
import os

# Settings
EXECUTABLE = "./build/cryton"  # <-- Change to your compiled C interpreter path
TEST_FILES = ["tests/testcase1.py", "tests/testFibonacci.py", "tests/testMulitplication.py", "tests/testIfInsideWhile.py"]
EXPECTED_OUTPUTS = ["tests/expected_outputs/testcase1.out",
                    "tests/expected_outputs/testFibonacci.out",
                    "tests/expected_outputs/testMulitplication.out",
                    "tests/expected_outputs/testIfInsideWhile.out"]

def run_test(test_file, expected_file):
    # Run your interpreter
    try:
        result = subprocess.run([EXECUTABLE, test_file], capture_output=True, text=True, timeout=5)
    except subprocess.TimeoutExpired:
        print(f"{test_file}: TIMEOUT ❌")
        return

    # Load expected output
    with open(expected_file, 'r') as f:
        expected_output = f.read().strip()

    # Get actual output
    actual_output = result.stdout.strip()

    # Compare
    if actual_output == expected_output:
        print(f"{os.path.basename(test_file)}: PASS ✅")
    else:
        print(f"{os.path.basename(test_file)}: FAIL ❌")
        print("Expected:")
        print(expected_output)
        print("Got:")
        print(actual_output)

def main():
    for test_file, expected_file in zip(TEST_FILES, EXPECTED_OUTPUTS):
        run_test(test_file, expected_file)

if __name__ == "__main__":
    main()
