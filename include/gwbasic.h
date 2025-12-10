#ifndef GWBASIC_H
#define GWBASIC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Forward declarations */
typedef struct BasicLine BasicLine;
typedef struct ForLoop ForLoop;
typedef struct GosubStack GosubStack;

/* Variable types */
typedef enum {
    VAR_INTEGER,
    VAR_SINGLE,
    VAR_DOUBLE,
    VAR_STRING
} VarType;

/* Value container for expression results */
typedef struct {
    VarType type;
    union {
        int64_t int_val;
        double double_val;
        char *string_val;
    } value;
} Value;

/* Variable structure */
typedef struct {
    char name[256];
    VarType type;
    union {
        int64_t int_val;
        float float_val;
        double double_val;
        char *string_val;
    } value;
    /* Array support */
    bool is_array;
    int dims[8];  /* Up to 8 dimensions */
    int dim_count;
    void *array_data;  /* Pointer to array storage */
} Variable;

/* Symbol table */
typedef struct {
    Variable *vars;
    size_t count;
    size_t capacity;
} SymbolTable;

/* FOR loop stack entry */
struct ForLoop {
    char var_name[256];
    int64_t end_val;
    int64_t step;
    BasicLine *loop_start;
    ForLoop *next;
};

/* GOSUB return stack entry */
struct GosubStack {
    BasicLine *return_line;
    GosubStack *next;
};

/* BASIC line structure */
struct BasicLine {
    int line_num;
    char *text;
    BasicLine *next;
};

/* Program structure */
typedef struct {
    BasicLine *first_line;
    BasicLine *current_line;
    SymbolTable *symbols;
    ForLoop *for_stack;
    GosubStack *gosub_stack;
    bool running;
} Program;

/* Function prototypes */
Program *program_new(void);
void program_free(Program *prog);
void program_add_line(Program *prog, int line_num, const char *text);
int program_run(Program *prog);
BasicLine *program_find_line(Program *prog, int line_num);

SymbolTable *symbol_table_new(void);
void symbol_table_free(SymbolTable *table);
Variable *symbol_table_get(SymbolTable *table, const char *name);
void symbol_table_set(SymbolTable *table, const char *name, VarType type, void *value);
void symbol_table_set_array(SymbolTable *table, const char *name, int *dims, int dim_count);

/* Expression evaluation */
Value *eval_expression(Program *prog, const char *expr);
void value_free(Value *val);
double value_to_double(Value *val);
int64_t value_to_int(Value *val);

/* Built-in functions */
double builtin_sin(double x);
double builtin_cos(double x);
double builtin_tan(double x);
double builtin_sqr(double x);
double builtin_abs(double x);
double builtin_int(double x);
double builtin_rnd(double x);
int builtin_len(const char *s);
char *builtin_left(const char *s, int n);
char *builtin_right(const char *s, int n);
char *builtin_mid(const char *s, int start, int len);

/* Utility functions */
int safe_parse_int(const char *str, int64_t *result);
double safe_parse_double(const char *str);

/* Assembly-optimized core functions */
extern int64_t asm_add_int(int64_t a, int64_t b);
extern int64_t asm_sub_int(int64_t a, int64_t b);
extern int64_t asm_mul_int(int64_t a, int64_t b);
extern int64_t asm_div_int(int64_t a, int64_t b);
extern double asm_add_double(double a, double b);
extern double asm_sub_double(double a, double b);
extern double asm_mul_double(double a, double b);
extern double asm_div_double(double a, double b);

#endif /* GWBASIC_H */
