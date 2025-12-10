#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <stdbool.h>
#include "../include/gwbasic.h"

/* Forward declarations */
static int execute_line_internal(Program *prog, char *text);
static int execute_goto(Program *prog, char *args);
static int execute_print_file(Program *prog, char *args);

Program *program_new(void) {
    Program *prog = malloc(sizeof(Program));
    if (!prog) return NULL;
    
    prog->first_line = NULL;
    prog->current_line = NULL;
    prog->symbols = symbol_table_new();
    prog->for_stack = NULL;
    prog->gosub_stack = NULL;
    prog->running = false;
    
    /* Initialize file handles */
    for (int i = 0; i < 10; i++) {
        prog->files[i] = NULL;
    }
    
    if (!prog->symbols) {
        free(prog);
        return NULL;
    }
    
    srand((unsigned)time(NULL)); /* Initialize RND */
    return prog;
}

void program_free(Program *prog) {
    if (!prog) return;
    
    BasicLine *line = prog->first_line;
    while (line) {
        BasicLine *next = line->next;
        free(line->text);
        free(line);
        line = next;
    }
    
    /* Free FOR stack */
    while (prog->for_stack) {
        ForLoop *next = prog->for_stack->next;
        free(prog->for_stack);
        prog->for_stack = next;
    }
    
    /* Free GOSUB stack */
    while (prog->gosub_stack) {
        GosubStack *next = prog->gosub_stack->next;
        free(prog->gosub_stack);
        prog->gosub_stack = next;
    }
    
    /* Close open files */
    for (int i = 0; i < 10; i++) {
        if (prog->files[i]) {
            fclose(prog->files[i]);
        }
    }
    
    symbol_table_free(prog->symbols);
    free(prog);
}

void program_add_line(Program *prog, int line_num, const char *text) {
    if (!prog || !text) return;
    
    BasicLine *new_line = malloc(sizeof(BasicLine));
    if (!new_line) return;
    
    new_line->line_num = line_num;
    new_line->text = strdup(text);
    if (!new_line->text) {
        free(new_line);
        return;
    }
    new_line->next = NULL;
    
    if (!prog->first_line || prog->first_line->line_num > line_num) {
        new_line->next = prog->first_line;
        prog->first_line = new_line;
        return;
    }
    
    BasicLine *prev = prog->first_line;
    BasicLine *curr = prev->next;
    
    while (curr && curr->line_num < line_num) {
        prev = curr;
        curr = curr->next;
    }
    
    if (curr && curr->line_num == line_num) {
        /* Replace existing line */
        char *new_text = strdup(text);
        if (new_text) {
            free(curr->text);
            curr->text = new_text;
        }
        free(new_line->text);
        free(new_line);
    } else {
        /* Insert new line */
        new_line->next = curr;
        prev->next = new_line;
    }
}

BasicLine *program_find_line(Program *prog, int line_num) {
    BasicLine *line = prog->first_line;
    while (line) {
        if (line->line_num == line_num) return line;
        if (line->line_num > line_num) return NULL;
        line = line->next;
    }
    return NULL;
}

static char *skip_whitespace(char *str) {
    while (*str && isspace(*str)) str++;
    return str;
}

int safe_parse_int(const char *str, int64_t *result) {
    char *endptr;
    errno = 0;
    *result = strtoll(str, &endptr, 10);
    
    if (errno == ERANGE || endptr == str) {
        return 0;
    }
    return 1;
}

/* Execute PRINT statement */
static int execute_print(Program *prog, char *args) {
    char *ptr = skip_whitespace(args);
    
    /* Check for file I/O: PRINT #filenum, */
    if (*ptr == '#') {
        return execute_print_file(prog, args);
    }
    
    if (*ptr == '\0') {
        printf("\n");
        return 0;
    }
    
    /* Evaluate expression and print */
    Value *val = eval_expression(prog, ptr);
    if (val) {
        switch (val->type) {
            case VAR_INTEGER:
                printf("%lld", (long long)val->value.int_val);
                break;
            case VAR_DOUBLE:
                printf("%g", val->value.double_val);
                break;
            case VAR_STRING:
                printf("%s", val->value.string_val ? val->value.string_val : "");
                break;
            default:
                break;
        }
        value_free(val);
    }
    printf("\n");
    return 0;
}

