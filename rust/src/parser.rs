use crate::ast::{Expr, Program, Stmt};
use crate::token::{Token, TokenType};

pub struct Parser {
    tokens: Vec<Token>,
    current: usize,
}

impl Parser {
    pub fn new(tokens: Vec<Token>) -> Self {
        Self { tokens, current: 0 }
    }

    pub fn parse_program(&mut self) -> Result<Program, String> {
        let mut program = Vec::new();
        self.skip_separators();

        while !self.is_at_end() {
            program.push(self.parse_statement()?);
            self.skip_separators();
        }

        Ok(program)
    }

    fn is_at_end(&self) -> bool {
        self.peek().token_type == TokenType::EndOfFile
    }

    fn peek(&self) -> &Token {
        &self.tokens[self.current]
    }

    fn previous(&self) -> &Token {
        &self.tokens[self.current - 1]
    }

    fn advance(&mut self) -> &Token {
        if !self.is_at_end() {
            self.current += 1;
        }
        self.previous()
    }

    fn check(&self, token_type: TokenType) -> bool {
        !self.is_at_end() && self.peek().token_type == token_type
    }

    fn match_token(&mut self, token_type: TokenType) -> bool {
        if self.check(token_type) {
            self.advance();
            true
        } else {
            false
        }
    }

    fn consume(&mut self, token_type: TokenType, message: &str) -> Result<(), String> {
        if self.check(token_type) {
            self.advance();
            Ok(())
        } else {
            Err(format!("{message} at line {}", self.peek().line))
        }
    }

    fn skip_separators(&mut self) {
        while self.match_token(TokenType::Newline) || self.match_token(TokenType::Semicolon) {}
    }

    fn parse_statement(&mut self) -> Result<Stmt, String> {
        if self.match_token(TokenType::Let) {
            return self.parse_let_statement();
        }
        if self.match_token(TokenType::Set) {
            return self.parse_assign_statement();
        }
        if self.match_token(TokenType::Print) {
            return self.parse_print_statement();
        }
        if self.match_token(TokenType::If) {
            return self.parse_if_statement();
        }
        if self.match_token(TokenType::While) {
            return self.parse_while_statement();
        }

        Err(format!("Unexpected statement at line {}", self.peek().line))
    }

    fn parse_let_statement(&mut self) -> Result<Stmt, String> {
        self.consume(TokenType::Identifier, "Expected variable name after 'let'")?;
        let name = self.previous().lexeme.clone();
        self.consume(TokenType::Be, "Expected 'be' after variable name")?;
        let value = self.parse_expression()?;
        Ok(Stmt::Let { name, value })
    }

    fn parse_assign_statement(&mut self) -> Result<Stmt, String> {
        self.consume(TokenType::Identifier, "Expected variable name after 'set'")?;
        let name = self.previous().lexeme.clone();
        self.consume(TokenType::To, "Expected 'to' after variable name")?;
        let value = self.parse_expression()?;
        Ok(Stmt::Assign { name, value })
    }

    fn parse_print_statement(&mut self) -> Result<Stmt, String> {
        let value = self.parse_expression()?;
        Ok(Stmt::Print { value })
    }

    fn parse_if_statement(&mut self) -> Result<Stmt, String> {
        let condition = self.parse_expression()?;
        self.consume(TokenType::Do, "Expected 'do' after if condition")?;
        self.skip_separators();

        let then_branch = self.parse_statements_until_else_or_end()?;
        let else_branch = if self.match_token(TokenType::Else) {
            self.skip_separators();
            Some(self.parse_statements_until(TokenType::End)?)
        } else {
            None
        };

        self.consume(TokenType::End, "Expected 'end' after if statement")?;
        Ok(Stmt::If {
            condition,
            then_branch,
            else_branch,
        })
    }

    fn parse_while_statement(&mut self) -> Result<Stmt, String> {
        let condition = self.parse_expression()?;
        self.consume(TokenType::Do, "Expected 'do' after while condition")?;
        self.skip_separators();
        let body = self.parse_statements_until(TokenType::End)?;
        self.consume(TokenType::End, "Expected 'end' after while statement")?;
        Ok(Stmt::While { condition, body })
    }

    fn parse_statements_until(&mut self, terminator: TokenType) -> Result<Vec<Stmt>, String> {
        let mut statements = Vec::new();
        self.skip_separators();

        while !self.check(terminator.clone()) && !self.is_at_end() {
            statements.push(self.parse_statement()?);
            self.skip_separators();
        }

        Ok(statements)
    }

