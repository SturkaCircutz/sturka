#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

enum class ExprKind {
    Integer,
    Variable,
    Unary,
    Binary
};

struct Expr {
    explicit Expr(ExprKind kind) : kind(kind) {}
    virtual ~Expr() = default;
    ExprKind kind;
};

struct IntegerExpr : Expr {
    explicit IntegerExpr(std::int64_t value) : Expr(ExprKind::Integer), value(value) {}
    std::int64_t value;
};

struct VariableExpr : Expr {
    explicit VariableExpr(std::string name) : Expr(ExprKind::Variable), name(std::move(name)) {}
    std::string name;
};

struct UnaryExpr : Expr {
    UnaryExpr(std::string op, std::unique_ptr<Expr> operand)
        : Expr(ExprKind::Unary), op(std::move(op)), operand(std::move(operand)) {}

    std::string op;
    std::unique_ptr<Expr> operand;
};

struct BinaryExpr : Expr {
    BinaryExpr(std::unique_ptr<Expr> left, std::string op, std::unique_ptr<Expr> right)
        : Expr(ExprKind::Binary), left(std::move(left)), op(std::move(op)), right(std::move(right)) {}

    std::unique_ptr<Expr> left;
    std::string op;
    std::unique_ptr<Expr> right;
};

enum class StmtKind {
    Let,
    Assign,
    Print,
    Block,
    If,
    While
};

struct Stmt {
    explicit Stmt(StmtKind kind) : kind(kind) {}
    virtual ~Stmt() = default;
    StmtKind kind;
};

struct LetStmt : Stmt {
    LetStmt(std::string name, std::unique_ptr<Expr> value)
        : Stmt(StmtKind::Let), name(std::move(name)), value(std::move(value)) {}

    std::string name;
    std::unique_ptr<Expr> value;
};

struct AssignStmt : Stmt {
    AssignStmt(std::string name, std::unique_ptr<Expr> value)
        : Stmt(StmtKind::Assign), name(std::move(name)), value(std::move(value)) {}

    std::string name;
    std::unique_ptr<Expr> value;
};

struct PrintStmt : Stmt {
    explicit PrintStmt(std::unique_ptr<Expr> value)
        : Stmt(StmtKind::Print), value(std::move(value)) {}

    std::unique_ptr<Expr> value;
};

struct BlockStmt : Stmt {
    BlockStmt() : Stmt(StmtKind::Block) {}
    std::vector<std::unique_ptr<Stmt>> statements;
};

struct IfStmt : Stmt {
    IfStmt(std::unique_ptr<Expr> condition,
           std::unique_ptr<BlockStmt> thenBranch,
           std::unique_ptr<BlockStmt> elseBranch)
        : Stmt(StmtKind::If),
          condition(std::move(condition)),
          thenBranch(std::move(thenBranch)),
          elseBranch(std::move(elseBranch)) {}

    std::unique_ptr<Expr> condition;
    std::unique_ptr<BlockStmt> thenBranch;
    std::unique_ptr<BlockStmt> elseBranch;
};

struct WhileStmt : Stmt {
    WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<BlockStmt> body)
        : Stmt(StmtKind::While), condition(std::move(condition)), body(std::move(body)) {}

    std::unique_ptr<Expr> condition;
    std::unique_ptr<BlockStmt> body;
};

using Program = std::vector<std::unique_ptr<Stmt>>;
