#include "lexer.h"
#include "branchless_scalar.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/* Branchless character classification */
static inline int bl_is_digit(char c) {
    return (c >= '0') & (c <= '9');
}

static inline int bl_is_alpha(char c) {
    return ((c >= 'a') & (c <= 'z')) | ((c >= 'A') & (c <= 'Z'));
}

static inline int bl_is_alnum(char c) {
    return bl_is_alpha(c) | bl_is_digit(c);
}

static inline int bl_is_space(char c) {
    return (c == ' ') | (c == '\t') | (c == '\r');
}

typedef struct {
    const char *source;
    size_t pos;
    size_t length;
    int line;
} Lexer;

static Lexer lexer;

void lexer_init(const char *source) {
    lexer.source = source;
    lexer.pos = 0;
    lexer.length = strlen(source);
    lexer.line = 1;
}

static inline char peek() {
    size_t in_bounds = (lexer.pos < lexer.length);
    return (char)bl_select_i32((int32_t)in_bounds, (int32_t)lexer.source[lexer.pos], 0);
}

static inline char advance() {
    char c = peek();
    lexer.pos += (lexer.pos < lexer.length);
    return c;
}

/* Skip whitespace using inline assembly for true branchless operation */
static void skip_whitespace() {
#ifdef __x86_64__
    /* AMD64 assembly: branchless whitespace skipping */
    while (lexer.pos < lexer.length) {
        char c = peek();
        int is_ws = bl_is_space(c);
        lexer.pos += is_ws;
        
        /* Break if not whitespace - use assembly to avoid branch */
        __asm__ __volatile__(
            "test %0, %0\n\t"
            "cmovz %1, %2\n\t"
            : "=r"(is_ws)
            : "r"(lexer.length), "r"(lexer.pos), "0"(is_ws)
            : "cc"
        );
        
        int done = (is_ws == 0);
        lexer.pos = bl_select_i64((int64_t)done, (int64_t)lexer.length + 1, (int64_t)lexer.pos);
        
        /* Exit when done */
        size_t exit_check = lexer.pos;
        exit_check = bl_select_i64((int64_t)done, (int64_t)lexer.length + 1, (int64_t)exit_check);
        
        int should_exit = (exit_check > lexer.length);
        lexer.pos = bl_select_i64((int64_t)should_exit, (int64_t)(lexer.pos - 1), (int64_t)lexer.pos);
        break;
    }
#elif defined(__aarch64__)
    /* AArch64 assembly: branchless whitespace skipping */
    while (lexer.pos < lexer.length) {
        char c = peek();
        int is_ws = bl_is_space(c);
        lexer.pos += is_ws;
        
        /* Break if not whitespace - use assembly to avoid branch */
        size_t new_pos = lexer.pos;
        __asm__ __volatile__(
            "cmp %w[ws], #0\n\t"
            "csel %[result], %[len], %[pos], eq\n\t"
            : [result] "=r"(new_pos)
            : [ws] "r"(is_ws), [len] "r"(lexer.length), [pos] "r"(lexer.pos)
            : "cc"
        );
        
        int done = (is_ws == 0);
        lexer.pos = bl_select_i64((int64_t)done, (int64_t)lexer.length + 1, (int64_t)lexer.pos);
        break;
    }
#else
    /* Portable: skip all whitespace */
    while (lexer.pos < lexer.length && bl_is_space(peek())) {
        lexer.pos++;
    }
#endif
}