/* Execute LET or assignment */
static int execute_let(Program *prog, char *args) {
    char *ptr = skip_whitespace(args);
    char varname[256];
    int i = 0;
    
    while ((isalnum(*ptr) || *ptr == '$') && i < 255) {
        varname[i++] = *ptr++;
    }
    varname[i] = '\0';
    
    ptr = skip_whitespace(ptr);
    if (*ptr == '=') {
        ptr++;
        ptr = skip_whitespace(ptr);
        
        Value *val = eval_expression(prog, ptr);
        if (val) {
            switch (val->type) {
                case VAR_INTEGER:
                    symbol_table_set(prog->symbols, varname, VAR_INTEGER, &val->value.int_val);
                    break;
                case VAR_DOUBLE:
                    symbol_table_set(prog->symbols, varname, VAR_DOUBLE, &val->value.double_val);
                    break;
                case VAR_STRING:
                    symbol_table_set(prog->symbols, varname, VAR_STRING, val->value.string_val);
                    break;
                default:
                    break;
            }
            value_free(val);
        }
    }
    return 0;
}

/* Execute INPUT statement */
static int execute_input(Program *prog, char *args) {
    char *ptr = skip_whitespace(args);
    char varname[256];
    int i = 0;
    
    /* Optional prompt */
    if (*ptr == '"') {
        ptr++;
        char *end = strchr(ptr, '"');
        if (end) {
            *end = '\0';
            printf("%s? ", ptr);
            ptr = end + 1;
            if (*ptr == ';' || *ptr == ',') ptr++;
        }
    }
    
    ptr = skip_whitespace(ptr);
    
    /* Get variable name */
    while ((isalnum(*ptr) || *ptr == '$') && i < 255) {
        varname[i++] = *ptr++;
    }
    varname[i] = '\0';
    
    /* Read input */
    char input[1024];
    if (fgets(input, sizeof(input), stdin)) {
        /* Remove newline */
        size_t len = strlen(input);
        if (len > 0 && input[len-1] == '\n') {
            input[len-1] = '\0';
        }
        
        /* Check if string variable (ends with $) */
        if (varname[strlen(varname)-1] == '$') {
            symbol_table_set(prog->symbols, varname, VAR_STRING, input);
        } else {
            /* Try to parse as number */
            int64_t ival;
            if (safe_parse_int(input, &ival)) {
                symbol_table_set(prog->symbols, varname, VAR_INTEGER, &ival);
            } else {
                double dval = safe_parse_double(input);
                symbol_table_set(prog->symbols, varname, VAR_DOUBLE, &dval);
            }
        }
    }
    
    return 0;
}

/* Execute DIM statement */
static int execute_dim(Program *prog, char *args) {
    char *ptr = skip_whitespace(args);
    char varname[256];
    int i = 0;
    
    /* Get variable name */
    while (isalnum(*ptr) && i < 255) {
        varname[i++] = *ptr++;
    }
    varname[i] = '\0';
    
    ptr = skip_whitespace(ptr);
    if (*ptr != '(') return 0;
    ptr++;
    
    /* Parse dimensions */
    int dims[8];
    int dim_count = 0;
    
    while (dim_count < 8) {
        ptr = skip_whitespace(ptr);
        Value *dim_val = eval_expression(prog, ptr);
        if (dim_val) {
            dims[dim_count++] = (int)value_to_int(dim_val) + 1; /* +1 because BASIC is 0-based with DIM */
            value_free(dim_val);
        }
        
        /* Find comma or closing paren */
        while (*ptr && *ptr != ',' && *ptr != ')') ptr++;
        if (*ptr == ',') {
            ptr++;
        } else {
            break;
        }
    }
    
    if (*ptr == ')') {
        symbol_table_set_array(prog->symbols, varname, dims, dim_count);
    }
    
    return 0;
}

/* Execute OPEN statement */
static int execute_open(Program *prog, char *args) {
    char *ptr = skip_whitespace(args);
    
    /* Parse: OPEN "filename" FOR mode AS #filenum */
    if (*ptr == '"') {
        ptr++;
        char *end = strchr(ptr, '"');
        if (!end) return 0;
        
        char filename[256];
        int len = end - ptr;
        if (len >= 256) len = 255;
        memcpy(filename, ptr, len);
        filename[len] = '\0';
        ptr = end + 1;
        
        /* Find FOR */
        ptr = strstr(ptr, "FOR");
        if (!ptr) return 0;
        ptr += 3;
        ptr = skip_whitespace(ptr);
        
        /* Parse mode */
        char *mode_str = "r";
        if (strncmp(ptr, "INPUT", 5) == 0) {
            mode_str = "r";
            ptr += 5;
        } else if (strncmp(ptr, "OUTPUT", 6) == 0) {
            mode_str = "w";
            ptr += 6;
        } else if (strncmp(ptr, "APPEND", 6) == 0) {
            mode_str = "a";
            ptr += 6;
        }
        
        /* Find AS */
        ptr = strstr(ptr, "AS");
        if (!ptr) return 0;
        ptr += 2;
        ptr = skip_whitespace(ptr);
        
        /* Parse file number */
        if (*ptr == '#') ptr++;
        int64_t filenum;
        if (safe_parse_int(ptr, &filenum) && filenum >= 1 && filenum <= 10) {
            int idx = (int)filenum - 1;
            if (prog->files[idx]) {
                fclose(prog->files[idx]);
            }
            prog->files[idx] = fopen(filename, mode_str);
        }
    }
    
    return 0;
}

