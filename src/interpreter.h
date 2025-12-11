#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "types.h"

Interpreter *interpreter_create(void);
void interpreter_destroy(Interpreter *interp);
void interpreter_load_program(Interpreter *interp, const char *source);
void interpreter_run(Interpreter *interp);

#endif /* INTERPRETER_H */
