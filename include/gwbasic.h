#ifndef GWBASIC_H
#define GWBASIC_H

#include <stdint.h>
#include <stddef.h>

/* Variable types */
typedef enum {
    VAR_INTEGER,
    VAR_SINGLE,
    VAR_DOUBLE,
    VAR_STRING
} VarType;

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
} Variable;

/* Symbol table */
typedef struct {
    Variable *vars;
    size_t count;
    size_t capacity;
} SymbolTable;

/* BASIC line structure */
typedef struct BasicLine {
    int line_num;
    char *text;
    struct BasicLine *next;
} BasicLine;

/* Program structure */
typedef struct {
    BasicLine *first_line;
    BasicLine *current_line;
    SymbolTable *symbols;
} Program;

/* Function prototypes */
Program *program_new(void);
void program_free(Program *prog);
void program_add_line(Program *prog, int line_num, const char *text);
int program_run(Program *prog);

SymbolTable *symbol_table_new(void);
void symbol_table_free(SymbolTable *table);
Variable *symbol_table_get(SymbolTable *table, const char *name);
void symbol_table_set(SymbolTable *table, const char *name, VarType type, void *value);

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