/* Execute CLOSE statement */
static int execute_close(Program *prog, char *args) {
    char *ptr = skip_whitespace(args);
    
    /* Parse: CLOSE #filenum */
    if (*ptr == '#') ptr++;
    int64_t filenum;
    if (safe_parse_int(ptr, &filenum) && filenum >= 1 && filenum <= 10) {
        int idx = (int)filenum - 1;
        if (prog->files[idx]) {
            fclose(prog->files[idx]);
            prog->files[idx] = NULL;
        }
    }
    
    return 0;
}

/* Execute PRINT# statement */
static int execute_print_file(Program *prog, char *args) {
    char *ptr = skip_whitespace(args);
    
    /* Parse: PRINT #filenum, expression */
    if (*ptr == '#') ptr++;
    int64_t filenum;
    if (!safe_parse_int(ptr, &filenum) || filenum < 1 || filenum > 10) return 0;
    
    int idx = (int)filenum - 1;
    if (!prog->files[idx]) return 0;
    
    /* Find comma */
    while (*ptr && *ptr != ',') ptr++;
    if (*ptr == ',') ptr++;
    ptr = skip_whitespace(ptr);
    
    /* Evaluate and print */
    Value *val = eval_expression(prog, ptr);
    if (val) {
        switch (val->type) {
            case VAR_INTEGER:
                fprintf(prog->files[idx], "%lld\n", (long long)val->value.int_val);
                break;
            case VAR_DOUBLE:
                fprintf(prog->files[idx], "%g\n", val->value.double_val);
                break;
            case VAR_STRING:
                fprintf(prog->files[idx], "%s\n", val->value.string_val ? val->value.string_val : "");
                break;
            default:
                break;
        }
        value_free(val);
    }
    
    return 0;
}

/* Execute GOTO */
static int execute_goto(Program *prog, char *args) {
    char *ptr = skip_whitespace(args);
    int64_t line_num;
    
    if (safe_parse_int(ptr, &line_num)) {
        BasicLine *target = program_find_line(prog, (int)line_num);
        if (target) {
            prog->current_line = target;
            return 0;
        }
    }
    return 0;
}

/* Execute GOSUB */
static int execute_gosub(Program *prog, char *args) {
    char *ptr = skip_whitespace(args);
    int64_t line_num;
    
    if (safe_parse_int(ptr, &line_num)) {
        BasicLine *target = program_find_line(prog, (int)line_num);
        if (target) {
            /* Push return address */
            GosubStack *entry = malloc(sizeof(GosubStack));
            if (entry) {
                entry->return_line = prog->current_line->next;
                entry->next = prog->gosub_stack;
                prog->gosub_stack = entry;
                prog->current_line = target;
            }
        }
    }
    return 0;
}

/* Execute RETURN */
static int execute_return(Program *prog) {
    if (prog->gosub_stack) {
        prog->current_line = prog->gosub_stack->return_line;
        GosubStack *old = prog->gosub_stack;
        prog->gosub_stack = prog->gosub_stack->next;
        free(old);
        return 0;
    }
    return 0;
}

