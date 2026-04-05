#[derive(Debug, Clone, PartialEq, Eq)]
pub enum TokenType {
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
    Semicolon,
}

#[derive(Debug, Clone)]
pub struct Token {
    pub token_type: TokenType,
    pub lexeme: String,
    pub line: usize,
}
