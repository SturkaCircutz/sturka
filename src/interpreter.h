#pragma once

#include "ast.h"

#include <cstdint>
#include <string>
#include <unordered_map>

class Interpreter {
public:
    void execute(const Program& program);

private:
    void executeStatement(const Stmt& stmt);
    void executeBlock(const BlockStmt& block);
    std::int64_t evaluate(const Expr& expr);

    std::unordered_map<std::string, std::int64_t> variables_;
};
