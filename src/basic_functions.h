#ifndef BASIC_FUNCTIONS_H
#define BASIC_FUNCTIONS_H

#include <stdint.h>

/* Mathematical functions */
double basic_abs(double x);
int basic_sgn(double x);
int64_t basic_int(double x);
double basic_sqr(double x);
double basic_sin(double x);
double basic_cos(double x);
double basic_tan(double x);
double basic_atn(double x);
double basic_log(double x);
double basic_exp(double x);
double basic_rnd(double x);

/* String functions */
int basic_len(const char *s);
char *basic_left(const char *s, int n);
char *basic_right(const char *s, int n);
char *basic_mid(const char *s, int start, int n);
char *basic_chr(int code);
int basic_asc(const char *s);
double basic_val(const char *s);
char *basic_str(double x);
int basic_instr(int start, const char *s1, const char *s2);
char *basic_string(int n, char c);
char *basic_space(int n);

/* Utility functions */
double basic_max(double a, double b);
double basic_min(double a, double b);

#endif /* BASIC_FUNCTIONS_H */
