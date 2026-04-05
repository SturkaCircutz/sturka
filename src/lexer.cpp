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
    case '/':
        if (match('/')) {
            while (!isAtEnd() && peek() != '\n') {
                advance();
            }
        } else {
            throw std::runtime_error("Use English keywords like 'over' instead of '/' at line " + std::to_string(line_));
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

    const std::string text = source_.substr(start_, current_ - start_);
    if (text == "let") {
        addToken(TokenType::Let, text);
    } else if (text == "set") {
        addToken(TokenType::Set, text);
    } else if (text == "be") {
        addToken(TokenType::Be, text);
    } else if (text == "to") {
        addToken(TokenType::To, text);
    } else if (text == "print") {
        addToken(TokenType::Print, text);
    } else if (text == "if") {
        addToken(TokenType::If, text);
    } else if (text == "else") {
        addToken(TokenType::Else, text);
    } else if (text == "while") {
        addToken(TokenType::While, text);
    } else if (text == "do") {
        addToken(TokenType::Do, text);
    } else if (text == "end") {
        addToken(TokenType::End, text);
    } else if (text == "is") {
        addToken(TokenType::Is, text);
    } else if (text == "not") {
        addToken(TokenType::Not, text);
    } else if (text == "less") {
        addToken(TokenType::Less, text);
    } else if (text == "than") {
        addToken(TokenType::Than, text);
    } else if (text == "greater") {
        addToken(TokenType::Greater, text);
    } else if (text == "or") {
        addToken(TokenType::Or, text);
    } else if (text == "equal") {
        addToken(TokenType::Equal, text);
    } else if (text == "plus") {
        addToken(TokenType::Plus, text);
    } else if (text == "minus") {
        addToken(TokenType::Minus, text);
    } else if (text == "times") {
        addToken(TokenType::Times, text);
    } else if (text == "over") {
        addToken(TokenType::Over, text);
    } else if (text == "negative") {
        addToken(TokenType::Negative, text);
    } else {
        addToken(TokenType::Identifier, text);
    }
}
