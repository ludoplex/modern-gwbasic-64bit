#include "lexer.h"
#include "branchless_scalar.h"
#include <string.h>
#include <stdlib.h>

/* Character class enumeration */
typedef enum {
    CLASS_INVALID = 0,
    CLASS_DIGIT,
    CLASS_ALPHA,
    CLASS_STRING,
    CLASS_LPAREN,
    CLASS_RPAREN,
    CLASS_COMMA,
    CLASS_SEMICOLON,
    CLASS_COLON,
    CLASS_NEWLINE,
    CLASS_SPACE,
    CLASS_PLUS,
    CLASS_MINUS,
    CLASS_STAR,
    CLASS_SLASH,
    CLASS_EQ,
    CLASS_LT,
    CLASS_GT,
    CLASS_DOLLAR,
    CLASS_UNDERSCORE,
    CLASS_DOT,
    CLASS_EOF,
    CLASS_COUNT
} CharClass;

/* Precomputed character classification table - 256 entries */
static const uint8_t char_class[256] = {
    [0] = CLASS_EOF,
    [1 ... 8] = CLASS_INVALID,
    [9] = CLASS_SPACE,      /* tab */
    [10] = CLASS_NEWLINE,
    [11 ... 12] = CLASS_INVALID,
    [13] = CLASS_SPACE,     /* carriage return */
    [14 ... 31] = CLASS_INVALID,
    [32] = CLASS_SPACE,
    [33] = CLASS_INVALID,
    [34] = CLASS_STRING,    /* " */
    [35] = CLASS_INVALID,
    [36] = CLASS_DOLLAR,    /* $ */
    [37 ... 39] = CLASS_INVALID,
    [40] = CLASS_LPAREN,
    [41] = CLASS_RPAREN,
    [42] = CLASS_STAR,
    [43] = CLASS_PLUS,
    [44] = CLASS_COMMA,
    [45] = CLASS_MINUS,
    [46] = CLASS_DOT,
    [47] = CLASS_SLASH,
    [48 ... 57] = CLASS_DIGIT,
    [58] = CLASS_COLON,
    [59] = CLASS_SEMICOLON,
    [60] = CLASS_LT,
    [61] = CLASS_EQ,
    [62] = CLASS_GT,
    [63 ... 64] = CLASS_INVALID,
    [65 ... 90] = CLASS_ALPHA,  /* A-Z */
    [91 ... 94] = CLASS_INVALID,
    [95] = CLASS_UNDERSCORE,
    [96] = CLASS_INVALID,
    [97 ... 122] = CLASS_ALPHA, /* a-z */
    [123 ... 127] = CLASS_INVALID,
    [128 ... 255] = CLASS_INVALID
};

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

/* Branchless peek with bounds checking */
static inline char peek_at(size_t pos) {
    size_t in_bounds = (pos < lexer.length);
    size_t safe_pos = bl_select_i64((int64_t)in_bounds, (int64_t)pos, 0);
    return (char)bl_select_i32((int32_t)in_bounds, (int32_t)lexer.source[safe_pos], 0);
}

static inline char peek() {
    return peek_at(lexer.pos);
}

/* Branchless character class lookup */
static inline uint8_t get_char_class(char c) {
    return char_class[(uint8_t)c];
}

/* Skip whitespace using unrolled arithmetic - NO LOOPS */
#define SKIP_WS_STEP(i) \
    c##i = peek_at(lexer.pos); is_ws##i = (get_char_class(c##i) == CLASS_SPACE); lexer.pos = lexer.pos + is_ws##i

