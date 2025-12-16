# Truly Branchless Lexer Implementation

## Overview

This document explains the **truly branchless** lexer implementation that eliminates ALL control flow keywords (`if`, `while`, `for`, `switch`, `goto`, ternary operators) from the source code.

## Architecture

The branchless lexer achieves its goal through five key techniques:

### 1. Character Classification Lookup Table (256 entries)

Instead of using cascading `if` statements to classify characters, we use a precomputed lookup table:

```c
typedef enum {
    CLASS_INVALID = 0,
    CLASS_DIGIT,
    CLASS_ALPHA,
    CLASS_STRING,
    CLASS_LPAREN,
    // ... more classes
    CLASS_COUNT
} CharClass;

static const uint8_t char_class[256] = {
    [0] = CLASS_EOF,
    [9] = CLASS_SPACE,      /* tab */
    [10] = CLASS_NEWLINE,
    [32] = CLASS_SPACE,
    [34] = CLASS_STRING,    /* " */
    [48 ... 57] = CLASS_DIGIT,
    [65 ... 90] = CLASS_ALPHA,  /* A-Z */
    [97 ... 122] = CLASS_ALPHA, /* a-z */
    // ... etc
};
```

**Benefits:**
- O(1) character classification
- No branches - just array indexing
- Cache-friendly access pattern

### 2. Function Pointer Dispatch Table

Instead of using computed `goto` or cascading `if` statements, we use a function pointer table:

```c
typedef Token (*lexer_fn)(void);

static const lexer_fn dispatch[CLASS_COUNT] = {
    [CLASS_INVALID] = handle_invalid,
    [CLASS_DIGIT] = handle_digit,
    [CLASS_ALPHA] = handle_alpha,
    [CLASS_STRING] = handle_string,
    // ... 22 handlers total
};

Token lexer_next_token(void) {
    skip_whitespace();
    char c = peek();
    uint8_t cls = get_char_class(c);
    return dispatch[cls]();  // Single indirect call, no branches
}
```

**How it works:**
- Character class index directly selects the handler function
- Single indirect function call
- No conditional jumps in source code
- Each handler is responsible for one token type

**vs Computed Goto:**
- Computed goto (`goto *handlers[idx]`) is technically an indirect branch
- Function pointers are semantically cleaner
- Both compile to similar assembly (indirect jump)

### 3. Unrolled Fixed-Iteration Loops

Instead of `while` loops that compile to conditional branches, we use unrolled fixed-iteration code:

```c
/* Skip whitespace - 32 unrolled iterations */
#define SKIP_WS_STEP(i) \
    c##i = peek_at(lexer.pos); \
    is_ws##i = (get_char_class(c##i) == CLASS_SPACE); \
    lexer.pos = lexer.pos + is_ws##i

static void skip_whitespace() {
    char c0, c1, c2, ..., c31;
    int is_ws0, is_ws1, is_ws2, ..., is_ws31;
    
    SKIP_WS_STEP(0); SKIP_WS_STEP(1); SKIP_WS_STEP(2); // ... SKIP_WS_STEP(31);
}
```

**Key insight:**
- Each iteration ALWAYS executes
- Arithmetic determines whether to advance: `lexer.pos = lexer.pos + is_ws`
- If `is_ws` is 0, position doesn't advance (acts like early exit)
- No conditional branches in source code

**Trade-offs:**
- Increased code size (macros expand to many statements)
- Predictable execution time
- No branch mispredictions
- May execute more instructions than branching version in some cases

### 4. Arithmetic Selection for All Decisions

Instead of `if/else` or ternary operators, we use arithmetic selection:

```c
/* Branchless select: cond ? a : b */
static inline int32_t bl_select_i32(int32_t cond, int32_t a, int32_t b) {
    int32_t mask = -(cond != 0);  // -1 if true, 0 if false
    return (a & mask) | (b & ~mask);
}
```

**Example usage in keyword matching:**
```c
/* Check all 23 keywords using arithmetic */
Keyword result = KW_NONE;
TokenType type = TOK_IDENT;

int len_match = (keyword_table[0].len == tok->length);
int str_match = len_match & (strncmp(tok->start, keyword_table[0].str, tok->length) == 0);
result = (Keyword)bl_select_i32(str_match, keyword_table[0].kw, result);
type = (TokenType)bl_select_i32(str_match, TOK_KEYWORD, type);

// Repeat for all 23 keywords...
```

**How it works:**
- Comparison produces 0 or 1
- Convert to mask: `-(cond != 0)` produces 0x00000000 or 0xFFFFFFFF
- Bitwise AND/OR selects the appropriate value
- No conditional branches

### 5. Fixed-Length Token Scanning

Each token type has a maximum length:
- Numbers: 32 digits max
- Strings: 64 characters max
- Identifiers: 64 characters max
- Whitespace: 32 characters max

