#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "../include/gwbasic.h"

/* Fast math operations - direct calls for performance */
double builtin_sin(double x) { return sin(x); }
double builtin_cos(double x) { return cos(x); }
double builtin_tan(double x) { return tan(x); }
double builtin_sqr(double x) { return sqrt(x); }
double builtin_abs(double x) { return fabs(x); }
double builtin_int(double x) { return floor(x); }
double builtin_rnd(double x) { 
    (void)x; /* unused */
    return (double)rand() / (double)RAND_MAX; 
}

/* Fast string operations - minimal allocations */
int builtin_len(const char *s) {
    return s ? (int)strlen(s) : 0;
}

char *builtin_left(const char *s, int n) {
    if (!s || n <= 0) return strdup("");
    int len = (int)strlen(s);
    if (n > len) n = len;
    char *result = malloc(n + 1);
    if (!result) return NULL;
    memcpy(result, s, n);
    result[n] = '\0';
    return result;
}

char *builtin_right(const char *s, int n) {
    if (!s || n <= 0) return strdup("");
    int len = (int)strlen(s);
    if (n > len) n = len;
    char *result = malloc(n + 1);
    if (!result) return NULL;
    memcpy(result, s + (len - n), n);
    result[n] = '\0';
    return result;
}

char *builtin_mid(const char *s, int start, int len) {
    if (!s || start < 1 || len <= 0) return strdup("");
    int slen = (int)strlen(s);
    start--; /* Convert to 0-based */
    if (start >= slen) return strdup("");
    if (start + len > slen) len = slen - start;
    char *result = malloc(len + 1);
    if (!result) return NULL;
    memcpy(result, s + start, len);
    result[len] = '\0';
    return result;
}

double safe_parse_double(const char *str) {
    char *endptr;
    double result = strtod(str, &endptr);
    if (endptr == str) return 0.0;
    return result;
}

void value_free(Value *val) {
    if (!val) return;
    if (val->type == VAR_STRING && val->value.string_val) {
        free(val->value.string_val);
    }
    free(val);
}

double value_to_double(Value *val) {
    if (!val) return 0.0;
    switch (val->type) {
        case VAR_INTEGER: return (double)val->value.int_val;
        case VAR_DOUBLE: return val->value.double_val;
        case VAR_SINGLE: return (double)val->value.int_val; /* fallback */
        case VAR_STRING: return val->value.string_val ? atof(val->value.string_val) : 0.0;
    }
    return 0.0;
}

int64_t value_to_int(Value *val) {
    if (!val) return 0;
    switch (val->type) {
        case VAR_INTEGER: return val->value.int_val;
        case VAR_DOUBLE: return (int64_t)val->value.double_val;
        case VAR_SINGLE: return val->value.int_val; /* fallback */
        case VAR_STRING: return val->value.string_val ? atoll(val->value.string_val) : 0;
    }
    return 0;
}

/* Simple recursive descent expression parser */
static const char *skip_ws(const char *s) {
    while (*s && isspace(*s)) s++;
    return s;
}

/* Forward declarations for recursive parsing */
static Value *parse_expr(Program *prog, const char **expr);
static Value *parse_term(Program *prog, const char **expr);
static Value *parse_factor(Program *prog, const char **expr);

