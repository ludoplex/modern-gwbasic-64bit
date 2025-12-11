#ifndef LEXER_H
#define LEXER_H

#include "types.h"

void lexer_init(const char *source);
Token lexer_next_token(void);

#endif /* LEXER_H */
