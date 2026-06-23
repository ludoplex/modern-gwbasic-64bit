# Modern GW-BASIC 64-bit Interpreter

A modern reimplementation of the GW-BASIC interpreter designed for 64-bit architectures (AMD64 and ARM64), featuring an assembly-optimized core engine for maximum performance.

## Features

- **64-bit Native Support**: Designed from the ground up for 64-bit architectures
- **Assembly-Optimized Core**: Critical arithmetic operations implemented in native assembly for both AMD64 and ARM64
- **Multi-Architecture**: Supports both x86_64 (AMD64) and AArch64 (ARM64) processors
- **GW-BASIC Compatible**: Implements core GW-BASIC language features
- **Interactive Mode**: Classic line-by-line interactive programming experience
- **File Execution**: Load and run BASIC programs from files

## Architecture

The interpreter consists of:

- **C Core**: High-level program management, parsing, and control flow written in C
- **Assembly Engine**: Low-level arithmetic and data operations in architecture-specific assembly
  - `asm_core_amd64.S`: AMD64/x86_64 optimized operations
  - `asm_core_arm64.S`: ARM64/AArch64 optimized operations

### Assembly-Optimized Operations

The following operations are implemented in native assembly for performance:

- Integer arithmetic (ADD, SUB, MUL, DIV) using 64-bit registers
- Double-precision floating-point operations using SIMD instructions
- Zero-overhead function calls using native calling conventions

## Building

### Prerequisites

- GCC or Clang compiler
- GNU Assembler (as)
- Make

### Compilation

```bash
make
```

The Makefile automatically detects your architecture and builds the appropriate assembly module.

### Architecture Detection

The build system automatically detects:
- **AMD64** (x86_64): Uses `asm_core_amd64.S`
- **ARM64** (aarch64/arm64): Uses `asm_core_arm64.S`

## Usage

### Interactive Mode

```bash
./gwbasic
```

In interactive mode, you can:
- Enter numbered lines to build a program
- Use `LIST` to view your program
- Use `RUN` to execute your program
- Use `NEW` to clear the program
- Use `HELP` to see available commands

Example session:
```
Ok
10 PRINT "Hello, World!"
Ok
20 END
Ok
RUN
Hello, World!
Ok
```

### Running a Program File

```bash
./gwbasic examples/hello.bas
```

## Supported BASIC Statements

Currently implemented:

- `PRINT` - Output text or variables
- `LET` - Assign values to variables (optional keyword)
- `END` - End program execution

### Variables

- Integer variables: `X = 10`
- String variables: `NAME = "Hello"`
- Implicit LET: `X = 5` (without LET keyword)

## Example Programs

### Hello World
```basic
10 PRINT "Hello, World!"
20 END
```

### Variables
```basic
10 LET X = 10
20 LET Y = 20
30 PRINT X
40 PRINT Y
50 END
```

### Strings
```basic
10 LET MESSAGE = "Welcome to GW-BASIC!"
20 PRINT MESSAGE
30 END
```

## Project Structure

```
.
├── src/
│   ├── main.c              # Main interpreter loop
│   ├── program.c           # Program management and execution
│   ├── symbol_table.c      # Variable storage and retrieval
│   ├── asm_core_amd64.S    # AMD64 assembly optimizations
│   └── asm_core_arm64.S    # ARM64 assembly optimizations
├── include/
│   └── gwbasic.h           # Header file with data structures
├── examples/
│   ├── hello.bas           # Hello World example
│   ├── variables.bas       # Variable example
│   └── string.bas          # String example
└── Makefile                # Build system with arch detection
```

## Technical Details

### AMD64 Implementation

The AMD64 version uses:
- 64-bit general-purpose registers (RAX, RDI, RSI, etc.)
- SSE2 scalar double-precision instructions (ADDSD, SUBSD, MULSD, DIVSD)
- System V AMD64 ABI calling convention

### ARM64 Implementation

The ARM64 version uses:
- 64-bit general-purpose registers (X0, X1, etc.)
- NEON double-precision floating-point instructions (FADD, FSUB, FMUL, FDIV)
- ARM64 calling convention

### Performance

By implementing critical operations in assembly, the interpreter achieves:
- Zero function call overhead for arithmetic
- Native register usage without C abstractions
- SIMD instruction utilization for floating-point operations
- Optimal code generation for 64-bit architectures

## Future Enhancements

Planned features for future releases:

- [ ] Control flow statements (IF-THEN-ELSE, GOTO, GOSUB)
- [ ] Loops (FOR-NEXT, WHILE-WEND)
- [ ] Arrays and subscripting
- [ ] Built-in mathematical functions (SIN, COS, TAN, etc.)
- [ ] File I/O operations
- [ ] Graphics commands (LINE, CIRCLE, PSET)
- [ ] Sound generation (SOUND, PLAY)
- [ ] Full GW-BASIC tokenization format support
- [ ] Optimization of string operations in assembly

## Contributing

This is part of the Modern GW-BASIC 64-bit project. Contributions are welcome!

## License

This interpreter is created as part of the modern-gwbasic-64bit repository, which preserves and modernizes GW-BASIC programs and resources.

## References

- Original GW-BASIC by Microsoft
- [GW-BASIC User's Guide](http://antonis.de/qbebooks/index.htm#gwbasman)
- System V AMD64 ABI
- ARM Architecture Reference Manual
