#pragma once

#include <string>

enum class TokenType {
    EndOfFile,
    Newline,
    Number,
    Identifier,
    Let,
    Set,
    Be,
    To,
    Print,
    If,
    Else,
    While,
    Do,
    End,
    Is,
    Not,
    Less,
    Than,
    Greater,
    Or,
    Equal,
    Plus,
    Minus,
    Times,
    Over,
    Negative,
    LeftParen,
    RightParen,
    Semicolon
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
};
