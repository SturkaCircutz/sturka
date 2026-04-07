#ifndef STURKA_C_AST_H
#define STURKA_C_AST_H

#include <stddef.h>
#include <stdint.h>

typedef enum ExprKind {
    EXPR_INTEGER,
    EXPR_VARIABLE,
    EXPR_UNARY,
    EXPR_BINARY
} ExprKind;

typedef struct Expr Expr;

struct Expr {
    ExprKind kind;
    union {
        int64_t integer_value;
        char* variable_name;
        struct {
            char* op;
            Expr* operand;
        } unary;
        struct {
            Expr* left;
            char* op;
            Expr* right;
        } binary;
    } as;
};

typedef enum StmtKind {
    STMT_LET,
    STMT_ASSIGN,
    STMT_PRINT,
    STMT_IF,
    STMT_WHILE
} StmtKind;

typedef struct Stmt Stmt;

typedef struct StmtArray {
    Stmt** items;
    size_t count;
    size_t capacity;
} StmtArray;

struct Stmt {
    StmtKind kind;
    union {
        struct {
            char* name;
            Expr* value;
        } let_stmt;
        struct {
            char* name;
            Expr* value;
        } assign_stmt;
        struct {
            Expr* value;
        } print_stmt;
        struct {
            Expr* condition;
            StmtArray then_branch;
            StmtArray else_branch;
            int has_else_branch;
        } if_stmt;
        struct {
            Expr* condition;
            StmtArray body;
        } while_stmt;
    } as;
};

Expr* expr_make_integer(int64_t value);
Expr* expr_make_variable(const char* name);
Expr* expr_make_unary(const char* op, Expr* operand);
Expr* expr_make_binary(Expr* left, const char* op, Expr* right);

Stmt* stmt_make_let(const char* name, Expr* value);
Stmt* stmt_make_assign(const char* name, Expr* value);
Stmt* stmt_make_print(Expr* value);
Stmt* stmt_make_if(Expr* condition, StmtArray then_branch, StmtArray else_branch, int has_else_branch);
Stmt* stmt_make_while(Expr* condition, StmtArray body);

void stmt_array_push(StmtArray* array, Stmt* stmt);
void stmt_array_free(StmtArray* array);
void expr_free(Expr* expr);
void stmt_free(Stmt* stmt);

#endif
