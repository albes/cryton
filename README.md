# Cryton

Cryton is simple interpreter for a custom, Python-like language that supports simple categories.

## Build instructions:

Make sure you have a [GCC](https://gcc.gnu.org/) installed.

If you have `make` installed on your system, just run this command from the root directory:

```shell
make
```

Otherwise run:

```shell
mkdir build
gcc bigint.c interpreter.c main.c object.c parser.c scanner.c table.c value.c -o build/cryton -lreadline
```

## Run the interpreter:

To run code in a file:

```shell
./build/cryton ./CodeExamples/Example_1.py
```

To start the interpreter in interactive mode (REPL), run:

```shell
./build/cryton
```