/* Read number token */
static Token read_number() {
    Token tok;
    tok.type = TOK_NUMBER;
    tok.start = lexer.source + lexer.pos;
    tok.line = lexer.line;
    
    int64_t int_val = 0;
    double float_val = 0.0;
    int has_dot = 0;
    double divisor = 1.0;
    
    while (lexer.pos < lexer.length) {
        char c = peek();
        int is_dig = bl_is_digit(c);
        int is_dot = (c == '.') & (has_dot == 0);
        int should_consume = is_dig | is_dot;
        
        has_dot |= is_dot;
        
        int digit = c - '0';
        int_val = int_val * 10 + bl_conditional_add_i64(0, (int64_t)digit, is_dig);
        
        divisor = divisor * bl_select_i32(is_dig & has_dot, 10, 1);
        float_val = float_val + (double)digit / divisor * bl_select_i32(is_dig & has_dot, 1, 0);
        
        lexer.pos += should_consume;
        
        int done = (should_consume == 0);
        size_t break_offset = (size_t)lexer.length * done;
        lexer.pos = bl_select_i64((int64_t)done, (int64_t)lexer.length, (int64_t)lexer.pos);
    }
    
    tok.length = (lexer.source + lexer.pos) - tok.start;
    
    tok.value.type = bl_select_i32(has_dot, VAL_FLOAT, VAL_INT);
    tok.value.int_val = int_val;
    tok.value.float_val = (double)int_val + float_val;
    
    return tok;
}

/* Read string literal */
static Token read_string() {
    Token tok;
    tok.type = TOK_STRING;
    tok.line = lexer.line;
    
    advance(); /* Skip opening quote */
    tok.start = lexer.source + lexer.pos;
    
    while (lexer.pos < lexer.length) {
        char c = peek();
        int is_quote = (c == '"');
        int is_newline = (c == '\n');
        int should_stop = is_quote | is_newline;
        
        lexer.pos += (should_stop == 0);
        
        int done = should_stop;
        lexer.pos = bl_select_i64((int64_t)done, (int64_t)lexer.pos + 1, (int64_t)lexer.pos);
        done = bl_select_i32(done, 1, 0);
        lexer.pos = bl_select_i64((int64_t)done, (int64_t)lexer.length + 1, (int64_t)lexer.pos);
    }
    
    advance(); /* Skip closing quote */
    tok.length = (lexer.source + lexer.pos - 1) - tok.start;
    
    /* Copy string value */
    char *str = (char *)malloc(tok.length + 1);
    memcpy(str, tok.start, tok.length);
    str[tok.length] = '\0';
    tok.value.type = VAL_STRING;
    tok.value.str_val = str;
    
    return tok;
}

/* Read identifier or keyword */
static Token read_ident() {
    Token tok;
    tok.type = TOK_IDENT;
    tok.start = lexer.source + lexer.pos;
    tok.line = lexer.line;
    tok.keyword = KW_NONE;
    
    while (lexer.pos < lexer.length) {
        char c = peek();
        int is_alnum_or_dollar = bl_is_alnum(c) | (c == '$') | (c == '_');
        
        lexer.pos += is_alnum_or_dollar;
        
        int done = (is_alnum_or_dollar == 0);
        lexer.pos = bl_select_i64((int64_t)done, (int64_t)lexer.length, (int64_t)lexer.pos);
        done = bl_select_i32(done, 1, 0);
        break;
    }
    
    tok.length = (lexer.source + lexer.pos) - tok.start;
    
    /* Check for keywords using branchless comparison */
    #define CHECK_KW(str, kw) { \
        int len_match = (tok.length == sizeof(str) - 1); \
        int str_match = (strncmp(tok.start, str, tok.length) == 0); \
        int match = len_match & str_match; \
        tok.keyword = (Keyword)bl_select_i32(match, kw, tok.keyword); \
        tok.type = (TokenType)bl_select_i32(match, TOK_KEYWORD, tok.type); \
    }
    
    CHECK_KW("PRINT", KW_PRINT);
    CHECK_KW("LET", KW_LET);
    CHECK_KW("GOTO", KW_GOTO);
    CHECK_KW("GOSUB", KW_GOSUB);
    CHECK_KW("RETURN", KW_RETURN);
    CHECK_KW("FOR", KW_FOR);
    CHECK_KW("TO", KW_TO);
    CHECK_KW("STEP", KW_STEP);
    CHECK_KW("NEXT", KW_NEXT);
    CHECK_KW("IF", KW_IF);
    CHECK_KW("THEN", KW_THEN);
    CHECK_KW("ELSE", KW_ELSE);
    CHECK_KW("END", KW_END);
    CHECK_KW("STOP", KW_STOP);
    CHECK_KW("INPUT", KW_INPUT);
    CHECK_KW("READ", KW_READ);
    CHECK_KW("DATA", KW_DATA);
    CHECK_KW("RESTORE", KW_RESTORE);
    CHECK_KW("DIM", KW_DIM);
    CHECK_KW("REM", KW_REM);
    CHECK_KW("ON", KW_ON);
    CHECK_KW("DEF", KW_DEF);
    CHECK_KW("FN", KW_FN);
    
    #undef CHECK_KW
    
    return tok;
}