static void skip_whitespace() {
    /* Unrolled - scan up to 32 whitespace chars */
    char c0, c1, c2, c3, c4, c5, c6, c7;
    char c8, c9, c10, c11, c12, c13, c14, c15;
    char c16, c17, c18, c19, c20, c21, c22, c23;
    char c24, c25, c26, c27, c28, c29, c30, c31;
    int is_ws0, is_ws1, is_ws2, is_ws3, is_ws4, is_ws5, is_ws6, is_ws7;
    int is_ws8, is_ws9, is_ws10, is_ws11, is_ws12, is_ws13, is_ws14, is_ws15;
    int is_ws16, is_ws17, is_ws18, is_ws19, is_ws20, is_ws21, is_ws22, is_ws23;
    int is_ws24, is_ws25, is_ws26, is_ws27, is_ws28, is_ws29, is_ws30, is_ws31;
    
    SKIP_WS_STEP(0); SKIP_WS_STEP(1); SKIP_WS_STEP(2); SKIP_WS_STEP(3);
    SKIP_WS_STEP(4); SKIP_WS_STEP(5); SKIP_WS_STEP(6); SKIP_WS_STEP(7);
    SKIP_WS_STEP(8); SKIP_WS_STEP(9); SKIP_WS_STEP(10); SKIP_WS_STEP(11);
    SKIP_WS_STEP(12); SKIP_WS_STEP(13); SKIP_WS_STEP(14); SKIP_WS_STEP(15);
    SKIP_WS_STEP(16); SKIP_WS_STEP(17); SKIP_WS_STEP(18); SKIP_WS_STEP(19);
    SKIP_WS_STEP(20); SKIP_WS_STEP(21); SKIP_WS_STEP(22); SKIP_WS_STEP(23);
    SKIP_WS_STEP(24); SKIP_WS_STEP(25); SKIP_WS_STEP(26); SKIP_WS_STEP(27);
    SKIP_WS_STEP(28); SKIP_WS_STEP(29); SKIP_WS_STEP(30); SKIP_WS_STEP(31);
}

/* Keyword hash table */
typedef struct {
    const char *str;
    uint8_t len;
    Keyword kw;
} KeywordEntry;

static const KeywordEntry keyword_table[] = {
    {"PRINT", 5, KW_PRINT},
    {"LET", 3, KW_LET},
    {"GOTO", 4, KW_GOTO},
    {"GOSUB", 5, KW_GOSUB},
    {"RETURN", 6, KW_RETURN},
    {"FOR", 3, KW_FOR},
    {"TO", 2, KW_TO},
    {"STEP", 4, KW_STEP},
    {"NEXT", 4, KW_NEXT},
    {"IF", 2, KW_IF},
    {"THEN", 4, KW_THEN},
    {"ELSE", 4, KW_ELSE},
    {"END", 3, KW_END},
    {"STOP", 4, KW_STOP},
    {"INPUT", 5, KW_INPUT},
    {"READ", 4, KW_READ},
    {"DATA", 4, KW_DATA},
    {"RESTORE", 7, KW_RESTORE},
    {"DIM", 3, KW_DIM},
    {"REM", 3, KW_REM},
    {"ON", 2, KW_ON},
    {"DEF", 3, KW_DEF},
    {"FN", 2, KW_FN},
};

#define KEYWORD_COUNT (sizeof(keyword_table) / sizeof(KeywordEntry))

