# Example GW-BASIC Programs

This directory contains example programs to demonstrate the branchless GW-BASIC interpreter.

## Programs

### hello.bas
Basic "Hello, World!" program demonstrating simple PRINT statements.

```bash
./gwbasic examples/hello.bas
```

### counting.bas
Simple counting demonstration.

```bash
./gwbasic examples/counting.bas
```

### multiline.bas
Multiple PRINT statements to show program execution flow.

```bash
./gwbasic examples/multiline.bas
```

## Running Examples

From the project root directory:

```bash
make
./gwbasic examples/hello.bas
```

Or run all examples:

```bash
for f in examples/*.bas; do
    echo "Running $f:"
    ./gwbasic "$f"
    echo
done
```
