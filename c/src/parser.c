#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The parser follows the same recursive-descent structure as the C++ and Rust versions. */

static const Token* peek(const Parser* parser) { return &parser->tokens->items[parser->current]; }
static const Token* previous(const Parser* parser) { return &parser->tokens->items[parser->current - 1]; }
static int is_at_end(const Parser* parser) { return peek(parser)->type == TOKEN_END_OF_FILE; }

static const Token* advance(Parser* parser) {
    if (!is_at_end(parser)) parser->current++;
    return previous(parser);
}

static int check(const Parser* parser, TokenType type) {
    return !is_at_end(parser) && peek(parser)->type == type;
}

static int match(Parser* parser, TokenType type) {
    if (!check(parser, type)) return 0;
    advance(parser);
    return 1;
}

static int consume(Parser* parser, TokenType type, const char* message) {
    if (check(parser, type)) {
        advance(parser);
        return 1;
    }
    snprintf(parser->error, sizeof(parser->error), "%s at line %d", message, peek(parser)->line);
    return 0;
}

static void skip_separators(Parser* parser) {
    while (match(parser, TOKEN_NEWLINE) || match(parser, TOKEN_SEMICOLON)) {}
}

static Expr* parse_expression(Parser* parser);
static int parse_statement(Parser* parser, Stmt** out_stmt);

static Expr* parse_primary(Parser* parser) {
    if (match(parser, TOKEN_NUMBER)) {
        return expr_make_integer(strtoll(previous(parser)->lexeme, NULL, 10));
    }
    if (match(parser, TOKEN_IDENTIFIER)) {
        return expr_make_variable(previous(parser)->lexeme);
    }
    if (match(parser, TOKEN_LEFT_PAREN)) {
        Expr* expr = parse_expression(parser);
        if (expr == NULL) return NULL;
        if (!consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after expression")) {
            expr_free(expr);
            return NULL;
        }
        return expr;
    }
    snprintf(parser->error, sizeof(parser->error), "Expected expression at line %d", peek(parser)->line);
    return NULL;
}

static Expr* parse_unary(Parser* parser) {
    if (match(parser, TOKEN_NEGATIVE)) {
        Expr* operand = parse_unary(parser);
        if (operand == NULL) return NULL;
        return expr_make_unary("-", operand);
    }
    return parse_primary(parser);
}

static Expr* parse_factor(Parser* parser) {
    Expr* expr = parse_unary(parser);
    if (expr == NULL) return NULL;
    while (match(parser, TOKEN_TIMES) || match(parser, TOKEN_OVER)) {
        const char* op = previous(parser)->type == TOKEN_TIMES ? "*" : "/";
        Expr* right = parse_unary(parser);
        if (right == NULL) {
            expr_free(expr);
            return NULL;
        }
        expr = expr_make_binary(expr, op, right);
    }
    return expr;
}

static Expr* parse_term(Parser* parser) {
    Expr* expr = parse_factor(parser);
    if (expr == NULL) return NULL;
    while (match(parser, TOKEN_PLUS) || match(parser, TOKEN_MINUS)) {
        const char* op = previous(parser)->type == TOKEN_PLUS ? "+" : "-";
        Expr* right = parse_factor(parser);
        if (right == NULL) {
            expr_free(expr);
            return NULL;
        }
        expr = expr_make_binary(expr, op, right);
    }
    return expr;
}

static Expr* parse_comparison(Parser* parser) {
    Expr* expr = parse_term(parser);
    if (expr == NULL) return NULL;
    while (match(parser, TOKEN_IS)) {
        const char* op = NULL;
        if (match(parser, TOKEN_LESS)) {
            if (!consume(parser, TOKEN_THAN, "Expected 'than' after 'less'")) { expr_free(expr); return NULL; }
            op = "<";
            if (match(parser, TOKEN_OR)) {
                if (!consume(parser, TOKEN_EQUAL, "Expected 'equal' after 'or'") ||
                    !consume(parser, TOKEN_TO, "Expected 'to' after 'equal'")) {
                    expr_free(expr);
                    return NULL;
                }
                op = "<=";
            }
        } else if (match(parser, TOKEN_GREATER)) {
            if (!consume(parser, TOKEN_THAN, "Expected 'than' after 'greater'")) { expr_free(expr); return NULL; }
            op = ">";
            if (match(parser, TOKEN_OR)) {
                if (!consume(parser, TOKEN_EQUAL, "Expected 'equal' after 'or'") ||
                    !consume(parser, TOKEN_TO, "Expected 'to' after 'equal'")) {
                    expr_free(expr);
                    return NULL;
                }
                op = ">=";
            }
        } else {
            parser->current--;
            break;
        }

        Expr* right = parse_term(parser);
        if (right == NULL) {
            expr_free(expr);
            return NULL;
        }
        expr = expr_make_binary(expr, op, right);
    }
    return expr;
}

