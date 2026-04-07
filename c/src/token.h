#ifndef STURKA_C_TOKEN_H
#define STURKA_C_TOKEN_H

#include <stddef.h>

typedef enum TokenType {
    TOKEN_END_OF_FILE,
    TOKEN_NEWLINE,
    TOKEN_NUMBER,
    TOKEN_IDENTIFIER,
    TOKEN_LET,
    TOKEN_SET,
    TOKEN_BE,
    TOKEN_TO,
    TOKEN_PRINT,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_DO,
    TOKEN_END,
    TOKEN_IS,
    TOKEN_NOT,
    TOKEN_LESS,
    TOKEN_THAN,
    TOKEN_GREATER,
    TOKEN_OR,
    TOKEN_EQUAL,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_TIMES,
    TOKEN_OVER,
    TOKEN_NEGATIVE,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_SEMICOLON
} TokenType;

typedef struct Token {
    TokenType type;
    char* lexeme;
    int line;
} Token;

typedef struct TokenArray {
    Token* items;
    size_t count;
    size_t capacity;
} TokenArray;

#endif
