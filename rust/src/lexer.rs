use crate::token::{Token, TokenType};

pub struct Lexer {
    source: Vec<char>,
    tokens: Vec<Token>,
    start: usize,
    current: usize,
    line: usize,
}

impl Lexer {
    pub fn new(source: String) -> Self {
        Self {
            source: source.chars().collect(),
            tokens: Vec::new(),
            start: 0,
            current: 0,
            line: 1,
        }
    }

    pub fn tokenize(mut self) -> Result<Vec<Token>, String> {
        while !self.is_at_end() {
            self.start = self.current;
            self.scan_token()?;
        }

        self.tokens.push(Token {
            token_type: TokenType::EndOfFile,
            lexeme: String::new(),
            line: self.line,
        });

        Ok(self.tokens)
    }

    fn is_at_end(&self) -> bool {
        self.current >= self.source.len()
    }

    fn advance(&mut self) -> char {
        let ch = self.source[self.current];
        self.current += 1;
        ch
    }

    fn peek(&self) -> char {
        if self.is_at_end() {
            '\0'
        } else {
            self.source[self.current]
        }
    }

    fn match_char(&mut self, expected: char) -> bool {
        if self.is_at_end() || self.source[self.current] != expected {
            return false;
        }
        self.current += 1;
        true
    }

    fn add_token(&mut self, token_type: TokenType, lexeme: String) {
        self.tokens.push(Token {
            token_type,
            lexeme,
            line: self.line,
        });
    }

    fn scan_token(&mut self) -> Result<(), String> {
        let ch = self.advance();
        match ch {
            '(' => self.add_token(TokenType::LeftParen, "(".to_string()),
            ')' => self.add_token(TokenType::RightParen, ")".to_string()),
            '/' => {
                if self.match_char('/') {
                    while !self.is_at_end() && self.peek() != '\n' {
                        self.advance();
                    }
                } else {
                    return Err(format!(
                        "Use English keywords like 'over' instead of '/' at line {}",
                        self.line
                    ));
                }
            }
            ';' => self.add_token(TokenType::Semicolon, ";".to_string()),
            ' ' | '\r' | '\t' => {}
            '\n' => {
                self.add_token(TokenType::Newline, "\\n".to_string());
                self.line += 1;
            }
            _ if ch.is_ascii_digit() => self.scan_number(),
            _ if ch.is_ascii_alphabetic() || ch == '_' => self.scan_identifier(),
            _ => {
                return Err(format!(
                    "Unexpected character '{}' at line {}",
                    ch, self.line
                ))
            }
        }

        Ok(())
    }

    fn scan_number(&mut self) {
        while self.peek().is_ascii_digit() {
            self.advance();
        }

        let lexeme: String = self.source[self.start..self.current].iter().collect();
        self.add_token(TokenType::Number, lexeme);
    }

    fn scan_identifier(&mut self) {
        while self.peek().is_ascii_alphanumeric() || self.peek() == '_' {
            self.advance();
        }

        let lexeme: String = self.source[self.start..self.current].iter().collect();
        let token_type = match lexeme.as_str() {
            "let" => TokenType::Let,
            "set" => TokenType::Set,
            "be" => TokenType::Be,
            "to" => TokenType::To,
            "print" => TokenType::Print,
            "if" => TokenType::If,
            "else" => TokenType::Else,
            "while" => TokenType::While,
            "do" => TokenType::Do,
            "end" => TokenType::End,
            "is" => TokenType::Is,
            "not" => TokenType::Not,
            "less" => TokenType::Less,
            "than" => TokenType::Than,
            "greater" => TokenType::Greater,
            "or" => TokenType::Or,
            "equal" => TokenType::Equal,
            "plus" => TokenType::Plus,
            "minus" => TokenType::Minus,
            "times" => TokenType::Times,
            "over" => TokenType::Over,
            "negative" => TokenType::Negative,
            _ => TokenType::Identifier,
        };

        self.add_token(token_type, lexeme);
    }
}
