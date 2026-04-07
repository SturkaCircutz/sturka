#include "ast.h"

#include <stdlib.h>
#include <string.h>

/* The AST stores copied strings so parser and interpreter lifetimes stay independent. */

static char* copy_text(const char* text) {
    size_t length = strlen(text);
    char* result = (char*)malloc(length + 1);
    if (result == NULL) {
        return NULL;
    }
    memcpy(result, text, length + 1);
    return result;
}

Expr* expr_make_integer(int64_t value) {
    Expr* expr = (Expr*)calloc(1, sizeof(Expr));
    expr->kind = EXPR_INTEGER;
    expr->as.integer_value = value;
    return expr;
}

Expr* expr_make_variable(const char* name) {
    Expr* expr = (Expr*)calloc(1, sizeof(Expr));
    expr->kind = EXPR_VARIABLE;
    expr->as.variable_name = copy_text(name);
    return expr;
}

Expr* expr_make_unary(const char* op, Expr* operand) {
    Expr* expr = (Expr*)calloc(1, sizeof(Expr));
    expr->kind = EXPR_UNARY;
    expr->as.unary.op = copy_text(op);
    expr->as.unary.operand = operand;
    return expr;
}

Expr* expr_make_binary(Expr* left, const char* op, Expr* right) {
    Expr* expr = (Expr*)calloc(1, sizeof(Expr));
    expr->kind = EXPR_BINARY;
    expr->as.binary.left = left;
    expr->as.binary.op = copy_text(op);
    expr->as.binary.right = right;
    return expr;
}

Stmt* stmt_make_let(const char* name, Expr* value) {
    Stmt* stmt = (Stmt*)calloc(1, sizeof(Stmt));
    stmt->kind = STMT_LET;
    stmt->as.let_stmt.name = copy_text(name);
    stmt->as.let_stmt.value = value;
    return stmt;
}

Stmt* stmt_make_assign(const char* name, Expr* value) {
    Stmt* stmt = (Stmt*)calloc(1, sizeof(Stmt));
    stmt->kind = STMT_ASSIGN;
    stmt->as.assign_stmt.name = copy_text(name);
    stmt->as.assign_stmt.value = value;
    return stmt;
}

Stmt* stmt_make_print(Expr* value) {
    Stmt* stmt = (Stmt*)calloc(1, sizeof(Stmt));
    stmt->kind = STMT_PRINT;
    stmt->as.print_stmt.value = value;
    return stmt;
}

Stmt* stmt_make_if(Expr* condition, StmtArray then_branch, StmtArray else_branch, int has_else_branch) {
    Stmt* stmt = (Stmt*)calloc(1, sizeof(Stmt));
    stmt->kind = STMT_IF;
    stmt->as.if_stmt.condition = condition;
    stmt->as.if_stmt.then_branch = then_branch;
    stmt->as.if_stmt.else_branch = else_branch;
    stmt->as.if_stmt.has_else_branch = has_else_branch;
    return stmt;
}

Stmt* stmt_make_while(Expr* condition, StmtArray body) {
    Stmt* stmt = (Stmt*)calloc(1, sizeof(Stmt));
    stmt->kind = STMT_WHILE;
    stmt->as.while_stmt.condition = condition;
    stmt->as.while_stmt.body = body;
    return stmt;
}

void stmt_array_push(StmtArray* array, Stmt* stmt) {
    if (array->count == array->capacity) {
        size_t new_capacity = array->capacity == 0 ? 8 : array->capacity * 2;
        Stmt** items = (Stmt**)realloc(array->items, new_capacity * sizeof(Stmt*));
        if (items == NULL) {
            return;
        }
        array->items = items;
        array->capacity = new_capacity;
    }
    array->items[array->count++] = stmt;
}

void expr_free(Expr* expr) {
    if (expr == NULL) return;

    switch (expr->kind) {
    case EXPR_INTEGER:
        break;
    case EXPR_VARIABLE:
        free(expr->as.variable_name);
        break;
    case EXPR_UNARY:
        free(expr->as.unary.op);
        expr_free(expr->as.unary.operand);
        break;
    case EXPR_BINARY:
        expr_free(expr->as.binary.left);
        free(expr->as.binary.op);
        expr_free(expr->as.binary.right);
        break;
    }

    free(expr);
}

void stmt_free(Stmt* stmt) {
    if (stmt == NULL) return;

    switch (stmt->kind) {
    case STMT_LET:
        free(stmt->as.let_stmt.name);
        expr_free(stmt->as.let_stmt.value);
        break;
    case STMT_ASSIGN:
        free(stmt->as.assign_stmt.name);
        expr_free(stmt->as.assign_stmt.value);
        break;
    case STMT_PRINT:
        expr_free(stmt->as.print_stmt.value);
        break;
    case STMT_IF:
        expr_free(stmt->as.if_stmt.condition);
        stmt_array_free(&stmt->as.if_stmt.then_branch);
        if (stmt->as.if_stmt.has_else_branch) {
            stmt_array_free(&stmt->as.if_stmt.else_branch);
        }
        break;
    case STMT_WHILE:
        expr_free(stmt->as.while_stmt.condition);
        stmt_array_free(&stmt->as.while_stmt.body);
        break;
    }

    free(stmt);
}

void stmt_array_free(StmtArray* array) {
    size_t i;
    for (i = 0; i < array->count; ++i) {
        stmt_free(array->items[i]);
    }
    free(array->items);
    array->items = NULL;
    array->count = 0;
    array->capacity = 0;
}
