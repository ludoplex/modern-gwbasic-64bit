#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "../include/gwbasic.h"

Program *program_new(void) {
    Program *prog = malloc(sizeof(Program));
    if (!prog) return NULL;
    
    prog->first_line = NULL;
    prog->current_line = NULL;
    prog->symbols = symbol_table_new();
    
    if (!prog->symbols) {
        free(prog);
        return NULL;
    }
    
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
        // Replace existing line
        char *new_text = strdup(text);
        if (new_text) {
            free(curr->text);
            curr->text = new_text;
        }
        free(new_line->text);
        free(new_line);
    } else {
        // Insert new line
        new_line->next = curr;
        prev->next = new_line;
    }
}

static char *skip_whitespace(char *str) {
    while (*str && isspace(*str)) str++;
    return str;
}

static int safe_parse_int(const char *str, int64_t *result) {
    char *endptr;
    errno = 0;
    *result = strtoll(str, &endptr, 10);
    
    if (errno == ERANGE || endptr == str) {
        return 0; // Parse error
    }
    return 1; // Success
}

static int execute_print(Program *prog, char *args) {
    char *ptr = skip_whitespace(args);
    
    if (*ptr == '"') {
        ptr++;
        char *end = strchr(ptr, '"');
        if (end) {
            *end = '\0';
            printf("%s", ptr);
        }
    } else if (isdigit(*ptr) || *ptr == '-') {
        int64_t val;
        if (safe_parse_int(ptr, &val)) {
            printf("%lld", (long long)val);
        }
    } else if (isalpha(*ptr)) {
        char varname[256];
        int i = 0;
        while (isalnum(*ptr) && i < 255) {
            varname[i++] = *ptr++;
        }
        varname[i] = '\0';
        
        Variable *var = symbol_table_get(prog->symbols, varname);
        if (var) {
            switch (var->type) {
                case VAR_INTEGER:
                    printf("%lld", (long long)var->value.int_val);
                    break;
                case VAR_SINGLE:
                    printf("%f", var->value.float_val);
                    break;
                case VAR_DOUBLE:
                    printf("%lf", var->value.double_val);
                    break;
                case VAR_STRING:
                    printf("%s", var->value.string_val ? var->value.string_val : "");
                    break;
            }
        }
    }
    
    printf("\n");
    return 0;
}

static int execute_let(Program *prog, char *args) {
    char *ptr = skip_whitespace(args);
    char varname[256];
    int i = 0;
    
    while (isalnum(*ptr) && i < 255) {
        varname[i++] = *ptr++;
    }
    varname[i] = '\0';
    
    ptr = skip_whitespace(ptr);
    if (*ptr == '=') {
        ptr++;
        ptr = skip_whitespace(ptr);
        
        if (*ptr == '"') {
            ptr++;
            char *end = strchr(ptr, '"');
            if (end) {
                *end = '\0';
                symbol_table_set(prog->symbols, varname, VAR_STRING, ptr);
            }
        } else {
            int64_t val;
            if (safe_parse_int(ptr, &val)) {
                symbol_table_set(prog->symbols, varname, VAR_INTEGER, &val);
            }
        }
    }
    
    return 0;
}

static int execute_line(Program *prog, BasicLine *line) {
    char *text = line->text;
    char *ptr = skip_whitespace(text);
    
    if (strncmp(ptr, "PRINT", 5) == 0) {
        return execute_print(prog, ptr + 5);
    } else if (strncmp(ptr, "LET", 3) == 0) {
        return execute_let(prog, ptr + 3);
    } else if (isalpha(*ptr)) {
        // Implicit LET
        return execute_let(prog, ptr);
    } else if (strncmp(ptr, "END", 3) == 0) {
        return 1; // Signal end
    }
    
    return 0;
}

int program_run(Program *prog) {
    if (!prog || !prog->first_line) return -1;
    
    prog->current_line = prog->first_line;
    
    while (prog->current_line) {
        int result = execute_line(prog, prog->current_line);
        if (result != 0) {
            return result > 0 ? 0 : result;
        }
        prog->current_line = prog->current_line->next;
    }
    
    return 0;
}