    fn parse_statements_until_else_or_end(&mut self) -> Result<Vec<Stmt>, String> {
        let mut statements = Vec::new();
        self.skip_separators();

        while !self.check(TokenType::Else) && !self.check(TokenType::End) && !self.is_at_end() {
            statements.push(self.parse_statement()?);
            self.skip_separators();
        }

        Ok(statements)
    }

    fn parse_expression(&mut self) -> Result<Expr, String> {
        self.parse_equality()
    }

    fn parse_equality(&mut self) -> Result<Expr, String> {
        let mut expr = self.parse_comparison()?;

        while self.match_token(TokenType::Is) {
            if self.check(TokenType::Less) || self.check(TokenType::Greater) {
                self.current -= 1;
                break;
            }

            let op = if self.match_token(TokenType::Not) { "!=" } else { "==" };
            let right = self.parse_comparison()?;
            expr = Expr::Binary {
                left: Box::new(expr),
                op: op.to_string(),
                right: Box::new(right),
            };
        }

        Ok(expr)
    }

    fn parse_comparison(&mut self) -> Result<Expr, String> {
        let mut expr = self.parse_term()?;

        while self.match_token(TokenType::Is) {
            let op = if self.match_token(TokenType::Less) {
                self.consume(TokenType::Than, "Expected 'than' after 'less'")?;
                if self.match_token(TokenType::Or) {
                    self.consume(TokenType::Equal, "Expected 'equal' after 'or'")?;
                    self.consume(TokenType::To, "Expected 'to' after 'equal'")?;
                    "<="
                } else {
                    "<"
                }
            } else if self.match_token(TokenType::Greater) {
                self.consume(TokenType::Than, "Expected 'than' after 'greater'")?;
                if self.match_token(TokenType::Or) {
                    self.consume(TokenType::Equal, "Expected 'equal' after 'or'")?;
                    self.consume(TokenType::To, "Expected 'to' after 'equal'")?;
                    ">="
                } else {
                    ">"
                }
            } else {
                self.current -= 1;
                break;
            };

            let right = self.parse_term()?;
            expr = Expr::Binary {
                left: Box::new(expr),
                op: op.to_string(),
                right: Box::new(right),
            };
        }

        Ok(expr)
    }

    fn parse_term(&mut self) -> Result<Expr, String> {
        let mut expr = self.parse_factor()?;

        loop {
            let op = if self.match_token(TokenType::Plus) {
                Some("+")
            } else if self.match_token(TokenType::Minus) {
                Some("-")
            } else {
                None
            };

            let Some(op) = op else {
                break;
            };

            let right = self.parse_factor()?;
            expr = Expr::Binary {
                left: Box::new(expr),
                op: op.to_string(),
                right: Box::new(right),
            };
        }

        Ok(expr)
    }

    fn parse_factor(&mut self) -> Result<Expr, String> {
        let mut expr = self.parse_unary()?;

        loop {
            let op = if self.match_token(TokenType::Times) {
                Some("*")
            } else if self.match_token(TokenType::Over) {
                Some("/")
            } else {
                None
            };

            let Some(op) = op else {
                break;
            };

            let right = self.parse_unary()?;
            expr = Expr::Binary {
                left: Box::new(expr),
                op: op.to_string(),
                right: Box::new(right),
            };
        }

        Ok(expr)
    }

    fn parse_unary(&mut self) -> Result<Expr, String> {
        if self.match_token(TokenType::Negative) {
            return Ok(Expr::Unary {
                op: "-".to_string(),
                operand: Box::new(self.parse_unary()?),
            });
        }

        self.parse_primary()
    }

    fn parse_primary(&mut self) -> Result<Expr, String> {
        if self.match_token(TokenType::Number) {
            let value = self
                .previous()
                .lexeme
                .parse::<i64>()
                .map_err(|_| format!("Invalid integer at line {}", self.previous().line))?;
            return Ok(Expr::Integer(value));
        }

        if self.match_token(TokenType::Identifier) {
            return Ok(Expr::Variable(self.previous().lexeme.clone()));
        }

        if self.match_token(TokenType::LeftParen) {
            let expr = self.parse_expression()?;
            self.consume(TokenType::RightParen, "Expected ')' after expression")?;
            return Ok(expr);
        }

        Err(format!("Expected expression at line {}", self.peek().line))
    }
}
