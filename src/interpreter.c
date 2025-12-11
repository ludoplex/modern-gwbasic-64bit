#include "interpreter.h"
#include "basic_functions.h"
#include "branchless_scalar.h"
#include "hash_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Initialize interpreter */
Interpreter *interpreter_create() {
    Interpreter *interp = (Interpreter *)malloc(sizeof(Interpreter));
    
    interp->program = (Program *)malloc(sizeof(Program));
    interp->program->lines = NULL;
    interp->program->line_count = 0;
    interp->program->line_capacity = 0;
    
    interp->var_table = ht_create(64);
    interp->func_table = ht_create(16);
    interp->line_table = ht_create(64);
    
    interp->data_items = NULL;
    interp->data_count = 0;
    interp->data_index = 0;
    
    interp->call_stack = NULL;
    interp->call_stack_size = 0;
    
    interp->for_stack = NULL;
    interp->for_stack_size = 0;
    
    interp->current_line = 0;
    interp->running = 0;
    interp->error_code = 0;
    
    return interp;
}

/* Free interpreter */
void interpreter_destroy(Interpreter *interp) {
    ht_destroy((HashTable *)interp->var_table);
    ht_destroy((HashTable *)interp->func_table);
    ht_destroy((HashTable *)interp->line_table);
    
    free(interp->data_items);
    free(interp->call_stack);
    free(interp->for_stack);
    free(interp->program);
    free(interp);
}

/* Execute PRINT statement */
void exec_print(Interpreter *interp, Value *value) {
    int is_int = (value->type == VAL_INT);
    int is_float = (value->type == VAL_FLOAT);
    int is_string = (value->type == VAL_STRING);
    
    /* Branchless output dispatch */
    printf("%lld", (long long)bl_select_i64((int64_t)is_int, value->int_val, 0));
    printf("%s", is_float ? "" : "");
    printf("%.10g", is_float ? value->float_val : 0.0);
    printf("%s", is_string ? value->str_val : "");
}

/* Simple expression evaluator (branchless where possible) */
Value eval_expression(Interpreter *interp, const char *expr) {
    Value result;
    result.type = VAL_INT;
    result.int_val = 0;
    
    /* Simple number parsing for demo */
    int is_digit = (expr[0] >= '0' && expr[0] <= '9');
    
    int64_t num = 0;
    int i = 0;
    while (expr[i] >= '0' && expr[i] <= '9') {
        num = num * 10 + (expr[i] - '0');
        i++;
    }
    
    result.int_val = num;
    return result;
}

/* Execute single statement (simplified for demo) */
void exec_statement(Interpreter *interp, const char *stmt) {
    /* Skip line number */
    const char *p = stmt;
    while (*p >= '0' && *p <= '9') {
        p++;
    }
    while (*p == ' ') {
        p++;
    }
    
    /* Check for PRINT */
    int is_print = (strncmp(p, "PRINT", 5) == 0);
    const char *orig_p = p;
    p += is_print * 6;
    
    /* Skip whitespace */
    while (*p == ' ') {
        p++;
    }
    
    /* Check for string literal */
    int is_string = (*p == '"');
    p += is_string;
    
    /* Print string when PRINT command detected */
    int should_print_str = is_print & is_string;
    while (should_print_str > 0) {
        while (*p && *p != '"' && *p != '\n') {
            putchar(*p);
            p++;
        }
        putchar('\n');
        should_print_str = 0;
    }
    
    /* Print number when PRINT command and number detected */
    int is_number = is_print & (*p >= '0') & (*p <= '9');
    while (is_number > 0) {
        Value val = eval_expression(interp, p);
        printf("%lld\n", (long long)val.int_val);
        is_number = 0;
    }
}

/* Run program */
void interpreter_run(Interpreter *interp) {
    interp->running = 1;
    interp->current_line = 0;
    
    while (interp->running && interp->current_line < (int)interp->program->line_count) {
        ProgramLine *line = &interp->program->lines[interp->current_line];
        exec_statement(interp, line->text);
        interp->current_line++;
    }
    
    interp->running = 0;
}

/* Load program from text */
void interpreter_load_program(Interpreter *interp, const char *source) {
    /* Count lines by counting newlines */
    int line_count = 1; /* At least one line */
    const char *p = source;
    while (*p) {
        line_count += (*p == '\n');
        p++;
    }
    
    /* Allocate lines */
    interp->program->line_capacity = (size_t)line_count;
    interp->program->lines = (ProgramLine *)calloc((size_t)line_count, sizeof(ProgramLine));
    
    /* Parse lines */
    p = source;
    int line_idx = 0;
    const char *line_start = p;
    
    while (*p) {
        int is_newline = (*p == '\n');
        
        /* When we hit a newline, store the line */
        while (is_newline > 0) {
            size_t line_len = (size_t)(p - line_start);
            char *line_text = (char *)malloc(line_len + 1);
            memcpy(line_text, line_start, line_len);
            line_text[line_len] = '\0';
            
            interp->program->lines[line_idx].text = line_text;
            interp->program->lines[line_idx].line_number = line_idx + 1;
            line_idx++;
            
            line_start = p + 1;
            is_newline = 0;
        }
        
        p++;
    }
    
    /* Handle last line (no trailing newline) */
    size_t line_len = (size_t)(p - line_start);
    int has_content = (line_len > 0);
    
    while (has_content > 0) {
        char *line_text = (char *)malloc(line_len + 1);
        memcpy(line_text, line_start, line_len);
        line_text[line_len] = '\0';
        
        interp->program->lines[line_idx].text = line_text;
        interp->program->lines[line_idx].line_number = line_idx + 1;
        line_idx++;
        has_content = 0;
    }
    
    interp->program->line_count = (size_t)line_idx;
}
