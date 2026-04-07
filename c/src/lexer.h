#ifndef STURKA_C_LEXER_H
#define STURKA_C_LEXER_H

#include "token.h"

typedef struct Lexer {
    const char* source;
    size_t length;
    size_t start;
    size_t current;
    int line;
    TokenArray tokens;
    char error[256];
} Lexer;

void lexer_init(Lexer* lexer, const char* source);
int lexer_tokenize(Lexer* lexer);
void lexer_free_tokens(TokenArray* tokens);

#endif
