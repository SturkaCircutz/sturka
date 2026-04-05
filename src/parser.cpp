#include "parser.h"

#include <stdexcept>

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

Program Parser::parseProgram() {
    Program program;
    skipSeparators();

    while (!isAtEnd()) {
        program.push_back(parseStatement());
        skipSeparators();
    }

    return program;
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::EndOfFile;
}

const Token& Parser::peek() const {
    return tokens_[current_];
}

const Token& Parser::previous() const {
    return tokens_[current_ - 1];
}

const Token& Parser::advance() {
    if (!isAtEnd()) {
        ++current_;
    }
    return previous();
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) {
        return false;
    }
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (!check(type)) {
        return false;
    }
    advance();
    return true;
}

void Parser::consume(TokenType type, const std::string& message) {
    if (!check(type)) {
        throw std::runtime_error(message + " at line " + std::to_string(peek().line));
    }
    advance();
}

void Parser::skipSeparators() {
    while (match(TokenType::Newline) || match(TokenType::Semicolon)) {
    }
}

std::unique_ptr<Stmt> Parser::parseStatement() {
    if (match(TokenType::Let)) {
        return parseLetStatement();
    }
    if (match(TokenType::Set)) {
        return parseAssignStatement();
    }
    if (match(TokenType::Print)) {
        return parsePrintStatement();
    }
    if (match(TokenType::If)) {
        return parseIfStatement();
    }
    if (match(TokenType::While)) {
        return parseWhileStatement();
    }

    throw std::runtime_error("Unexpected statement at line " + std::to_string(peek().line));
}

std::unique_ptr<Stmt> Parser::parseLetStatement() {
    consume(TokenType::Identifier, "Expected variable name after 'let'");
    const std::string name = previous().lexeme;
    consume(TokenType::Be, "Expected 'be' after variable name");
    return std::make_unique<LetStmt>(name, parseExpression());
}

std::unique_ptr<Stmt> Parser::parseAssignStatement() {
    consume(TokenType::Identifier, "Expected variable name after 'set'");
    const std::string name = previous().lexeme;
    consume(TokenType::To, "Expected 'to' after variable name");
    return std::make_unique<AssignStmt>(name, parseExpression());
}

std::unique_ptr<Stmt> Parser::parsePrintStatement() {
    return std::make_unique<PrintStmt>(parseExpression());
}

std::unique_ptr<Stmt> Parser::parseIfStatement() {
    auto condition = parseExpression();
    consume(TokenType::Do, "Expected 'do' after if condition");
    skipSeparators();

    auto thenBranch = parseStatementsUntilElseOrEnd();
    std::unique_ptr<BlockStmt> elseBranch;
    if (match(TokenType::Else)) {
        skipSeparators();
        elseBranch = parseStatementsUntil(TokenType::End);
    }

    consume(TokenType::End, "Expected 'end' after if statement");
    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::parseWhileStatement() {
    auto condition = parseExpression();
    consume(TokenType::Do, "Expected 'do' after while condition");
    skipSeparators();
    auto body = parseStatementsUntil(TokenType::End);
    consume(TokenType::End, "Expected 'end' after while statement");
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<BlockStmt> Parser::parseStatementsUntil(TokenType terminator) {
    auto block = std::make_unique<BlockStmt>();
    skipSeparators();

    while (!check(terminator) && !isAtEnd()) {
        block->statements.push_back(parseStatement());
        skipSeparators();
    }

    return block;
}

std::unique_ptr<BlockStmt> Parser::parseStatementsUntilElseOrEnd() {
    auto block = std::make_unique<BlockStmt>();
    skipSeparators();

    while (!check(TokenType::Else) && !check(TokenType::End) && !isAtEnd()) {
        block->statements.push_back(parseStatement());
        skipSeparators();
    }

    return block;
}

std::unique_ptr<Expr> Parser::parseExpression() {
    return parseEquality();
}

std::unique_ptr<Expr> Parser::parseEquality() {
    auto expr = parseComparison();

    while (match(TokenType::Is)) {
        std::string op = "==";
        if (match(TokenType::Not)) {
            op = "!=";
        } else if (check(TokenType::Less) || check(TokenType::Greater)) {
            --current_;
            break;
        }

        auto right = parseComparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseComparison() {
    auto expr = parseTerm();

    while (match(TokenType::Is)) {
        std::string op;
        if (match(TokenType::Less)) {
            consume(TokenType::Than, "Expected 'than' after 'less'");
            op = "<";
            if (match(TokenType::Or)) {
                consume(TokenType::Equal, "Expected 'equal' after 'or'");
                consume(TokenType::To, "Expected 'to' after 'equal'");
                op = "<=";
            }
        } else if (match(TokenType::Greater)) {
            consume(TokenType::Than, "Expected 'than' after 'greater'");
            op = ">";
            if (match(TokenType::Or)) {
                consume(TokenType::Equal, "Expected 'equal' after 'or'");
                consume(TokenType::To, "Expected 'to' after 'equal'");
                op = ">=";
            }
        } else {
            --current_;
            break;
        }

        auto right = parseTerm();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseTerm() {
    auto expr = parseFactor();

    while (match(TokenType::Plus) || match(TokenType::Minus)) {
        const std::string op = previous().type == TokenType::Plus ? "+" : "-";
        auto right = parseFactor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseFactor() {
    auto expr = parseUnary();

    while (match(TokenType::Times) || match(TokenType::Over)) {
        const std::string op = previous().type == TokenType::Times ? "*" : "/";
        auto right = parseUnary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseUnary() {
    if (match(TokenType::Negative)) {
        return std::make_unique<UnaryExpr>("-", parseUnary());
    }

    return parsePrimary();
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    if (match(TokenType::Number)) {
        return std::make_unique<IntegerExpr>(std::stoll(previous().lexeme));
    }

    if (match(TokenType::Identifier)) {
        return std::make_unique<VariableExpr>(previous().lexeme);
    }

    if (match(TokenType::LeftParen)) {
        auto expr = parseExpression();
        consume(TokenType::RightParen, "Expected ')' after expression");
        return expr;
    }

    throw std::runtime_error("Expected expression at line " + std::to_string(peek().line));
}