static Value *parse_factor(Program *prog, const char **expr) {
    const char *s = skip_ws(*expr);
    Value *result = malloc(sizeof(Value));
    if (!result) return NULL;
    
    /* String literal */
    if (*s == '"') {
        s++;
        const char *end = strchr(s, '"');
        if (!end) {
            result->type = VAR_STRING;
            result->value.string_val = strdup("");
            *expr = s;
            return result;
        }
        int len = end - s;
        result->type = VAR_STRING;
        result->value.string_val = malloc(len + 1);
        if (result->value.string_val) {
            memcpy(result->value.string_val, s, len);
            result->value.string_val[len] = '\0';
        }
        *expr = end + 1;
        return result;
    }
    
    /* Parentheses */
    if (*s == '(') {
        s++;
        *expr = s;
        Value *val = parse_expr(prog, expr);
        s = skip_ws(*expr);
        if (*s == ')') s++;
        *expr = s;
        return val;
    }
    
    /* Number */
    if (isdigit(*s) || *s == '-' || *s == '.') {
        char *endptr;
        double d = strtod(s, &endptr);
        if (endptr > s) {
            if (strchr(s, '.') || strchr(s, 'e') || strchr(s, 'E')) {
                result->type = VAR_DOUBLE;
                result->value.double_val = d;
            } else {
                result->type = VAR_INTEGER;
                result->value.int_val = (int64_t)d;
            }
            *expr = endptr;
            return result;
        }
    }
    
    /* Variable or function */
    if (isalpha(*s)) {
        char name[256];
        int i = 0;
        while ((isalnum(*s) || *s == '$') && i < 255) {
            name[i++] = *s++;
        }
        name[i] = '\0';
        
        /* Check for function calls */
        s = skip_ws(s);
        if (*s == '(') {
            s++;
            *expr = s;
            Value *arg = parse_expr(prog, expr);
            s = skip_ws(*expr);
            if (*s == ')') s++;
            *expr = s;
            
            /* Built-in functions */
            
            /* String functions */
            if (strcmp(name, "LEN") == 0) {
                result->type = VAR_INTEGER;
                if (arg->type == VAR_STRING) {
                    result->value.int_val = builtin_len(arg->value.string_val);
                } else {
                    result->value.int_val = 0;
                }
                value_free(arg);
                return result;
            } else if (strcmp(name, "LEFT$") == 0 || strcmp(name, "RIGHT$") == 0 || strcmp(name, "MID$") == 0) {
                /* These need additional parameters - parse them */
                s = skip_ws(*expr);
                if (*s == ',') {
                    s++;
                    *expr = s;
                    Value *arg2 = parse_expr(prog, expr);
                    s = skip_ws(*expr);
                    
                    if (strcmp(name, "LEFT$") == 0) {
                        result->type = VAR_STRING;
                        if (arg->type == VAR_STRING) {
                            result->value.string_val = builtin_left(arg->value.string_val, (int)value_to_int(arg2));
                        } else {
                            result->value.string_val = strdup("");
                        }
                    } else if (strcmp(name, "RIGHT$") == 0) {
                        result->type = VAR_STRING;
                        if (arg->type == VAR_STRING) {
                            result->value.string_val = builtin_right(arg->value.string_val, (int)value_to_int(arg2));
                        } else {
                            result->value.string_val = strdup("");
                        }
                    } else if (strcmp(name, "MID$") == 0) {
                        /* MID$ can have 2 or 3 parameters */
                        int len = -1;
                        if (*s == ',') {
                            s++;
                            *expr = s;
                            Value *arg3 = parse_expr(prog, expr);
                            len = (int)value_to_int(arg3);
                            value_free(arg3);
                            s = skip_ws(*expr);
                        }
                        
                        result->type = VAR_STRING;
                        if (arg->type == VAR_STRING) {
                            int start = (int)value_to_int(arg2);
                            if (len == -1) len = 999999; /* Large number for rest of string */
                            result->value.string_val = builtin_mid(arg->value.string_val, start, len);
                        } else {
                            result->value.string_val = strdup("");
                        }
                    }
                    
                    value_free(arg2);
                    if (*s == ')') s++;
                    *expr = s;
                    value_free(arg);
                    return result;
                }
            }
            
            /* Math functions */
            double val = value_to_double(arg);
            value_free(arg);
            
            result->type = VAR_DOUBLE;
            if (strcmp(name, "SIN") == 0) result->value.double_val = builtin_sin(val);
            else if (strcmp(name, "COS") == 0) result->value.double_val = builtin_cos(val);
            else if (strcmp(name, "TAN") == 0) result->value.double_val = builtin_tan(val);
            else if (strcmp(name, "SQR") == 0) result->value.double_val = builtin_sqr(val);
            else if (strcmp(name, "ABS") == 0) result->value.double_val = builtin_abs(val);
            else if (strcmp(name, "INT") == 0) result->value.double_val = builtin_int(val);
            else if (strcmp(name, "RND") == 0) result->value.double_val = builtin_rnd(val);
            else if (strcmp(name, "ASC") == 0) {
                /* ASC returns ASCII value of first character */
                result->type = VAR_INTEGER;
                result->value.int_val = (int64_t)val;
            }
            else result->value.double_val = 0.0;
            
            return result;
        }
        
        /* Variable lookup */
        *expr = s;
        Variable *var = symbol_table_get(prog->symbols, name);
        if (var) {
            result->type = var->type;
            switch (var->type) {
                case VAR_INTEGER:
                    result->value.int_val = var->value.int_val;
                    break;
                case VAR_DOUBLE:
                    result->value.double_val = var->value.double_val;
                    break;
                case VAR_SINGLE:
                    result->value.double_val = var->value.float_val;
                    break;
                case VAR_STRING:
                    result->value.string_val = var->value.string_val ? strdup(var->value.string_val) : strdup("");
                    break;
            }
        } else {
            result->type = VAR_INTEGER;
            result->value.int_val = 0;
        }
        return result;
    }
    
    /* Default: zero */
    result->type = VAR_INTEGER;
    result->value.int_val = 0;
    *expr = s;
    return result;
}

