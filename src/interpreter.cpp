#include "interpreter.h"

#include <iostream>
#include <stdexcept>

void Interpreter::execute(const Program& program) {
    for (const auto& statement : program) {
        executeStatement(*statement);
    }
}

void Interpreter::executeStatement(const Stmt& stmt) {
    switch (stmt.kind) {
    case StmtKind::Let: {
        const auto& letStmt = static_cast<const LetStmt&>(stmt);
        if (variables_.count(letStmt.name) != 0) {
            throw std::runtime_error("Variable already defined: " + letStmt.name);
        }
        variables_[letStmt.name] = evaluate(*letStmt.value);
        break;
    }
    case StmtKind::Assign: {
        const auto& assignStmt = static_cast<const AssignStmt&>(stmt);
        auto it = variables_.find(assignStmt.name);
        if (it == variables_.end()) {
            throw std::runtime_error("Undefined variable: " + assignStmt.name);
        }
        it->second = evaluate(*assignStmt.value);
        break;
    }
    case StmtKind::Print: {
        const auto& printStmt = static_cast<const PrintStmt&>(stmt);
        std::cout << evaluate(*printStmt.value) << '\n';
        break;
    }
    case StmtKind::Block: {
        executeBlock(static_cast<const BlockStmt&>(stmt));
        break;
    }
    case StmtKind::If: {
        const auto& ifStmt = static_cast<const IfStmt&>(stmt);
        if (evaluate(*ifStmt.condition) != 0) {
            executeBlock(*ifStmt.thenBranch);
        } else if (ifStmt.elseBranch) {
            executeBlock(*ifStmt.elseBranch);
        }
        break;
    }
    case StmtKind::While: {
        const auto& whileStmt = static_cast<const WhileStmt&>(stmt);
        while (evaluate(*whileStmt.condition) != 0) {
            executeBlock(*whileStmt.body);
        }
        break;
    }
    }
}

void Interpreter::executeBlock(const BlockStmt& block) {
    for (const auto& statement : block.statements) {
        executeStatement(*statement);
    }
}

std::int64_t Interpreter::evaluate(const Expr& expr) {
    switch (expr.kind) {
    case ExprKind::Integer:
        return static_cast<const IntegerExpr&>(expr).value;
    case ExprKind::Variable: {
        const auto& name = static_cast<const VariableExpr&>(expr).name;
        auto it = variables_.find(name);
        if (it == variables_.end()) {
            throw std::runtime_error("Undefined variable: " + name);
        }
        return it->second;
    }
    case ExprKind::Unary: {
        const auto& unaryExpr = static_cast<const UnaryExpr&>(expr);
        std::int64_t operand = evaluate(*unaryExpr.operand);
        if (unaryExpr.op == "-") {
            return -operand;
        }
        throw std::runtime_error("Unsupported unary operator: " + unaryExpr.op);
    }
    case ExprKind::Binary: {
        const auto& binaryExpr = static_cast<const BinaryExpr&>(expr);
        std::int64_t left = evaluate(*binaryExpr.left);
        std::int64_t right = evaluate(*binaryExpr.right);

        if (binaryExpr.op == "+") return left + right;
        if (binaryExpr.op == "-") return left - right;
        if (binaryExpr.op == "*") return left * right;
        if (binaryExpr.op == "/") {
            if (right == 0) {
                throw std::runtime_error("Division by zero");
            }
            return left / right;
        }
        if (binaryExpr.op == "==") return left == right;
        if (binaryExpr.op == "!=") return left != right;
        if (binaryExpr.op == "<") return left < right;
        if (binaryExpr.op == "<=") return left <= right;
        if (binaryExpr.op == ">") return left > right;
        if (binaryExpr.op == ">=") return left >= right;

        throw std::runtime_error("Unsupported binary operator: " + binaryExpr.op);
    }
    }

    throw std::runtime_error("Unknown expression kind");
}