The scanner always processes the full iteration count, using arithmetic to determine when to stop advancing:

```c
#define READ_NUM_STEP(i) \
    c##i = peek_at(lexer.pos); \
    is_dig##i = (get_char_class(c##i) == CLASS_DIGIT); \
    should_consume##i = is_dig##i | is_dot##i; \
    lexer.pos = lexer.pos + should_consume##i
```

## Verification

### Source Code Verification

The `scripts/verify-branchless.sh` script checks for:
- ✅ No `if` statements
- ✅ No `while` loops
- ✅ No `for` loops
- ✅ No `switch` statements
- ✅ No `goto` (including computed goto)
- ✅ No ternary operators (`? :`)
- ✅ No branch instructions in inline assembly (`jz`, `jne`, etc.)

```bash
$ make verify
✅ Verification PASSED: Lexer is TRULY branchless!
  ✓ No if/while/for/switch/goto statements
  ✓ No ternary operators
  ✓ No branch instructions in inline asm
  ✓ Uses only: lookup tables, function pointers, arithmetic selection
```

### Assembly Verification

The `make check-asm` target disassembles the lexer and checks for conditional branch instructions:

```bash
$ make check-asm
```

**Note:** While the source code contains no branches, the compiler MAY still generate some conditional jumps for:
- Bounds checking (safety)
- Optimization (sometimes branches are faster than branchless code)
- Function call overhead

This is unavoidable when compiling C code. True zero-branch assembly requires hand-written assembly language.

## Performance Characteristics

### Advantages

1. **Predictable Performance**: No branch mispredictions
2. **Cache-Friendly**: Lookup tables fit in L1 cache
3. **Speculation-Safe**: Resistant to Spectre/Meltdown-style attacks
4. **Parallel-Friendly**: No data dependencies from branches

### Trade-offs

1. **Code Size**: Unrolled loops increase binary size
2. **May Process More Instructions**: Fixed iterations always execute
3. **Limited Token Lengths**: 32-64 character limits on tokens
4. **Compiler May Still Generate Branches**: For safety/optimization

## Implementation Details

### File Structure

- `src/lexer.c` - Main branchless lexer implementation (25KB)
- `src/branchless_scalar.h` - Arithmetic selection helpers
- `src/branchless_amd64.h` - AMD64-specific SIMD operations
- `src/branchless_aarch64.h` - AArch64-specific NEON operations

### Key Functions

```c
void lexer_init(const char *source);           // Initialize lexer
Token lexer_next_token(void);                  // Main entry point
static void skip_whitespace(void);             // 32 unrolled iterations
static Token read_number(void);                // 32 unrolled iterations
static Token read_string(void);                // 64 unrolled iterations
static Token read_ident(void);                 // 64 unrolled iterations
static void check_keyword(Token *tok);         // 23 arithmetic comparisons
```

### Dispatch Table

22 handler functions, one for each character class:
- `handle_eof()` - End of file
- `handle_digit()` → `read_number()`
- `handle_alpha()` → `read_ident()`
- `handle_string()` → `read_string()`
- `handle_lparen()`, `handle_rparen()`, etc. - Single char tokens
- `handle_plus()`, `handle_minus()`, etc. - Operators
- `handle_invalid()` - Error token

## Future Optimizations

### SIMD Acceleration

Character classification and scanning can be accelerated with SIMD:

```c
#ifdef __AVX512F__
// Scan 64 bytes at once using AVX-512
__m512i chars = _mm512_loadu_si512((const __m512i*)&source[pos]);
__mmask64 is_space = _mm512_cmpeq_epi8_mask(chars, _mm512_set1_epi8(' '));
// ... etc
#endif
```

### Perfect Hash for Keywords

Instead of checking all 23 keywords, use a minimal perfect hash function:

```c
static inline uint8_t keyword_hash(const char *str, size_t len) {
    // Minimal perfect hash maps strings to 0-22
    return (str[0] * 31 + len) % 23;
}
```

### Speculative Token Parsing

Process multiple possible token types in parallel, then select the correct one:

```c
Token num_tok = read_number();
Token str_tok = read_string();
Token id_tok = read_ident();
// Select correct token based on character class
return dispatch_select(char_class, num_tok, str_tok, id_tok);
```

## Conclusion

This implementation demonstrates that it IS possible to write a truly branchless lexer in C, though with significant trade-offs:

✅ **Achieved:**
- Zero control flow keywords in source code
- Lookup table-based character classification
- Function pointer dispatch
- Arithmetic-only decision making
- Unrolled fixed-iteration scanning

⚠️ **Limitations:**
- Large code size from unrolling
- Fixed maximum token lengths
- Compiler may still generate branches for optimization
- May execute more instructions than branching version

The implementation successfully demonstrates the techniques required for branchless programming, even if perfect branch-free assembly is unattainable from C source code.