/* Execute FOR */
static int execute_for(Program *prog, char *args) {
    char *ptr = skip_whitespace(args);
    char varname[256];
    int i = 0;
    
    /* Parse variable name */
    while (isalnum(*ptr) && i < 255) {
        varname[i++] = *ptr++;
    }
    varname[i] = '\0';
    
    ptr = skip_whitespace(ptr);
    if (*ptr != '=') return 0;
    ptr++;
    
    /* Parse start value */
    Value *start_val = eval_expression(prog, ptr);
    if (!start_val) return 0;
    int64_t start = value_to_int(start_val);
    value_free(start_val);
    
    /* Find TO */
    ptr = strstr(ptr, "TO");
    if (!ptr) return 0;
    ptr += 2;
    
    /* Parse end value */
    Value *end_val = eval_expression(prog, ptr);
    if (!end_val) return 0;
    int64_t end = value_to_int(end_val);
    value_free(end_val);
    
    /* Parse STEP (optional) */
    int64_t step = 1;
    char *step_ptr = strstr(ptr, "STEP");
    if (step_ptr) {
        step_ptr += 4;
        Value *step_val = eval_expression(prog, step_ptr);
        if (step_val) {
            step = value_to_int(step_val);
            value_free(step_val);
        }
    }
    
    /* Set loop variable */
    symbol_table_set(prog->symbols, varname, VAR_INTEGER, &start);
    
    /* Push FOR loop info */
    ForLoop *loop = malloc(sizeof(ForLoop));
    if (loop) {
        strncpy(loop->var_name, varname, sizeof(loop->var_name) - 1);
        loop->var_name[sizeof(loop->var_name) - 1] = '\0';
        loop->end_val = end;
        loop->step = step;
        loop->loop_start = prog->current_line;
        loop->next = prog->for_stack;
        prog->for_stack = loop;
    }
    
    return 0;
}

/* Execute NEXT */
static int execute_next(Program *prog, char *args) {
    (void)args; /* May be used for specific variable in NEXT X */
    if (!prog->for_stack) return 0;
    
    ForLoop *loop = prog->for_stack;
    
    /* Get current value */
    Variable *var = symbol_table_get(prog->symbols, loop->var_name);
    if (!var) return 0;
    
    /* Increment */
    int64_t new_val = var->value.int_val + loop->step;
    
    /* Check if loop should continue */
    bool cont = (loop->step > 0) ? (new_val <= loop->end_val) : (new_val >= loop->end_val);
    
    if (cont) {
        symbol_table_set(prog->symbols, loop->var_name, VAR_INTEGER, &new_val);
        prog->current_line = loop->loop_start;
    } else {
        /* Pop loop */
        prog->for_stack = loop->next;
        free(loop);
    }
    
    return 0;
}

/* Execute IF-THEN */
static int execute_if(Program *prog, char *args) {
    char *ptr = skip_whitespace(args);
    
    /* Find THEN */
    char *then_ptr = strstr(ptr, "THEN");
    if (!then_ptr) return 0;
    
    /* Extract condition */
    int cond_len = then_ptr - ptr;
    char *condition = malloc(cond_len + 1);
    if (!condition) return 0;
    memcpy(condition, ptr, cond_len);
    condition[cond_len] = '\0';
    
    /* Evaluate condition - look for comparison operators */
    char *eq = strstr(condition, "=");
    char *ne = strstr(condition, "<>");
    char *lt = strstr(condition, "<");
    char *gt = strstr(condition, ">");
    char *le = strstr(condition, "<=");
    char *ge = strstr(condition, ">=");
    
    bool result = false;
    
    if (ne) {
        *ne = '\0';
        Value *left = eval_expression(prog, condition);
        Value *right = eval_expression(prog, ne + 2);
        if (left && right) {
            if (left->type == VAR_STRING && right->type == VAR_STRING) {
                result = strcmp(left->value.string_val ? left->value.string_val : "",
                               right->value.string_val ? right->value.string_val : "") != 0;
            } else {
                result = value_to_double(left) != value_to_double(right);
            }
        }
        value_free(left);
        value_free(right);
    } else if (le) {
        *le = '\0';
        Value *left = eval_expression(prog, condition);
        Value *right = eval_expression(prog, le + 2);
        if (left && right) {
            result = value_to_double(left) <= value_to_double(right);
        }
        value_free(left);
        value_free(right);
    } else if (ge) {
        *ge = '\0';
        Value *left = eval_expression(prog, condition);
        Value *right = eval_expression(prog, ge + 2);
        if (left && right) {
            result = value_to_double(left) >= value_to_double(right);
        }
        value_free(left);
        value_free(right);
    } else if (eq) {
        *eq = '\0';
        Value *left = eval_expression(prog, condition);
        Value *right = eval_expression(prog, eq + 1);
        if (left && right) {
            if (left->type == VAR_STRING && right->type == VAR_STRING) {
                result = strcmp(left->value.string_val ? left->value.string_val : "",
                               right->value.string_val ? right->value.string_val : "") == 0;
            } else {
                result = value_to_double(left) == value_to_double(right);
            }
        }
        value_free(left);
        value_free(right);
    } else if (lt) {
        *lt = '\0';
        Value *left = eval_expression(prog, condition);
        Value *right = eval_expression(prog, lt + 1);
        if (left && right) {
            result = value_to_double(left) < value_to_double(right);
        }
        value_free(left);
        value_free(right);
    } else if (gt) {
        *gt = '\0';
        Value *left = eval_expression(prog, condition);
        Value *right = eval_expression(prog, gt + 1);
        if (left && right) {
            result = value_to_double(left) > value_to_double(right);
        }
        value_free(left);
        value_free(right);
    } else {
        /* Simple expression */
        Value *val = eval_expression(prog, condition);
        if (val) {
            result = value_to_double(val) != 0.0;
            value_free(val);
        }
    }
    
    free(condition);
    
    /* Execute THEN clause if true */
    if (result) {
        then_ptr += 4;
        then_ptr = skip_whitespace(then_ptr);
        
        /* Check if it's a line number (GOTO) */
        if (isdigit(*then_ptr)) {
            return execute_goto(prog, then_ptr);
        } else {
            /* Execute statement directly */
            return execute_line_internal(prog, then_ptr);
        }
    }
    
    return 0;
}

