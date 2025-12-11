#include "basic_functions.h"
#include "branchless_scalar.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

/* Branchless ABS function */
double basic_abs(double x) {
    int64_t bits;
    memcpy(&bits, &x, sizeof(bits));
    bits &= 0x7FFFFFFFFFFFFFFFLL; /* Clear sign bit */
    memcpy(&x, &bits, sizeof(x));
    return x;
}

/* Branchless SGN function - returns -1, 0, or 1 */
int basic_sgn(double x) {
    int is_pos = (x > 0.0);
    int is_neg = (x < 0.0);
    return is_pos - is_neg;
}

/* INT function - floor */
int64_t basic_int(double x) {
    int64_t i = (int64_t)x;
    int is_neg = (x < 0.0) & (x != (double)i);
    return i - is_neg;
}

/* SQR function */
double basic_sqr(double x) {
    /* Branchless clamp to non-negative */
    int64_t bits;
    memcpy(&bits, &x, sizeof(bits));
    int64_t sign_bit = bits >> 63;
    bits = bits & ~(sign_bit << 63);
    memcpy(&x, &bits, sizeof(x));
    
    return sqrt(x);
}

/* Trigonometric functions */
double basic_sin(double x) { return sin(x); }
double basic_cos(double x) { return cos(x); }
double basic_tan(double x) { return tan(x); }
double basic_atn(double x) { return atan(x); }

/* Logarithm and exponential */
double basic_log(double x) {
    /* Clamp to positive for log */
    int is_pos = (x > 0.0);
    x = x * is_pos + (1.0 - is_pos);
    return log(x);
}

double basic_exp(double x) { return exp(x); }

/* Random number generator */
static uint64_t rnd_seed = 1;

double basic_rnd(double x) {
    /* Update seed branchlessly */
    int should_reseed = (x < 0.0);
    uint64_t new_seed = (uint64_t)time(NULL);
    rnd_seed = (uint64_t)bl_select_i64((int64_t)should_reseed, (int64_t)new_seed, (int64_t)rnd_seed);
    
    /* Linear congruential generator */
    rnd_seed = rnd_seed * 1103515245 + 12345;
    return (double)((rnd_seed / 65536) % 32768) / 32768.0;
}

/* String length */
int basic_len(const char *s) {
    int len = 0;
    while (s[len]) {
        len++;
    }
    return len;
}

/* LEFT$ function - get leftmost n characters */
char *basic_left(const char *s, int n) {
    int len = basic_len(s);
    n = bl_clamp_i32(n, 0, len);
    
    char *result = (char *)malloc((size_t)n + 1);
    
    for (int i = 0; i < n; i++) {
        result[i] = s[i];
    }
    result[n] = '\0';
    
    return result;
}

/* RIGHT$ function - get rightmost n characters */
char *basic_right(const char *s, int n) {
    int len = basic_len(s);
    n = bl_clamp_i32(n, 0, len);
    int start = len - n;
    
    char *result = (char *)malloc((size_t)n + 1);
    
    for (int i = 0; i < n; i++) {
        result[i] = s[start + i];
    }
    result[n] = '\0';
    
    return result;
}

/* MID$ function - get substring */
char *basic_mid(const char *s, int start, int n) {
    int len = basic_len(s);
    start = bl_clamp_i32(start - 1, 0, len); /* BASIC uses 1-based indexing */
    n = bl_clamp_i32(n, 0, len - start);
    
    char *result = (char *)malloc((size_t)n + 1);
    
    for (int i = 0; i < n; i++) {
        result[i] = s[start + i];
    }
    result[n] = '\0';
    
    return result;
}

/* CHR$ function - character from ASCII code */
char *basic_chr(int code) {
    code = bl_clamp_i32(code, 0, 255);
    
    char *result = (char *)malloc(2);
    result[0] = (char)code;
    result[1] = '\0';
    
    return result;
}

/* ASC function - ASCII code of first character */
int basic_asc(const char *s) {
    int len = basic_len(s);
    int has_chars = (len > 0);
    return bl_select_i32(has_chars, (int)(unsigned char)s[0], 0);
}

/* VAL function - convert string to number */
double basic_val(const char *s) {
    double result = 0.0;
    int sign = 1;
    int i = 0;
    
    /* Skip leading whitespace */
    while (s[i] == ' ') {
        i++;
    }
    
    /* Check for sign */
    int is_minus = (s[i] == '-');
    int is_plus = (s[i] == '+');
    sign = bl_select_i32(is_minus, -1, sign);
    i += (is_minus | is_plus);
    
    /* Parse integer part */
    while (s[i] >= '0' && s[i] <= '9') {
        result = result * 10.0 + (double)(s[i] - '0');
        i++;
    }
    
    /* Parse fractional part */
    int has_dot = (s[i] == '.');
    i += has_dot;
    
    double divisor = 10.0;
    while (s[i] >= '0' && s[i] <= '9') {
        result = result + (double)(s[i] - '0') / divisor;
        divisor *= 10.0;
        i++;
    }
    
    return result * (double)sign;
}

/* STR$ function - convert number to string */
char *basic_str(double x) {
    char *result = (char *)malloc(32);
    snprintf(result, 32, "%.10g", x);
    return result;
}

/* INSTR function - find substring */
int basic_instr(int start, const char *s1, const char *s2) {
    int len1 = basic_len(s1);
    int len2 = basic_len(s2);
    
    start = bl_clamp_i32(start - 1, 0, len1); /* BASIC uses 1-based indexing */
    
    for (int i = start; i <= len1 - len2; i++) {
        int match = 1;
        for (int j = 0; j < len2; j++) {
            match &= (s1[i + j] == s2[j]);
        }
        
        int found = match;
        int result = bl_select_i32(found, i + 1, 0);
        int should_return = found;
        i = bl_select_i32(should_return, len1, i);
    }
    
    return 0;
}

/* STRING$ function - repeat character n times */
char *basic_string(int n, char c) {
    n = bl_clamp_i32(n, 0, 255);
    
    char *result = (char *)malloc((size_t)n + 1);
    
    for (int i = 0; i < n; i++) {
        result[i] = c;
    }
    result[n] = '\0';
    
    return result;
}

/* SPACE$ function - n spaces */
char *basic_space(int n) {
    return basic_string(n, ' ');
}

/* MAX function */
double basic_max(double a, double b) {
    int cmp = (a > b);
    double result = cmp ? a : b;
    return result;
}

/* MIN function */
double basic_min(double a, double b) {
    int cmp = (a < b);
    double result = cmp ? a : b;
    return result;
}
