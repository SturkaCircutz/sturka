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

bool Parser::checkNext(TokenType type) const {
    if (current_ + 1 >= tokens_.size()) {
        return false;
    }
    return tokens_[current_ + 1].type == type;
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
    if (match(TokenType::Print)) {
        return parsePrintStatement();
    }
    if (match(TokenType::If)) {
        return parseIfStatement();
    }
    if (match(TokenType::While)) {
        return parseWhileStatement();
    }
    if (check(TokenType::Identifier) && checkNext(TokenType::Assign)) {
        return parseAssignStatement();
    }

    throw std::runtime_error("Unexpected statement at line " + std::to_string(peek().line));
}

std::unique_ptr<Stmt> Parser::parseLetStatement() {
    consume(TokenType::Identifier, "Expected variable name after 'let'");
    std::string name = previous().lexeme;
    consume(TokenType::Assign, "Expected '=' after variable name");
    return std::make_unique<LetStmt>(name, parseExpression());
}

std::unique_ptr<Stmt> Parser::parseAssignStatement() {
    consume(TokenType::Identifier, "Expected variable name");
    std::string name = previous().lexeme;
    consume(TokenType::Assign, "Expected '=' after variable name");
    return std::make_unique<AssignStmt>(name, parseExpression());
}

std::unique_ptr<Stmt> Parser::parsePrintStatement() {
    consume(TokenType::LeftParen, "Expected '(' after 'print'");
    auto value = parseExpression();
    consume(TokenType::RightParen, "Expected ')' after print expression");
    return std::make_unique<PrintStmt>(std::move(value));
}

std::unique_ptr<Stmt> Parser::parseIfStatement() {
    auto condition = parseExpression();
    auto thenBranch = parseBlock();

    std::unique_ptr<BlockStmt> elseBranch;
    skipSeparators();
    if (match(TokenType::Else)) {
        elseBranch = parseBlock();
    }

    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::parseWhileStatement() {
    auto condition = parseExpression();
    auto body = parseBlock();
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<BlockStmt> Parser::parseBlock() {
    skipSeparators();
    consume(TokenType::LeftBrace, "Expected '{' to start block");
    auto block = std::make_unique<BlockStmt>();
    skipSeparators();

    while (!check(TokenType::RightBrace) && !isAtEnd()) {
        block->statements.push_back(parseStatement());
        skipSeparators();
    }

    consume(TokenType::RightBrace, "Expected '}' after block");
    return block;
}

std::unique_ptr<Expr> Parser::parseExpression() {
    return parseEquality();
}

std::unique_ptr<Expr> Parser::parseEquality() {
    auto expr = parseComparison();

    while (match(TokenType::EqualEqual) || match(TokenType::BangEqual)) {
        std::string op = previous().lexeme;
        auto right = parseComparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseComparison() {
    auto expr = parseTerm();

    while (match(TokenType::Less) || match(TokenType::LessEqual) ||
           match(TokenType::Greater) || match(TokenType::GreaterEqual)) {
        std::string op = previous().lexeme;
        auto right = parseTerm();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseTerm() {
    auto expr = parseFactor();

    while (match(TokenType::Plus) || match(TokenType::Minus)) {
        std::string op = previous().lexeme;
        auto right = parseFactor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseFactor() {
    auto expr = parseUnary();

    while (match(TokenType::Star) || match(TokenType::Slash)) {
        std::string op = previous().lexeme;
        auto right = parseUnary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseUnary() {
    if (match(TokenType::Minus)) {
        return std::make_unique<UnaryExpr>(previous().lexeme, parseUnary());
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