/* Branchless keyword lookup - unrolled comparison */
#define CHECK_KW(i) \
    len_match##i = (keyword_table[i].len == tok->length); \
    str_match##i = len_match##i & (strncmp(tok->start, keyword_table[i].str, tok->length) == 0); \
    result = (Keyword)bl_select_i32(str_match##i, keyword_table[i].kw, result); \
    type = (TokenType)bl_select_i32(str_match##i, TOK_KEYWORD, type)

static void check_keyword(Token *tok) {
    Keyword result = KW_NONE;
    TokenType type = TOK_IDENT;
    int len_match0, len_match1, len_match2, len_match3, len_match4, len_match5, len_match6, len_match7;
    int len_match8, len_match9, len_match10, len_match11, len_match12, len_match13, len_match14, len_match15;
    int len_match16, len_match17, len_match18, len_match19, len_match20, len_match21, len_match22;
    int str_match0, str_match1, str_match2, str_match3, str_match4, str_match5, str_match6, str_match7;
    int str_match8, str_match9, str_match10, str_match11, str_match12, str_match13, str_match14, str_match15;
    int str_match16, str_match17, str_match18, str_match19, str_match20, str_match21, str_match22;
    
    CHECK_KW(0); CHECK_KW(1); CHECK_KW(2); CHECK_KW(3); CHECK_KW(4); CHECK_KW(5); CHECK_KW(6); CHECK_KW(7);
    CHECK_KW(8); CHECK_KW(9); CHECK_KW(10); CHECK_KW(11); CHECK_KW(12); CHECK_KW(13); CHECK_KW(14); CHECK_KW(15);
    CHECK_KW(16); CHECK_KW(17); CHECK_KW(18); CHECK_KW(19); CHECK_KW(20); CHECK_KW(21); CHECK_KW(22);
    
    tok->keyword = result;
    tok->type = type;
}

/* Read number token - unrolled fixed iterations */
#define READ_NUM_STEP(i) \
    c##i = peek_at(lexer.pos); \
    is_dig##i = (get_char_class(c##i) == CLASS_DIGIT); \
    is_dot##i = ((get_char_class(c##i) == CLASS_DOT) & (has_dot == 0)); \
    should_consume##i = is_dig##i | is_dot##i; \
    has_dot = has_dot | is_dot##i; \
    digit##i = c##i - '0'; \
    int_val = int_val * bl_select_i64(should_consume##i, 10, 1) + bl_conditional_add_i64(0, (int64_t)digit##i, is_dig##i); \
    lexer.pos = lexer.pos + should_consume##i

static Token read_number() {
    Token tok;
    tok.type = TOK_NUMBER;
    tok.start = lexer.source + lexer.pos;
    tok.line = lexer.line;
    tok.keyword = KW_NONE;
    
    int64_t int_val = 0;
    int has_dot = 0;
    
    /* Declare variables for unrolled loop */
    char c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15;
    char c16, c17, c18, c19, c20, c21, c22, c23, c24, c25, c26, c27, c28, c29, c30, c31;
    int is_dig0, is_dig1, is_dig2, is_dig3, is_dig4, is_dig5, is_dig6, is_dig7;
    int is_dig8, is_dig9, is_dig10, is_dig11, is_dig12, is_dig13, is_dig14, is_dig15;
    int is_dig16, is_dig17, is_dig18, is_dig19, is_dig20, is_dig21, is_dig22, is_dig23;
    int is_dig24, is_dig25, is_dig26, is_dig27, is_dig28, is_dig29, is_dig30, is_dig31;
    int is_dot0, is_dot1, is_dot2, is_dot3, is_dot4, is_dot5, is_dot6, is_dot7;
    int is_dot8, is_dot9, is_dot10, is_dot11, is_dot12, is_dot13, is_dot14, is_dot15;
    int is_dot16, is_dot17, is_dot18, is_dot19, is_dot20, is_dot21, is_dot22, is_dot23;
    int is_dot24, is_dot25, is_dot26, is_dot27, is_dot28, is_dot29, is_dot30, is_dot31;
    int should_consume0, should_consume1, should_consume2, should_consume3, should_consume4, should_consume5, should_consume6, should_consume7;
    int should_consume8, should_consume9, should_consume10, should_consume11, should_consume12, should_consume13, should_consume14, should_consume15;
    int should_consume16, should_consume17, should_consume18, should_consume19, should_consume20, should_consume21, should_consume22, should_consume23;
    int should_consume24, should_consume25, should_consume26, should_consume27, should_consume28, should_consume29, should_consume30, should_consume31;
    int digit0, digit1, digit2, digit3, digit4, digit5, digit6, digit7;
    int digit8, digit9, digit10, digit11, digit12, digit13, digit14, digit15;
    int digit16, digit17, digit18, digit19, digit20, digit21, digit22, digit23;
    int digit24, digit25, digit26, digit27, digit28, digit29, digit30, digit31;
    
    /* Unrolled loop - scan up to 32 digits */
    READ_NUM_STEP(0); READ_NUM_STEP(1); READ_NUM_STEP(2); READ_NUM_STEP(3);
    READ_NUM_STEP(4); READ_NUM_STEP(5); READ_NUM_STEP(6); READ_NUM_STEP(7);
    READ_NUM_STEP(8); READ_NUM_STEP(9); READ_NUM_STEP(10); READ_NUM_STEP(11);
    READ_NUM_STEP(12); READ_NUM_STEP(13); READ_NUM_STEP(14); READ_NUM_STEP(15);
    READ_NUM_STEP(16); READ_NUM_STEP(17); READ_NUM_STEP(18); READ_NUM_STEP(19);
    READ_NUM_STEP(20); READ_NUM_STEP(21); READ_NUM_STEP(22); READ_NUM_STEP(23);
    READ_NUM_STEP(24); READ_NUM_STEP(25); READ_NUM_STEP(26); READ_NUM_STEP(27);
    READ_NUM_STEP(28); READ_NUM_STEP(29); READ_NUM_STEP(30); READ_NUM_STEP(31);
    
    tok.length = (lexer.source + lexer.pos) - tok.start;
    tok.value.type = bl_select_i32(has_dot, VAL_FLOAT, VAL_INT);
    tok.value.int_val = int_val;
    tok.value.float_val = (double)int_val;
    
    return tok;
}

/* Read string literal - unrolled fixed iterations */
#define READ_STR_STEP(i) \
    c##i = peek_at(lexer.pos); \
    is_quote##i = (c##i == '"'); \
    is_newline##i = (c##i == '\n'); \
    is_eof##i = (c##i == 0); \
    should_stop##i = is_quote##i | is_newline##i | is_eof##i; \
    should_advance##i = (should_stop##i == 0); \
    lexer.pos = lexer.pos + should_advance##i

static Token read_string() {
    Token tok;
    tok.type = TOK_STRING;
    tok.line = lexer.line;
    tok.keyword = KW_NONE;
    
    /* Skip opening quote */
    lexer.pos = lexer.pos + 1;
    tok.start = lexer.source + lexer.pos;
    
    /* Declare variables - scanning up to 64 chars */
    char c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15;
    char c16, c17, c18, c19, c20, c21, c22, c23, c24, c25, c26, c27, c28, c29, c30, c31;
    char c32, c33, c34, c35, c36, c37, c38, c39, c40, c41, c42, c43, c44, c45, c46, c47;
    char c48, c49, c50, c51, c52, c53, c54, c55, c56, c57, c58, c59, c60, c61, c62, c63;
    int is_quote0, is_quote1, is_quote2, is_quote3, is_quote4, is_quote5, is_quote6, is_quote7;
    int is_quote8, is_quote9, is_quote10, is_quote11, is_quote12, is_quote13, is_quote14, is_quote15;
    int is_quote16, is_quote17, is_quote18, is_quote19, is_quote20, is_quote21, is_quote22, is_quote23;
    int is_quote24, is_quote25, is_quote26, is_quote27, is_quote28, is_quote29, is_quote30, is_quote31;
    int is_quote32, is_quote33, is_quote34, is_quote35, is_quote36, is_quote37, is_quote38, is_quote39;
    int is_quote40, is_quote41, is_quote42, is_quote43, is_quote44, is_quote45, is_quote46, is_quote47;
    int is_quote48, is_quote49, is_quote50, is_quote51, is_quote52, is_quote53, is_quote54, is_quote55;
    int is_quote56, is_quote57, is_quote58, is_quote59, is_quote60, is_quote61, is_quote62, is_quote63;
    int is_newline0, is_newline1, is_newline2, is_newline3, is_newline4, is_newline5, is_newline6, is_newline7;
    int is_newline8, is_newline9, is_newline10, is_newline11, is_newline12, is_newline13, is_newline14, is_newline15;
    int is_newline16, is_newline17, is_newline18, is_newline19, is_newline20, is_newline21, is_newline22, is_newline23;
    int is_newline24, is_newline25, is_newline26, is_newline27, is_newline28, is_newline29, is_newline30, is_newline31;
    int is_newline32, is_newline33, is_newline34, is_newline35, is_newline36, is_newline37, is_newline38, is_newline39;
    int is_newline40, is_newline41, is_newline42, is_newline43, is_newline44, is_newline45, is_newline46, is_newline47;
    int is_newline48, is_newline49, is_newline50, is_newline51, is_newline52, is_newline53, is_newline54, is_newline55;
    int is_newline56, is_newline57, is_newline58, is_newline59, is_newline60, is_newline61, is_newline62, is_newline63;
    int is_eof0, is_eof1, is_eof2, is_eof3, is_eof4, is_eof5, is_eof6, is_eof7;
    int is_eof8, is_eof9, is_eof10, is_eof11, is_eof12, is_eof13, is_eof14, is_eof15;
    int is_eof16, is_eof17, is_eof18, is_eof19, is_eof20, is_eof21, is_eof22, is_eof23;
    int is_eof24, is_eof25, is_eof26, is_eof27, is_eof28, is_eof29, is_eof30, is_eof31;
    int is_eof32, is_eof33, is_eof34, is_eof35, is_eof36, is_eof37, is_eof38, is_eof39;
    int is_eof40, is_eof41, is_eof42, is_eof43, is_eof44, is_eof45, is_eof46, is_eof47;
    int is_eof48, is_eof49, is_eof50, is_eof51, is_eof52, is_eof53, is_eof54, is_eof55;
    int is_eof56, is_eof57, is_eof58, is_eof59, is_eof60, is_eof61, is_eof62, is_eof63;
    int should_stop0, should_stop1, should_stop2, should_stop3, should_stop4, should_stop5, should_stop6, should_stop7;
    int should_stop8, should_stop9, should_stop10, should_stop11, should_stop12, should_stop13, should_stop14, should_stop15;
    int should_stop16, should_stop17, should_stop18, should_stop19, should_stop20, should_stop21, should_stop22, should_stop23;
    int should_stop24, should_stop25, should_stop26, should_stop27, should_stop28, should_stop29, should_stop30, should_stop31;
    int should_stop32, should_stop33, should_stop34, should_stop35, should_stop36, should_stop37, should_stop38, should_stop39;
    int should_stop40, should_stop41, should_stop42, should_stop43, should_stop44, should_stop45, should_stop46, should_stop47;
    int should_stop48, should_stop49, should_stop50, should_stop51, should_stop52, should_stop53, should_stop54, should_stop55;
    int should_stop56, should_stop57, should_stop58, should_stop59, should_stop60, should_stop61, should_stop62, should_stop63;
    int should_advance0, should_advance1, should_advance2, should_advance3, should_advance4, should_advance5, should_advance6, should_advance7;
    int should_advance8, should_advance9, should_advance10, should_advance11, should_advance12, should_advance13, should_advance14, should_advance15;
    int should_advance16, should_advance17, should_advance18, should_advance19, should_advance20, should_advance21, should_advance22, should_advance23;
    int should_advance24, should_advance25, should_advance26, should_advance27, should_advance28, should_advance29, should_advance30, should_advance31;
    int should_advance32, should_advance33, should_advance34, should_advance35, should_advance36, should_advance37, should_advance38, should_advance39;
    int should_advance40, should_advance41, should_advance42, should_advance43, should_advance44, should_advance45, should_advance46, should_advance47;
    int should_advance48, should_advance49, should_advance50, should_advance51, should_advance52, should_advance53, should_advance54, should_advance55;
    int should_advance56, should_advance57, should_advance58, should_advance59, should_advance60, should_advance61, should_advance62, should_advance63;
    
    /* Unrolled loop */
    READ_STR_STEP(0); READ_STR_STEP(1); READ_STR_STEP(2); READ_STR_STEP(3);
    READ_STR_STEP(4); READ_STR_STEP(5); READ_STR_STEP(6); READ_STR_STEP(7);
    READ_STR_STEP(8); READ_STR_STEP(9); READ_STR_STEP(10); READ_STR_STEP(11);
    READ_STR_STEP(12); READ_STR_STEP(13); READ_STR_STEP(14); READ_STR_STEP(15);
    READ_STR_STEP(16); READ_STR_STEP(17); READ_STR_STEP(18); READ_STR_STEP(19);
    READ_STR_STEP(20); READ_STR_STEP(21); READ_STR_STEP(22); READ_STR_STEP(23);
    READ_STR_STEP(24); READ_STR_STEP(25); READ_STR_STEP(26); READ_STR_STEP(27);
    READ_STR_STEP(28); READ_STR_STEP(29); READ_STR_STEP(30); READ_STR_STEP(31);
    READ_STR_STEP(32); READ_STR_STEP(33); READ_STR_STEP(34); READ_STR_STEP(35);
    READ_STR_STEP(36); READ_STR_STEP(37); READ_STR_STEP(38); READ_STR_STEP(39);
    READ_STR_STEP(40); READ_STR_STEP(41); READ_STR_STEP(42); READ_STR_STEP(43);
    READ_STR_STEP(44); READ_STR_STEP(45); READ_STR_STEP(46); READ_STR_STEP(47);
    READ_STR_STEP(48); READ_STR_STEP(49); READ_STR_STEP(50); READ_STR_STEP(51);
    READ_STR_STEP(52); READ_STR_STEP(53); READ_STR_STEP(54); READ_STR_STEP(55);
    READ_STR_STEP(56); READ_STR_STEP(57); READ_STR_STEP(58); READ_STR_STEP(59);
    READ_STR_STEP(60); READ_STR_STEP(61); READ_STR_STEP(62); READ_STR_STEP(63);
    
    tok.length = (lexer.source + lexer.pos) - tok.start;
    
    /* Skip closing quote */
    lexer.pos = lexer.pos + 1;
    
    /* Copy string value */
    char *str = (char *)malloc(tok.length + 1);
    memcpy(str, tok.start, tok.length);
    str[tok.length] = '\0';
    tok.value.type = VAL_STRING;
    tok.value.str_val = str;
    
    return tok;
}

/* Read identifier - unrolled fixed iterations */
#define READ_IDENT_STEP(i) \
    c##i = peek_at(lexer.pos); \
    cls##i = get_char_class(c##i); \
    is_valid##i = (cls##i == CLASS_ALPHA) | (cls##i == CLASS_DIGIT) | (cls##i == CLASS_DOLLAR) | (cls##i == CLASS_UNDERSCORE); \
    lexer.pos = lexer.pos + is_valid##i

static Token read_ident() {
    Token tok;
    tok.type = TOK_IDENT;
    tok.start = lexer.source + lexer.pos;
    tok.line = lexer.line;
    tok.keyword = KW_NONE;
    
    /* Declare variables - scanning up to 64 chars */
    char c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15;
    char c16, c17, c18, c19, c20, c21, c22, c23, c24, c25, c26, c27, c28, c29, c30, c31;
    char c32, c33, c34, c35, c36, c37, c38, c39, c40, c41, c42, c43, c44, c45, c46, c47;
    char c48, c49, c50, c51, c52, c53, c54, c55, c56, c57, c58, c59, c60, c61, c62, c63;
    uint8_t cls0, cls1, cls2, cls3, cls4, cls5, cls6, cls7, cls8, cls9, cls10, cls11, cls12, cls13, cls14, cls15;
    uint8_t cls16, cls17, cls18, cls19, cls20, cls21, cls22, cls23, cls24, cls25, cls26, cls27, cls28, cls29, cls30, cls31;
    uint8_t cls32, cls33, cls34, cls35, cls36, cls37, cls38, cls39, cls40, cls41, cls42, cls43, cls44, cls45, cls46, cls47;
    uint8_t cls48, cls49, cls50, cls51, cls52, cls53, cls54, cls55, cls56, cls57, cls58, cls59, cls60, cls61, cls62, cls63;
    int is_valid0, is_valid1, is_valid2, is_valid3, is_valid4, is_valid5, is_valid6, is_valid7;
    int is_valid8, is_valid9, is_valid10, is_valid11, is_valid12, is_valid13, is_valid14, is_valid15;
    int is_valid16, is_valid17, is_valid18, is_valid19, is_valid20, is_valid21, is_valid22, is_valid23;
    int is_valid24, is_valid25, is_valid26, is_valid27, is_valid28, is_valid29, is_valid30, is_valid31;
    int is_valid32, is_valid33, is_valid34, is_valid35, is_valid36, is_valid37, is_valid38, is_valid39;
    int is_valid40, is_valid41, is_valid42, is_valid43, is_valid44, is_valid45, is_valid46, is_valid47;
    int is_valid48, is_valid49, is_valid50, is_valid51, is_valid52, is_valid53, is_valid54, is_valid55;
    int is_valid56, is_valid57, is_valid58, is_valid59, is_valid60, is_valid61, is_valid62, is_valid63;
    
    /* Unrolled loop */
    READ_IDENT_STEP(0); READ_IDENT_STEP(1); READ_IDENT_STEP(2); READ_IDENT_STEP(3);
    READ_IDENT_STEP(4); READ_IDENT_STEP(5); READ_IDENT_STEP(6); READ_IDENT_STEP(7);
    READ_IDENT_STEP(8); READ_IDENT_STEP(9); READ_IDENT_STEP(10); READ_IDENT_STEP(11);
    READ_IDENT_STEP(12); READ_IDENT_STEP(13); READ_IDENT_STEP(14); READ_IDENT_STEP(15);
    READ_IDENT_STEP(16); READ_IDENT_STEP(17); READ_IDENT_STEP(18); READ_IDENT_STEP(19);
    READ_IDENT_STEP(20); READ_IDENT_STEP(21); READ_IDENT_STEP(22); READ_IDENT_STEP(23);
    READ_IDENT_STEP(24); READ_IDENT_STEP(25); READ_IDENT_STEP(26); READ_IDENT_STEP(27);
    READ_IDENT_STEP(28); READ_IDENT_STEP(29); READ_IDENT_STEP(30); READ_IDENT_STEP(31);
    READ_IDENT_STEP(32); READ_IDENT_STEP(33); READ_IDENT_STEP(34); READ_IDENT_STEP(35);
    READ_IDENT_STEP(36); READ_IDENT_STEP(37); READ_IDENT_STEP(38); READ_IDENT_STEP(39);
    READ_IDENT_STEP(40); READ_IDENT_STEP(41); READ_IDENT_STEP(42); READ_IDENT_STEP(43);
    READ_IDENT_STEP(44); READ_IDENT_STEP(45); READ_IDENT_STEP(46); READ_IDENT_STEP(47);
    READ_IDENT_STEP(48); READ_IDENT_STEP(49); READ_IDENT_STEP(50); READ_IDENT_STEP(51);
    READ_IDENT_STEP(52); READ_IDENT_STEP(53); READ_IDENT_STEP(54); READ_IDENT_STEP(55);
    READ_IDENT_STEP(56); READ_IDENT_STEP(57); READ_IDENT_STEP(58); READ_IDENT_STEP(59);
    READ_IDENT_STEP(60); READ_IDENT_STEP(61); READ_IDENT_STEP(62); READ_IDENT_STEP(63);
    
    tok.length = (lexer.source + lexer.pos) - tok.start;
    
    /* Check for keywords */
    check_keyword(&tok);
    
    return tok;
}

/* Forward declarations for dispatch */
static Token handle_eof();
static Token handle_digit();
static Token handle_alpha();
static Token handle_string();
static Token handle_lparen();
static Token handle_rparen();
static Token handle_comma();
static Token handle_semicolon();
static Token handle_colon();
static Token handle_newline();
static Token handle_space();
static Token handle_plus();
static Token handle_minus();
static Token handle_star();
static Token handle_slash();
static Token handle_eq();
static Token handle_lt();
static Token handle_gt();
static Token handle_dollar();
static Token handle_underscore();
static Token handle_dot();
static Token handle_invalid();

/* Function pointer dispatch table indexed by character class */
typedef Token (*lexer_fn)(void);
static const lexer_fn dispatch[CLASS_COUNT] = {
    [CLASS_INVALID] = handle_invalid,
    [CLASS_DIGIT] = handle_digit,
    [CLASS_ALPHA] = handle_alpha,
    [CLASS_STRING] = handle_string,
    [CLASS_LPAREN] = handle_lparen,
    [CLASS_RPAREN] = handle_rparen,
    [CLASS_COMMA] = handle_comma,
    [CLASS_SEMICOLON] = handle_semicolon,
    [CLASS_COLON] = handle_colon,
    [CLASS_NEWLINE] = handle_newline,
    [CLASS_SPACE] = handle_space,
    [CLASS_PLUS] = handle_plus,
    [CLASS_MINUS] = handle_minus,
    [CLASS_STAR] = handle_star,
    [CLASS_SLASH] = handle_slash,
    [CLASS_EQ] = handle_eq,
    [CLASS_LT] = handle_lt,
    [CLASS_GT] = handle_gt,
    [CLASS_DOLLAR] = handle_dollar,
    [CLASS_UNDERSCORE] = handle_underscore,
    [CLASS_DOT] = handle_dot,
    [CLASS_EOF] = handle_eof
};

/* Handler implementations */
static Token handle_eof() {
    Token tok;
    tok.type = TOK_EOF;
    tok.line = lexer.line;
    tok.length = 0;
    tok.start = lexer.source + lexer.pos;
    tok.keyword = KW_NONE;
    return tok;
}

static Token handle_digit() {
    return read_number();
}

static Token handle_alpha() {
    return read_ident();
}

static Token handle_string() {
    return read_string();
}

static Token make_single_char_token(TokenType type) {
    Token tok;
    tok.type = type;
    tok.line = lexer.line;
    tok.start = lexer.source + lexer.pos;
    tok.length = 1;
    tok.keyword = KW_NONE;
    lexer.pos = lexer.pos + 1;
    return tok;
}

static Token handle_lparen() {
    return make_single_char_token(TOK_LPAREN);
}

static Token handle_rparen() {
    return make_single_char_token(TOK_RPAREN);
}

static Token handle_comma() {
    return make_single_char_token(TOK_COMMA);
}

static Token handle_semicolon() {
    return make_single_char_token(TOK_SEMICOLON);
}

static Token handle_colon() {
    return make_single_char_token(TOK_COLON);
}

static Token handle_newline() {
    Token tok = make_single_char_token(TOK_NEWLINE);
    lexer.line = lexer.line + 1;
    return tok;
}

static Token handle_space() {
    skip_whitespace();
    return lexer_next_token();
}

static Token make_operator_token(Operator op) {
    Token tok;
    tok.type = TOK_OPERATOR;
    tok.line = lexer.line;
    tok.start = lexer.source + lexer.pos;
    tok.length = 1;
    tok.op = op;
    tok.keyword = KW_NONE;
    lexer.pos = lexer.pos + 1;
    return tok;
}

static Token handle_plus() {
    return make_operator_token(OP_ADD);
}

static Token handle_minus() {
    return make_operator_token(OP_SUB);
}

static Token handle_star() {
    return make_operator_token(OP_MUL);
}

static Token handle_slash() {
    return make_operator_token(OP_DIV);
}

static Token handle_eq() {
    return make_operator_token(OP_EQ);
}

static Token handle_lt() {
    return make_operator_token(OP_LT);
}

static Token handle_gt() {
    return make_operator_token(OP_GT);
}

static Token handle_dollar() {
    return read_ident();
}

static Token handle_underscore() {
    return read_ident();
}

static Token handle_dot() {
    return read_number();
}

static Token handle_invalid() {
    Token tok;
    tok.type = TOK_ERROR;
    tok.line = lexer.line;
    tok.start = lexer.source + lexer.pos;
    tok.length = 1;
    tok.keyword = KW_NONE;
    lexer.pos = lexer.pos + 1;
    return tok;
}

/* Main token dispatcher - pure function pointer dispatch, NO BRANCHES */
Token lexer_next_token(void) {
    skip_whitespace();
    
    char c = peek();
    uint8_t cls = get_char_class(c);
    
    /* Direct function pointer dispatch - single indirect call, no branches */
    return dispatch[cls]();
}
