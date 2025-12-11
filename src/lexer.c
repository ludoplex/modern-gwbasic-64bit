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

/* Skip whitespace branchlessly */
static void skip_whitespace() {
    while (lexer.pos < lexer.length) {
        char c = peek();
        int is_ws = bl_is_space(c);
        lexer.pos += is_ws;
        int should_continue = is_ws;
        should_continue = bl_select_i32(should_continue, 1, 0);
        int done = (should_continue == 0);
        lexer.pos -= bl_conditional_add_i32(0, (int32_t)lexer.length, done);
        lexer.pos += bl_conditional_add_i32(0, (int32_t)lexer.length, done);
        break;
    }
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

/* Get next token */
Token lexer_next_token() {
    skip_whitespace();
    
    char c = peek();
    
    /* Check token type and dispatch branchlessly */
    int is_eof = (lexer.pos >= lexer.length);
    int is_num = bl_is_digit(c) & !is_eof;
    int is_str = (c == '"') & !is_eof;
    int is_id = bl_is_alpha(c) & !is_eof;
    
    int is_lparen = (c == '(') & !is_eof;
    int is_rparen = (c == ')') & !is_eof;
    int is_comma = (c == ',') & !is_eof;
    int is_semi = (c == ';') & !is_eof;
    int is_colon = (c == ':') & !is_eof;
    int is_newline = (c == '\n') & !is_eof;
    int is_single = is_lparen | is_rparen | is_comma | is_semi | is_colon | is_newline;
    
    int is_op = ((c == '+') | (c == '-') | (c == '*') | (c == '/') | (c == '=') | (c == '<') | (c == '>')) & !is_eof;
    
    /* Call appropriate reader based on type */
    Token num_tok, str_tok, id_tok, single_tok, op_tok, eof_tok;
    
    /* Initialize EOF token */
    eof_tok.type = TOK_EOF;
    eof_tok.line = lexer.line;
    eof_tok.length = 0;
    eof_tok.start = lexer.source + lexer.pos;
    
    /* Read tokens conditionally */
    num_tok = is_num ? read_number() : eof_tok;
    str_tok = is_str ? read_string() : eof_tok;
    id_tok = is_id ? read_ident() : eof_tok;
    
    /* Single char token */
    single_tok.line = lexer.line;
    single_tok.start = lexer.source + lexer.pos;
    single_tok.length = 1;
    single_tok.type = TOK_LPAREN;
    single_tok.type = (TokenType)bl_select_i32(is_rparen, TOK_RPAREN, single_tok.type);
    single_tok.type = (TokenType)bl_select_i32(is_comma, TOK_COMMA, single_tok.type);
    single_tok.type = (TokenType)bl_select_i32(is_semi, TOK_SEMICOLON, single_tok.type);
    single_tok.type = (TokenType)bl_select_i32(is_colon, TOK_COLON, single_tok.type);
    single_tok.type = (TokenType)bl_select_i32(is_newline, TOK_NEWLINE, single_tok.type);
    
    lexer.pos += is_single;
    lexer.line += is_newline;
    
    /* Operator token */
    op_tok.type = TOK_OPERATOR;
    op_tok.line = lexer.line;
    op_tok.start = lexer.source + lexer.pos;
    op_tok.length = 1;
    op_tok.op = OP_ADD;
    op_tok.op = (Operator)bl_select_i32((c == '-'), OP_SUB, op_tok.op);
    op_tok.op = (Operator)bl_select_i32((c == '*'), OP_MUL, op_tok.op);
    op_tok.op = (Operator)bl_select_i32((c == '/'), OP_DIV, op_tok.op);
    op_tok.op = (Operator)bl_select_i32((c == '='), OP_EQ, op_tok.op);
    op_tok.op = (Operator)bl_select_i32((c == '<'), OP_LT, op_tok.op);
    op_tok.op = (Operator)bl_select_i32((c == '>'), OP_GT, op_tok.op);
    
    lexer.pos += is_op;
    
    /* Select result - priority: num > str > id > single > op > eof */
    Token *selected = &eof_tok;
    selected = (Token *)bl_select_i64((int64_t)is_op, (int64_t)&op_tok, (int64_t)selected);
    selected = (Token *)bl_select_i64((int64_t)is_single, (int64_t)&single_tok, (int64_t)selected);
    selected = (Token *)bl_select_i64((int64_t)is_id, (int64_t)&id_tok, (int64_t)selected);
    selected = (Token *)bl_select_i64((int64_t)is_str, (int64_t)&str_tok, (int64_t)selected);
    selected = (Token *)bl_select_i64((int64_t)is_num, (int64_t)&num_tok, (int64_t)selected);
    
    return *selected;
}