static Value *parse_term(Program *prog, const char **expr) {
    Value *left = parse_factor(prog, expr);
    
    while (1) {
        const char *s = skip_ws(*expr);
        if (*s == '*') {
            s++;
            *expr = s;
            Value *right = parse_factor(prog, expr);
            
            if (left->type == VAR_INTEGER && right->type == VAR_INTEGER) {
                left->value.int_val = asm_mul_int(left->value.int_val, right->value.int_val);
            } else {
                double l = value_to_double(left);
                double r = value_to_double(right);
                left->type = VAR_DOUBLE;
                left->value.double_val = asm_mul_double(l, r);
            }
            value_free(right);
        } else if (*s == '/') {
            s++;
            *expr = s;
            Value *right = parse_factor(prog, expr);
            
            if (left->type == VAR_INTEGER && right->type == VAR_INTEGER) {
                left->value.int_val = asm_div_int(left->value.int_val, right->value.int_val);
            } else {
                double l = value_to_double(left);
                double r = value_to_double(right);
                left->type = VAR_DOUBLE;
                left->value.double_val = asm_div_double(l, r);
            }
            value_free(right);
        } else {
            break;
        }
    }
    
    return left;
}

static Value *parse_expr(Program *prog, const char **expr) {
    Value *left = parse_term(prog, expr);
    
    while (1) {
        const char *s = skip_ws(*expr);
        if (*s == '+') {
            s++;
            *expr = s;
            Value *right = parse_term(prog, expr);
            
            if (left->type == VAR_INTEGER && right->type == VAR_INTEGER) {
                left->value.int_val = asm_add_int(left->value.int_val, right->value.int_val);
            } else if (left->type == VAR_STRING || right->type == VAR_STRING) {
                /* String concatenation */
                char *l_str = (left->type == VAR_STRING) ? left->value.string_val : "";
                char *r_str = (right->type == VAR_STRING) ? right->value.string_val : "";
                int len = strlen(l_str ? l_str : "") + strlen(r_str ? r_str : "");
                char *result = malloc(len + 1);
                if (result) {
                    strcpy(result, l_str ? l_str : "");
                    strcat(result, r_str ? r_str : "");
                }
                if (left->type == VAR_STRING) free(left->value.string_val);
                left->type = VAR_STRING;
                left->value.string_val = result;
            } else {
                double l = value_to_double(left);
                double r = value_to_double(right);
                left->type = VAR_DOUBLE;
                left->value.double_val = asm_add_double(l, r);
            }
            value_free(right);
        } else if (*s == '-') {
            s++;
            *expr = s;
            Value *right = parse_term(prog, expr);
            
            if (left->type == VAR_INTEGER && right->type == VAR_INTEGER) {
                left->value.int_val = asm_sub_int(left->value.int_val, right->value.int_val);
            } else {
                double l = value_to_double(left);
                double r = value_to_double(right);
                left->type = VAR_DOUBLE;
                left->value.double_val = asm_sub_double(l, r);
            }
            value_free(right);
        } else {
            break;
        }
    }
    
    return left;
}

Value *eval_expression(Program *prog, const char *expr) {
    const char *ptr = expr;
    return parse_expr(prog, &ptr);
}
