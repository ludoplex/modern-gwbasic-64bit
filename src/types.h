#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stddef.h>

/* Token types for lexer */
typedef enum {
    TOK_EOF = 0,
    TOK_NUMBER,
    TOK_STRING,
    TOK_IDENT,
    TOK_KEYWORD,
    TOK_OPERATOR,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_COMMA,
    TOK_SEMICOLON,
    TOK_COLON,
    TOK_NEWLINE,
    TOK_ERROR
} TokenType;

/* Keyword IDs */
typedef enum {
    KW_NONE = 0,
    KW_PRINT,
    KW_LET,
    KW_GOTO,
    KW_GOSUB,
    KW_RETURN,
    KW_FOR,
    KW_TO,
    KW_STEP,
    KW_NEXT,
    KW_IF,
    KW_THEN,
    KW_ELSE,
    KW_END,
    KW_STOP,
    KW_INPUT,
    KW_READ,
    KW_DATA,
    KW_RESTORE,
    KW_DIM,
    KW_REM,
    KW_ON,
    KW_DEF,
    KW_FN,
    KW_OPEN,
    KW_CLOSE,
    KW_LINE,
    KW_POKE,
    KW_PEEK,
    KW_CLS,
    KW_RUN,
    KW_LIST,
    KW_NEW,
    KW_SAVE,
    KW_LOAD
} Keyword;

/* Operator types */
typedef enum {
    OP_NONE = 0,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_POW,
    OP_EQ,
    OP_NE,
    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE,
    OP_AND,
    OP_OR,
    OP_NOT,
    OP_ASSIGN
} Operator;

/* Value types */
typedef enum {
    VAL_NONE = 0,
    VAL_INT,
    VAL_FLOAT,
    VAL_STRING
} ValueType;

/* Runtime value */
typedef struct {
    ValueType type;
    union {
        int64_t int_val;
        double float_val;
        char *str_val;
    };
} Value;

/* Token */
typedef struct {
    TokenType type;
    Keyword keyword;
    Operator op;
    const char *start;
    size_t length;
    int line;
    Value value;
} Token;

/* AST Node types */
typedef enum {
    NODE_NONE = 0,
    NODE_NUMBER,
    NODE_STRING,
    NODE_IDENT,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_CALL,
    NODE_ARRAY_ACCESS,
    NODE_STATEMENT,
    NODE_PROGRAM
} NodeType;

/* Forward declaration */
struct ASTNode;

/* AST Node */
typedef struct ASTNode {
    NodeType type;
    Value value;
    Operator op;
    Keyword keyword;
    char *name;
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *condition;
    struct ASTNode **children;
    size_t child_count;
    int line_number;
} ASTNode;

/* Program line */
typedef struct {
    int line_number;
    char *text;
    ASTNode *ast;
    size_t bytecode_offset;
} ProgramLine;

/* BASIC program */
typedef struct {
    ProgramLine *lines;
    size_t line_count;
    size_t line_capacity;
} Program;

/* Interpreter state */
typedef struct {
    Program *program;
    void *var_table;        /* HashTable for variables */
    void *func_table;       /* HashTable for user-defined functions */
    void *line_table;       /* HashTable for line numbers */
    void *data_items;       /* Array of data items */
    size_t data_count;
    size_t data_index;
    void *call_stack;       /* Array for GOSUB return addresses */
    size_t call_stack_size;
    void *for_stack;        /* Array for FOR loop state */
    size_t for_stack_size;
    int current_line;
    int running;
    int error_code;
} Interpreter;

#endif /* TYPES_H */