static Expr* parse_equality(Parser* parser) {
    Expr* expr = parse_comparison(parser);
    if (expr == NULL) return NULL;
    while (match(parser, TOKEN_IS)) {
        const char* op = "==";
        if (check(parser, TOKEN_LESS) || check(parser, TOKEN_GREATER)) {
            parser->current--;
            break;
        }
        if (match(parser, TOKEN_NOT)) op = "!=";
        Expr* right = parse_comparison(parser);
        if (right == NULL) {
            expr_free(expr);
            return NULL;
        }
        expr = expr_make_binary(expr, op, right);
    }
    return expr;
}

static Expr* parse_expression(Parser* parser) { return parse_equality(parser); }

static int parse_statements_until(Parser* parser, TokenType terminator, StmtArray* statements) {
    skip_separators(parser);
    while (!check(parser, terminator) && !is_at_end(parser)) {
        Stmt* stmt = NULL;
        if (!parse_statement(parser, &stmt)) return 0;
        stmt_array_push(statements, stmt);
        skip_separators(parser);
    }
    return 1;
}

static int parse_statements_until_else_or_end(Parser* parser, StmtArray* statements) {
    skip_separators(parser);
    while (!check(parser, TOKEN_ELSE) && !check(parser, TOKEN_END) && !is_at_end(parser)) {
        Stmt* stmt = NULL;
        if (!parse_statement(parser, &stmt)) return 0;
        stmt_array_push(statements, stmt);
        skip_separators(parser);
    }
    return 1;
}

static int parse_statement(Parser* parser, Stmt** out_stmt) {
    if (match(parser, TOKEN_LET)) {
        Expr* value;
        const char* name;
        if (!consume(parser, TOKEN_IDENTIFIER, "Expected variable name after 'let'")) return 0;
        name = previous(parser)->lexeme;
        if (!consume(parser, TOKEN_BE, "Expected 'be' after variable name")) return 0;
        value = parse_expression(parser);
        if (value == NULL) return 0;
        *out_stmt = stmt_make_let(name, value);
        return 1;
    }
    if (match(parser, TOKEN_SET)) {
        Expr* value;
        const char* name;
        if (!consume(parser, TOKEN_IDENTIFIER, "Expected variable name after 'set'")) return 0;
        name = previous(parser)->lexeme;
        if (!consume(parser, TOKEN_TO, "Expected 'to' after variable name")) return 0;
        value = parse_expression(parser);
        if (value == NULL) return 0;
        *out_stmt = stmt_make_assign(name, value);
        return 1;
    }
    if (match(parser, TOKEN_PRINT)) {
        Expr* value = parse_expression(parser);
        if (value == NULL) return 0;
        *out_stmt = stmt_make_print(value);
        return 1;
    }
    if (match(parser, TOKEN_IF)) {
        Expr* condition = parse_expression(parser);
        StmtArray then_branch = {0};
        StmtArray else_branch = {0};
        int has_else_branch = 0;
        if (condition == NULL) return 0;
        if (!consume(parser, TOKEN_DO, "Expected 'do' after if condition")) { expr_free(condition); return 0; }
        if (!parse_statements_until_else_or_end(parser, &then_branch)) { expr_free(condition); stmt_array_free(&then_branch); return 0; }
        if (match(parser, TOKEN_ELSE)) {
            has_else_branch = 1;
            if (!parse_statements_until(parser, TOKEN_END, &else_branch)) {
                expr_free(condition);
                stmt_array_free(&then_branch);
                stmt_array_free(&else_branch);
                return 0;
            }
        }
        if (!consume(parser, TOKEN_END, "Expected 'end' after if statement")) {
            expr_free(condition);
            stmt_array_free(&then_branch);
            stmt_array_free(&else_branch);
            return 0;
        }
        *out_stmt = stmt_make_if(condition, then_branch, else_branch, has_else_branch);
        return 1;
    }
    if (match(parser, TOKEN_WHILE)) {
        Expr* condition = parse_expression(parser);
        StmtArray body = {0};
        if (condition == NULL) return 0;
        if (!consume(parser, TOKEN_DO, "Expected 'do' after while condition")) { expr_free(condition); return 0; }
        if (!parse_statements_until(parser, TOKEN_END, &body)) { expr_free(condition); stmt_array_free(&body); return 0; }
        if (!consume(parser, TOKEN_END, "Expected 'end' after while statement")) { expr_free(condition); stmt_array_free(&body); return 0; }
        *out_stmt = stmt_make_while(condition, body);
        return 1;
    }
    snprintf(parser->error, sizeof(parser->error), "Unexpected statement at line %d", peek(parser)->line);
    return 0;
}

void parser_init(Parser* parser, const TokenArray* tokens) {
    memset(parser, 0, sizeof(*parser));
    parser->tokens = tokens;
}

int parser_parse_program(Parser* parser, StmtArray* program) {
    skip_separators(parser);
    while (!is_at_end(parser)) {
        Stmt* stmt = NULL;
        if (!parse_statement(parser, &stmt)) return 0;
        stmt_array_push(program, stmt);
        skip_separators(parser);
    }
    return 1;
}
