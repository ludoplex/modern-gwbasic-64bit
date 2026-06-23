# Modern GW-BASIC 64-bit - Feature List

## ✅ Implemented Features (v0.2)

### Control Flow
- **IF-THEN**: Conditional execution with comparison operators
  - Operators: `=`, `<>`, `<`, `>`, `<=`, `>=`
  - Direct statement execution or line number jumps
- **GOTO**: Unconditional jump to line number
- **GOSUB/RETURN**: Subroutine calls with return stack

### Loops
- **FOR-NEXT**: Counted loops with STEP support
  - Example: `FOR I = 1 TO 10 STEP 2`
  - Nested loops supported

### Variables & Data
- **LET**: Variable assignment (explicit or implicit)
- **DIM**: Array dimensioning (up to 8 dimensions)
- **Types**: Integer, Double, String
- Variable name support: alphanumeric + `$` for strings

### I/O
- **PRINT**: Output expressions, variables, strings
- **INPUT**: User input with optional prompts
  - Example: `INPUT "Enter name"; NAME$`
- **REM**: Comments

### Math Functions (Assembly-Optimized)
- **SIN(x)**: Sine function
- **COS(x)**: Cosine function
- **TAN(x)**: Tangent function
- **SQR(x)**: Square root
- **ABS(x)**: Absolute value
- **INT(x)**: Integer truncation
- **RND(x)**: Random number (0-1)

### String Functions
- **LEN(s$)**: String length
- **LEFT$(s$, n)**: Left n characters
- **RIGHT$(s$, n)**: Right n characters
- **MID$(s$, start, len)**: Middle substring
- **ASC(c)**: ASCII value of character

### Expressions
- **Arithmetic**: `+`, `-`, `*`, `/`
- **String concatenation**: `+`
- **Operator precedence**: Proper order of operations
- **Parentheses**: Grouping support

### Performance Features
- **Assembly core**: All arithmetic uses native x86_64/ARM64 instructions
- **Direct calls**: No vtable overhead for functions
- **Minimal allocations**: String operations use memcpy
- **Fast loops**: 1000 iterations in ~2ms

## 📋 Planned Features (Phase 2)

### Arrays
- [ ] Array subscripting in expressions
  - Example: `A(5) = 10`
- [ ] Multi-dimensional arrays
- [ ] Array copying operations

### Additional Loops
- [ ] WHILE-WEND loops
- [ ] DO-LOOP variants

### More Functions
- [ ] **CHR$(n)**: Character from ASCII
- [ ] **STR$(n)**: Number to string
- [ ] **VAL(s$)**: String to number
- [ ] **INSTR**: Find substring
- [ ] **ATN**: Arctangent
- [ ] **LOG**: Natural logarithm
- [ ] **EXP**: Exponential

### File I/O
- [ ] **OPEN**: Open file for I/O
- [ ] **CLOSE**: Close file
- [ ] **INPUT#**: Read from file
- [ ] **PRINT#**: Write to file
- [ ] **EOF**: End of file test
- [ ] **LINE INPUT**: Read line from file

### Graphics Commands
- [ ] **SCREEN**: Set graphics mode
- [ ] **PSET**: Set pixel
- [ ] **LINE**: Draw line
- [ ] **CIRCLE**: Draw circle
- [ ] **PAINT**: Fill area
- [ ] **CLS**: Clear screen
- [ ] **COLOR**: Set color

### Sound
- [ ] **SOUND**: Generate tone
- [ ] **PLAY**: Play music string
- [ ] **BEEP**: System beep

### Memory Operations (for speed)
- [ ] **PEEK**: Read memory byte
- [ ] **POKE**: Write memory byte
- [ ] **DEF SEG**: Set memory segment

### System
- [ ] **SYSTEM**: Exit to OS
- [ ] **SHELL**: Execute OS command
- [ ] **TIMER**: Get system timer

## 🎯 Performance Goals

Following GW-BASIC's speed philosophy:

### Achieved
✅ Direct memory operations (no heap for integers)
✅ Minimal abstraction (direct function calls)
✅ Fast arithmetic (assembly-optimized)
✅ Unbuffered output (printf for speed)

### In Progress
- [ ] String pooling (reduce allocations)
- [ ] Array direct access (pointer arithmetic)
- [ ] Inline critical operations
- [ ] Loop unrolling for nested loops

## 🔧 Platform Support

### Current
- ✅ Linux AMD64 (x86_64)
- ✅ Linux ARM64 (AArch64)

### Planned
- [ ] Windows 11 (native or Cosmopolitan C)
- [ ] macOS (Intel and Apple Silicon)
- [ ] Cross-compilation support

## 📊 Benchmark Targets

Goal: Beat Rust on GW-BASIC's traditional strengths

| Operation | Target | Current | Status |
|-----------|--------|---------|--------|
| FOR loop (1000 iter) | <5ms | ~2ms | ✅ |
| String concat (1000x) | <10ms | TBD | 🔄 |
| Math functions (1000x) | <5ms | ~2ms | ✅ |
| Array access (1000x) | <1ms | TBD | 🔄 |

## 🚀 Usage Examples

See `examples/` directory for complete programs:
- `comprehensive_demo.bas` - Full feature demonstration
- `performance.bas` - Speed test
- `for_loop.bas` - Loop examples
- `if_then.bas` - Conditionals
- `goto_gosub.bas` - Control flow
- `math_functions.bas` - Math operations
- `string_functions.bas` - String manipulation

## 📝 Notes

This interpreter prioritizes:
1. **Speed** over safety (like original GW-BASIC)
2. **Direct operations** over abstraction
3. **Assembly optimization** for critical paths
4. **Minimal allocations** for performance
5. **Simple, fast code** over complex optimizations
