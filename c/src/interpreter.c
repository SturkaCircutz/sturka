#include "interpreter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The C interpreter uses a simple linear variable table to stay readable. */

static char* copy_text(const char* text) {
    size_t length = strlen(text);
    char* result = (char*)malloc(length + 1);
    if (result == NULL) return NULL;
    memcpy(result, text, length + 1);
    return result;
}

static int find_variable(const Interpreter* interpreter, const char* name) {
    size_t i;
    for (i = 0; i < interpreter->count; ++i) {
        if (strcmp(interpreter->variables[i].name, name) == 0) return (int)i;
    }
    return -1;
}

static int ensure_capacity(Interpreter* interpreter) {
    if (interpreter->count == interpreter->capacity) {
        size_t new_capacity = interpreter->capacity == 0 ? 8 : interpreter->capacity * 2;
        Variable* items = (Variable*)realloc(interpreter->variables, new_capacity * sizeof(Variable));
        if (items == NULL) {
            snprintf(interpreter->error, sizeof(interpreter->error), "Out of memory while storing variables");
            return 0;
        }
        interpreter->variables = items;
        interpreter->capacity = new_capacity;
    }
    return 1;
}

static int evaluate_expr(Interpreter* interpreter, const Expr* expr, long long* out_value);

static int execute_statement(Interpreter* interpreter, const Stmt* stmt) {
    long long value;
    size_t i;
    int index;

    switch (stmt->kind) {
    case STMT_LET:
        if (find_variable(interpreter, stmt->as.let_stmt.name) >= 0) {
            snprintf(interpreter->error, sizeof(interpreter->error), "Variable already defined: %s", stmt->as.let_stmt.name);
            return 0;
        }
        if (!evaluate_expr(interpreter, stmt->as.let_stmt.value, &value) || !ensure_capacity(interpreter)) return 0;
        interpreter->variables[interpreter->count].name = copy_text(stmt->as.let_stmt.name);
        interpreter->variables[interpreter->count].value = value;
        interpreter->count++;
        return 1;
    case STMT_ASSIGN:
        index = find_variable(interpreter, stmt->as.assign_stmt.name);
        if (index < 0) {
            snprintf(interpreter->error, sizeof(interpreter->error), "Undefined variable: %s", stmt->as.assign_stmt.name);
            return 0;
        }
        if (!evaluate_expr(interpreter, stmt->as.assign_stmt.value, &value)) return 0;
        interpreter->variables[index].value = value;
        return 1;
    case STMT_PRINT:
        if (!evaluate_expr(interpreter, stmt->as.print_stmt.value, &value)) return 0;
        printf("%lld\n", value);
        return 1;
    case STMT_IF:
        if (!evaluate_expr(interpreter, stmt->as.if_stmt.condition, &value)) return 0;
        if (value != 0) {
            for (i = 0; i < stmt->as.if_stmt.then_branch.count; ++i) {
                if (!execute_statement(interpreter, stmt->as.if_stmt.then_branch.items[i])) return 0;
            }
        } else if (stmt->as.if_stmt.has_else_branch) {
            for (i = 0; i < stmt->as.if_stmt.else_branch.count; ++i) {
                if (!execute_statement(interpreter, stmt->as.if_stmt.else_branch.items[i])) return 0;
            }
        }
        return 1;
    case STMT_WHILE:
        while (1) {
            if (!evaluate_expr(interpreter, stmt->as.while_stmt.condition, &value)) return 0;
            if (value == 0) break;
            for (i = 0; i < stmt->as.while_stmt.body.count; ++i) {
                if (!execute_statement(interpreter, stmt->as.while_stmt.body.items[i])) return 0;
            }
        }
        return 1;
    }

    snprintf(interpreter->error, sizeof(interpreter->error), "Unknown statement kind");
    return 0;
}

static int evaluate_expr(Interpreter* interpreter, const Expr* expr, long long* out_value) {
    long long left;
    long long right;
    long long operand;
    int index;

    switch (expr->kind) {
    case EXPR_INTEGER:
        *out_value = expr->as.integer_value;
        return 1;
    case EXPR_VARIABLE:
        index = find_variable(interpreter, expr->as.variable_name);
        if (index < 0) {
            snprintf(interpreter->error, sizeof(interpreter->error), "Undefined variable: %s", expr->as.variable_name);
            return 0;
        }
        *out_value = interpreter->variables[index].value;
        return 1;
    case EXPR_UNARY:
        if (!evaluate_expr(interpreter, expr->as.unary.operand, &operand)) return 0;
        if (strcmp(expr->as.unary.op, "-") == 0) {
            *out_value = -operand;
            return 1;
        }
        snprintf(interpreter->error, sizeof(interpreter->error), "Unsupported unary operator: %s", expr->as.unary.op);
        return 0;
    case EXPR_BINARY:
        if (!evaluate_expr(interpreter, expr->as.binary.left, &left)) return 0;
        if (!evaluate_expr(interpreter, expr->as.binary.right, &right)) return 0;

        if (strcmp(expr->as.binary.op, "+") == 0) *out_value = left + right;
        else if (strcmp(expr->as.binary.op, "-") == 0) *out_value = left - right;
        else if (strcmp(expr->as.binary.op, "*") == 0) *out_value = left * right;
        else if (strcmp(expr->as.binary.op, "/") == 0) {
            if (right == 0) {
                snprintf(interpreter->error, sizeof(interpreter->error), "Division by zero");
                return 0;
            }
            *out_value = left / right;
        } else if (strcmp(expr->as.binary.op, "==") == 0) *out_value = left == right;
        else if (strcmp(expr->as.binary.op, "!=") == 0) *out_value = left != right;
        else if (strcmp(expr->as.binary.op, "<") == 0) *out_value = left < right;
        else if (strcmp(expr->as.binary.op, "<=") == 0) *out_value = left <= right;
        else if (strcmp(expr->as.binary.op, ">") == 0) *out_value = left > right;
        else if (strcmp(expr->as.binary.op, ">=") == 0) *out_value = left >= right;
        else {
            snprintf(interpreter->error, sizeof(interpreter->error), "Unsupported binary operator: %s", expr->as.binary.op);
            return 0;
        }
        return 1;
    }

    snprintf(interpreter->error, sizeof(interpreter->error), "Unknown expression kind");
    return 0;
}

void interpreter_init(Interpreter* interpreter) {
    memset(interpreter, 0, sizeof(*interpreter));
}

int interpreter_execute(Interpreter* interpreter, const StmtArray* program) {
    size_t i;
    for (i = 0; i < program->count; ++i) {
        if (!execute_statement(interpreter, program->items[i])) return 0;
    }
    return 1;
}

void interpreter_free(Interpreter* interpreter) {
    size_t i;
    for (i = 0; i < interpreter->count; ++i) free(interpreter->variables[i].name);
    free(interpreter->variables);
    interpreter->variables = NULL;
    interpreter->count = 0;
    interpreter->capacity = 0;
}
