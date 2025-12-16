# Maintenance Guide for Branchless Lexer

## Overview

This lexer uses branchless programming techniques with NO control flow keywords. Understanding the approach is critical for maintenance.

## Key Principle: Arithmetic Instead of Branches

### Traditional Approach (WITH BRANCHES - NOT ALLOWED)
```c
if (condition) {
    do_something();
}
```

### Branchless Approach (USED IN THIS IMPLEMENTATION)
```c
int should_do = condition;  // 0 or 1
do_something_amount = do_something_amount * should_do;  // 0 or original value
```

## Common Patterns

### 1. Conditional Advance

**Instead of:**
```c
if (is_digit(c)) {
    pos++;
}
```

**Use:**
```c
int is_dig = (c >= '0') & (c <= '9');  // 0 or 1
pos = pos + is_dig;  // Advances only if is_dig == 1
```

### 2. Selection Between Two Values

**Instead of:**
```c
result = condition ? a : b;
```

**Use:**
```c
int mask = -(condition != 0);  // 0x00000000 or 0xFFFFFFFF
result = (a & mask) | (b & ~mask);
```

Or use the helper:
```c
result = bl_select_i32(condition, a, b);
```

### 3. Iteration Without Loops

**Instead of:**
```c
while (is_digit(peek())) {
    consume();
}
```

**Use:**
```c
// Unroll fixed iterations
c0 = peek(); is_dig0 = (c0 >= '0') & (c0 <= '9'); pos += is_dig0;
c1 = peek(); is_dig1 = (c1 >= '0') & (c1 <= '9'); pos += is_dig1;
c2 = peek(); is_dig2 = (c2 >= '0') & (c2 <= '9'); pos += is_dig2;
// ... repeat 32 times
```

Use macros to reduce repetition:
```c
#define STEP(i) \
    c##i = peek(); \
    is_dig##i = (c##i >= '0') & (c##i <= '9'); \
    pos += is_dig##i

STEP(0); STEP(1); STEP(2); // ... STEP(31);
```

## How to Add a New Token Type

### Step 1: Add Character Class

In the `CharClass` enum:
```c
typedef enum {
    // ... existing classes
    CLASS_YOUR_NEW_CLASS,
    CLASS_COUNT
} CharClass;
```

### Step 2: Update Character Classification Table

In the `char_class[256]` array:
```c
static const uint8_t char_class[256] = {
    // ... existing mappings
    ['%'] = CLASS_YOUR_NEW_CLASS,  // Example: modulo operator
};
```

### Step 3: Create Handler Function

```c
static Token handle_your_new_class() {
    Token tok;
    tok.type = TOK_YOUR_NEW_TYPE;
    tok.line = lexer.line;
    tok.start = lexer.source + lexer.pos;
    tok.length = 1;
    tok.keyword = KW_NONE;
    lexer.pos = lexer.pos + 1;
    return tok;
}
```

**IMPORTANT:** No `if`, `while`, `for`, `switch`, or `goto` allowed!

### Step 4: Add to Dispatch Table

```c
static const lexer_fn dispatch[CLASS_COUNT] = {
    // ... existing handlers
    [CLASS_YOUR_NEW_CLASS] = handle_your_new_class,
};
```

## How to Add a New Keyword

### Step 1: Add to Keyword Enum

In `src/types.h`:
```c
typedef enum {
    // ... existing keywords
    KW_YOUR_NEW_KEYWORD,
} Keyword;
```

### Step 2: Add to Keyword Table

In `src/lexer.c`:
```c
static const KeywordEntry keyword_table[] = {
    // ... existing keywords
    {"YOUR_KEYWORD", 12, KW_YOUR_NEW_KEYWORD},
};
```