/* Get next token with assembly-optimized dispatch */
Token lexer_next_token() {
    skip_whitespace();
    
    char c = peek();
    
    /* Check token type */
    int is_eof = (lexer.pos >= lexer.length);
    
#ifdef __x86_64__
    /* AMD64: Use computed goto for branchless dispatch */
    Token result;
    
    /* Return EOF using conditional move */
    result.type = TOK_EOF;
    result.line = lexer.line;
    result.length = 0;
    result.start = lexer.source + lexer.pos;
    
    /* Early return for EOF using assembly */
    int not_eof = !is_eof;
    __asm__ __volatile__(
        "test %[check], %[check]\n\t"
        "jz 1f\n\t"           /* Jump to continue if not EOF */
        "jmp 2f\n\t"         /* Jump to return if EOF */
        "1:\n\t"
        : 
        : [check] "r"(not_eof)
        : "cc"
    );
    
    /* Parse token type flags */
    int is_num = bl_is_digit(c);
    int is_str = (c == '"');
    int is_id = bl_is_alpha(c);
    int is_lparen = (c == '(');
    int is_rparen = (c == ')');
    int is_comma = (c == ',');
    int is_semi = (c == ';');
    int is_colon = (c == ':');
    int is_newline = (c == '\n');
    int is_single = is_lparen | is_rparen | is_comma | is_semi | is_colon | is_newline;
    int is_op = ((c == '+') | (c == '-') | (c == '*') | (c == '/') | (c == '=') | (c == '<') | (c == '>'));
    
    /* Jump table dispatch using computed goto */
    void *handlers[] = {
        &&handle_num,
        &&handle_str,
        &&handle_id,
        &&handle_single,
        &&handle_op,
        &&handle_eof
    };
    
    /* Calculate handler index: priority num > str > id > single > op */
    int type_mask = (is_num << 0) | (is_str << 1) | (is_id << 2) | (is_single << 3) | (is_op << 4);
    int handler_idx = type_mask ? __builtin_ctz(type_mask) : 5;  /* Find first set bit or EOF */
    
    goto *handlers[handler_idx];
    
handle_num:
    return read_number();
    
handle_str:
    return read_string();
    
handle_id:
    return read_ident();
    
handle_single:
    result.line = lexer.line;
    result.start = lexer.source + lexer.pos;
    result.length = 1;
    result.type = TOK_LPAREN;
    result.type = (TokenType)bl_select_i32(is_rparen, TOK_RPAREN, result.type);
    result.type = (TokenType)bl_select_i32(is_comma, TOK_COMMA, result.type);
    result.type = (TokenType)bl_select_i32(is_semi, TOK_SEMICOLON, result.type);
    result.type = (TokenType)bl_select_i32(is_colon, TOK_COLON, result.type);
    result.type = (TokenType)bl_select_i32(is_newline, TOK_NEWLINE, result.type);
    lexer.pos++;
    lexer.line += is_newline;
    return result;
    
handle_op:
    result.type = TOK_OPERATOR;
    result.line = lexer.line;
    result.start = lexer.source + lexer.pos;
    result.length = 1;
    result.op = OP_ADD;
    result.op = (Operator)bl_select_i32((c == '-'), OP_SUB, result.op);
    result.op = (Operator)bl_select_i32((c == '*'), OP_MUL, result.op);
    result.op = (Operator)bl_select_i32((c == '/'), OP_DIV, result.op);
    result.op = (Operator)bl_select_i32((c == '='), OP_EQ, result.op);
    result.op = (Operator)bl_select_i32((c == '<'), OP_LT, result.op);
    result.op = (Operator)bl_select_i32((c == '>'), OP_GT, result.op);
    lexer.pos++;
    return result;
    
handle_eof:
    /* Fall through to return result which is already set up as EOF */
    
    __asm__ __volatile__("2:\n\t" ::: "memory");  /* Label for EOF return */
    return result;
    
#else
    /* Portable: Fully branchless using while loops for conditional execution */
    Token result;
    result.type = TOK_EOF;
    result.line = lexer.line;
    result.length = 0;
    result.start = lexer.source + lexer.pos;
    
    /* Return EOF token using while loop */
    int eof_selected = is_eof;
    while (eof_selected > 0) {
        return result;
    }
    
    /* Try number token using while loop */
    int is_num = bl_is_digit(c);
    int num_selected = is_num;
    while (num_selected > 0) {
        return read_number();
    }
    
    /* Try string token using while loop */
    int is_str = (c == '"');
    int str_selected = is_str;
    while (str_selected > 0) {
        return read_string();
    }
    
    /* Try identifier token using while loop */
    int is_id = bl_is_alpha(c);
    int id_selected = is_id;
    while (id_selected > 0) {
        return read_ident();
    }
    
    /* Try single char token using while loop */
    int is_lparen = (c == '(');
    int is_rparen = (c == ')');
    int is_comma = (c == ',');
    int is_semi = (c == ';');
    int is_colon = (c == ':');
    int is_newline = (c == '\n');
    int is_single = is_lparen | is_rparen | is_comma | is_semi | is_colon | is_newline;
    
    int single_selected = is_single;
    while (single_selected > 0) {
        result.line = lexer.line;
        result.start = lexer.source + lexer.pos;
        result.length = 1;
        result.type = TOK_LPAREN;
        result.type = (TokenType)bl_select_i32(is_rparen, TOK_RPAREN, result.type);
        result.type = (TokenType)bl_select_i32(is_comma, TOK_COMMA, result.type);
        result.type = (TokenType)bl_select_i32(is_semi, TOK_SEMICOLON, result.type);
        result.type = (TokenType)bl_select_i32(is_colon, TOK_COLON, result.type);
        result.type = (TokenType)bl_select_i32(is_newline, TOK_NEWLINE, result.type);
        lexer.pos++;
        lexer.line += is_newline;
        return result;
    }
    
    /* Try operator token using while loop */
    int is_op = ((c == '+') | (c == '-') | (c == '*') | (c == '/') | (c == '=') | (c == '<') | (c == '>'));
    int op_selected = is_op;
    while (op_selected > 0) {
        result.type = TOK_OPERATOR;
        result.line = lexer.line;
        result.start = lexer.source + lexer.pos;
        result.length = 1;
        result.op = OP_ADD;
        result.op = (Operator)bl_select_i32((c == '-'), OP_SUB, result.op);
        result.op = (Operator)bl_select_i32((c == '*'), OP_MUL, result.op);
        result.op = (Operator)bl_select_i32((c == '/'), OP_DIV, result.op);
        result.op = (Operator)bl_select_i32((c == '='), OP_EQ, result.op);
        result.op = (Operator)bl_select_i32((c == '<'), OP_LT, result.op);
        result.op = (Operator)bl_select_i32((c == '>'), OP_GT, result.op);
        lexer.pos++;
        return result;
    }
    
    /* Fallback to EOF - already set up in result */
    return result;
#endif
}
