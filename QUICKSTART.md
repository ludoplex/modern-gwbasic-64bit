# Quick Start Guide - Modern GW-BASIC 64-bit

## Build

```bash
make
```

The Makefile automatically detects your architecture (AMD64 or ARM64) and builds accordingly.

## Run a Program

```bash
./gwbasic examples/demo.bas
```

## Interactive Mode

```bash
./gwbasic
```

Then enter:
```basic
10 PRINT "Hello!"
20 END
RUN
```

## Available Commands

In interactive mode:
- `LIST` - Show program
- `RUN` - Execute program
- `NEW` - Clear program
- `HELP` - Show help
- `QUIT` or `EXIT` - Exit interpreter

## Example Programs

- `examples/hello.bas` - Simple hello world
- `examples/demo.bas` - Full demonstration
- `examples/variables.bas` - Variable usage
- `examples/string.bas` - String handling
- `examples/test.bas` - Comprehensive test

## Supported Statements

- `PRINT "text"` or `PRINT variable`
- `LET variable = value` or `variable = value`
- `REM comment`
- `END`

## Variable Types

- Integers: `X = 42`
- Strings: `NAME = "Hello"`

## Architecture

The interpreter uses assembly-optimized operations for:
- Integer arithmetic (ADD, SUB, MUL, DIV)
- Floating-point operations (using SIMD)
- 64-bit native register usage

## Documentation

- `INTERPRETER.md` - Full user manual
- `SUMMARY.md` - Implementation details
- `README.md` - Repository overview

## Clean Build

```bash
make clean
make
```
