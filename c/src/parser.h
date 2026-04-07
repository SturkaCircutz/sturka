#ifndef STURKA_C_PARSER_H
#define STURKA_C_PARSER_H

#include "ast.h"
#include "token.h"

typedef struct Parser {
    const TokenArray* tokens;
    size_t current;
    char error[256];
} Parser;

void parser_init(Parser* parser, const TokenArray* tokens);
int parser_parse_program(Parser* parser, StmtArray* program);

#endif
