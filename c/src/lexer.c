#include "lexer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* This file keeps tokenization simple and explicit so the C version is easy to extend. */

static char* copy_slice(const char* source, size_t start, size_t end) {
    size_t length = end - start;
    char* text = (char*)malloc(length + 1);
    if (text == NULL) {
        return NULL;
    }
    memcpy(text, source + start, length);
    text[length] = '\0';
    return text;
}

static void token_array_push(TokenArray* array, Token token) {
    if (array->count == array->capacity) {
        size_t new_capacity = array->capacity == 0 ? 16 : array->capacity * 2;
        Token* items = (Token*)realloc(array->items, new_capacity * sizeof(Token));
        if (items == NULL) {
            return;
        }
        array->items = items;
        array->capacity = new_capacity;
    }
    array->items[array->count++] = token;
}

static int is_at_end(const Lexer* lexer) {
    return lexer->current >= lexer->length;
}

static char advance(Lexer* lexer) {
    return lexer->source[lexer->current++];
}

static char peek(const Lexer* lexer) {
    return is_at_end(lexer) ? '\0' : lexer->source[lexer->current];
}

static int match_char(Lexer* lexer, char expected) {
    if (is_at_end(lexer) || lexer->source[lexer->current] != expected) {
        return 0;
    }
    lexer->current++;
    return 1;
}

static int add_token(Lexer* lexer, TokenType type, size_t start, size_t end) {
    Token token;
    token.type = type;
    token.lexeme = copy_slice(lexer->source, start, end);
    token.line = lexer->line;
    if (token.lexeme == NULL) {
        snprintf(lexer->error, sizeof(lexer->error), "Out of memory while lexing");
        return 0;
    }
    token_array_push(&lexer->tokens, token);
    return 1;
}

static int scan_number(Lexer* lexer) {
    while (isdigit((unsigned char)peek(lexer))) {
        advance(lexer);
    }
    return add_token(lexer, TOKEN_NUMBER, lexer->start, lexer->current);
}

static int scan_identifier(Lexer* lexer) {
    char* text;
    TokenType type = TOKEN_IDENTIFIER;
    Token token;

    while (isalnum((unsigned char)peek(lexer)) || peek(lexer) == '_') {
        advance(lexer);
    }

    text = copy_slice(lexer->source, lexer->start, lexer->current);
    if (text == NULL) {
        snprintf(lexer->error, sizeof(lexer->error), "Out of memory while lexing");
        return 0;
    }

    if (strcmp(text, "let") == 0) type = TOKEN_LET;
    else if (strcmp(text, "set") == 0) type = TOKEN_SET;
    else if (strcmp(text, "be") == 0) type = TOKEN_BE;
    else if (strcmp(text, "to") == 0) type = TOKEN_TO;
    else if (strcmp(text, "print") == 0) type = TOKEN_PRINT;
    else if (strcmp(text, "if") == 0) type = TOKEN_IF;
    else if (strcmp(text, "else") == 0) type = TOKEN_ELSE;
    else if (strcmp(text, "while") == 0) type = TOKEN_WHILE;
    else if (strcmp(text, "do") == 0) type = TOKEN_DO;
    else if (strcmp(text, "end") == 0) type = TOKEN_END;
    else if (strcmp(text, "is") == 0) type = TOKEN_IS;
    else if (strcmp(text, "not") == 0) type = TOKEN_NOT;
    else if (strcmp(text, "less") == 0) type = TOKEN_LESS;
    else if (strcmp(text, "than") == 0) type = TOKEN_THAN;
    else if (strcmp(text, "greater") == 0) type = TOKEN_GREATER;
    else if (strcmp(text, "or") == 0) type = TOKEN_OR;
    else if (strcmp(text, "equal") == 0) type = TOKEN_EQUAL;
    else if (strcmp(text, "plus") == 0) type = TOKEN_PLUS;
    else if (strcmp(text, "minus") == 0) type = TOKEN_MINUS;
    else if (strcmp(text, "times") == 0) type = TOKEN_TIMES;
    else if (strcmp(text, "over") == 0) type = TOKEN_OVER;
    else if (strcmp(text, "negative") == 0) type = TOKEN_NEGATIVE;

    token.type = type;
    token.lexeme = text;
    token.line = lexer->line;
    token_array_push(&lexer->tokens, token);
    return 1;
}

void lexer_init(Lexer* lexer, const char* source) {
    memset(lexer, 0, sizeof(*lexer));
    lexer->source = source;
    lexer->length = strlen(source);
    lexer->line = 1;
}

int lexer_tokenize(Lexer* lexer) {
    while (!is_at_end(lexer)) {
        char c;
        lexer->start = lexer->current;
        c = advance(lexer);
        switch (c) {
        case '(':
            if (!add_token(lexer, TOKEN_LEFT_PAREN, lexer->start, lexer->current)) return 0;
            break;
        case ')':
            if (!add_token(lexer, TOKEN_RIGHT_PAREN, lexer->start, lexer->current)) return 0;
            break;
        case '/':
            if (match_char(lexer, '/')) {
                while (!is_at_end(lexer) && peek(lexer) != '\n') {
                    advance(lexer);
                }
            } else {
                snprintf(lexer->error, sizeof(lexer->error),
                         "Use English keywords like 'over' instead of '/' at line %d", lexer->line);
                return 0;
            }
            break;
        case ';':
            if (!add_token(lexer, TOKEN_SEMICOLON, lexer->start, lexer->current)) return 0;
            break;
        case ' ':
        case '\r':
        case '\t':
            break;
        case '\n':
            if (!add_token(lexer, TOKEN_NEWLINE, lexer->start, lexer->current)) return 0;
            lexer->line++;
            break;
        default:
            if (isdigit((unsigned char)c)) {
                if (!scan_number(lexer)) return 0;
            } else if (isalpha((unsigned char)c) || c == '_') {
                if (!scan_identifier(lexer)) return 0;
            } else {
                snprintf(lexer->error, sizeof(lexer->error),
                         "Unexpected character '%c' at line %d", c, lexer->line);
                return 0;
            }
            break;
        }
    }

    return add_token(lexer, TOKEN_END_OF_FILE, lexer->current, lexer->current);
}

void lexer_free_tokens(TokenArray* tokens) {
    size_t i;
    for (i = 0; i < tokens->count; ++i) {
        free(tokens->items[i].lexeme);
    }
    free(tokens->items);
    tokens->items = NULL;
    tokens->count = 0;
    tokens->capacity = 0;
}
