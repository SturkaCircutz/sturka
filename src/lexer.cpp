#include "lexer.h"

#include <cctype>
#include <stdexcept>

Lexer::Lexer(std::string source) : source_(std::move(source)) {}

std::vector<Token> Lexer::tokenize() {
    while (!isAtEnd()) {
        start_ = current_;
        scanToken();
    }

    tokens_.push_back({TokenType::EndOfFile, "", line_});
    return tokens_;
}

bool Lexer::isAtEnd() const {
    return current_ >= source_.size();
}

char Lexer::advance() {
    return source_[current_++];
}

char Lexer::peek() const {
    if (isAtEnd()) {
        return '\0';
    }
    return source_[current_];
}

char Lexer::peekNext() const {
    if (current_ + 1 >= source_.size()) {
        return '\0';
    }
    return source_[current_ + 1];
}

bool Lexer::match(char expected) {
    if (isAtEnd() || source_[current_] != expected) {
        return false;
    }
    ++current_;
    return true;
}

void Lexer::addToken(TokenType type, const std::string& lexeme) {
    tokens_.push_back({type, lexeme, line_});
}

void Lexer::scanToken() {
    char c = advance();
    switch (c) {
    case '(':
        addToken(TokenType::LeftParen, "(");
        break;
    case ')':
        addToken(TokenType::RightParen, ")");
        break;
    case '{':
        addToken(TokenType::LeftBrace, "{");
        break;
    case '}':
        addToken(TokenType::RightBrace, "}");
        break;
    case '+':
        addToken(TokenType::Plus, "+");
        break;
    case '-':
        addToken(TokenType::Minus, "-");
        break;
    case '*':
        addToken(TokenType::Star, "*");
        break;
    case '/':
        if (match('/')) {
            while (!isAtEnd() && peek() != '\n') {
                advance();
            }
        } else {
            addToken(TokenType::Slash, "/");
        }
        break;
    case '=':
        if (match('=')) {
            addToken(TokenType::EqualEqual, "==");
        } else {
            addToken(TokenType::Assign, "=");
        }
        break;
    case '!':
        if (!match('=')) {
            throw std::runtime_error("Unexpected '!' at line " + std::to_string(line_));
        }
        addToken(TokenType::BangEqual, "!=");
        break;
    case '<':
        if (match('=')) {
            addToken(TokenType::LessEqual, "<=");
        } else {
            addToken(TokenType::Less, "<");
        }
        break;
    case '>':
        if (match('=')) {
            addToken(TokenType::GreaterEqual, ">=");
        } else {
            addToken(TokenType::Greater, ">");
        }
        break;
    case ';':
        addToken(TokenType::Semicolon, ";");
        break;
    case ' ':
    case '\r':
    case '\t':
        break;
    case '\n':
        addToken(TokenType::Newline, "\\n");
        ++line_;
        break;
    default:
        if (std::isdigit(static_cast<unsigned char>(c))) {
            scanNumber();
        } else if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            scanIdentifier();
        } else {
            throw std::runtime_error("Unexpected character '" + std::string(1, c) + "' at line " + std::to_string(line_));
        }
        break;
    }
}

void Lexer::scanNumber() {
    while (std::isdigit(static_cast<unsigned char>(peek()))) {
        advance();
    }

    addToken(TokenType::Number, source_.substr(start_, current_ - start_));
}

void Lexer::scanIdentifier() {
    while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_') {
        advance();
    }

    std::string text = source_.substr(start_, current_ - start_);
    if (text == "let") {
        addToken(TokenType::Let, text);
    } else if (text == "print") {
        addToken(TokenType::Print, text);
    } else if (text == "if") {
        addToken(TokenType::If, text);
    } else if (text == "else") {
        addToken(TokenType::Else, text);
    } else if (text == "while") {
        addToken(TokenType::While, text);
    } else {
        addToken(TokenType::Identifier, text);
    }
}