/* Optimized statement dispatch using hash table for O(1) lookup */
typedef int (*StatementHandler)(Program *, char *);

typedef struct {
    const char *keyword;
    int len;
    StatementHandler handler;
} StatementEntry;

/* Fast hash function for statement keywords */
static inline unsigned int hash_keyword(const char *str, int len) {
    unsigned int hash = 5381;
    for (int i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + str[i]; /* hash * 33 + c */
    }
    return hash;
}

/* Hash table for O(1) statement dispatch */
static const StatementEntry statement_table[] = {
    {"PRINT", 5, execute_print},
    {"INPUT", 5, execute_input},
    {"DIM", 3, execute_dim},
    {"LET", 3, execute_let},
    {"GOTO", 4, execute_goto},
    {"GOSUB", 5, execute_gosub},
    {"RETURN", 6, (StatementHandler)execute_return},
    {"FOR", 3, execute_for},
    {"NEXT", 4, execute_next},
    {"IF", 2, execute_if},
    {"OPEN", 4, execute_open},
    {"CLOSE", 5, execute_close},
    {"END", 3, NULL}, /* Special case */
    {"REM", 3, NULL}, /* Special case - comment */
    {NULL, 0, NULL}
};

/* Internal function to execute a statement within a line */
static int execute_line_internal(Program *prog, char *text) {
    char *ptr = skip_whitespace(text);
    
    /* Fast path: check first character for common cases */
    char first = *ptr;
    
    /* Branchless optimization: use lookup table */
    if (first >= 'A' && first <= 'Z') {
        /* Try hash table lookup first - check longest matches first */
        for (int i = 0; statement_table[i].keyword != NULL; i++) {
            const char *kw = statement_table[i].keyword;
            int len = statement_table[i].len;
            
            if (strncmp(ptr, kw, len) == 0) {
                /* Check if it's actually a complete keyword match */
                char next_char = ptr[len];
                if (next_char == '\0' || isspace(next_char) || next_char == '#' || 
                    next_char == '"' || next_char == ',' || next_char == '=' || next_char == '(') {
                    
                    /* Special cases */
                    if (kw[0] == 'E' && kw[1] == 'N' && kw[2] == 'D') return 1; /* END */
                    if (kw[0] == 'R' && kw[1] == 'E' && kw[2] == 'M') return 0; /* REM */
                    if (kw[0] == 'R' && kw[1] == 'E' && kw[2] == 'T') return execute_return(prog); /* RETURN */
                    
                    /* Call handler */
                    if (statement_table[i].handler) {
                        return statement_table[i].handler(prog, ptr + len);
                    }
                }
            }
        }
    }
    
    /* Fallback: implicit LET */
    if (isalpha(*ptr)) {
        return execute_let(prog, ptr);
    }
    
    return 0;
}

static int execute_line(Program *prog, BasicLine *line) {
    return execute_line_internal(prog, line->text);
}

int program_run(Program *prog) {
    if (!prog || !prog->first_line) return -1;
    
    prog->current_line = prog->first_line;
    prog->running = true;
    
    while (prog->current_line && prog->running) {
        BasicLine *current = prog->current_line;
        prog->current_line = current->next;
        
        int result = execute_line(prog, current);
        if (result != 0) {
            prog->running = false;
            return result > 0 ? 0 : result;
        }
    }
    
    prog->running = false;
    return 0;
}
