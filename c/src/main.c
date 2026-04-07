#include "ast.h"
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* This entry point mirrors the other implementations and keeps the C version easy to test. */

static char* read_file(const char* path) {
    FILE* file = fopen(path, "rb");
    long length;
    char* buffer;
    if (file == NULL) return NULL;

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (char*)malloc((size_t)length + 1);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }
    if (fread(buffer, 1, (size_t)length, file) != (size_t)length) {
        fclose(file);
        free(buffer);
        return NULL;
    }
    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

int main(int argc, char** argv) {
    char* source;
    Lexer lexer;
    Parser parser;
    Interpreter interpreter;
    StmtArray program = {0};

    if (argc != 2) {
        fprintf(stderr, "Usage: sturka-c <file.sturka>\n");
        return 1;
    }
    if (strlen(argv[1]) < 7 || strcmp(argv[1] + strlen(argv[1]) - 7, ".sturka") != 0) {
        fprintf(stderr, "Expected a .sturka source file\n");
        return 1;
    }

    source = read_file(argv[1]);
    if (source == NULL) {
        fprintf(stderr, "Could not open file: %s\n", argv[1]);
        return 1;
    }

    lexer_init(&lexer, source);
    if (!lexer_tokenize(&lexer)) {
        fprintf(stderr, "sturka-c error: %s\n", lexer.error);
        free(source);
        lexer_free_tokens(&lexer.tokens);
        return 1;
    }

    parser_init(&parser, &lexer.tokens);
    if (!parser_parse_program(&parser, &program)) {
        fprintf(stderr, "sturka-c error: %s\n", parser.error);
        free(source);
        lexer_free_tokens(&lexer.tokens);
        stmt_array_free(&program);
        return 1;
    }

    interpreter_init(&interpreter);
    if (!interpreter_execute(&interpreter, &program)) {
        fprintf(stderr, "sturka-c error: %s\n", interpreter.error);
        interpreter_free(&interpreter);
        stmt_array_free(&program);
        lexer_free_tokens(&lexer.tokens);
        free(source);
        return 1;
    }

    interpreter_free(&interpreter);
    stmt_array_free(&program);
    lexer_free_tokens(&lexer.tokens);
    free(source);
    return 0;
}
