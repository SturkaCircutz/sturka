#pragma once

#include <string>

enum class TokenType {
    EndOfFile,
    Newline,
    Number,
    Identifier,
    Let,
    Print,
    If,
    Else,
    While,
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    Plus,
    Minus,
    Star,
    Slash,
    Assign,
    EqualEqual,
    BangEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    Semicolon
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
};
