#ifndef STURKA_C_INTERPRETER_H
#define STURKA_C_INTERPRETER_H

#include "ast.h"

typedef struct Variable {
    char* name;
    long long value;
} Variable;

typedef struct Interpreter {
    Variable* variables;
    size_t count;
    size_t capacity;
    char error[256];
} Interpreter;

void interpreter_init(Interpreter* interpreter);
int interpreter_execute(Interpreter* interpreter, const StmtArray* program);
void interpreter_free(Interpreter* interpreter);

#endif