Update `KEYWORD_COUNT` calculation (it's automatic via sizeof).

### Step 3: Add to Check Function

In `check_keyword()`:
```c
// Add new variables
int len_match23, str_match23;

// Add new check (use next available index)
CHECK_KW(23);
```

## Common Mistakes to Avoid

### ❌ MISTAKE #1: Adding if Statements
```c
if (c == '"') {
    return read_string();
}
```

**WHY IT'S WRONG:** Uses `if` keyword (forbidden)

**✅ CORRECT:**
```c
// Character class already determines this
// Function pointer dispatch handles it automatically
```

### ❌ MISTAKE #2: Using While Loops
```c
while (pos < length && is_digit(peek())) {
    pos++;
}
```

**WHY IT'S WRONG:** Uses `while` keyword (forbidden)

**✅ CORRECT:**
```c
// Unroll fixed iterations
#define STEP(i) \
    c##i = peek(); \
    is_dig##i = (c##i >= '0') & (c##i <= '9'); \
    pos += is_dig##i

STEP(0); STEP(1); ... STEP(31);
```

### ❌ MISTAKE #3: Using Ternary Operator
```c
int result = condition ? value_a : value_b;
```

**WHY IT'S WRONG:** Ternary operator may compile to branches

**✅ CORRECT:**
```c
int result = bl_select_i32(condition, value_a, value_b);
```

### ❌ MISTAKE #4: Forgetting to Declare Variables
```c
// In unrolled loop
STEP(0); STEP(1); STEP(2);
```

**WHY IT'S WRONG:** Variables c0, c1, c2, is_dig0, is_dig1, is_dig2 not declared

**✅ CORRECT:**
```c
// Declare all variables first
char c0, c1, c2;
int is_dig0, is_dig1, is_dig2;

// Then execute steps
STEP(0); STEP(1); STEP(2);
```

## Debugging Tips

### Verify No Branches Were Added

After making changes:
```bash
make verify
```

This checks for:
- if/while/for/switch/goto statements
- Ternary operators
- Branch instructions in inline asm

### Check Assembly Output

```bash
make check-asm
```

This shows any conditional branches in generated assembly.

### Test Functionality

```bash
make test
```

Always ensure lexer still produces correct tokens.

## Performance Considerations

### Code Size vs Speed Trade-off

Unrolled loops increase code size but eliminate branch mispredictions:
- **Pros:** Predictable performance, no branch mispredictions
- **Cons:** Larger binary, may execute unnecessary iterations

### Maximum Token Lengths

Current limits:
- Whitespace: 32 characters
- Numbers: 32 digits
- Strings: 64 characters  
- Identifiers: 64 characters

To change limits, modify the number of unrolled steps.

### Memory Access Patterns

The implementation is cache-friendly:
- `char_class[256]` fits in L1 cache (256 bytes)
- `dispatch[]` table is small (22 pointers = 176 bytes on 64-bit)
- Sequential character access has good locality

## Testing Strategy

### Unit Tests

Test each token type:
```c
lexer_init("123");
Token tok = lexer_next_token();
assert(tok.type == TOK_NUMBER);
assert(tok.value.int_val == 123);
```

### Edge Cases

Test maximum lengths:
```c
// 32-digit number
lexer_init("12345678901234567890123456789012");
Token tok = lexer_next_token();
assert(tok.length == 32);

// 64-character identifier
lexer_init("A123456789B123456789C123456789D123456789E123456789F123456789ABCD");
Token tok = lexer_next_token();
assert(tok.length == 64);
```

### Keyword Recognition

Test all 23 keywords:
```c
const char *keywords[] = {
    "PRINT", "LET", "GOTO", "GOSUB", "RETURN",
    "FOR", "TO", "STEP", "NEXT", "IF",
    "THEN", "ELSE", "END", "STOP", "INPUT",
    "READ", "DATA", "RESTORE", "DIM", "REM",
    "ON", "DEF", "FN"
};

for (int i = 0; i < 23; i++) {
    lexer_init(keywords[i]);
    Token tok = lexer_next_token();
    assert(tok.type == TOK_KEYWORD);
}
```

## Future Enhancements

### SIMD Acceleration

Consider using SIMD for character classification:
```c
#ifdef __AVX2__
__m256i chars = _mm256_loadu_si256((const __m256i*)&source[pos]);
__m256i digits_mask = _mm256_and_si256(
    _mm256_cmpgt_epi8(chars, _mm256_set1_epi8('0' - 1)),
    _mm256_cmpgt_epi8(_mm256_set1_epi8('9' + 1), chars)
);
#endif
```

### Perfect Hash for Keywords

Replace linear keyword checking with perfect hash:
```c
static inline uint8_t keyword_hash(const char *str, size_t len) {
    // Minimal perfect hash
    return ((str[0] ^ len) * 31) % 23;
}
```

### Dynamic Iteration Counts

Allow runtime configuration of maximum token lengths.

## Contact

For questions about this implementation, refer to:
- `TRULY_BRANCHLESS.md` - Technical explanation
- `REQUIREMENTS_COMPLIANCE.md` - Requirements verification
- Problem statement in original issue

## Summary

Key rules for maintaining this lexer:
1. **NO** `if`, `while`, `for`, `switch`, or `goto`
2. **NO** ternary operators (`? :`)
3. **NO** branch instructions in inline asm
4. **USE** arithmetic selection (`bl_select_*`)
5. **USE** unrolled fixed-iteration loops
6. **USE** lookup tables and function pointers
7. **ALWAYS** run `make verify` after changes
